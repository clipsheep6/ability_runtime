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
#include "ipc_skeleton.h"
#include "js_runtime_utils.h"
#include "napi_common.h"
#include "want_agent_helper.h"
#include "tokenid_kit.h"

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

void TriggerCompleteCallBack::SetCallbackInfo(napi_env env, NativeReference* ref)
{
    triggerCompleteInfo_.env = env;
    triggerCompleteInfo_.nativeRef.reset(ref);
}

void TriggerCompleteCallBack::SetWantAgentInstance(WantAgent* wantAgent)
{
    triggerCompleteInfo_.wantAgent = wantAgent;
}

napi_value RetErrMsg(napi_env env, napi_value lastParam, int32_t errorCode)
{
    HILOG_DEBUG("RetErrMsg called");
    auto complete = [err = errorCode] (napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_DEBUG("error information output");
        task.Reject(env, CreateJsError(env, err, "PARAMETER_ERROR"));
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("RetErrMsg",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));

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
    if (dataWorkerData->resultData == "canceled") {
        HILOG_INFO("wantAgent is canceled");
        delete dataWorkerData;
        dataWorkerData = nullptr;
        delete work;
        return;
    }
    napi_value args[ARGC_TWO] = {0};
    napi_value objValueFirst = nullptr;
    napi_create_object(dataWorkerData->env, &objValueFirst);
    if (objValueFirst == nullptr) {
        HILOG_ERROR("Failed to get objValueFirst");
        delete dataWorkerData;
        dataWorkerData = nullptr;
        delete work;
        return;
    }
    napi_value objValueSecond = nullptr;
    napi_create_object(dataWorkerData->env, &objValueSecond);
    if (objValueSecond == nullptr) {
        HILOG_ERROR("Failed to get objValueSecond");
        delete dataWorkerData;
        dataWorkerData = nullptr;
        delete work;
        return;
    }

#ifdef ENABLE_ERRCODE
    objValueFirst = CreateJsUndefined(dataWorkerData->env);
#else
    napi_set_named_property(dataWorkerData->env, objValueFirst, "code",
        CreateJsValue(dataWorkerData->env, BUSINESS_ERROR_CODE_OK));
#endif
    napi_set_named_property(dataWorkerData->env, objValueSecond, "wantAgent",
        JsWantAgent::WrapWantAgent(dataWorkerData->env, dataWorkerData->wantAgent));
    napi_set_named_property(dataWorkerData->env, objValueSecond, "want",
        CreateJsWant(dataWorkerData->env, dataWorkerData->want));
    napi_set_named_property(dataWorkerData->env, objValueSecond, "finalCode",
        CreateJsValue(dataWorkerData->env, dataWorkerData->resultCode));
    napi_set_named_property(dataWorkerData->env, objValueSecond, "finalData",
        CreateJsValue(dataWorkerData->env, dataWorkerData->resultData));
    napi_set_named_property(dataWorkerData->env, objValueSecond, "extraInfo",
        CreateJsWantParams(dataWorkerData->env, dataWorkerData->resultExtras));
    args[ARGC_ZERO] = objValueFirst;
    args[ARGC_ONE] = objValueSecond;

    napi_value value = dataWorkerData->nativeRef->GetNapiValue();
    napi_value callback = dataWorkerData->nativeRef->GetNapiValue();
    napi_call_function(dataWorkerData->env, value, callback, ARGC_TWO, args, nullptr);
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
    napi_get_uv_event_loop(triggerCompleteInfo_.env, &loop);
#endif  // NAPI_VERSION >= 2
    if (loop == nullptr) {
        HILOG_INFO("loop instance is nullptr");
        return;
    }

    uv_work_t* work = new (std::nothrow) uv_work_t;
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
    dataWorker->env = triggerCompleteInfo_.env;
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

void JsWantAgent::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_INFO("JsWantAgent::Finalizer is called");
    std::unique_ptr<JsWantAgent>(static_cast<JsWantAgent*>(data));
}

napi_value JsWantAgent::Equal(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnEqual(env, info) : nullptr;
};

napi_value JsWantAgent::GetWant(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnGetWant(env, info) : nullptr;
};

napi_value JsWantAgent::GetOperationType(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnGetOperationType(env, info) : nullptr;
};

