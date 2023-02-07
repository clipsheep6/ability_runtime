/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ability_runtime/js_ability_context.h"

#include <cstdint>

#include "hitrace_meter.h"
#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "ability_runtime/js_caller_complex.h"
#include "napi_common_ability.h"
#include "napi_common_start_options.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "start_options.h"
#include "want.h"
#include "event_handler.h"
#include "hitrace_meter.h"

#ifdef SUPPORT_GRAPHICS
#include "pixel_map_napi.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
constexpr int32_t INDEX_TWO = 2;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr int32_t ERROR_CODE_ONE = 1;

class StartAbilityByCallParameters {
public:
    int err = 0;
    sptr<IRemoteObject> remoteCallee = nullptr;
    std::shared_ptr<CallerCallBack> callerCallBack = nullptr;
    std::mutex mutexlock;
    std::condition_variable condition;
};

void JsAbilityContext::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_INFO("JsAbilityContext::Finalizer is called");
    std::unique_ptr<JsAbilityContext>(static_cast<JsAbilityContext*>(data));
}

NativeValue* JsAbilityContext::StartAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbility(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::StartRecentAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbility(*engine, *info, true) : nullptr;
}

NativeValue* JsAbilityContext::StartAbilityWithAccount(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbilityWithAccount(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::StartAbilityByCall(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbilityByCall(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::StartAbilityForResult(NativeEngine* engine, NativeCallbackInfo* info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbilityForResult(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::StartAbilityForResultWithAccount(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbilityForResultWithAccount(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::StartServiceExtensionAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartExtensionAbility(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::StartServiceExtensionAbilityWithAccount(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartExtensionAbilityWithAccount(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::StopServiceExtensionAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStopExtensionAbility(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::StopServiceExtensionAbilityWithAccount(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStopExtensionAbilityWithAccount(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::ConnectAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnConnectAbility(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::ConnectAbilityWithAccount(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnConnectAbilityWithAccount(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::DisconnectAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnDisconnectAbility(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::TerminateSelf(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelf(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::TerminateSelfWithResult(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelfWithResult(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::RestoreWindowStage(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnRestoreWindowStage(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::IsTerminating(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnIsTerminating(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info, bool isStartRecent)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("OnStartAbility is called.");

    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[0]), want);
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = 1;
    HILOG_INFO("Start ability, ability name is %{public}s.", want.GetElement().GetAbilityName().c_str());
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && info.argv[1]->TypeOf() == NATIVE_OBJECT) {
        HILOG_INFO("OnStartAbility start options is used.");
        AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[1]), startOptions);
        unwrapArgc++;
    }

    if (isStartRecent) {
        HILOG_INFO("OnStartRecentAbility is called");
        want.SetParam(Want::PARAM_RESV_START_RECENT, true);
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_, want, startOptions, unwrapArgc](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto innerErrorCode = (unwrapArgc == 1) ?
                context->StartAbility(want, -1) : context->StartAbility(want, startOptions, -1);
            if (innerErrorCode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
            }
        };

    NativeValue* lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnStartAbility",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::OnStartAbilityWithAccount(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnStartAbilityWithAccount is called");
    if (info.argc < ARGC_TWO) {
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }
    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[0]), want);
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = 1;
    HILOG_INFO("abilityName=%{public}s", want.GetElement().GetAbilityName().c_str());
    int32_t accountId = 0;
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[1]), accountId)) {
        HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    unwrapArgc++;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_TWO && info.argv[INDEX_TWO]->TypeOf() == NATIVE_OBJECT) {
        HILOG_INFO("OnStartAbilityWithAccount start options is used.");
        AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[ARGC_TWO]), startOptions);
        unwrapArgc++;
    }
    AsyncTask::CompleteCallback complete =
        [weak = context_, want, accountId, startOptions, unwrapArgc](
            NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }

                auto innerErrorCode = (unwrapArgc == INDEX_TWO) ?
                    context->StartAbilityWithAccount(want, accountId, -1) : context->StartAbilityWithAccount(
                        want, accountId, startOptions, -1);
                if (innerErrorCode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
                }
        };

    NativeValue* lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnStartAbilityWithAccount",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::OnStartAbilityByCall(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("JsAbilityContext::%{public}s, called", __func__);
    if (info.argc < ARGC_ONE) {
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    AAFwk::Want want;
    if (info.argv[0]->TypeOf() != NATIVE_OBJECT ||
        !JsAbilityContext::UnWrapWant(engine, info.argv[0], want)) {
        HILOG_ERROR("Failed to parse want!");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    InheritWindowMode(want);

    std::shared_ptr<StartAbilityByCallParameters> calls = std::make_shared<StartAbilityByCallParameters>();
    NativeValue* lastParam = ((info.argc == ARGC_TWO) ? info.argv[ARGC_ONE] : nullptr);
    NativeValue* retsult = nullptr;

    calls->callerCallBack = std::make_shared<CallerCallBack>();

    auto callBackDone = [calldata = calls] (const sptr<IRemoteObject> &obj) {
        HILOG_DEBUG("OnStartAbilityByCall callBackDone mutexlock");
        std::unique_lock<std::mutex> lock(calldata->mutexlock);
        HILOG_DEBUG("OnStartAbilityByCall callBackDone remoteCallee assignment");
        calldata->remoteCallee = obj;
        calldata->condition.notify_all();
        HILOG_INFO("OnStartAbilityByCall callBackDone is called end");
    };

    auto releaseListen = [](const std::string &str) {
        HILOG_INFO("OnStartAbilityByCall releaseListen is called %{public}s", str.c_str());
    };

    auto callExecute = [calldata = calls] () {
        constexpr int CALLER_TIME_OUT = 10; // 10s
        std::unique_lock<std::mutex> lock(calldata->mutexlock);
        if (calldata->remoteCallee != nullptr) {
            HILOG_INFO("OnStartAbilityByCall callExecute callee isn`t nullptr");
            return;
        }

        if (calldata->condition.wait_for(lock, std::chrono::seconds(CALLER_TIME_OUT)) == std::cv_status::timeout) {
            HILOG_ERROR("OnStartAbilityByCall callExecute waiting callee timeout");
            calldata->err = -1;
        }
        HILOG_DEBUG("OnStartAbilityByCall callExecute end");
    };

    auto callComplete = [weak = context_, calldata = calls] (
        NativeEngine& engine, AsyncTask& task, int32_t status) {
        if (calldata->err != 0) {
            HILOG_ERROR("OnStartAbilityByCall callComplete err is %{public}d", calldata->err);
            task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }

        auto context = weak.lock();
        if (context != nullptr && calldata->callerCallBack != nullptr && calldata->remoteCallee != nullptr) {
            auto releaseCallAbilityFunc = [weak] (
                const std::shared_ptr<CallerCallBack> &callback) -> ErrCode {
                auto contextForRelease = weak.lock();
                if (contextForRelease == nullptr) {
                    HILOG_ERROR("releaseCallAbilityFunction, context is nullptr");
                    return -1;
                }
                return contextForRelease->ReleaseCall(callback);
            };
            task.Resolve(engine,
                CreateJsCallerComplex(
                    engine, releaseCallAbilityFunc, calldata->remoteCallee, calldata->callerCallBack));
        } else {
            HILOG_ERROR("OnStartAbilityByCall callComplete params error %{public}s is nullptr",
                context == nullptr ? "context" :
                    (calldata->remoteCallee == nullptr ? "remoteCallee" : "callerCallBack"));
            task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
        }

        HILOG_DEBUG("OnStartAbilityByCall callComplete end");
    };

    calls->callerCallBack->SetCallBack(callBackDone);
    calls->callerCallBack->SetOnRelease(releaseListen);

    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_ERROR("OnStartAbilityByCall context is nullptr");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return engine.CreateUndefined();
    }

    auto ret = context->StartAbilityByCall(want, calls->callerCallBack);
    if (ret != 0) {
        HILOG_ERROR("OnStartAbilityByCall StartAbility is failed");
        ThrowErrorByNativeErr(engine, ret);
        return engine.CreateUndefined();
    }

    if (calls->remoteCallee == nullptr) {
        HILOG_INFO("OnStartAbilityByCall async wait execute");
        AsyncTask::Schedule("JsAbilityContext::OnStartAbilityByCall",
            engine,
            CreateAsyncTaskWithLastParam(
                engine, lastParam, std::move(callExecute), std::move(callComplete), &retsult));
    } else {
        HILOG_INFO("OnStartAbilityByCall promiss return result execute");
        AsyncTask::Schedule("JsAbilityContext::OnStartAbilityByCall", engine,
            CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(callComplete), &retsult));
    }

    HILOG_DEBUG("JsAbilityContext::%{public}s, called end", __func__);
    return retsult;
}

NativeValue* JsAbilityContext::OnStartAbilityForResult(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnStartAbilityForResult is called");

    if (info.argc == ARGC_ZERO) {
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    AAFwk::Want want;
    if (!JsAbilityContext::UnWrapWant(engine, info.argv[0], want)) {
        HILOG_ERROR("%s Failed to parse want!", __func__);
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = 1;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && info.argv[1]->TypeOf() == NATIVE_OBJECT) {
        HILOG_INFO("OnStartAbilityForResult start options is used.");
        AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[1]), startOptions);
        unwrapArgc++;
    }

    NativeValue* lastParam = info.argc > unwrapArgc ? info.argv[unwrapArgc] : nullptr;
    NativeValue* result = nullptr;
    std::unique_ptr<AsyncTask> uasyncTask =
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<AsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [&engine, asyncTask](int resultCode, const AAFwk::Want& want) {
        HILOG_INFO("OnStartAbilityForResult async callback is called");
        NativeValue* abilityResult = JsAbilityContext::WrapAbilityResult(engine, resultCode, want);
        if (abilityResult == nullptr) {
            HILOG_WARN("wrap abilityResult failed");
            asyncTask->Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
        } else {
            asyncTask->Resolve(engine, abilityResult);
        }
    };
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_WARN("context is released");
        asyncTask->Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    } else {
        want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
        curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
        (unwrapArgc == 1) ? context->StartAbilityForResult(want, curRequestCode_, std::move(task)) :
            context->StartAbilityForResult(want, startOptions, curRequestCode_, std::move(task));
    }
    HILOG_INFO("OnStartAbilityForResult is called end");
    return result;
}

NativeValue* JsAbilityContext::OnStartAbilityForResultWithAccount(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnStartAbilityForResultWithAccount is called");
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }
    AAFwk::Want want;
    if (!JsAbilityContext::UnWrapWant(engine, info.argv[0], want)) {
        HILOG_ERROR("%s Failed to parse want!", __func__);
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = 1;
    int32_t accountId = 0;
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[1]), accountId)) {
        HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    unwrapArgc++;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_TWO && info.argv[INDEX_TWO]->TypeOf() == NATIVE_OBJECT) {
        HILOG_INFO("OnStartAbilityForResultWithAccount start options is used.");
        AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_TWO]), startOptions);
        unwrapArgc++;
    }
    NativeValue* lastParam = info.argc > unwrapArgc ? info.argv[unwrapArgc] : nullptr;
    NativeValue* result = nullptr;
    std::unique_ptr<AsyncTask> uasyncTask =
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<AsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [&engine, asyncTask](int resultCode, const AAFwk::Want& want) {
        HILOG_INFO("OnStartAbilityForResultWithAccount async callback is called");
        NativeValue* abilityResult = JsAbilityContext::WrapAbilityResult(engine, resultCode, want);
        if (abilityResult == nullptr) {
            HILOG_WARN("wrap abilityResult failed");
            asyncTask->Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
        } else {
            asyncTask->Resolve(engine, abilityResult);
        }
        HILOG_INFO("OnStartAbilityForResultWithAccount async callback is called end");
    };
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_WARN("context is released");
        asyncTask->Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    } else {
        curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
        (unwrapArgc == INDEX_TWO) ? context->StartAbilityForResultWithAccount(
            want, accountId, curRequestCode_, std::move(task)) : context->StartAbilityForResultWithAccount(
                want, accountId, startOptions, curRequestCode_, std::move(task));
    }
    HILOG_INFO("OnStartAbilityForResultWithAccount is called end");
    return result;
}

NativeValue* JsAbilityContext::OnStartExtensionAbility(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnStartExtensionAbility is called.");
    if (info.argc < ARGC_ONE) {
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    AAFwk::Want want;
    if (!JsAbilityContext::UnWrapWant(engine, info.argv[0], want)) {
        HILOG_ERROR("Failed to parse want!");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_, want](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto errcode = context->StartServiceExtensionAbility(want);
            if (errcode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, errcode));
            }
        };

    NativeValue* lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnStartExtensionAbility",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::OnStartExtensionAbilityWithAccount(NativeEngine& engine, const NativeCallbackInfo& info)
{
    HILOG_INFO("OnStartExtensionAbilityWithAccount is called.");
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("param is too few.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    AAFwk::Want want;
    int32_t accountId = -1;
    if (!JsAbilityContext::UnWrapWant(engine, info.argv[0], want) ||
        !OHOS::AppExecFwk::UnwrapInt32FromJS2(reinterpret_cast<napi_env>(&engine),
                                              reinterpret_cast<napi_value>(info.argv[1]), accountId)) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_, want, accountId](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto errcode = context->StartServiceExtensionAbility(want, accountId);
            if (errcode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, errcode));
            }
        };

    NativeValue* lastParam = (info.argc > ARGC_TWO) ? info.argv[ARGC_TWO] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnStartExtensionAbilityWithAccount",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::OnStopExtensionAbility(NativeEngine& engine, const NativeCallbackInfo& info)
{
    HILOG_INFO("OnStopExtensionAbility is called.");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("param is too few for stop extension ability.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    AAFwk::Want want;
    if (!JsAbilityContext::UnWrapWant(engine, info.argv[0], want)) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_, want](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto errcode = context->StopServiceExtensionAbility(want);
            if (errcode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, errcode));
            }
        };

    NativeValue* lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnStopExtensionAbility",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::OnStopExtensionAbilityWithAccount(NativeEngine& engine, const NativeCallbackInfo& info)
{
    HILOG_INFO("OnStartExtensionAbilityWithAccount is called.");
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("param is too few for stop extension ability with account.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    int32_t accountId = -1;
    AAFwk::Want want;
    if (!JsAbilityContext::UnWrapWant(engine, info.argv[0], want) ||
        !OHOS::AppExecFwk::UnwrapInt32FromJS2(reinterpret_cast<napi_env>(&engine),
                                              reinterpret_cast<napi_value>(info.argv[1]), accountId)) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_, want, accountId](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto errcode = context->StopServiceExtensionAbility(want, accountId);
            if (errcode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, errcode));
            }
        };

    NativeValue* lastParam = (info.argc > ARGC_TWO) ? info.argv[ARGC_TWO] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnStopExtensionAbilityWithAccount",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::OnTerminateSelfWithResult(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnTerminateSelfWithResult is called");

    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    int resultCode = 0;
    AAFwk::Want want;
    if (!JsAbilityContext::UnWrapAbilityResult(engine, info.argv[0], resultCode, want)) {
        HILOG_ERROR("%s Failed to parse ability result!", __func__);
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    auto abilityContext = context_.lock();
    if (abilityContext != nullptr) {
        abilityContext->SetTerminating(true);
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_, want, resultCode](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            auto errorCode = context->TerminateAbilityWithResult(want, resultCode);
            if (errorCode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, errorCode));
            }
        };

    NativeValue* lastParam = (info.argc > ARGC_ONE) ? info.argv[1] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnTerminateSelfWithResult",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    HILOG_INFO("OnTerminateSelfWithResult is called end");
    return result;
}

