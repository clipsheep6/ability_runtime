/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "napi_want_agent.h"

#include <cstdio>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

#include "ability_runtime_error_util.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "napi_common.h"
#include "want_agent_helper.h"
using namespace OHOS::AbilityRuntime;
namespace OHOS {
#define NAPI_ASSERT_RETURN_NULL(env, assertion, message)    \
do {                                                        \
    if (!(assertion)) {                                     \
        HILOG_INFO(message);                                \
        return nullptr;                                     \
    }                                                       \
} while (0)
namespace {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr uint8_t INDEX_ONE = 1;
constexpr uint8_t INDEX_TWO = 2;
constexpr int32_t ERR_NOT_OK = -1;
constexpr int32_t BUSINESS_ERROR_CODE_OK = 0;
constexpr int32_t PARAMETER_ERROR = -1;
} // namespace


TriggerCompleteCallBack::TriggerCompleteCallBack()
{}

TriggerCompleteCallBack::~TriggerCompleteCallBack()
{}

void TriggerCompleteCallBack::SetCallbackInfo(NativeEngine &engine, NativeReference* ref)
{
    triggerCompleteInfo_.engine = &engine;
    triggerCompleteInfo_.nativeRef.reset(ref);
}

void TriggerCompleteCallBack::SetWantAgentInstance(WantAgent* wantAgent)
{
    triggerCompleteInfo_.wantAgent = wantAgent;
}

NativeValue* RetErrMsg(NativeEngine &engine, NativeValue* lastParam, int32_t errorCode)
{
    HILOG_DEBUG("RetErrMsg called");
    auto complete = [err = errorCode] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_DEBUG("error information output");
        task.Reject(engine, CreateJsError(engine, err, "PARAMETER_ERROR"));
    };

    NativeValue* result = nullptr;
    AsyncTask::Schedule("RetErrMsg",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));

    return result;
}

napi_value CreateErrorValue(napi_env env, int32_t errCode)
{
    HILOG_INFO("enter, errorCode[%{public}d]", errCode);
    napi_value error =  NapiGetNull(env);
    if (errCode == NO_ERROR) {
        return error;
    }

    napi_value code = nullptr;
    napi_create_int32(env, errCode, &code);

    std::string errMsg = AbilityRuntimeErrorUtil::GetErrMessage(errCode);
    napi_value message = nullptr;
    napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &message);

    napi_create_error(env, nullptr, message, &error);
    napi_set_named_property(env, error, "code", code);
    return error;
}

void NapiThrow(napi_env env, int32_t errCode)
{
    HILOG_INFO("enter");

    napi_throw(env, CreateErrorValue(env, errCode));
}

auto OnSendFinishedUvAfterWorkCallback = [](uv_work_t* work, int status) {
    HILOG_INFO("TriggerCompleteCallBack::OnSendFinishedUvAfterWorkCallback:status = %{public}d", status);

    TriggerReceiveDataWorker* dataWorkerData = static_cast<TriggerReceiveDataWorker *>(work->data);
    if (dataWorkerData == nullptr) {
        HILOG_ERROR("TriggerReceiveDataWorker instance(uv_work_t) is nullptr");
        delete work;
        return;
    }
    NativeValue* args[ARGC_TWO] = {0};
    NativeValue* objValueFirst = dataWorkerData->engine->CreateObject();
    NativeObject* objectFirst = ConvertNativeValueTo<NativeObject>(objValueFirst);

    if (objectFirst == nullptr) {
        HILOG_ERROR("Failed to get objectFirst");
        delete dataWorkerData;
        dataWorkerData = nullptr;
        delete work;
        return;
    }
    NativeValue* objValueSecond = dataWorkerData->engine->CreateObject();
    NativeObject* objectSecond = ConvertNativeValueTo<NativeObject>(objValueSecond);

    if (objectSecond == nullptr) {
        HILOG_ERROR("Failed to get objectSecond");
        delete dataWorkerData;
        dataWorkerData = nullptr;
        delete work;
        return;
    }

#ifdef ENABLE_ERRCODE
    objValueFirst = dataWorkerData->engine->CreateUndefined();
    objectFirst =  ConvertNativeValueTo<NativeObject>(objValueFirst);
#else
    objectFirst->SetProperty("code", CreateJsValue(*(dataWorkerData->engine), BUSINESS_ERROR_CODE_OK));
#endif
    objectSecond->SetProperty("wantAgent", JsWantAgent::WrapWantAgent(*(dataWorkerData->engine),
        dataWorkerData->wantAgent));
    objectSecond->SetProperty("want", CreateJsWant(*(dataWorkerData->engine), dataWorkerData->want));
    objectSecond->SetProperty("finalCode", CreateJsValue(*(dataWorkerData->engine), dataWorkerData->resultCode));
    objectSecond->SetProperty("finalData", CreateJsValue(*(dataWorkerData->engine), dataWorkerData->resultData));
    objectSecond->SetProperty("extraInfo", CreateJsWantParams(*(dataWorkerData->engine), dataWorkerData->resultExtras));
    args[ARGC_ZERO] = objValueFirst;
    args[ARGC_ONE] = objValueSecond;

    NativeValue* value = dataWorkerData->nativeRef->Get();
    NativeValue* callback = dataWorkerData->nativeRef->Get();
    dataWorkerData->engine->CallFunction(value, callback, args, ARGC_TWO);
    delete dataWorkerData;
    dataWorkerData = nullptr;
    delete work;
};