napi_value JsWantAgent::GetBundleName(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnGetBundleName(env, info) : nullptr;
};

napi_value JsWantAgent::GetUid(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnGetUid(env, info) : nullptr;
};

napi_value JsWantAgent::Cancel(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnCancel(env, info) : nullptr;
};

napi_value JsWantAgent::Trigger(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnTrigger(env, info) : nullptr;
};

napi_value JsWantAgent::GetWantAgent(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnGetWantAgent(env, info) : nullptr;
};

napi_value JsWantAgent::NapiGetWant(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnNapiGetWant(env, info) : nullptr;
};

napi_value JsWantAgent::NapiTrigger(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnNapiTrigger(env, info) : nullptr;
};

napi_value JsWantAgent::NapiGetWantAgent(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnNapiGetWantAgent(env, info) : nullptr;
};

napi_value JsWantAgent::NapiGetOperationType(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnNapiGetOperationType(env, info) : nullptr;
};

napi_value JsWantAgent::OnEqual(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    WantAgent* pWantAgentFirst = nullptr;
    WantAgent* pWantAgentSecond = nullptr;
    if (argc < ARGC_TWO || argc > ARGC_THREE) {
        HILOG_ERROR("Not enough params");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
#endif
        return CreateJsUndefined(env);
    }

    napi_value lastParam = (argc >= ARGC_THREE) ? argv[INDEX_TWO] : nullptr;
    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        HILOG_ERROR("Wrong argument type. Object expected.");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
#else
        return RetErrMsg(env, lastParam, ERR_NOT_OK);
#endif
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgentFirst));
    if (pWantAgentFirst == nullptr) {
        HILOG_ERROR("Parse pWantAgentFirst failed");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
#else
        return RetErrMsg(env, lastParam, ERR_NOT_OK);
#endif
    }

    if (!CheckTypeForNapiValue(env, argv[1], napi_object)) {
        HILOG_ERROR("Wrong argument type. Object expected.");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
#else
        return RetErrMsg(env, lastParam, ERR_NOT_OK);
#endif
    }

    UnwrapWantAgent(env, argv[1], reinterpret_cast<void **>(&pWantAgentSecond));
    if (pWantAgentSecond == nullptr) {
        HILOG_ERROR("Parse pWantAgentSceond failed");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
#else
        return RetErrMsg(env, lastParam, ERR_NOT_OK);
#endif
    }

    std::shared_ptr<WantAgent> wantAgentFirst = std::make_shared<WantAgent>(*pWantAgentFirst);
    std::shared_ptr<WantAgent> wantAgentSecond = std::make_shared<WantAgent>(*pWantAgentSecond);
    NapiAsyncTask::CompleteCallback complete =
        [wantAgentFirst, wantAgentSecond](napi_env env, NapiAsyncTask &task, int32_t status) {
            bool ret = false;
            ErrCode retCode = WantAgentHelper::IsEquals(wantAgentFirst, wantAgentSecond);
#ifdef ENABLE_ERRCODE
            if (retCode == ERR_NOT_OK) {
                ret = false;
                task.ResolveWithNoError(env, CreateJsValue(env, ret));
            } else if (retCode == ERR_OK) {
                ret = true;
                task.ResolveWithNoError(env, CreateJsValue(env, ret));
            } else {
                task.Reject(env, CreateJsError(env, retCode, AbilityRuntimeErrorUtil::GetErrMessage(retCode)));
            }
#else
            if (retCode != ERR_OK) {
                ret = false;
                task.Resolve(env, CreateJsValue(env, ret));
            } else {
                ret = true;
                task.Resolve(env, CreateJsValue(env, ret));
            }
#endif
        };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnEqual",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWantAgent::OnGetWant(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    HILOG_DEBUG("enter, argc = %{public}d", static_cast<int32_t>(argc));
    int32_t errCode = BUSINESS_ERROR_CODE_OK;
    WantAgent* pWantAgent = nullptr;
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        HILOG_ERROR("Not enough arguments");
        return CreateJsUndefined(env);
    }

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        HILOG_ERROR("Wrong parameter type. Object expected.");
        errCode = ERR_NOT_OK;
        return RetErrMsg(env, lastParam, errCode);
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent error");
        errCode = ERR_NOT_OK;
        return RetErrMsg(env, lastParam, errCode);
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    NapiAsyncTask::CompleteCallback complete = [wantAgent](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnGetWant NapiAsyncTask is called");
        std::shared_ptr<Want> want = WantAgentHelper::GetWant(wantAgent);
        if (want == nullptr) {
            task.Reject(env, CreateJsError(env, ERR_NOT_OK, "WantAgentHelper::GetWant result nullptr."));
            return;
        }
        task.Resolve(env, CreateJsWant(env, *(want)));
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnGetWant",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWantAgent::OnGetOperationType(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    HILOG_DEBUG("JsWantAgent::OnGetOperationType enter, argc = %{public}d", static_cast<int32_t>(argc));
    int32_t errCode = BUSINESS_ERROR_CODE_OK;
    WantAgent* pWantAgent = nullptr;
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        return CreateJsUndefined(env);
    }

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        errCode = ERR_NOT_OK;
        return RetErrMsg(env, lastParam, errCode);
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
        errCode = ERR_NOT_OK;
        return RetErrMsg(env, lastParam, errCode);
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    NapiAsyncTask::CompleteCallback complete = [wantAgent](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnGetOperationType NapiAsyncTask is called");
        auto ret = WantAgentHelper::GetType(wantAgent);
        task.Resolve(env, CreateJsValue(env, ret));
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnGetOperationType",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWantAgent::OnGetBundleName(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    WantAgent* pWantAgent = nullptr;
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
#endif
        return CreateJsUndefined(env);
    }

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        HILOG_ERROR("Wrong argument type. Object expected.");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
#else
        return RetErrMsg(env, lastParam, ERR_NOT_OK);