NativeValue* JsAbilityContext::OnConnectAbility(NativeEngine& engine, NativeCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Connect ability called.");
    // only support two params
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("Connect ability failed, not enough params.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    // unwrap want
    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[0]), want);
    HILOG_INFO("Connect ability called, callee:%{public}s.%{public}s.",
        want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());

    // unwarp connection
    sptr<JSAbilityConnection> connection = new JSAbilityConnection(engine);
    connection->SetJsConnectionObject(info.argv[1]);
    int64_t connectId = g_serialNumber;
    ConnectionKey key;
    key.id = g_serialNumber;
    key.want = want;
    connection->SetConnectionId(key.id);
    abilityConnects_.emplace(key, connection);
    if (g_serialNumber < INT32_MAX) {
        g_serialNumber++;
    } else {
        g_serialNumber = 0;
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_, want, connection, connectId](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_ERROR("Connect ability failed, context is released.");
                task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                return;
            }
            if (!context->ConnectAbility(want, connection)) {
                connection->CallJsFailed(ERROR_CODE_ONE);
            }
            task.Resolve(engine, engine.CreateUndefined());
        };
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnConnectAbility",
        engine, CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, std::move(complete), &result));
    return engine.CreateNumber(connectId);
}

NativeValue* JsAbilityContext::OnConnectAbilityWithAccount(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnConnectAbility is called");
    // only support three params
    if (info.argc < ARGC_THREE) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    // unwrap want
    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[0]), want);
    HILOG_INFO("%{public}s bundlename:%{public}s abilityname:%{public}s",
        __func__,
        want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());

    int32_t accountId = 0;
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[1]), accountId)) {
        HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    // unwarp connection
    sptr<JSAbilityConnection> connection = new JSAbilityConnection(engine);
    connection->SetJsConnectionObject(info.argv[INDEX_TWO]);
    int64_t connectId = g_serialNumber;
    ConnectionKey key;
    key.id = g_serialNumber;
    key.want = want;
    connection->SetConnectionId(key.id);
    abilityConnects_.emplace(key, connection);
    if (g_serialNumber < INT32_MAX) {
        g_serialNumber++;
    } else {
        g_serialNumber = 0;
    }
    AsyncTask::CompleteCallback complete =
        [weak = context_, want, accountId, connection, connectId](
            NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_ERROR("context is released");
                    task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                    return;
                }
                HILOG_INFO("context->ConnectAbilityWithAccount connection:%{public}d", (int32_t)connectId);
                if (!context->ConnectAbilityWithAccount(want, accountId, connection)) {
                    connection->CallJsFailed(ERROR_CODE_ONE);
                }
                task.Resolve(engine, engine.CreateUndefined());
        };
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnConnectAbilityWithAccount",
        engine, CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, std::move(complete), &result));
    return engine.CreateNumber(connectId);
}

