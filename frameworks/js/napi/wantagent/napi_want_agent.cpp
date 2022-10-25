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

void TriggerCompleteCallBack::SetCallbackInfo(const napi_env &env, const napi_ref &ref)
{
    triggerCompleteInfo_.env = env;
    triggerCompleteInfo_.ref = ref;
}

void TriggerCompleteCallBack::SetWantAgentInstance(const std::shared_ptr<WantAgent> &wantAgent)
{
    triggerCompleteInfo_.wantAgent = wantAgent;
}

napi_value JSParaError(const napi_env &env, const bool bCallback)
{
    if (bCallback) {
        return NapiGetNull(env);
    }
    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    napi_create_promise(env, &deferred, &promise);
    napi_reject_deferred(env, deferred, GetCallbackErrorResult(env, PARAMETER_ERROR));
    return promise;
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

auto OnSendFinishedUvAfterWorkCallback = [](uv_work_t *work, int status) {
    HILOG_INFO("TriggerCompleteCallBack::OnSendFinishedUvAfterWorkCallback:status = %{public}d", status);

    TriggerReceiveDataWorker *dataWorkerData = static_cast<TriggerReceiveDataWorker *>(work->data);
    if (dataWorkerData == nullptr) {
        HILOG_INFO("TriggerReceiveDataWorker instance(uv_work_t) is nullptr");
        delete work;
        return;
    }
    napi_value result[NUMBER_OF_PARAMETERS_TWO] = {0};

    result[0] = GetCallbackErrorResult(dataWorkerData->env, BUSINESS_ERROR_CODE_OK);
    napi_create_object(dataWorkerData->env, &result[1]);
    // wrap wantAgent
    napi_value wantAgentClass = nullptr;
    auto constructorcb = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
        return thisVar;
    };
    napi_define_class(
        dataWorkerData->env, "WantAgentClass", NAPI_AUTO_LENGTH, constructorcb, nullptr, 0, nullptr, &wantAgentClass);
    napi_value jsWantAgent = nullptr;
    napi_new_instance(dataWorkerData->env, wantAgentClass, 0, nullptr, &jsWantAgent);
    auto finalizecb = [](napi_env env, void *data, void *hint) {};
    napi_wrap(dataWorkerData->env, jsWantAgent, (void *)dataWorkerData->wantAgent.get(), finalizecb, nullptr, nullptr);
    napi_set_named_property(dataWorkerData->env, result[1], "wantAgent", jsWantAgent);
    //  wrap want
    napi_value jsWant = WrapWant(dataWorkerData->env, dataWorkerData->want);
    napi_set_named_property(dataWorkerData->env, result[1], "want", jsWant);
    // wrap finalCode
    napi_value jsFinalCode = nullptr;
    napi_create_int32(dataWorkerData->env, dataWorkerData->resultCode, &jsFinalCode);
    napi_set_named_property(dataWorkerData->env, result[1], "finalCode", jsFinalCode);
    // wrap finalData
    napi_value jsFinalData = nullptr;
    napi_create_string_utf8(dataWorkerData->env, dataWorkerData->resultData.c_str(), NAPI_AUTO_LENGTH, &jsFinalData);
    napi_set_named_property(dataWorkerData->env, result[1], "finalData", jsFinalData);
    // wrap extraInfo
    napi_value jsExtraInfo = WrapWantParams(dataWorkerData->env, dataWorkerData->resultExtras);
    napi_set_named_property(dataWorkerData->env, result[1], "extraInfo", jsExtraInfo);

    napi_value callResult = nullptr;
    napi_value undefined = nullptr;
    napi_value callback = nullptr;
    napi_get_undefined(dataWorkerData->env, &undefined);
    napi_get_reference_value(dataWorkerData->env, dataWorkerData->ref, &callback);
    napi_call_function(dataWorkerData->env, undefined, callback, NUMBER_OF_PARAMETERS_TWO, &result[0], &callResult);

    delete dataWorkerData;
    dataWorkerData = nullptr;
    delete work;
};