#endif
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
#else
        return RetErrMsg(env, lastParam, ERR_NOT_OK);
#endif
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    NapiAsyncTask::CompleteCallback complete = [wantAgent](napi_env env, NapiAsyncTask &task, int32_t status) {
        std::string bundleName = "";
#ifdef ENABLE_ERRCODE
        ErrCode result = WantAgentHelper::GetBundleName(wantAgent, bundleName);
        if (result != NO_ERROR) {
            task.Reject(env, CreateJsError(env, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
        } else {
            task.ResolveWithNoError(env, CreateJsValue(env, bundleName));
        }
#else
        WantAgentHelper::GetBundleName(wantAgent, bundleName);
        task.Resolve(env, CreateJsValue(env, bundleName));
#endif
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnGetBundleName",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWantAgent::OnGetUid(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    WantAgent* pWantAgent = nullptr;
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
#endif
        return CreateJsUndefined(env);
    }

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        HILOG_ERROR("Wrong argument type. Object expected.");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
#else
        return RetErrMsg(env, lastParam, ERR_NOT_OK);
#endif
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
#else
        return RetErrMsg(env, lastParam, ERR_NOT_OK);
#endif
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    NapiAsyncTask::CompleteCallback complete = [wantAgent](napi_env env, NapiAsyncTask &task, int32_t status) {
        int uid = -1;
#ifdef ENABLE_ERRCODE
        ErrCode result = WantAgentHelper::GetUid(wantAgent, uid);
        if (result != NO_ERROR) {
            task.Reject(env, CreateJsError(env, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
        } else {
            task.ResolveWithNoError(env, CreateJsValue(env, uid));
        }
#else
        WantAgentHelper::GetUid(wantAgent, uid);
        task.Resolve(env, CreateJsValue(env, uid));
#endif
    };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnGetUid",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWantAgent::OnCancel(napi_env env, napi_callback_info info)
{
    WantAgent* pWantAgent = nullptr;
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
#endif
        return CreateJsUndefined(env);
    }

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        HILOG_ERROR("Wrong argument type. Object expected.");
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
#else
        return RetErrMsg(env, lastParam, ERR_NOT_OK);
#endif
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
#ifdef ENABLE_ERRCODE
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
#else
        return RetErrMsg(env, lastParam, ERR_NOT_OK);
#endif
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    NapiAsyncTask::CompleteCallback complete =
        [wantAgent](napi_env env, NapiAsyncTask &task, int32_t status) {
#ifdef ENABLE_ERRCODE
            ErrCode result = WantAgentHelper::Cancel(wantAgent);
            if (result != NO_ERROR) {
                task.Reject(env, CreateJsError(env, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
            } else {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            }
#else
            WantAgentHelper::Cancel(wantAgent);
            task.Resolve(env, CreateJsUndefined(env));
#endif
        };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWantAgent::OnCancel",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWantAgent::OnTrigger(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("%{public}s is called", __FUNCTION__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_THREE) {
        HILOG_ERROR("Not enough params");
        return CreateJsUndefined(env);
    }

    std::shared_ptr<WantAgent> wantAgent = nullptr;
    TriggerInfo triggerInfo;
    auto triggerObj = std::make_shared<TriggerCompleteCallBack>();
    int32_t errCode = UnWrapTriggerInfoParam(env, info, wantAgent, triggerInfo, triggerObj);
    if (errCode != BUSINESS_ERROR_CODE_OK) {
        return RetErrMsg(env, argv[ARGC_TWO], errCode);
    }

    WantAgentHelper::TriggerWantAgent(wantAgent, triggerObj, triggerInfo);
    return CreateJsNull(env);
}

int32_t JsWantAgent::UnWrapTriggerInfoParam(napi_env env, napi_callback_info info,
    std::shared_ptr<WantAgent> &wantAgent, TriggerInfo &triggerInfo,
    std::shared_ptr<TriggerCompleteCallBack> &triggerObj)
{
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_THREE) {
        HILOG_ERROR("Not enough params");
        return ERR_NOT_OK;
    }

    if (!CheckTypeForNapiValue(env, argv[ARGC_ZERO], napi_object)) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        return ERR_NOT_OK;
    }
    WantAgent* pWantAgent = nullptr;
    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));

    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
        return ERR_NOT_OK;
    }
    wantAgent = std::make_shared<WantAgent>(*pWantAgent);

    int32_t ret = GetTriggerInfo(env, argv[ARGC_ONE], triggerInfo);
    if (ret != BUSINESS_ERROR_CODE_OK) {
        HILOG_ERROR("Get trigger info error");
        return ret;
    }

    napi_ref ref = nullptr;
    napi_create_reference(env, argv[ARGC_TWO], 1, &ref);
    triggerObj->SetCallbackInfo(env, reinterpret_cast<NativeReference*>(ref));
    triggerObj->SetWantAgentInstance(pWantAgent);

    return BUSINESS_ERROR_CODE_OK;
}