NativeValue* JsAbilityContext::OnDisconnectAbility(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnDisconnectAbility is called");
    // only support one or two params
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    // unwrap want
    AAFwk::Want want;
    // unwrap connectId
    int64_t connectId = -1;
    sptr<JSAbilityConnection> connection = nullptr;
    napi_get_value_int64(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[0]), &connectId);
    HILOG_INFO("Disconnect ability begin, connection:%{public}d.", (int32_t)connectId);
    auto item = std::find_if(abilityConnects_.begin(),
        abilityConnects_.end(),
        [&connectId](const std::map<ConnectionKey, sptr<JSAbilityConnection>>::value_type &obj) {
            return connectId == obj.first.id;
        });
    if (item != abilityConnects_.end()) {
        // match id
        want = item->first.want;
        connection = item->second;
        HILOG_INFO("%{public}s find conn ability exist", __func__);
    } else {
        HILOG_INFO("%{public}s not find conn exist.", __func__);
    }
    // begin disconnect
    AsyncTask::CompleteCallback complete =
        [weak = context_, want, connection](
            NativeEngine& engine, AsyncTask& task, int32_t status) {
            HILOG_INFO("OnDisconnectAbility begin");
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("OnDisconnectAbility context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            if (connection == nullptr) {
                HILOG_WARN("connection nullptr");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            HILOG_INFO("context->DisconnectAbility");
            context->DisconnectAbility(want, connection);
            task.Resolve(engine, engine.CreateUndefined());
        };

    NativeValue* lastParam = (info.argc > ARGC_ONE) ? info.argv[1] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnDisconnectAbility",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::OnTerminateSelf(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnTerminateSelf is called");
    auto abilityContext = context_.lock();
    if (abilityContext != nullptr) {
        abilityContext->SetTerminating(true);
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            auto errcode = context->TerminateSelf();
            if (errcode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, errcode));
            }
        };

    NativeValue* lastParam = (info.argc > ARGC_ZERO) ? info.argv[ARGC_ZERO] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnTerminateSelf",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::OnRestoreWindowStage(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnRestoreWindowStage is called, argc = %{public}d", static_cast<int>(info.argc));
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("OnRestoreWindowStage need one parameters");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }
    auto context = context_.lock();
    if (!context) {
        HILOG_ERROR("OnRestoreWindowStage context is released");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return engine.CreateUndefined();
    }
    auto errcode = context->RestoreWindowStage(engine, info.argv[0]);
    if (errcode != 0) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsError(engine, errcode, "RestoreWindowStage failed.");
    }
    return engine.CreateUndefined();
}