void TriggerCompleteCallBack::OnSendFinished(
    const AAFwk::Want &want, int resultCode, const std::string &resultData, const AAFwk::WantParams &resultExtras)
{
    HILOG_INFO("TriggerCompleteCallBack::OnSendFinished start");
    if (triggerCompleteInfo_.nativeRef == nullptr) {
        HILOG_INFO("triggerCompleteInfo_ CallBack is nullptr");
        return;
    }

    uv_loop_t* loop = nullptr;
#if NAPI_VERSION >= NUMBER_OF_PARAMETERS_TWO
    loop = triggerCompleteInfo_.engine->GetUVLoop();
#endif  // NAPI_VERSION >= 2
    if (loop == nullptr) {
        HILOG_INFO("loop instance is nullptr");
        return;
    }

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOG_INFO("uv_work_t instance is nullptr");
        return;
    }
    TriggerReceiveDataWorker* dataWorker = new (std::nothrow) TriggerReceiveDataWorker();
    if (dataWorker == nullptr) {
        HILOG_INFO("TriggerReceiveDataWorker instance is nullptr");
        delete work;
        work = nullptr;
        return;
    }
    dataWorker->want = want;
    dataWorker->resultCode = resultCode;
    dataWorker->resultData = resultData;
    dataWorker->resultExtras = resultExtras;
    dataWorker->engine = triggerCompleteInfo_.engine;
    dataWorker->nativeRef = std::move(triggerCompleteInfo_.nativeRef);
    dataWorker->wantAgent = triggerCompleteInfo_.wantAgent;
    work->data = static_cast<void *>(dataWorker);
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {}, OnSendFinishedUvAfterWorkCallback);
    if (ret != 0) {
        delete dataWorker;
        dataWorker = nullptr;
        delete work;
        work = nullptr;
    }

    HILOG_INFO("TriggerCompleteCallBack::OnSendFinished end");
}

void JsWantAgent::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_INFO("JsWantAgent::Finalizer is called");
    std::unique_ptr<JsWantAgent>(static_cast<JsWantAgent*>(data));
}