int32_t JsWantAgent::GetTriggerInfo(napi_env env, napi_value param, TriggerInfo &triggerInfo)
{
    if (!CheckTypeForNapiValue(env, param, napi_object)) {
        HILOG_ERROR("param type mismatch!");
        return ERR_NOT_OK;
    }

    int32_t code = -1;
    napi_value jsCode = nullptr;
    napi_get_named_property(env, param, "code", &jsCode);
    if (!ConvertFromJsValue(env, jsCode, code)) {
        HILOG_ERROR("GetTriggerInfo convert code error!");
        return ERR_NOT_OK;
    }

    std::shared_ptr<AAFwk::Want> want = nullptr;
    bool hasProperty = false;
    napi_has_named_property(env, param, "want", &hasProperty);
    if (hasProperty) {
        napi_value jsWant = nullptr;
        napi_get_named_property(env, param, "want", &jsWant);
        want = std::make_shared<AAFwk::Want>();
        if (!UnwrapWant(env, jsWant, *want)) {
            HILOG_ERROR("GetTriggerInfo convert want error!");
            return ERR_NOT_OK;
        }
    }

    std::string permission = "";
    napi_has_named_property(env, param, "permission", &hasProperty);
    if (hasProperty) {
        napi_value jsPermission = nullptr;
        napi_get_named_property(env, param, "permission", &jsPermission);
        if (!ConvertFromJsValue(env, jsPermission, permission)) {
            HILOG_ERROR("GetTriggerInfo convert permission error!");
            return ERR_NOT_OK;
        }
    }

    std::shared_ptr<AAFwk::WantParams> extraInfo = nullptr;
    bool hasExtraInfo = false;
    napi_value jsExtraInfo = nullptr;
    napi_has_named_property(env, param, "extraInfos", &hasExtraInfo);
    if (hasExtraInfo) {
        napi_get_named_property(env, param, "extraInfos", &jsExtraInfo);
    } else {
        napi_has_named_property(env, param, "extraInfo", &hasExtraInfo);
        if (hasExtraInfo) {
            napi_get_named_property(env, param, "extraInfo", &jsExtraInfo);
        }
    }
    if (hasExtraInfo) {
        extraInfo = std::make_shared<AAFwk::WantParams>();
        if (!UnwrapWantParams(env, (jsExtraInfo), *extraInfo)) {
            HILOG_ERROR("GetTriggerInfo convert extraInfo error!");
            return ERR_NOT_OK;
        }
    }

    TriggerInfo triggerInfoData(permission, extraInfo, want, code);
    triggerInfo = triggerInfoData;
    return BUSINESS_ERROR_CODE_OK;
}