NativeValue* JsAbilityContext::OnIsTerminating(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnIsTerminating is called");
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_ERROR("OnIsTerminating context is nullptr");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return engine.CreateUndefined();
    }
    return engine.CreateBoolean(context->IsTerminating());
}

bool JsAbilityContext::UnWrapWant(NativeEngine& engine, NativeValue* argv, AAFwk::Want& want)
{
    if (argv == nullptr) {
        HILOG_WARN("%s argv == nullptr!", __func__);
        return false;
    }
    return AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(argv), want);
}

NativeValue* JsAbilityContext::WrapWant(NativeEngine& engine, const AAFwk::Want& want)
{
    return reinterpret_cast<NativeValue*>(AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&engine), want));
}

bool JsAbilityContext::UnWrapAbilityResult(NativeEngine& engine, NativeValue* argv, int& resultCode, AAFwk::Want& want)
{
    if (argv == nullptr) {
        HILOG_WARN("%s argv == nullptr!", __func__);
        return false;
    }
    if (argv->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_WARN("%s invalid type of abilityResult!", __func__);
        return false;
    }
    NativeObject* jObj = ConvertNativeValueTo<NativeObject>(argv);
    NativeValue* jResultCode = jObj->GetProperty("resultCode");
    if (jResultCode == nullptr) {
        HILOG_WARN("%s jResultCode == nullptr!", __func__);
        return false;
    }
    if (jResultCode->TypeOf() != NativeValueType::NATIVE_NUMBER) {
        HILOG_WARN("%s invalid type of resultCode!", __func__);
        return false;
    }
    resultCode = int64_t(*ConvertNativeValueTo<NativeNumber>(jObj->GetProperty("resultCode")));
    NativeValue* jWant = jObj->GetProperty("want");
    if (jWant == nullptr) {
        HILOG_WARN("%s jWant == nullptr!", __func__);
        return false;
    }
    if (jWant->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_WARN("%s invalid type of want!", __func__);
        return false;
    }
    return JsAbilityContext::UnWrapWant(engine, jWant, want);
}