void TriggerCompleteCallBack::OnSendFinished(
    const AAFwk::Want &want, int resultCode, const std::string &resultData, const AAFwk::WantParams &resultExtras)
{
    HILOG_INFO("TriggerCompleteCallBack::OnSendFinished start");
    if (triggerCompleteInfo_.ref == nullptr) {
        HILOG_INFO("triggerCompleteInfo_ CallBack is nullptr");
        return;
    }
    uv_loop_s *loop = nullptr;
#if NAPI_VERSION >= NUMBER_OF_PARAMETERS_TWO
    napi_get_uv_event_loop(triggerCompleteInfo_.env, &loop);
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
    TriggerReceiveDataWorker *dataWorker = new (std::nothrow) TriggerReceiveDataWorker();
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
    dataWorker->ref = triggerCompleteInfo_.ref;
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

NativeValue* JsWantAgent::Equal(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsWantAgent *me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnEqual(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::GetWant(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsWantAgent *me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnGetWant(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::GetOperationType(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsWantAgent *me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnGetOperationType(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::GetBundleName(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsWantAgent *me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnGetBundleName(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::GetUid(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsWantAgent *me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnGetUid(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::Cancel(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsWantAgent *me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnCancel(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::Trigger(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsWantAgent *me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnTrigger(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::NapiGetWant(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsWantAgent *me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnNapiGetWant(*engine, *info) : nullptr;
};

NativeValue* JsWantAgent::NapiTrigger(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsWantAgent *me = CheckParamsAndGetThis<JsWantAgent>(engine, info);
    return (me != nullptr) ? me->OnNapiTrigger(*engine, *info) : nullptr;
};


NativeValue* JsWantAgent::OnEqual(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("enter, argc = %{public}d", static_cast<int32_t>(info.argc));
    auto env = reinterpret_cast<napi_env>(&engine);
    WantAgent *pWantAgentFirst = nullptr;
    WantAgent *pWantAgentSecond = nullptr;
    if (info.argc < ARGC_TWO || info.argc > ARGC_THREE) {
        HILOG_ERROR("Not enough params");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    napi_unwrap(env, reinterpret_cast<napi_value>(info.argv[0]), (void **)&(pWantAgentFirst));
    if (pWantAgentFirst == nullptr) {
        HILOG_ERROR("Parse pWantAgentFirst failed");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    if (info.argv[1]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    napi_unwrap(env, reinterpret_cast<napi_value>(info.argv[1]), (void **)&(pWantAgentSecond));
    if (pWantAgentSecond == nullptr) {
        HILOG_ERROR("Parse pWantAgentSceond failed");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    std::shared_ptr<WantAgent> wantAgentFirst = std::make_shared<WantAgent>(*pWantAgentFirst);
    std::shared_ptr<WantAgent> wantAgentSecond = std::make_shared<WantAgent>(*pWantAgentSecond);
    AsyncTask::CompleteCallback complete =
        [wantAgentFirst, wantAgentSecond](NativeEngine &engine, AsyncTask &task, int32_t status) {
            HILOG_DEBUG("OnEqual AsyncTask is called");
            bool ret = WantAgentHelper::JudgeEquality(wantAgentFirst, wantAgentSecond);
            task.Resolve(engine, CreateJsValue(engine, ret));
        };
    NativeValue *lastParam = (info.argc >= ARGC_THREE) ? info.argv[INDEX_TWO] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnEqual",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnGetWant(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    auto env = reinterpret_cast<napi_env>(&engine);
    WantAgent *pWantAgent = nullptr;
    if (info.argc > ARGC_TWO || info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    napi_unwrap(env, reinterpret_cast<napi_value>(info.argv[0]), (void **)&(pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
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
    NativeValue *lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnGetWant",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnGetOperationType(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("JsWantAgent::OnGetOperationType enter, argc = %{public}d", static_cast<int32_t>(info.argc));
    auto env = reinterpret_cast<napi_env>(&engine);
    WantAgent *pWantAgent = nullptr;
    if (info.argc > ARGC_TWO || info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    napi_unwrap(env, reinterpret_cast<napi_value>(info.argv[0]), (void **)&(pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    AsyncTask::CompleteCallback complete = [wantAgent](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnGetOperationType AsyncTask is called");
        auto ret = WantAgentHelper::GetType(wantAgent);
        task.Resolve(engine, CreateJsValue(engine, ret));
    };
    NativeValue *lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnGetOperationType",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnGetBundleName(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("JsWantAgent::OnGetBundleName enter, argc = %{public}d", static_cast<int32_t>(info.argc));
    auto env = reinterpret_cast<napi_env>(&engine);
    WantAgent *pWantAgent = nullptr;
    if (info.argc > ARGC_TWO || info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    napi_unwrap(env, reinterpret_cast<napi_value>(info.argv[0]), (void **)&(pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    AsyncTask::CompleteCallback complete = [wantAgent](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnGetBundleName AsyncTask is called");
        auto ret = WantAgentHelper::GetBundleName(wantAgent);
        task.Resolve(engine, CreateJsValue(engine, ret));
    };
    NativeValue *lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnGetBundleName",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnGetUid(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("JsWantAgent::OnGetUid enter, argc = %{public}d", static_cast<int32_t>(info.argc));
    auto env = reinterpret_cast<napi_env>(&engine);
    WantAgent *pWantAgent = nullptr;
    if (info.argc > ARGC_TWO || info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    napi_unwrap(env, reinterpret_cast<napi_value>(info.argv[0]), (void **)&(pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    AsyncTask::CompleteCallback complete = [wantAgent](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnGetUid AsyncTask is called");
        auto ret = WantAgentHelper::GetUid(wantAgent);
        task.Resolve(engine, CreateJsValue(engine, ret));
    };
    NativeValue *lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnGetUid",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnCancel(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s is called", __FUNCTION__);
    auto env = reinterpret_cast<napi_env>(&engine);
    WantAgent *pWantAgent = nullptr;
    if (info.argc > ARGC_TWO || info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    napi_unwrap(env, reinterpret_cast<napi_value>(info.argv[0]), (void **)&(pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("Parse pWantAgent failed");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    AsyncTask::CompleteCallback complete =
        [wantAgent](NativeEngine &engine, AsyncTask &task, int32_t status) {
            HILOG_DEBUG("OnCancel AsyncTask is called");
            WantAgentHelper::Cancel(wantAgent);
            task.Resolve(engine, engine.CreateUndefined());
        };

    NativeValue* lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWantAgent::OnCancel",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWantAgent::OnTrigger(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s is called", __FUNCTION__);
    auto env = reinterpret_cast<napi_env>(&engine);
    if (info.argc != ARGC_THREE) {
        HILOG_ERROR("Not enough params");
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
    }

    std::shared_ptr<WantAgent> wantAgent = nullptr;
    TriggerInfo triggerInfo;
    auto triggerObj = std::make_shared<TriggerCompleteCallBack>();
    int32_t errCode = UnWrapTriggerInfoParam(engine, info, wantAgent, triggerInfo, triggerObj);
    if (errCode != BUSINESS_ERROR_CODE_OK) {
        return reinterpret_cast<NativeValue*>(JSParaError(env, false));
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
    auto env = reinterpret_cast<napi_env>(&engine);

    if (info.argv[ARGC_ZERO]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        return ERR_NOT_OK;
    }
    WantAgent *pWantAgent = nullptr;
    napi_unwrap(env, reinterpret_cast<napi_value>(info.argv[ARGC_ZERO]), (void **)&(pWantAgent));

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

    napi_ref callback[2] = {0};
    napi_create_reference(env, reinterpret_cast<napi_value>(info.argv[ARGC_TWO]), 1, &callback[0]);
    triggerObj->SetCallbackInfo(env, callback[0]);
    triggerObj->SetWantAgentInstance(wantAgent);

    return BUSINESS_ERROR_CODE_OK;
}

int32_t JsWantAgent::GetTriggerInfo(NativeEngine &engine, NativeValue *param, TriggerInfo &triggerInfo)
{
    if (param->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("param type mismatch!");
        return ERR_NOT_OK;
    }

    NativeObject *objectParam = ConvertNativeValueTo<NativeObject>(param);

    int32_t code = -1;
    NativeValue *jsCode = objectParam->GetProperty("code");
    if (!ConvertFromJsValue(engine, jsCode, code)) {
        return ERR_NOT_OK;
    }

    NativeValue *jsWant = objectParam->GetProperty("want");
    std::shared_ptr<AAFwk::Want> want = nullptr;
    if (jsWant != nullptr) {
        want = std::make_shared<AAFwk::Want>();
        if (!UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(jsWant), *want)) {
            return ERR_NOT_OK;
        }
    }

    std::string permission = "";
    NativeValue *jsPermission = objectParam->GetProperty("permission");
    if (!ConvertFromJsValue(engine, jsPermission, permission)) {
        return ERR_NOT_OK;
    }

    NativeValue *jsExtraInfo = objectParam->GetProperty("extraInfo");
    std::shared_ptr<AAFwk::WantParams> extraInfo = nullptr;
    if (jsExtraInfo != nullptr) {
        extraInfo = std::make_shared<AAFwk::WantParams>();
        if (!UnwrapWantParams(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(jsExtraInfo),
            *extraInfo)) {
            return ERR_NOT_OK;
        }
    }

    TriggerInfo triggerInfoData(permission, extraInfo, want, code);
    triggerInfo = triggerInfoData;
    return BUSINESS_ERROR_CODE_OK;
}

NativeValue* JsWantAgent::OnNapiGetWant(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    auto env = reinterpret_cast<napi_env>(&engine);
    WantAgent *pWantAgent = nullptr;
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

    napi_unwrap(env, reinterpret_cast<napi_value>(info.argv[0]), (void **)&(pWantAgent));
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
    NativeValue *lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
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

    WantAgentHelper::TriggerWantAgent(wantAgent, triggerObj, triggerInfo);
    return engine.CreateNull();
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

    HILOG_DEBUG("JsWantAgentInit BindNativeFunction called");
    const char *moduleName = "JsWantAgent";
    BindNativeFunction(*engine, *object, "equal", moduleName, JsWantAgent::Equal);
    BindNativeFunction(*engine, *object, "getWant", moduleName, JsWantAgent::GetWant);
    BindNativeFunction(*engine, *object, "getoperationtype", moduleName, JsWantAgent::GetOperationType);
    BindNativeFunction(*engine, *object, "getBundleName", moduleName, JsWantAgent::GetBundleName);
    BindNativeFunction(*engine, *object, "getUid", moduleName, JsWantAgent::GetUid);
    BindNativeFunction(*engine, *object, "cancel", moduleName, JsWantAgent::Cancel);
    BindNativeFunction(*engine, *object, "trigger", moduleName, JsWantAgent::Trigger);
    HILOG_DEBUG("JsWantAgentInit end");
    return exportObj;
}

napi_value WantAgentInit(napi_env env, napi_value exports)
{
    HILOG_INFO("napi_moudule Init start...");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getWantAgent", NAPI_GetWantAgent),
        DECLARE_NAPI_FUNCTION("getOperationType", NAPI_GetOperationType),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    HILOG_INFO("napi_moudule Init end...");
    return reinterpret_cast<napi_value>(JsWantAgentInit(reinterpret_cast<NativeEngine*>(env),
        reinterpret_cast<NativeValue*>(exports)));
}

void SetNamedPropertyByInteger(napi_env env, napi_value dstObj, int32_t objName, const std::string &propName)
{
    napi_value prop = nullptr;
    if (napi_create_int32(env, objName, &prop) == napi_ok) {
        napi_set_named_property(env, dstObj, propName.c_str(), prop);
    }
}

napi_value WantAgentFlagsInit(napi_env env, napi_value exports)
{
    HILOG_INFO("%{public}s, called", __func__);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_ZERO, "ONE_TIME_FLAG");
    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_ONE, "NO_BUILD_FLAG");
    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_TWO, "CANCEL_PRESENT_FLAG");
    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_THREE, "UPDATE_PRESENT_FLAG");
    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_FOUR, "CONSTANT_FLAG");
    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_FIVE, "REPLACE_ELEMENT");
    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_SIX, "REPLACE_ACTION");
    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_SEVEN, "REPLACE_URI");
    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_EIGHT, "REPLACE_ENTITIES");
    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_NINE, "REPLACE_BUNDLE");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("WantAgentFlags", obj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

napi_value WantAgentOperationTypeInit(napi_env env, napi_value exports)
{
    HILOG_INFO("%{public}s, called", __func__);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_ZERO, "UNKNOWN_TYPE");
    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_ONE, "START_ABILITY");
    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_TWO, "START_ABILITIES");
    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_THREE, "START_SERVICE");
    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_FOUR, "SEND_COMMON_EVENT");
    SetNamedPropertyByInteger(env, obj, NUMBER_OF_PARAMETERS_FIVE, "START_FOREGROUND_SERVICE");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("OperationType", obj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

auto NAPI_GetWantAgentWrapExecuteCallBack = [](napi_env env, void *data) {
    HILOG_INFO("GetWantAgent called(CallBack Mode)...");
    AsyncGetWantAgentCallbackInfo *asyncCallbackInfo = static_cast<AsyncGetWantAgentCallbackInfo *>(data);
    WantAgentInfo wantAgentInfo(asyncCallbackInfo->requestCode,
        asyncCallbackInfo->operationType,
        asyncCallbackInfo->wantAgentFlags,
        asyncCallbackInfo->wants,
        asyncCallbackInfo->extraInfo);
    asyncCallbackInfo->wantAgent =
        WantAgentHelper::GetWantAgent(asyncCallbackInfo->context, wantAgentInfo);
    if (asyncCallbackInfo->wantAgent == nullptr) {
        HILOG_INFO("GetWantAgent instance is nullptr...");
    }
    int32_t code = WantAgentHelper::GetHashCode(asyncCallbackInfo->wantAgent);
    std::lock_guard<std::recursive_mutex> guard(g_mutex);
    g_WantAgentMap->emplace(asyncCallbackInfo, code);
};

auto NAPI_GetWantAgentWrapCompleteCallBack = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("GetWantAgent compeleted(CallBack Mode)...");
    AsyncGetWantAgentCallbackInfo *asyncCallbackInfo = static_cast<AsyncGetWantAgentCallbackInfo *>(data);
    napi_value result[NUMBER_OF_PARAMETERS_TWO] = {0};
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;

    result[0] = GetCallbackErrorResult(asyncCallbackInfo->env, BUSINESS_ERROR_CODE_OK);

    napi_value wantAgentClass = nullptr;
    napi_define_class(env,
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
    napi_new_instance(env, wantAgentClass, 0, nullptr, &result[1]);
    napi_wrap(env,
        result[1],
        static_cast<void *>(asyncCallbackInfo->wantAgent.get()),
        [](napi_env env, void *data, void *hint) {},
        nullptr,
        nullptr);
    napi_get_undefined(env, &undefined);
    napi_get_reference_value(env, asyncCallbackInfo->callback[0], &callback);
    napi_call_function(env, undefined, callback, NUMBER_OF_PARAMETERS_TWO, &result[0], &callResult);

    if (asyncCallbackInfo->callback[0] != nullptr) {
        napi_delete_reference(env, asyncCallbackInfo->callback[0]);
    }
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
};

auto NAPI_GetWantAgentWrapPromiseCompleteCallBack = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("GetWantAgent compeleted(Promise Mode)...");
    AsyncGetWantAgentCallbackInfo *asyncCallbackInfo = static_cast<AsyncGetWantAgentCallbackInfo *>(data);
    napi_value wantAgentClass = nullptr;
    napi_define_class(env,
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
    napi_wrap(env,
        result,
        static_cast<void *>(asyncCallbackInfo->wantAgent.get()),
        [](napi_env env, void *data, void *hint) {},
        nullptr,
        nullptr);
    napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
};

napi_value NAPI_GetWantAgentWrap(
    napi_env env, napi_callback_info info, bool callBackMode, AsyncGetWantAgentCallbackInfo &asyncCallbackInfo)
{
    HILOG_INFO("NAPI_GetWantAgentWrap called...");
    if (callBackMode) {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_GetWantAgentCallBack", NAPI_AUTO_LENGTH, &resourceName);

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_GetWantAgentWrapExecuteCallBack,
            NAPI_GetWantAgentWrapCompleteCallBack,
            static_cast<void *>(&asyncCallbackInfo),
            &asyncCallbackInfo.asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo.asyncWork));
        // create reutrn
        napi_value ret = nullptr;
        NAPI_CALL(env, napi_create_int32(env, 0, &ret));
        return ret;
    } else {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_GetWantAgentPromise", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo.deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_GetWantAgentWrapExecuteCallBack,
            NAPI_GetWantAgentWrapPromiseCompleteCallBack,
            static_cast<void *>(&asyncCallbackInfo),
            &asyncCallbackInfo.asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo.asyncWork);
        return promise;
    }
}

napi_value NAPI_GetWantAgentWants(napi_env env, napi_value jsWantAgentInfo, const WantAgentWantsParas &paras)
{
    napi_valuetype jsWantAgentInfoType = napi_valuetype::napi_null;
    NAPI_CALL(env, napi_typeof(env, jsWantAgentInfo, &jsWantAgentInfoType));
    if (jsWantAgentInfoType != napi_object) {
        HILOG_ERROR("param type mismatch!");
        return nullptr;
    }

    napi_value jsWants = GetPropertyValueByPropertyName(env, jsWantAgentInfo, "wants", napi_object);
    bool isArray = false;
    if (jsWants == nullptr || napi_is_array(env, jsWants, &isArray) != napi_ok || !isArray) {
        return nullptr;
    }

    uint32_t wantsLen = 0;
    napi_get_array_length(env, jsWants, &wantsLen);
    for (uint32_t i = 0; i < wantsLen; i++) {
        std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
        napi_value jsWant = nullptr;
        napi_get_element(env, jsWants, i, &jsWant);
        if (!UnwrapWant(env, jsWant, *want)) {
            return nullptr;
        }
        paras.wants.emplace_back(want);
    }

    // Get operationType
    if (!UnwrapInt32ByPropertyName(env, jsWantAgentInfo, "operationType", paras.operationType)) {
        return nullptr;
    }
    // Get requestCode
    if (!UnwrapInt32ByPropertyName(env, jsWantAgentInfo, "requestCode", paras.requestCode)) {
        return nullptr;
    }
    // Get wantAgentFlags
    napi_value JsWantAgentFlags = GetPropertyValueByPropertyName(env, jsWantAgentInfo, "wantAgentFlags", napi_object);
    if (JsWantAgentFlags != nullptr) {
        uint32_t arrayLength = 0;
        NAPI_CALL(env, napi_get_array_length(env, JsWantAgentFlags, &arrayLength));
        HILOG_INFO("property is array, length=%{public}d", arrayLength);
        for (uint32_t i = 0; i < arrayLength; i++) {
            napi_value napiWantAgentFlags = nullptr;
            napi_get_element(env, JsWantAgentFlags, i, &napiWantAgentFlags);
            napi_valuetype valuetype0 = napi_valuetype::napi_null;
            NAPI_CALL(env, napi_typeof(env, napiWantAgentFlags, &valuetype0));
            if (valuetype0 != napi_number) {
                HILOG_ERROR("Wrong argument type. Numbers expected.");
                return nullptr;
            }
            int32_t value0 = 0;
            NAPI_CALL(env, napi_get_value_int32(env, napiWantAgentFlags, &value0));
            paras.wantAgentFlags.emplace_back(static_cast<WantAgentConstant::Flags>(value0));
        }
    }
    // Get extraInfo
    napi_value JsExtraInfo = GetPropertyValueByPropertyName(env, jsWantAgentInfo, "extraInfo", napi_object);
    if (JsExtraInfo != nullptr) {
        if (!UnwrapWantParams(env, JsExtraInfo, paras.extraInfo)) {
            return nullptr;
        }
    }
    return NapiGetNull(env);
}

napi_value NAPI_GetWantAgent(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_OF_PARAMETERS_TWO;
    napi_value argv[NUMBER_OF_PARAMETERS_TWO] = {};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);
    napi_value jsWantAgentInfo = argv[0];

    // Get wants
    std::vector<std::shared_ptr<AAFwk::Want>> wants = {};
    int32_t operationType = -1;
    int32_t requestCode = -1;
    std::vector<WantAgentConstant::Flags> wantAgentFlags = {};
    AAFwk::WantParams extraInfo = {};
    WantAgentWantsParas paras = {
        .wants = wants,
        .operationType = operationType,
        .requestCode = requestCode,
        .wantAgentFlags = wantAgentFlags,
        .extraInfo = extraInfo,
    };
    napi_value ret = NAPI_GetWantAgentWants(env, jsWantAgentInfo, paras);
    if (ret == nullptr) {
        HILOG_ERROR("Failed to Get wantAgent wants.");
        return JSParaError(env, false);
    }

    bool callBackMode = false;
    if (argc >= NUMBER_OF_PARAMETERS_TWO) {
        napi_valuetype valuetype;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        if (valuetype != napi_function) {
            HILOG_ERROR("Wrong argument type. Function expected.");
            return JSParaError(env, false);
        }
        callBackMode = true;
    }

    AsyncGetWantAgentCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncGetWantAgentCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("Failed to create object.");
        return JSParaError(env, callBackMode);
    }
    asyncCallbackInfo->wants = wants;
    asyncCallbackInfo->operationType =
        static_cast<WantAgentConstant::OperationType>(operationType);
    asyncCallbackInfo->requestCode = requestCode;
    asyncCallbackInfo->wantAgentFlags = wantAgentFlags;
    asyncCallbackInfo->extraInfo.reset(new (std::nothrow) AAFwk::WantParams(extraInfo));
    asyncCallbackInfo->context = OHOS::AbilityRuntime::Context::GetApplicationContext();

    if (callBackMode) {
        napi_create_reference(env, argv[1], 1, &asyncCallbackInfo->callback[0]);
    }
    ret = NAPI_GetWantAgentWrap(env, info, callBackMode, *asyncCallbackInfo);
    if (ret == nullptr) {
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
    }

    return ((callBackMode) ? (NapiGetNull(env)) : (ret));
}

napi_value GetCallbackErrorResult(napi_env env, int errCode)
{
    napi_value result = nullptr;
    napi_value eCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", eCode));
    return result;
}

napi_value NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

auto NAPI_GetOperationTypeWrapExecuteCallBack = [](napi_env env, void *data) {
    HILOG_INFO("GetOperationType called...");
    AsyncGetOperationTypeCallbackInfo *asyncCallbackInfo = static_cast<AsyncGetOperationTypeCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("asyncCallbackInfo is nullptr.");
        return;
    }
    if (asyncCallbackInfo->newInterface) {
        asyncCallbackInfo->errorCode = WantAgentHelper::GetType(
            asyncCallbackInfo->wantAgent, asyncCallbackInfo->operationType);
    } else {
        asyncCallbackInfo->operationType = static_cast<int32_t>(
            WantAgentHelper::GetType(asyncCallbackInfo->wantAgent));
    }
};

auto NAPI_GetOperationTypeWrapCompleteCallBack = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("GetOperationType completed(CallBack Mode)...");
    AsyncGetOperationTypeCallbackInfo *asyncCallbackInfo = static_cast<AsyncGetOperationTypeCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("asyncCallbackInfo is nullptr.");
        return;
    }
    napi_value result[NUMBER_OF_PARAMETERS_TWO] = {0};
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;

    if (asyncCallbackInfo->newInterface) {
        result[0] = CreateErrorValue(asyncCallbackInfo->env, asyncCallbackInfo->errorCode);
    } else {
        result[0] = GetCallbackErrorResult(asyncCallbackInfo->env, BUSINESS_ERROR_CODE_OK);
    }
    napi_create_int32(env, asyncCallbackInfo->operationType, &result[1]);
    napi_get_undefined(env, &undefined);
    napi_get_reference_value(env, asyncCallbackInfo->callback[0], &callback);
    napi_call_function(env, undefined, callback, NUMBER_OF_PARAMETERS_TWO, &result[0], &callResult);

    if (asyncCallbackInfo->callback[0] != nullptr) {
        napi_delete_reference(env, asyncCallbackInfo->callback[0]);
    }
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
};

auto NAPI_GetOperationTypeWrapPromiseCompleteCallBack = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("GetOperationType completed(promise Mode)...");
    AsyncGetOperationTypeCallbackInfo *asyncCallbackInfo = static_cast<AsyncGetOperationTypeCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("asyncCallbackInfo is nullptr.");
        return;
    }

    napi_value result = nullptr;
    napi_create_int32(env, asyncCallbackInfo->operationType, &result);
    if (asyncCallbackInfo->newInterface && asyncCallbackInfo->errorCode != NO_ERROR) {
        napi_reject_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred,
            CreateErrorValue(asyncCallbackInfo->env, asyncCallbackInfo->errorCode));
    } else {
        napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
    }

    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
};

napi_value NAPI_GetOperationTypeWrap(
    napi_env env, napi_callback_info info, bool callBackMode, AsyncGetOperationTypeCallbackInfo &asyncCallbackInfo)
{
    HILOG_INFO("NAPI_GetOperationTypeWrap called...");
    if (callBackMode) {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_GetOperationTypeCallBack", NAPI_AUTO_LENGTH, &resourceName);

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_GetOperationTypeWrapExecuteCallBack,
            NAPI_GetOperationTypeWrapCompleteCallBack,
            static_cast<void *>(&asyncCallbackInfo),
            &asyncCallbackInfo.asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo.asyncWork));
        // create return
        napi_value ret = nullptr;
        NAPI_CALL(env, napi_create_int32(env, 0, &ret));
        return ret;
    } else {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_GetOperationTypePromise", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo.deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_GetOperationTypeWrapExecuteCallBack,
            NAPI_GetOperationTypeWrapPromiseCompleteCallBack,
            static_cast<void *>(&asyncCallbackInfo),
            &asyncCallbackInfo.asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo.asyncWork);
        return promise;
    }
}

napi_value NAPI_GetOperationType(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_OF_PARAMETERS_TWO;
    napi_value argv[NUMBER_OF_PARAMETERS_TWO] = {};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    napi_valuetype wantAgentType = napi_valuetype::napi_null;
    napi_typeof(env, argv[0], &wantAgentType);
    if (wantAgentType != napi_object) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        return JSParaError(env, false);
    }

    WantAgent *pWantAgent = nullptr;
    napi_unwrap(env, argv[0], (void **)&(pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("WantAgent napi_unwrap error");
        return JSParaError(env, false);
    }

    bool callBackMode = false;
    if (argc >= NUMBER_OF_PARAMETERS_TWO) {
        napi_valuetype valuetype = napi_valuetype::napi_null;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        if (valuetype != napi_function) {
            HILOG_ERROR("Wrong argument type. Function expected.");
            return JSParaError(env, false);
        }
        callBackMode = true;
    }
    AsyncGetOperationTypeCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncGetOperationTypeCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("Failed to create object.");
        return JSParaError(env, callBackMode);
    }
    asyncCallbackInfo->wantAgent = std::make_shared<WantAgent>(*pWantAgent);

    if (callBackMode) {
        napi_create_reference(env, argv[1], 1, &asyncCallbackInfo->callback[0]);
    }
    napi_value ret = NAPI_GetOperationTypeWrap(env, info, callBackMode, *asyncCallbackInfo);
    if (ret == nullptr) {
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
    }
    return ((callBackMode) ? (NapiGetNull(env)) : (ret));
}

napi_value GetOperationType(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_OF_PARAMETERS_TWO;
    napi_value argv[NUMBER_OF_PARAMETERS_TWO] = {};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    napi_valuetype wantAgentType = napi_valuetype::napi_null;
    napi_typeof(env, argv[0], &wantAgentType);
    if (wantAgentType != napi_object) {
        HILOG_ERROR("Wrong argument type. Object expected.");
        NapiThrow(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return JSParaError(env, false);
    }

    WantAgent *pWantAgent = nullptr;
    napi_unwrap(env, argv[0], (void **)&(pWantAgent));
    if (pWantAgent == nullptr) {
        HILOG_ERROR("WantAgent napi_unwrap error");
        NapiThrow(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return JSParaError(env, false);
    }

    bool callBackMode = false;
    if (argc >= NUMBER_OF_PARAMETERS_TWO) {
        napi_valuetype valuetype = napi_valuetype::napi_null;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        if (valuetype != napi_function) {
            HILOG_ERROR("Wrong argument type. Function expected.");
            NapiThrow(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
            return JSParaError(env, false);
        }
        callBackMode = true;
    }
    AsyncGetOperationTypeCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncGetOperationTypeCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
        .newInterface = true,
    };
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("Failed to create object.");
        return JSParaError(env, callBackMode);
    }
    asyncCallbackInfo->wantAgent = std::make_shared<WantAgent>(*pWantAgent);

    if (callBackMode) {
        napi_create_reference(env, argv[1], 1, &asyncCallbackInfo->callback[0]);
    }
    napi_value ret = NAPI_GetOperationTypeWrap(env, info, callBackMode, *asyncCallbackInfo);
    if (ret == nullptr) {
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
    }
    return ((callBackMode) ? (NapiGetNull(env)) : (ret));
}
}  // namespace OHOS