int32_t JsWantAgent::GetWantAgentParam(napi_env env, napi_callback_info info, WantAgentWantsParas &paras)
{
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        HILOG_ERROR("param type mismatch!");
        return PARAMETER_ERROR;
    }

    napi_value jsWants = nullptr;
    napi_get_named_property(env, argv[0], "wants", &jsWants);

    bool isArray = false;
    napi_is_array(env, jsWants, &isArray);
    if (!isArray) {
        HILOG_ERROR("wants is not array!");
        return PARAMETER_ERROR;
    }

    uint32_t length = 0;
    napi_get_array_length(env, jsWants, &length);
    for (uint32_t i = 0; i < length; i++) {
        std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
        napi_value jsWant = nullptr;
        napi_get_element(env, jsWants, i, &jsWant);
        if (!UnwrapWant(env, jsWant, *want)) {
            HILOG_ERROR("UnwrapWant failed!");
            return PARAMETER_ERROR;
        }
        paras.wants.emplace_back(want);
    }

    bool hasActionType = false;
    napi_has_named_property(env, argv[0], "actionType", &hasActionType);
    if (hasActionType) {
        napi_value jsActionType = nullptr;
        napi_get_named_property(env, argv[0], "actionType", &jsActionType);
        if (!ConvertFromJsValue(env, jsActionType, paras.operationType)) {
            HILOG_ERROR("Convert actionType failed!");
            return PARAMETER_ERROR;
        }
    }

    bool hasOperationType = false;
    napi_has_named_property(env, argv[0], "operationType", &hasOperationType);
    if (!hasActionType && hasOperationType) {
        napi_value jsOperationType = nullptr;
        napi_get_named_property(env, argv[0], "operationType", &jsOperationType);
        if (!ConvertFromJsValue(env, jsOperationType, paras.operationType)) {
            HILOG_ERROR("Convert operationType failed!");
            return PARAMETER_ERROR;
        }
    }

    napi_value jsRequestCode = nullptr;
    napi_get_named_property(env, argv[0], "requestCode", &jsRequestCode);
    if (!ConvertFromJsValue(env, jsRequestCode, paras.requestCode)) {
        HILOG_ERROR("Convert requestCode failed!");
        return PARAMETER_ERROR;
    }

    bool hasActionFlags = false;
    napi_has_named_property(env, argv[0], "actionFlags", &hasActionFlags);
    if (hasActionFlags) {
        napi_value jsActionFlags = nullptr;
        napi_get_named_property(env, argv[0], "actionFlags", &jsActionFlags);
        bool jsActionFlagsIsArray = false;
        napi_is_array(env, jsActionFlags, &jsActionFlagsIsArray);
        if (!jsActionFlagsIsArray) {
            HILOG_ERROR("actionFlags is not array!");
            return PARAMETER_ERROR;
        }

        uint32_t jsActionFlagsLen = 0;
        napi_get_array_length(env, jsActionFlags, &jsActionFlagsLen);
        for (uint32_t i = 0; i < jsActionFlagsLen; i++) {
            napi_value jsActionFlag = nullptr;
            napi_get_element(env, jsActionFlags, i, &jsActionFlag);
            if (!CheckTypeForNapiValue(env, jsActionFlag, napi_number)) {
                HILOG_ERROR("ActionFlag type error!");
                return PARAMETER_ERROR;
            }
            int32_t actionFlag = 0;
            if (!ConvertFromJsValue(env, jsActionFlag, actionFlag)) {
                HILOG_ERROR("Convert actionFlag failed!");
                return PARAMETER_ERROR;
            }
            paras.wantAgentFlags.emplace_back(static_cast<WantAgentConstant::Flags>(actionFlag));
        }
    }

    bool hasWantAgentFlags = false;
    napi_has_named_property(env, argv[0], "wantAgentFlags", &hasWantAgentFlags);
    if (!hasActionFlags && hasWantAgentFlags) {
        napi_value jsWantAgentFlags = nullptr;
        napi_get_named_property(env, argv[0], "wantAgentFlags", &jsWantAgentFlags);
        bool jsWantAgentFlagsIsArray = false;
        napi_is_array(env, jsWantAgentFlags, &jsWantAgentFlagsIsArray);
        if (!jsWantAgentFlagsIsArray) {
            HILOG_ERROR("wantAgentFlags is not array!");
            return PARAMETER_ERROR;
        }

        uint32_t jsWantAgentFlagsLen = 0;
        napi_get_array_length(env, jsWantAgentFlags, &jsWantAgentFlagsLen);
        for (uint32_t i = 0; i < jsWantAgentFlagsLen; i++) {
            napi_value jsWantAgentFlag = nullptr;
            napi_get_element(env, jsWantAgentFlags, i, &jsWantAgentFlag);
            if (!CheckTypeForNapiValue(env, jsWantAgentFlag, napi_number)) {
                HILOG_ERROR("WantAgentFlag type failed!");
                return PARAMETER_ERROR;
            }
            int32_t wantAgentFlag = 0;
            if (!ConvertFromJsValue(env, jsWantAgentFlag, wantAgentFlag)) {
                HILOG_ERROR("Convert WantAgentFlag failed!");
                return PARAMETER_ERROR;
            }
            paras.wantAgentFlags.emplace_back(static_cast<WantAgentConstant::Flags>(wantAgentFlag));
        }
    }

    bool hasExtraInfo = false;
    napi_value jsExtraInfo = nullptr;
    napi_has_named_property(env, argv[0], "extraInfos", &hasExtraInfo);
    if (hasExtraInfo) {
        napi_get_named_property(env, argv[0], "extraInfos", &jsExtraInfo);
    } else {
        napi_has_named_property(env, argv[0], "extraInfo", &hasExtraInfo);
        if (hasExtraInfo) {
            napi_get_named_property(env, argv[0], "extraInfo", &jsExtraInfo);
        }
    }
    if (hasExtraInfo) {
        if (!CheckTypeForNapiValue(env, jsExtraInfo, napi_object)) {
            HILOG_ERROR("ExtraInfo type error!");
            return PARAMETER_ERROR;
        }
        if (!UnwrapWantParams(env, (jsExtraInfo), paras.extraInfo)) {
            HILOG_ERROR("Convert extraInfo failed!");
            return PARAMETER_ERROR;
        }
    }
    return BUSINESS_ERROR_CODE_OK;
}