NativeValue* JsAbilityContext::WrapAbilityResult(NativeEngine& engine, const int& resultCode, const AAFwk::Want& want)
{
    NativeValue* jAbilityResult = engine.CreateObject();
    NativeObject* abilityResult = ConvertNativeValueTo<NativeObject>(jAbilityResult);
    abilityResult->SetProperty("resultCode", engine.CreateNumber(resultCode));
    abilityResult->SetProperty("want", JsAbilityContext::WrapWant(engine, want));
    return jAbilityResult;
}

void JsAbilityContext::InheritWindowMode(AAFwk::Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);
#ifdef SUPPORT_GRAPHICS
    // only split mode need inherit
    auto context = context_.lock();
    if (!context) {
        HILOG_ERROR("context is nullptr.");
        return;
    }
    auto windowMode = context->GetCurrentWindowMode();
    if (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }
    HILOG_INFO("%{public}s called end. window mode is %{public}d", __func__, windowMode);
#endif
}

void JsAbilityContext::ConfigurationUpdated(NativeEngine* engine, std::shared_ptr<NativeReference> &jsContext,
    const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    HILOG_INFO("ConfigurationUpdated begin.");
    if (jsContext == nullptr || config == nullptr) {
        HILOG_INFO("jsContext is nullptr.");
        return;
    }

    NativeValue* value = jsContext->Get();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(value);
    if (object == nullptr) {
        HILOG_INFO("object is nullptr.");
        return;
    }

    NativeValue* method = object->GetProperty("onUpdateConfiguration");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onUpdateConfiguration from object");
        return;
    }
    HILOG_INFO("JSAbilityConnection::CallFunction onUpdateConfiguration, success");

    HILOG_INFO("OnAbilityConnectDone begin NAPI_ohos_rpc_CreateJsRemoteObject");
    NativeValue* argv[] = {CreateJsConfiguration(*engine, *config)};
    engine->CallFunction(value, method, argv, ARGC_ONE);
}