NativeValue* JsWantAgent::Equal(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnEqual(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::GetWant(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnGetWant(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::GetOperationType(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnGetOperationType(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::GetBundleName(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnGetBundleName(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::GetUid(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnGetUid(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::Cancel(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnCancel(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::Trigger(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnTrigger(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::GetWantAgent(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnGetWantAgent(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::NapiGetWant(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnNapiGetWant(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::NapiTrigger(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnNapiTrigger(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::NapiGetWantAgent(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnNapiGetWantAgent(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::NapiGetOperationType(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnNapiGetOperationType(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::OnEqual(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("enter, argc = %{public}d", static_cast<int32_t>(info.argc));
    int32_t errCode = BUSINESS_ERROR_CODE_OK;
    WantAgent* pWantAgentFirst = nullptr;
    WantAgent* pWantAgentSecond = nullptr;
    if (info.argc < ARGC_TWO || info.argc > ARGC_THREE) {
        HILOG_ERROR("Not enough params");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
#endif
        return engine.CreateUndefined();
    }

    NativeValue* lastParam = (info.argc >= ARGC_THREE) ? info.argv[INDEX_TWO] : nullptr;
    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
#ifdef ENABLE_ERRCODE
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        AbilityRuntimeErrorUtil::Throw(engine, errCode);
        return engine.CreateUndefined();
#else
        errCode = ERR_NOT_OK;
        return RetErrMsg(engine, lastParam, errCode);
#endif
    }

    UnwrapWantAgent(engine, info.argv[0], reinterpret_cast<void **>(&pWantAgentFirst));
    if (pWantAgentFirst == nullptr) {
        HILOG_ERROR("Parse pWantAgentFirst failed");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
#else
        errCode = ERR_NOT_OK;
        return RetErrMsg(engine, lastParam, errCode);
#endif
    }

    if (info.argv[1]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
#else
        errCode = ERR_NOT_OK;
        return RetErrMsg(engine, lastParam, errCode);
#endif
    }

    UnwrapWantAgent(engine, info.argv[1], reinterpret_cast<void **>(&pWantAgentSecond));
    if (pWantAgentSecond == nullptr) {
        HILOG_ERROR("Parse pWantAgentSceond failed");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
#else
        errCode = ERR_NOT_OK;
        return RetErrMsg(engine, lastParam, errCode);
#endif
    }

    std::shared_ptr<WantAgent> wantAgentFirst = std::make_shared<WantAgent>(*pWantAgentFirst);
    std::shared_ptr<WantAgent> wantAgentSecond = std::make_shared<WantAgent>(*pWantAgentSecond);
    AsyncTask::CompleteCallback complete =
        [wantAgentFirst, wantAgentSecond](NativeEngine &engine, AsyncTask &task, int32_t status) {
            HILOG_DEBUG("OnEqual AsyncTask is called");
            bool ret = false;
            ErrCode retCode = WantAgentHelper::IsEquals(wantAgentFirst, wantAgentSecond);
#ifdef ENABLE_ERRCODE
            if (retCode == ERR_NOT_OK) {
                ret = false;
                task.ResolveWithNoError(engine, CreateJsValue(engine, ret));
            } else if (retCode == ERR_OK) {
                ret = true;
                task.ResolveWithNoError(engine, CreateJsValue(engine, ret));
            } else {
                task.Reject(engine, CreateJsError(engine, retCode, AbilityRuntimeErrorUtil::GetErrMessage(retCode)));
            }
#else
            if (retCode != ERR_OK) {
                ret = false;
                task.Resolve(engine, CreateJsValue(engine, ret));
            } else {
                ret = true;
                task.Resolve(engine, CreateJsValue(engine, ret));
            }
#endif
        };

    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnEqual",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnGetWant(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    int32_t errCode = BUSINESS_ERROR_CODE_OK;
    WantAgent* pWantAgent = nullptr;
    if (info.argc > ARGC_TWO || info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }

    NativeValue* lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        errCode = ERR_NOT_OK;
        return RetErrMsg(engine, lastParam, errCode);
    }

    UnwrapWantAgent(engine, info.argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
        errCode = ERR_NOT_OK;
        return RetErrMsg(engine, lastParam, errCode);
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    AsyncTask::CompleteCallback complete = [wantAgent](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnGetWant AsyncTask is called");
        std::shared_ptr<Want> want = WantAgentHelper::GetWant(wantAgent);
        if (want == nullptr) {
            task.Reject(engine, CreateJsError(engine, ERR_NOT_OK, "WantAgentHelper::GetWant result nullptr."));
            return;
        }
        task.Resolve(engine, CreateJsWant(engine, *(want)));
    };

    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnGetWant",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnGetOperationType(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("JsWantAgent::OnGetOperationType enter, argc = %{public}d", static_cast<int32_t>(info.argc));
    int32_t errCode = BUSINESS_ERROR_CODE_OK;
    WantAgent* pWantAgent = nullptr;
    if (info.argc > ARGC_TWO || info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }

    NativeValue* lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        errCode = ERR_NOT_OK;
        return RetErrMsg(engine, lastParam, errCode);
    }

    UnwrapWantAgent(engine, info.argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
        errCode = ERR_NOT_OK;
        return RetErrMsg(engine, lastParam, errCode);
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    AsyncTask::CompleteCallback complete = [wantAgent](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnGetOperationType AsyncTask is called");
        auto ret = WantAgentHelper::GetType(wantAgent);
        task.Resolve(engine, CreateJsValue(engine, ret));
    };

    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnGetOperationType",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnGetBundleName(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("JsWantAgent::OnGetBundleName enter, argc = %{public}d", static_cast<int32_t>(info.argc));
    int32_t errCode = BUSINESS_ERROR_CODE_OK;
    WantAgent* pWantAgent = nullptr;
    if (info.argc > ARGC_TWO || info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
#endif
        return engine.CreateUndefined();
    }

    NativeValue* lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
#else
        errCode = ERR_NOT_OK;
        return RetErrMsg(engine, lastParam, errCode);
#endif
    }

    UnwrapWantAgent(engine, info.argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
#ifdef ENABLE_ERRCODE
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        AbilityRuntimeErrorUtil::Throw(engine, errCode);
        return engine.CreateUndefined();
#else
        errCode = ERR_NOT_OK;
        return RetErrMsg(engine, lastParam, errCode);
#endif
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    AsyncTask::CompleteCallback complete = [wantAgent](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnGetBundleName AsyncTask is called");
        std::string bundleName = "";
#ifdef ENABLE_ERRCODE
        ErrCode result = WantAgentHelper::GetBundleName(wantAgent, bundleName);
        if (result != NO_ERROR) {
            task.Reject(engine, CreateJsError(engine, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
        } else {
            task.ResolveWithNoError(engine, CreateJsValue(engine, bundleName));
        }
#else
        WantAgentHelper::GetBundleName(wantAgent, bundleName);
        task.Resolve(engine, CreateJsValue(engine, bundleName));
#endif
    };

    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnGetBundleName",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnGetUid(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("JsWantAgent::OnGetUid enter, argc = %{public}d", static_cast<int32_t>(info.argc));
    int32_t errCode = BUSINESS_ERROR_CODE_OK;
    WantAgent* pWantAgent = nullptr;
    if (info.argc > ARGC_TWO || info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
#ifdef ENABLE_ERRCODE
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        AbilityRuntimeErrorUtil::Throw(engine, errCode);
#endif
        return engine.CreateUndefined();
    }

    NativeValue* lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
#ifdef ENABLE_ERRCODE
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        AbilityRuntimeErrorUtil::Throw(engine, errCode);
        return engine.CreateUndefined();
#else
        errCode = ERR_NOT_OK;
        return RetErrMsg(engine, lastParam, errCode);
#endif
    }

    UnwrapWantAgent(engine, info.argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
#else
        errCode = ERR_NOT_OK;
        return RetErrMsg(engine, lastParam, errCode);
#endif
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    AsyncTask::CompleteCallback complete = [wantAgent](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnGetUid AsyncTask is called");
        int uid = -1;
#ifdef ENABLE_ERRCODE
        ErrCode result = WantAgentHelper::GetUid(wantAgent, uid);
        if (result != NO_ERROR) {
            task.Reject(engine, CreateJsError(engine, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
        } else {
            task.ResolveWithNoError(engine, CreateJsValue(engine, uid));
        }
#else
        WantAgentHelper::GetUid(wantAgent, uid);
        task.Resolve(engine, CreateJsValue(engine, uid));
#endif
    };
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnGetUid",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnCancel(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s is called", __FUNCTION__);
    int32_t errCode = BUSINESS_ERROR_CODE_OK;
    WantAgent* pWantAgent = nullptr;
    if (info.argc > ARGC_TWO || info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
#endif
        return engine.CreateUndefined();
    }

    NativeValue* lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
#ifdef ENABLE_ERRCODE
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        AbilityRuntimeErrorUtil::Throw(engine, errCode);
        return engine.CreateUndefined();
#else
        errCode = ERR_NOT_OK;
        return RetErrMsg(engine, lastParam, errCode);
#endif
    }

    UnwrapWantAgent(engine, info.argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
#else
        errCode = ERR_NOT_OK;
        return RetErrMsg(engine, lastParam, errCode);
#endif
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    AsyncTask::CompleteCallback complete =
        [wantAgent](NativeEngine &engine, AsyncTask &task, int32_t status) {
            HILOG_DEBUG("OnCancel AsyncTask is called");
#ifdef ENABLE_ERRCODE
            ErrCode result = WantAgentHelper::Cancel(wantAgent);
            if (result != NO_ERROR) {
                task.Reject(engine, CreateJsError(engine, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
            } else {
                task.ResolveWithNoError(engine, engine.CreateUndefined());
            }
#else
            WantAgentHelper::Cancel(wantAgent);
            task.Resolve(engine, engine.CreateUndefined());
#endif
        };

    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnCancel",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnTrigger(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s is called", __FUNCTION__);
    if (info.argc != ARGC_THREE) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }

    std::shared_ptr<WantAgent> wantAgent = nullptr;
    TriggerInfo triggerInfo;
    auto triggerObj = std::make_shared<TriggerCompleteCallBack>();
    int32_t errCode = UnWrapTriggerInfoParam(engine, info, wantAgent, triggerInfo, triggerObj);
    if (errCode != BUSINESS_ERROR_CODE_OK) {
        return RetErrMsg(engine, info.argv[ARGC_TWO], errCode);
    }

    WantAgentHelper::TriggerWantAgent(wantAgent, triggerObj, triggerInfo);
    return engine.CreateNull();
}

int32_t JsWantAgent::UnWrapTriggerInfoParam(NativeEngine &engine, NativeCallbackInfo &info,
    std::shared_ptr<WantAgent> &wantAgent, TriggerInfo &triggerInfo,
    std::shared_ptr<TriggerCompleteCallBack> &triggerObj)
{
    if (info.argc != ARGC_THREE) {
        HILOG_ERROR("Not enough params");
        return ERR_NOT_OK;
    }

    if (info.argv[ARGC_ZERO]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        return ERR_NOT_OK;
    }
    WantAgent* pWantAgent = nullptr;
    UnwrapWantAgent(engine, info.argv[0], reinterpret_cast<void **>(&pWantAgent));

    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
        return ERR_NOT_OK;
    }
    wantAgent = std::make_shared<WantAgent>(*pWantAgent);

    int32_t ret = GetTriggerInfo(engine, info.argv[ARGC_ONE], triggerInfo);
    if (ret != BUSINESS_ERROR_CODE_OK) {
        HILOG_ERROR("Get trigger info error");
        return ret;
    }

    NativeReference* ref = engine.CreateReference(info.argv[ARGC_TWO], 1);
    triggerObj->SetCallbackInfo(engine, ref);
    triggerObj->SetWantAgentInstance(pWantAgent);

    return BUSINESS_ERROR_CODE_OK;
}

int32_t JsWantAgent::GetTriggerInfo(NativeEngine &engine, NativeValue* param, TriggerInfo &triggerInfo)
{
    HILOG_DEBUG("GetTriggerInfo called.");
    if (param->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("param type mismatch!");
        return ERR_NOT_OK;
    }

    NativeObject* objectParam = ConvertNativeValueTo<NativeObject>(param);

    int32_t code = -1;
    NativeValue* jsCode = objectParam->GetProperty("code");
    if (!ConvertFromJsValue(engine, jsCode, code)) {
        HILOG_ERROR("GetTriggerInfo convert code error!");
        return ERR_NOT_OK;
    }

    std::shared_ptr<AAFwk::Want> want = nullptr;
    if (objectParam->HasProperty("want")) {
        NativeValue* jsWant = objectParam->GetProperty("want");
        want = std::make_shared<AAFwk::Want>();
        if (!UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(jsWant), *want)) {
            HILOG_ERROR("GetTriggerInfo convert want error!");
            return ERR_NOT_OK;
        }
    }

    std::string permission = "";
    if (objectParam->HasProperty("permission")) {
        NativeValue* jsPermission = objectParam->GetProperty("permission");
        if (!ConvertFromJsValue(engine, jsPermission, permission)) {
            HILOG_ERROR("GetTriggerInfo convert permission error!");
            return ERR_NOT_OK;
        }
    }

    std::shared_ptr<AAFwk::WantParams> extraInfo = nullptr;
    if (objectParam->HasProperty("extraInfo")) {
        NativeValue* jsExtraInfo = objectParam->GetProperty("extraInfo");
        extraInfo = std::make_shared<AAFwk::WantParams>();
        if (!UnwrapWantParams(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(jsExtraInfo),
            *extraInfo)) {
            HILOG_ERROR("GetTriggerInfo convert extraInfo error!");
            return ERR_NOT_OK;
        }
    }

    TriggerInfo triggerInfoData(permission, extraInfo, want, code);
    triggerInfo = triggerInfoData;
    return BUSINESS_ERROR_CODE_OK;
}

int32_t JsWantAgent::GetWantAgentParam(NativeEngine &engine, NativeCallbackInfo &info, WantAgentWantsParas &paras)
{
    HILOG_DEBUG("GetWantAgentParam called.");
    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("param type mismatch!");
        return PARAMETER_ERROR;
    }
    NativeObject* paramObject = ConvertNativeValueTo<NativeObject>(info.argv[0]);

    NativeValue* jsWants = paramObject->GetProperty("wants");
    if (!jsWants->IsArray()) {
        HILOG_ERROR("wants is not array!");
        return PARAMETER_ERROR;
    }

    NativeArray* jsWantsArray = ConvertNativeValueTo<NativeArray>(jsWants);
    for (uint32_t i = 0; i < jsWantsArray->GetLength(); i++) {
        std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
        NativeValue* jsWant = jsWantsArray->GetElement(i);
        if (!UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(jsWant), *want)) {
            HILOG_ERROR("UnwrapWant failed!");
            return PARAMETER_ERROR;
        }
        paras.wants.emplace_back(want);
    }

    NativeValue* jsOperationType = paramObject->GetProperty("operationType");
    if (!ConvertFromJsValue(engine, jsOperationType, paras.operationType)) {
        HILOG_ERROR("Convert operationType failed!");
        return PARAMETER_ERROR;
    }

    NativeValue* jsRequestCode = paramObject->GetProperty("requestCode");
    if (!ConvertFromJsValue(engine, jsRequestCode, paras.requestCode)) {
        HILOG_ERROR("Convert requestCode failed!");
        return PARAMETER_ERROR;
    }

    if (paramObject->HasProperty("wantAgentFlags")) {
        NativeValue* jsWantAgentFlags = paramObject->GetProperty("wantAgentFlags");
        if (!jsWantAgentFlags->IsArray()) {
            HILOG_ERROR("wantAgentFlags is not array!");
            return PARAMETER_ERROR;
        }

        NativeArray* jsWantAgentFlagsArray = ConvertNativeValueTo<NativeArray>(jsWantAgentFlags);
        for (uint32_t i = 0; i < jsWantAgentFlagsArray->GetLength(); i++) {
            NativeValue* jsWantAgentFlag = jsWantAgentFlagsArray->GetElement(i);
            if (jsWantAgentFlag->TypeOf() != NativeValueType::NATIVE_NUMBER) {
                HILOG_ERROR("WantAgentFlag type error!");
                return PARAMETER_ERROR;
            }
            int32_t wantAgentFlag = 0;
            if (!ConvertFromJsValue(engine, jsWantAgentFlag, wantAgentFlag)) {
                HILOG_ERROR("Convert WantAgentFlag failed!");
                return PARAMETER_ERROR;
            }
            paras.wantAgentFlags.emplace_back(static_cast<WantAgentConstant::Flags>(wantAgentFlag));
        }
    }

    if (paramObject->HasProperty("extraInfo")) {
        NativeValue* jsExtraInfo = paramObject->GetProperty("extraInfo");
        if (jsExtraInfo->TypeOf() != NativeValueType::NATIVE_OBJECT) {
            HILOG_ERROR("ExtraInfo type error!");
            return PARAMETER_ERROR;
        }
        if (!UnwrapWantParams(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(jsExtraInfo),
            paras.extraInfo)) {
            HILOG_ERROR("Convert extraInfo failed!");
            return PARAMETER_ERROR;
        }
    }
    return BUSINESS_ERROR_CODE_OK;
}

NativeValue* JsWantAgent::WrapWantAgent(NativeEngine &engine, WantAgent* wantAgent)
{
    HILOG_DEBUG("WrapWantAgent called.");
    NativeCallback callback = [](NativeEngine* engine, NativeCallbackInfo* info) -> NativeValue* {
        return info->thisVar;
    };

    NativeValue* wantAgentClass = engine.DefineClass("WantAgentClass", callback, nullptr, nullptr, 0);
    NativeValue* result = engine.CreateInstance(wantAgentClass, nullptr, 0);

    NativeObject* nativeObject = reinterpret_cast<NativeObject*>(result->GetInterface(NativeObject::INTERFACE_ID));
    NativeFinalize nativeFinalize = [](NativeEngine* engine, void* data, void* hint) {
        HILOG_DEBUG("delete wantAgent.");
        auto wantAgent = static_cast<WantAgent*>(data);
        delete wantAgent;
        wantAgent = nullptr;
    };

    nativeObject->SetNativePointer(reinterpret_cast<void*>(wantAgent), nativeFinalize, nullptr);
    return result;
}

void JsWantAgent::UnwrapWantAgent(NativeEngine &engine, NativeValue* jsParam, void** result)
{
    HILOG_DEBUG("UnwrapWantAgent called.");
    if (jsParam == nullptr) {
        HILOG_ERROR("UnwrapWantAgent jsParam is nullptr!");
        return;
    }

    if (jsParam->TypeOf() != NATIVE_OBJECT) {
        HILOG_ERROR("UnwrapWantAgent jsParam type error!");
        return;
    }

    NativeObject *nativeObject = reinterpret_cast<NativeObject*>(jsParam->GetInterface(NativeObject::INTERFACE_ID));
    *result = nativeObject->GetNativePointer();
}

NativeValue* JsWantAgent::OnGetWantAgent(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("enter, argc = %{public}d", static_cast<int32_t>(info.argc));
    if (info.argc > ARGC_TWO || info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }

    NativeValue* lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    std::shared_ptr<WantAgentWantsParas> spParas = std::make_shared<WantAgentWantsParas>();
    int32_t ret = GetWantAgentParam(engine, info, *spParas);
    if (ret != 0) {
        HILOG_ERROR("Failed to get wantAgent param.");
        return RetErrMsg(engine, lastParam, ret);
    }

    AsyncTask::CompleteCallback complete = [weak = weak_from_this(), parasobj = spParas](NativeEngine &engine,
        AsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnGetWantAgent AsyncTask is called");
        auto self = weak.lock();
        std::shared_ptr<AAFwk::WantParams> extraInfo = std::make_shared<AAFwk::WantParams>(parasobj->extraInfo);
        WantAgentInfo wantAgentInfo(parasobj->requestCode,
                                    static_cast<WantAgentConstant::OperationType>(parasobj->operationType),
                                    parasobj->wantAgentFlags,
                                    parasobj->wants,
                                    extraInfo);

        auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
        std::shared_ptr<WantAgent> wantAgent = nullptr;
        WantAgentHelper::GetWantAgent(context, wantAgentInfo, wantAgent);
        WantAgent* pWantAgent = nullptr;
        if (wantAgent == nullptr) {
            HILOG_INFO("wantAgent is nullptr...");
        } else {
            pWantAgent = new WantAgent(wantAgent->GetPendingWant());
        }

        if (pWantAgent == nullptr) {
            HILOG_INFO("pWantAgent is nullptr...");
        }
        task.Resolve(engine, self->WrapWantAgent(engine, pWantAgent));
    };

    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnGetWantAgent",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnNapiGetWant(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    WantAgent* pWantAgent = nullptr;
    if (info.argc > ARGC_TWO || info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    UnwrapWantAgent(engine, info.argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    AsyncTask::CompleteCallback complete = [wantAgent](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnNapiGetWant AsyncTask is called");
        std::shared_ptr<Want> want = std::make_shared<Want>();
        ErrCode result = WantAgentHelper::GetWant(wantAgent, want);
        if (result != NO_ERROR) {
            task.Reject(engine, CreateJsError(engine, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
            return;
        }
        task.ResolveWithNoError(engine, CreateJsWant(engine, *(want)));
    };
    NativeValue* lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnNapiGetWant",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnNapiTrigger(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s is called", __FUNCTION__);
    if (info.argc != ARGC_THREE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::shared_ptr<WantAgent> wantAgent = nullptr;
    TriggerInfo triggerInfo;
    auto triggerObj = std::make_shared<TriggerCompleteCallBack>();
    int32_t errCode = UnWrapTriggerInfoParam(engine, info, wantAgent, triggerInfo, triggerObj);
    if (errCode != NO_ERROR) {
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    ErrCode result = WantAgentHelper::TriggerWantAgent(wantAgent, triggerObj, triggerInfo);
    if (result != ERR_OK) {
        return CreateJsError(engine, result, AbilityRuntimeErrorUtil::GetErrMessage(result));
    }
    return engine.CreateNull();
}

NativeValue* JsWantAgent::OnNapiGetWantAgent(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("enter, argc = %{public}d", static_cast<int32_t>(info.argc));
    if (info.argc > ARGC_TWO || info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::shared_ptr<WantAgentWantsParas> spParas = std::make_shared<WantAgentWantsParas>();
    int32_t ret = GetWantAgentParam(engine, info, *spParas);
    if (ret != 0) {
        HILOG_ERROR("Failed to get wantAgent param.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [weak = weak_from_this(), parasobj = spParas](NativeEngine &engine,
        AsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnNapiGetWantAgent AsyncTask is called");
        auto self = weak.lock();
        std::shared_ptr<AAFwk::WantParams> extraInfo = std::make_shared<AAFwk::WantParams>(parasobj->extraInfo);
        WantAgentInfo wantAgentInfo(parasobj->requestCode,
                                    static_cast<WantAgentConstant::OperationType>(parasobj->operationType),
                                    parasobj->wantAgentFlags,
                                    parasobj->wants,
                                    extraInfo);

        auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
        std::shared_ptr<WantAgent> wantAgent = nullptr;
        ErrCode result = WantAgentHelper::GetWantAgent(context, wantAgentInfo, wantAgent);
        if (result != NO_ERROR) {
            task.Reject(engine, CreateJsError(engine, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
        } else {
            WantAgent* pWantAgent = nullptr;
            if (wantAgent == nullptr) {
                result = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
                task.Reject(engine, CreateJsError(engine, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
                return;
            } else {
                pWantAgent = new (std::nothrow) WantAgent(wantAgent->GetPendingWant());
            }

            if (pWantAgent == nullptr) {
                HILOG_ERROR("pWantAgent is nullptr...");
                result = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
                task.Reject(engine, CreateJsError(engine, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
            } else {
                task.ResolveWithNoError(engine, self->WrapWantAgent(engine, pWantAgent));
            }
        }
    };

    NativeValue* lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnNapiGetWantAgent",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnNapiGetOperationType(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("JsWantAgent::OnNapiGetOperationType enter, argc = %{public}d", static_cast<int32_t>(info.argc));
    WantAgent* pWantAgent = nullptr;
    if (info.argc > ARGC_TWO || info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    UnwrapWantAgent(engine, info.argv[0], reinterpret_cast<void **>(&pWantAgent));

    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    AsyncTask::CompleteCallback complete = [wantAgent](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnNapiGetOperationType AsyncTask is called");
        int32_t operType;
        ErrCode ret = WantAgentHelper::GetType(wantAgent, operType);
        if (ret != NO_ERROR) {
            task.Reject(engine, CreateJsError(engine, ret, AbilityRuntimeErrorUtil::GetErrMessage(ret)));
        } else {
            task.ResolveWithNoError(engine, CreateJsValue(engine, operType));
        }
    };

    NativeValue* lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnNapiGetOperationType",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* WantAgentFlagsInit(NativeEngine* engine)
{
    HILOG_INFO("enter");

    if (engine == nullptr) {
        HILOG_ERROR("Invalid input parameters");
        return nullptr;
    }

    NativeValue* objValue = engine->CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("ONE_TIME_FLAG", CreateJsValue(*engine, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ZERO)));
    object->SetProperty("NO_BUILD_FLAG", CreateJsValue(*engine, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ONE)));
    object->SetProperty("CANCEL_PRESENT_FLAG", CreateJsValue(*engine, static_cast<int32_t>(NUMBER_OF_PARAMETERS_TWO)));
    object->SetProperty("UPDATE_PRESENT_FLAG", CreateJsValue(*engine,
        static_cast<int32_t>(NUMBER_OF_PARAMETERS_THREE)));
    object->SetProperty("CONSTANT_FLAG", CreateJsValue(*engine, static_cast<int32_t>(NUMBER_OF_PARAMETERS_FOUR)));
    object->SetProperty("REPLACE_ELEMENT", CreateJsValue(*engine, static_cast<int32_t>(NUMBER_OF_PARAMETERS_FIVE)));
    object->SetProperty("REPLACE_ACTION", CreateJsValue(*engine, static_cast<int32_t>(NUMBER_OF_PARAMETERS_SIX)));
    object->SetProperty("REPLACE_URI", CreateJsValue(*engine, static_cast<int32_t>(NUMBER_OF_PARAMETERS_SEVEN)));
    object->SetProperty("REPLACE_ENTITIES", CreateJsValue(*engine, static_cast<int32_t>(NUMBER_OF_PARAMETERS_EIGHT)));
    object->SetProperty("REPLACE_BUNDLE", CreateJsValue(*engine, static_cast<int32_t>(NUMBER_OF_PARAMETERS_NINE)));

    return objValue;
}

NativeValue* WantAgentOperationTypeInit(NativeEngine* engine)
{
    HILOG_INFO("enter");

    if (engine == nullptr) {
        HILOG_ERROR("Invalid input parameters");
        return nullptr;
    }

    NativeValue* objValue = engine->CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("UNKNOWN_TYPE", CreateJsValue(*engine, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ZERO)));
    object->SetProperty("START_ABILITY", CreateJsValue(*engine, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ONE)));
    object->SetProperty("START_ABILITIES", CreateJsValue(*engine, static_cast<int32_t>(NUMBER_OF_PARAMETERS_TWO)));
    object->SetProperty("START_SERVICE", CreateJsValue(*engine, static_cast<int32_t>(NUMBER_OF_PARAMETERS_THREE)));
    object->SetProperty("SEND_COMMON_EVENT", CreateJsValue(*engine, static_cast<int32_t>(NUMBER_OF_PARAMETERS_FOUR)));
    object->SetProperty("START_FOREGROUND_SERVICE", CreateJsValue(*engine,
        static_cast<int32_t>(NUMBER_OF_PARAMETERS_FIVE)));

    return objValue;
}

NativeValue* JsWantAgentInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_DEBUG("JsWantAgentInit is called");

    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("engine or exportObj null");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("object null");
        return nullptr;
    }

    std::unique_ptr<JsWantAgent> jsWantAgent = std::make_unique<JsWantAgent>();
    object->SetNativePointer(jsWantAgent.release(), JsWantAgent::Finalizer, nullptr);

    object->SetProperty("WantAgentFlags", WantAgentFlagsInit(engine));
    object->SetProperty("OperationType", WantAgentOperationTypeInit(engine));

    HILOG_DEBUG("JsWantAgentInit BindNativeFunction called");
    const char* moduleName = "JsWantAgent";
    BindNativeFunction(*engine, *object, "equal", moduleName, JsWantAgent::Equal);
    BindNativeFunction(*engine, *object, "getWant", moduleName, JsWantAgent::GetWant);
    BindNativeFunction(*engine, *object, "getOperationType", moduleName, JsWantAgent::GetOperationType);
    BindNativeFunction(*engine, *object, "getBundleName", moduleName, JsWantAgent::GetBundleName);
    BindNativeFunction(*engine, *object, "getUid", moduleName, JsWantAgent::GetUid);
    BindNativeFunction(*engine, *object, "cancel", moduleName, JsWantAgent::Cancel);
    BindNativeFunction(*engine, *object, "trigger", moduleName, JsWantAgent::Trigger);
    BindNativeFunction(*engine, *object, "getWantAgent", moduleName, JsWantAgent::GetWantAgent);
    HILOG_DEBUG("JsWantAgentInit end");
    return engine->CreateUndefined();
}

napi_value NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}
}  // namespace OHOS