napi_value JsWantAgent::WrapWantAgent(napi_env env, WantAgent* wantAgent)
{
    HILOG_DEBUG("WrapWantAgent called.");
    napi_value wantAgentClass = nullptr;
    napi_define_class(
        env,
        "WantAgentClass",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr,
        0,
        nullptr,
        &wantAgentClass);
    napi_value result = nullptr;
    napi_new_instance(env, wantAgentClass, 0, nullptr, &result);
    if (result == nullptr) {
        HILOG_ERROR("create instance failed.");
        delete wantAgent;
        wantAgent = nullptr;
        return nullptr;
    }

    napi_wrap(env,
        result,
        reinterpret_cast<void*>(wantAgent),
        [](napi_env env, void* data, void* hint) {
            HILOG_DEBUG("delete wantAgent.");
            auto agent = static_cast<WantAgent*>(data);
            delete agent;
            agent = nullptr;
        },
        nullptr,
        nullptr);
    return result;
}

void JsWantAgent::UnwrapWantAgent(napi_env env, napi_value jsParam, void** result)
{
    HILOG_DEBUG("UnwrapWantAgent called.");
    if (jsParam == nullptr) {
        HILOG_ERROR("UnwrapWantAgent jsParam is nullptr!");
        return;
    }

    if (!CheckTypeForNapiValue(env, jsParam, napi_object)) {
        HILOG_ERROR("UnwrapWantAgent jsParam type error!");
        return;
    }

    napi_unwrap(env, jsParam, result);
}