NativeValue* CreateJsAbilityContext(NativeEngine& engine, std::shared_ptr<AbilityContext> context,
                                    DetachCallback detach, AttachCallback attach)
{
    NativeValue* objValue = CreateJsBaseContext(engine, context, detach, attach);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsAbilityContext> jsContext = std::make_unique<JsAbilityContext>(context);
    object->SetNativePointer(jsContext.release(), JsAbilityContext::Finalizer, nullptr);

    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());

    auto abilityInfo = context->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        object->SetProperty("abilityInfo", CreateJsAbilityInfo(engine, *abilityInfo));
    }

    auto configuration = context->GetConfiguration();
    if (configuration != nullptr) {
        object->SetProperty("config", CreateJsConfiguration(engine, *configuration));
    }

    const char *moduleName = "JsAbilityContext";
    BindNativeFunction(engine, *object, "startAbility", moduleName, JsAbilityContext::StartAbility);
    BindNativeFunction(engine, *object, "startAbilityWithAccount", moduleName,
        JsAbilityContext::StartAbilityWithAccount);
    BindNativeFunction(engine, *object, "startAbilityByCall", moduleName, JsAbilityContext::StartAbilityByCall);
    BindNativeFunction(engine, *object, "startAbilityForResult", moduleName, JsAbilityContext::StartAbilityForResult);
    BindNativeFunction(engine, *object, "startAbilityForResultWithAccount", moduleName,
        JsAbilityContext::StartAbilityForResultWithAccount);
    BindNativeFunction(engine, *object, "startServiceExtensionAbility", moduleName,
        JsAbilityContext::StartServiceExtensionAbility);
    BindNativeFunction(engine, *object, "startServiceExtensionAbilityWithAccount", moduleName,
        JsAbilityContext::StartServiceExtensionAbilityWithAccount);
    BindNativeFunction(engine, *object, "stopServiceExtensionAbility", moduleName,
        JsAbilityContext::StopServiceExtensionAbility);
    BindNativeFunction(engine, *object, "stopServiceExtensionAbilityWithAccount", moduleName,
        JsAbilityContext::StopServiceExtensionAbilityWithAccount);
    BindNativeFunction(engine, *object, "connectAbility", moduleName, JsAbilityContext::ConnectAbility);
    BindNativeFunction(engine, *object, "connectServiceExtensionAbility", moduleName, JsAbilityContext::ConnectAbility);
    BindNativeFunction(engine, *object, "connectAbilityWithAccount", moduleName,
        JsAbilityContext::ConnectAbilityWithAccount);
    BindNativeFunction(engine, *object, "connectServiceExtensionAbilityWithAccount", moduleName,
        JsAbilityContext::ConnectAbilityWithAccount);
    BindNativeFunction(engine, *object, "disconnectAbility", moduleName, JsAbilityContext::DisconnectAbility);
    BindNativeFunction(
        engine, *object, "disconnectServiceExtensionAbility", moduleName, JsAbilityContext::DisconnectAbility);
    BindNativeFunction(engine, *object, "terminateSelf", moduleName, JsAbilityContext::TerminateSelf);
    BindNativeFunction(engine, *object, "terminateSelfWithResult", moduleName,
        JsAbilityContext::TerminateSelfWithResult);
    BindNativeFunction(engine, *object, "restoreWindowStage", moduleName, JsAbilityContext::RestoreWindowStage);
    BindNativeFunction(engine, *object, "isTerminating", moduleName, JsAbilityContext::IsTerminating);
    BindNativeFunction(engine, *object, "startRecentAbility", moduleName,
        JsAbilityContext::StartRecentAbility);

#ifdef SUPPORT_GRAPHICS
    BindNativeFunction(engine, *object, "setMissionLabel", moduleName, JsAbilityContext::SetMissionLabel);
    BindNativeFunction(engine, *object, "setMissionIcon", moduleName, JsAbilityContext::SetMissionIcon);
#endif
    return objValue;
}

JSAbilityConnection::JSAbilityConnection(NativeEngine& engine) : engine_(engine) {}

JSAbilityConnection::~JSAbilityConnection()
{
    uv_loop_t *loop = engine_.GetUVLoop();
    if (loop == nullptr) {
        HILOG_ERROR("~JSAbilityConnection: failed to get uv loop.");
        return;
    }

    ConnectCallback *cb = new (std::nothrow) ConnectCallback();
    if (cb == nullptr) {
        HILOG_ERROR("~JSAbilityConnection: failed to create cb.");
        return;
    }
    cb->jsConnectionObject_ = std::move(jsConnectionObject_);

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("~JSAbilityConnection: failed to create work.");
        delete cb;
        cb = nullptr;
        return;
    }
    work->data = reinterpret_cast<void *>(cb);
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
    [](uv_work_t *work, int status) {
        if (work == nullptr) {
            HILOG_ERROR("~JSAbilityConnection: work is nullptr.");
            return;
        }
        if (work->data == nullptr) {
            HILOG_ERROR("~JSAbilityConnection: data is nullptr.");
            delete work;
            work = nullptr;
            return;
        }
        ConnectCallback *cb = reinterpret_cast<ConnectCallback *>(work->data);
        delete cb;
        cb = nullptr;
        delete work;
        work = nullptr;
    });
    if (ret != 0) {
        if (cb != nullptr) {
            delete cb;
            cb = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
}

void JSAbilityConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

void JSAbilityConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("OnAbilityConnectDone begin, resultCode:%{public}d", resultCode);
    if (handler_ == nullptr) {
        HILOG_INFO("handler_ nullptr");
        return;
    }

    wptr<JSAbilityConnection> connection = this;
    auto task = [connection, element, remoteObject, resultCode] {
        sptr<JSAbilityConnection> connectionSptr = connection.promote();
        if (!connectionSptr) {
            HILOG_INFO("connectionSptr nullptr");
            return;
        }
        connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
    };
    handler_->PostTask(task, "OnAbilityConnectDone");
}

void JSAbilityConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("HandleOnAbilityConnectDone begin, resultCode:%{public}d", resultCode);
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    NativeValue* value = jsConnectionObject_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }
    NativeValue* methodOnConnect = obj->GetProperty("onConnect");
    if (methodOnConnect == nullptr) {
        HILOG_ERROR("Failed to get onConnect from object");
        return;
    }
    HILOG_INFO("JSAbilityConnection::CallFunction onConnect, success");

    // wrap RemoteObject
    HILOG_INFO("OnAbilityConnectDone begin NAPI_ohos_rpc_CreateJsRemoteObject");
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(
        reinterpret_cast<napi_env>(&engine_), remoteObject);
    NativeValue* nativeRemoteObject = reinterpret_cast<NativeValue*>(napiRemoteObject);
    NativeValue* argv[] = { ConvertElement(element), nativeRemoteObject };
    engine_.CallFunction(value, methodOnConnect, argv, ARGC_TWO);
    HILOG_INFO("OnAbilityConnectDone end");
}

void JSAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_INFO("OnAbilityDisconnectDone begin, resultCode:%{public}d", resultCode);
    if (handler_ == nullptr) {
        HILOG_INFO("handler_ nullptr");
        return;
    }

    wptr<JSAbilityConnection> connection = this;
    auto task = [connection, element, resultCode] {
        sptr<JSAbilityConnection> connectionSptr = connection.promote();
        if (!connectionSptr) {
            HILOG_INFO("connectionSptr nullptr");
            return;
        }
        connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
    };
    handler_->PostTask(task, "OnAbilityDisconnectDone");
}

void JSAbilityConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    HILOG_INFO("HandleOnAbilityDisconnectDone begin, resultCode:%{public}d", resultCode);
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    NativeValue* value = jsConnectionObject_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty("onDisconnect");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onDisconnect from object");
        return;
    }

    // release connect
    HILOG_INFO("OnAbilityDisconnectDone abilityConnects_.size:%{public}zu", abilityConnects_.size());
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    auto item = std::find_if(abilityConnects_.begin(), abilityConnects_.end(),
        [bundleName, abilityName, connectionId = connectionId_] (
            const std::map<ConnectionKey, sptr<JSAbilityConnection>>::value_type &obj) {
                return (bundleName == obj.first.want.GetBundle()) &&
                    (abilityName == obj.first.want.GetElement().GetAbilityName()) &&
                    connectionId == obj.first.id;
        });
    if (item != abilityConnects_.end()) {
        // match bundlename && abilityname
        abilityConnects_.erase(item);
        HILOG_INFO("OnAbilityDisconnectDone erase abilityConnects_.size:%{public}zu", abilityConnects_.size());
    }

    NativeValue* argv[] = { ConvertElement(element) };
    HILOG_INFO("OnAbilityDisconnectDone CallFunction success");
    engine_.CallFunction(value, method, argv, ARGC_ONE);
}

void JSAbilityConnection::CallJsFailed(int32_t errorCode)
{
    HILOG_INFO("CallJsFailed begin");
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    NativeValue* value = jsConnectionObject_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty("onFailed");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onFailed from object");
        return;
    }

    NativeValue* argv[] = {engine_.CreateNumber(errorCode)};
    HILOG_INFO("CallJsFailed CallFunction success");
    engine_.CallFunction(value, method, argv, ARGC_ONE);
    HILOG_INFO("CallJsFailed end");
}

NativeValue* JSAbilityConnection::ConvertElement(const AppExecFwk::ElementName &element)
{
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(reinterpret_cast<napi_env>(&engine_), element);
    return reinterpret_cast<NativeValue*>(napiElementName);
}

void JSAbilityConnection::SetJsConnectionObject(NativeValue* jsConnectionObject)
{
    jsConnectionObject_ = std::unique_ptr<NativeReference>(engine_.CreateReference(jsConnectionObject, 1));
}

#ifdef SUPPORT_GRAPHICS
NativeValue* JsAbilityContext::SetMissionLabel(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnSetMissionLabel(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::SetMissionIcon(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnSetMissionIcon(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::OnSetMissionLabel(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnSetMissionLabel is called, argc = %{public}d", static_cast<int>(info.argc));
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("OnSetMissionLabel, Not enough params");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    std::string label;
    if (!ConvertFromJsValue(engine, info.argv[0], label)) {
        HILOG_ERROR("OnSetMissionLabel, parse label failed.");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_, label](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            auto errcode = context->SetMissionLabel(label);
            if (errcode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, errcode));
            }
        };

    NativeValue* lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnSetMissionLabel",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::OnSetMissionIcon(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnSetMissionIcon is called, argc = %{public}d", static_cast<int>(info.argc));
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("OnSetMissionIcon, Not enough params");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    auto icon = OHOS::Media::PixelMapNapi::GetPixelMap(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[0]));
    if (!icon) {
        HILOG_ERROR("OnSetMissionIcon, parse icon failed.");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_, icon](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released when set mission icon");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            auto errcode = context->SetMissionIcon(icon);
            if (errcode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, errcode));
            }
        };

    NativeValue* lastParam = (info.argc > ARGC_ONE) ? info.argv[1] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnSetMissionIcon",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}
#endif
}  // namespace AbilityRuntime
}  // namespace OHOS