napi_value JsWantAgent::OnGetWantAgent(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    HILOG_DEBUG("enter, argc = %{public}d", static_cast<int32_t>(argc));
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        return CreateJsUndefined(env);
    }

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    std::shared_ptr<WantAgentWantsParas> spParas = std::make_shared<WantAgentWantsParas>();
    int32_t ret = GetWantAgentParam(env, info, *spParas);
    if (ret != 0) {
        HILOG_ERROR("Failed to get wantAgent parameter.");
        return RetErrMsg(env, lastParam, ret);
    }

    NapiAsyncTask::CompleteCallback complete = [weak = weak_from_this(), parasobj = spParas](napi_env env,
        NapiAsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnGetWantAgent NapiAsyncTask is called");
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
        task.Resolve(env, self->WrapWantAgent(env, pWantAgent));
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnGetWantAgent",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWantAgent::OnNapiGetWant(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    HILOG_DEBUG("enter, argc = %{public}d", static_cast<int32_t>(argc));
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    WantAgent* pWantAgent = nullptr;
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        HILOG_ERROR("This application is not system-app, can not use system-api");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    NapiAsyncTask::CompleteCallback complete = [wantAgent](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnNapiGetWant NapiAsyncTask is start");
        std::shared_ptr<Want> want = std::make_shared<Want>();
        ErrCode result = WantAgentHelper::GetWant(wantAgent, want);
        if (result != NO_ERROR) {
            task.Reject(env, CreateJsError(env, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsWant(env, *(want)));
    };
    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnNapiGetWant",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWantAgent::OnNapiTrigger(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("%{public}s is called", __FUNCTION__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_THREE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<WantAgent> wantAgent = nullptr;
    TriggerInfo triggerInfo;
    auto triggerObj = std::make_shared<TriggerCompleteCallBack>();
    int32_t errCode = UnWrapTriggerInfoParam(env, info, wantAgent, triggerInfo, triggerObj);
    if (errCode != NO_ERROR) {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    ErrCode result = WantAgentHelper::TriggerWantAgent(wantAgent, triggerObj, triggerInfo);
    if (result != ERR_OK) {
        return CreateJsError(env, result, AbilityRuntimeErrorUtil::GetErrMessage(result));
    }
    return CreateJsNull(env);
}

napi_value JsWantAgent::OnNapiGetWantAgent(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<WantAgentWantsParas> spParas = std::make_shared<WantAgentWantsParas>();
    int32_t ret = GetWantAgentParam(env, info, *spParas);
    if (ret != 0) {
        HILOG_ERROR("Failed to get wantAgent param.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete = [weak = weak_from_this(), parasobj = spParas](napi_env env,
        NapiAsyncTask &task, int32_t status) {
        auto self = weak.lock();
        std::shared_ptr<AAFwk::WantParams> extraInfo = std::make_shared<AAFwk::WantParams>(parasobj->extraInfo);
        WantAgentInfo wantAgentInfo(parasobj->requestCode,
                                    static_cast<WantAgentConstant::OperationType>(parasobj->operationType),
                                    parasobj->wantAgentFlags, parasobj->wants, extraInfo);

        auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
        std::shared_ptr<WantAgent> wantAgent = nullptr;
        ErrCode result = WantAgentHelper::GetWantAgent(context, wantAgentInfo, wantAgent);
        if (result != NO_ERROR) {
            task.Reject(env, CreateJsError(env, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
        } else {
            WantAgent* pWantAgent = nullptr;
            result = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
            if (wantAgent == nullptr) {
                task.Reject(env, CreateJsError(env, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
                return;
            }
            pWantAgent = new (std::nothrow) WantAgent(wantAgent->GetPendingWant());
            if (pWantAgent == nullptr) {
                task.Reject(env, CreateJsError(env, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
            } else {
                task.ResolveWithNoError(env, self->WrapWantAgent(env, pWantAgent));
            }
        }
    };

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnNapiGetWantAgent",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWantAgent::OnNapiGetOperationType(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    HILOG_DEBUG("JsWantAgent::OnNapiGetOperationType enter, argc = %{public}d", static_cast<int32_t>(argc));
    WantAgent* pWantAgent = nullptr;
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));

    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    NapiAsyncTask::CompleteCallback complete = [wantAgent](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnNapiGetOperationType NapiAsyncTask is called");
        int32_t operType;
        ErrCode ret = WantAgentHelper::GetType(wantAgent, operType);
        if (ret != NO_ERROR) {
            task.Reject(env, CreateJsError(env, ret, AbilityRuntimeErrorUtil::GetErrMessage(ret)));
        } else {
            task.ResolveWithNoError(env, CreateJsValue(env, operType));
        }
    };

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnNapiGetOperationType",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value WantAgentFlagsInit(napi_env env)
{
    HILOG_DEBUG("called");

    if (env == nullptr) {
        HILOG_ERROR("Invalid input parameters");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "ONE_TIME_FLAG",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ZERO)));
    napi_set_named_property(env, objValue, "NO_BUILD_FLAG",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ONE)));
    napi_set_named_property(env, objValue, "CANCEL_PRESENT_FLAG",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_TWO)));
    napi_set_named_property(env, objValue, "UPDATE_PRESENT_FLAG",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_THREE)));
    napi_set_named_property(env, objValue, "CONSTANT_FLAG",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_FOUR)));
    napi_set_named_property(env, objValue, "REPLACE_ELEMENT",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_FIVE)));
    napi_set_named_property(env, objValue, "REPLACE_ACTION",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_SIX)));
    napi_set_named_property(env, objValue, "REPLACE_URI",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_SEVEN)));
    napi_set_named_property(env, objValue, "REPLACE_ENTITIES",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_EIGHT)));
    napi_set_named_property(env, objValue, "REPLACE_BUNDLE",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_NINE)));

    return objValue;
}

napi_value WantAgentOperationTypeInit(napi_env env)
{
    HILOG_DEBUG("called");

    if (env == nullptr) {
        HILOG_ERROR("Invalid input parameters");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "UNKNOWN_TYPE",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ZERO)));
    napi_set_named_property(env, objValue, "START_ABILITY",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ONE)));
    napi_set_named_property(env, objValue, "START_ABILITIES",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_TWO)));
    napi_set_named_property(env, objValue, "START_SERVICE",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_THREE)));
    napi_set_named_property(env, objValue, "SEND_COMMON_EVENT",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_FOUR)));
    napi_set_named_property(env, objValue, "START_FOREGROUND_SERVICE",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_FIVE)));

    return objValue;
}

napi_value JsWantAgentInit(napi_env env, napi_value exportObj)
{
    HILOG_DEBUG("JsWantAgentInit is called");

    if (env == nullptr || exportObj == nullptr) {
        HILOG_ERROR("env or exportObj null.");
        return nullptr;
    }

    std::unique_ptr<JsWantAgent> jsWantAgent = std::make_unique<JsWantAgent>();
    napi_wrap(env, exportObj, jsWantAgent.release(), JsWantAgent::Finalizer, nullptr, nullptr);

    napi_set_named_property(env, exportObj, "WantAgentFlags", WantAgentFlagsInit(env));
    napi_set_named_property(env, exportObj, "OperationType", WantAgentOperationTypeInit(env));

    HILOG_DEBUG("JsWantAgentInit BindNativeFunction called");
    const char* moduleName = "JsWantAgent";
    BindNativeFunction(env, exportObj, "equal", moduleName, JsWantAgent::Equal);
    BindNativeFunction(env, exportObj, "getWant", moduleName, JsWantAgent::GetWant);
    BindNativeFunction(env, exportObj, "getOperationType", moduleName, JsWantAgent::GetOperationType);
    BindNativeFunction(env, exportObj, "getBundleName", moduleName, JsWantAgent::GetBundleName);
    BindNativeFunction(env, exportObj, "getUid", moduleName, JsWantAgent::GetUid);
    BindNativeFunction(env, exportObj, "cancel", moduleName, JsWantAgent::Cancel);
    BindNativeFunction(env, exportObj, "trigger", moduleName, JsWantAgent::Trigger);
    BindNativeFunction(env, exportObj, "getWantAgent", moduleName, JsWantAgent::GetWantAgent);
    HILOG_DEBUG("JsWantAgentInit end");
    return CreateJsUndefined(env);
}

napi_value NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}
}  // namespace OHOS
