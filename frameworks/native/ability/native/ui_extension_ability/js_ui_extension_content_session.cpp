/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "js_ui_extension_content_session.h"

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "event_handler.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_context.h"
#include "napi_common_start_options.h"
#include "napi_common_want.h"
#include "native_engine.h"
#include "native_value.h"
#include "want.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr const char* PERMISSION_PRIVACY_WINDOW = "ohos.permission.PRIVACY_WINDOW";
} // namespace

void UISessionAbilityResultListener::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    HILOG_DEBUG("begin.");
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData, false);
        }
        resultCallbacks_.erase(requestCode);
    }
    HILOG_DEBUG("end.");
}

bool UISessionAbilityResultListener::IsMatch(int requestCode)
{
    return resultCallbacks_.find(requestCode) != resultCallbacks_.end();
}

void UISessionAbilityResultListener::OnAbilityResultInner(int requestCode, int resultCode, const Want &resultData)
{
    HILOG_DEBUG("begin.");
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData, true);
        }
        resultCallbacks_.erase(requestCode);
    }
    HILOG_DEBUG("end.");
}

void UISessionAbilityResultListener::SaveResultCallbacks(int requestCode, RuntimeTask&& task)
{
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
}

JsUIExtensionContentSession::JsUIExtensionContentSession(
    NativeEngine& engine, sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow,
    std::weak_ptr<AbilityRuntime::Context> &context,
    std::shared_ptr<AbilityResultListeners>& abilityResultListeners)
    : engine_(engine), sessionInfo_(sessionInfo), uiWindow_(uiWindow), context_(context)
{
    listener_ = std::make_shared<UISessionAbilityResultListener>();
    if (abilityResultListeners == nullptr) {
        HILOG_ERROR("abilityResultListeners is nullptr");
    } else if (sessionInfo == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr");
    } else {
        abilityResultListeners->AddListener(sessionInfo->sessionToken, listener_);
    }
}

JsUIExtensionContentSession::JsUIExtensionContentSession(
    NativeEngine& engine, sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow)
    : engine_(engine), sessionInfo_(sessionInfo), uiWindow_(uiWindow) {}

void JsUIExtensionContentSession::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_DEBUG("JsUIExtensionContentSession Finalizer is called");
    std::unique_ptr<JsUIExtensionContentSession>(static_cast<JsUIExtensionContentSession*>(data));
}

NativeValue* JsUIExtensionContentSession::StartAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnStartAbility(*engine, *info) : nullptr;
}

NativeValue* JsUIExtensionContentSession::StartAbilityForResult(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnStartAbilityForResult(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::TerminateSelf(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelf(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::TerminateSelfWithResult(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelfWithResult(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::SendData(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnSendData(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::SetReceiveDataCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnSetReceiveDataCallback(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::LoadContent(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnLoadContent(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::SetWindowBackgroundColor(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnSetWindowBackgroundColor(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::SetWindowPrivacyMode(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnSetWindowPrivacyMode(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::OnStartAbility(NativeEngine& engine, NativeCallbackInfo &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("OnStartAbility is called");

    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    AAFwk::Want want;
    size_t unwrapArgc = 1;
    if (!OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[0]), want)) {
        HILOG_ERROR("Failed to parse want!");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    if (!want.HasParameter(Want::PARAM_BACK_TO_OTHER_MISSION_STACK)) {
        want.SetParam(Want::PARAM_BACK_TO_OTHER_MISSION_STACK, true);
    }
    HILOG_INFO("StartAbility, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    auto innerErrorCode = std::make_shared<int>(ERR_OK);
    AsyncTask::ExecuteCallback execute = StartAbilityExecuteCallback(want, unwrapArgc, engine, info, innerErrorCode);

    AsyncTask::CompleteCallback complete = [innerErrorCode](NativeEngine& engine, AsyncTask& task, int32_t status) {
        if (*innerErrorCode == 0) {
            task.ResolveWithNoError(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsErrorByNativeErr(engine, *innerErrorCode));
        }
    };

    NativeValue* lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
    NativeValue* result = nullptr;
    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        AddFreeInstallObserver(engine, want, lastParam);
        AsyncTask::Schedule("JsUIExtensionContentSession::OnStartAbility", engine,
            CreateAsyncTaskWithLastParam(engine, nullptr, std::move(execute), nullptr, &result));
    } else {
        AsyncTask::Schedule("JsUIExtensionContentSession::OnStartAbility", engine,
            CreateAsyncTaskWithLastParam(engine, lastParam, std::move(execute), std::move(complete), &result));
    }
    HILOG_DEBUG("OnStartAbility is called end");
    return result;
}

AsyncTask::ExecuteCallback JsUIExtensionContentSession::StartAbilityExecuteCallback(AAFwk::Want& want,
    size_t& unwrapArgc, NativeEngine& engine, NativeCallbackInfo &info, std::shared_ptr<int> &innerErrorCode)
{
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && info.argv[1]->TypeOf() == NATIVE_OBJECT) {
        HILOG_DEBUG("OnStartAbility start options is used.");
        AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[1]), startOptions);
        unwrapArgc++;
    }

    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    }
    AsyncTask::ExecuteCallback execute = [weak = context_, want, startOptions, unwrapArgc,
        sessionInfo = sessionInfo_, &observer = freeInstallObserver_, innerErrorCode]() {
        auto context = weak.lock();
        if (!context) {
            HILOG_WARN("context is released");
            *innerErrorCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }

        *innerErrorCode = (unwrapArgc == 1) ?
            AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByUIContentSession(want,
                context->GetToken(), sessionInfo, -1, -1) :
            AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByUIContentSession(want,
                startOptions, context->GetToken(), sessionInfo, -1, -1);
        if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
            *innerErrorCode != 0 && observer != nullptr) {
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
            observer->OnInstallFinished(bundleName, abilityName, startTime, *innerErrorCode);
        }
    };
    return execute;
}

NativeValue *JsUIExtensionContentSession::OnStartAbilityForResult(NativeEngine& engine, NativeCallbackInfo &info)
{
    if (info.argc == ARGC_ZERO) {
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    AAFwk::Want want;
    if (!UnWrapWant(engine, info.argv[0], want)) {
        HILOG_ERROR("Failed to parse want!");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    if (!want.HasParameter(Want::PARAM_BACK_TO_OTHER_MISSION_STACK)) {
        want.SetParam(Want::PARAM_BACK_TO_OTHER_MISSION_STACK, true);
    }
    size_t unwrapArgc = 1;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && info.argv[1]->TypeOf() == NATIVE_OBJECT) {
        HILOG_DEBUG("OnStartAbilityForResult start options is used.");
        AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[1]), startOptions);
        unwrapArgc++;
    }

    NativeValue* lastParam = info.argc > unwrapArgc ? info.argv[unwrapArgc] : nullptr;
    NativeValue* result = nullptr;
    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(engine, want, lastParam, true);
    }
    std::unique_ptr<AsyncTask> uasyncTask =
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<AsyncTask> asyncTask = std::move(uasyncTask);
    if (asyncTask == nullptr) {
        HILOG_ERROR("asyncTask is nullptr");
        return engine.CreateUndefined();
    }
    if (listener_ == nullptr) {
        HILOG_ERROR("listener_ is nullptr");
        return engine.CreateUndefined();
    }
    StartAbilityForResultRuntimeTask(engine, want, asyncTask, unwrapArgc, startOptions);
    return result;
}

void JsUIExtensionContentSession::StartAbilityForResultRuntimeTask(NativeEngine& engine,
    AAFwk::Want &want, std::shared_ptr<AsyncTask> asyncTask, size_t& unwrapArgc,
    AAFwk::StartOptions startOptions)
{
    RuntimeTask task = [&engine, asyncTask, &observer = freeInstallObserver_](int resultCode,
        const AAFwk::Want& want, bool isInner) {
        HILOG_DEBUG("OnStartAbilityForResult async callback is called");
        NativeValue* abilityResult = WrapAbilityResult(engine, resultCode, want);
        if (abilityResult == nullptr) {
            HILOG_WARN("wrap abilityResult failed");
            asyncTask->Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
        } else {
            if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
                resultCode != 0 && observer != nullptr) {
                std::string bundleName = want.GetElement().GetBundleName();
                std::string abilityName = want.GetElement().GetAbilityName();
                std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
                observer->OnInstallFinished(bundleName, abilityName, startTime,
                    static_cast<int>(GetJsErrorCodeByNativeError(resultCode)));
            } else if (isInner) {
                asyncTask->Reject(engine, CreateJsErrorByNativeErr(engine, resultCode));
            } else {
                asyncTask->ResolveWithNoError(engine, abilityResult);
            }
        }
    };
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_WARN("context is released");
        asyncTask->Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    } else {
        want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
        int curRequestCode = reinterpret_cast<UIExtensionContext*>(context.get())->GenerateCurRequestCode();
        listener_->SaveResultCallbacks(curRequestCode, std::move(task));
        ErrCode err = (unwrapArgc == 1) ?
            AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByUIContentSession(want,
                context->GetToken(), sessionInfo_, curRequestCode, -1) :
            AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByUIContentSession(want,
                startOptions, context->GetToken(), sessionInfo_, curRequestCode, -1);
        if (err != ERR_OK && err != AAFwk::START_ABILITY_WAITING) {
            HILOG_ERROR("StartAbilityForResult. ret=%{public}d", err);
            listener_->OnAbilityResultInner(curRequestCode, err, want);
        }
    }
}

NativeValue *JsUIExtensionContentSession::OnTerminateSelf(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    AsyncTask::CompleteCallback complete =
        [sessionInfo = sessionInfo_](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (sessionInfo == nullptr) {
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo);
            if (errorCode == 0) {
                task.ResolveWithNoError(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, errorCode));
            }
        };

    NativeValue* lastParam = (info.argc > ARGC_ZERO) ? info.argv[INDEX_ZERO] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::ScheduleHighQos("JsUIExtensionContentSession::OnTerminateSelf",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsUIExtensionContentSession::OnTerminateSelfWithResult(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("invalid param");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    int resultCode = 0;
    AAFwk::Want want;
    if (!UnWrapAbilityResult(engine, info.argv[INDEX_ZERO], resultCode, want)) {
        HILOG_ERROR("OnTerminateSelfWithResult Failed to parse ability result!");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [uiWindow = uiWindow_, sessionInfo = sessionInfo_, want, resultCode](NativeEngine& engine,
            AsyncTask& task, int32_t status) {
            if (uiWindow == nullptr) {
                HILOG_ERROR("uiWindow is nullptr");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto ret = uiWindow->TransferAbilityResult(resultCode, want);
            if (ret != Rosen::WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo);
            if (errorCode == 0) {
                task.ResolveWithNoError(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, errorCode));
            }
        };

    NativeValue* lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::ScheduleHighQos("JsUIExtensionContentSession::OnTerminateSelfWithResult",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsUIExtensionContentSession::OnSendData(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("invalid param");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    AAFwk::WantParams params;
    if (!AppExecFwk::UnwrapWantParams(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), params)) {
        HILOG_ERROR("OnSendData Failed to parse param!");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    if (uiWindow_ == nullptr) {
        HILOG_ERROR("uiWindow_ is nullptr");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
        return engine.CreateUndefined();
    }

    Rosen::WMError ret = uiWindow_->TransferExtensionData(params);
    if (ret == Rosen::WMError::WM_OK) {
        HILOG_DEBUG("TransferExtensionData success");
    } else {
        HILOG_ERROR("TransferExtensionData failed, ret=%{public}d", ret);
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return engine.CreateUndefined();
}

NativeValue *JsUIExtensionContentSession::OnSetReceiveDataCallback(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    if (info.argc < ARGC_ONE || info.argv[INDEX_ZERO]->TypeOf() != NATIVE_FUNCTION) {
        HILOG_ERROR("invalid param");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    if (!isRegistered) {
        if (uiWindow_ == nullptr) {
            HILOG_ERROR("uiWindow_ is nullptr");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
            return engine.CreateUndefined();
        }
        receiveDataCallback_ = std::make_shared<CallbackWrapper>();
        std::weak_ptr<CallbackWrapper> weakCallback(receiveDataCallback_);
        auto handler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
        uiWindow_->RegisterTransferComponentDataListener([&engine = engine_, handler, weakCallback](
            const AAFwk::WantParams& wantParams) {
            if (handler) {
                handler->PostTask([&engine, weakCallback, wantParams]() {
                    JsUIExtensionContentSession::CallReceiveDataCallback(engine, weakCallback, wantParams);
                });
            }
        });
        isRegistered = true;
    }

    NativeValue* callback = info.argv[INDEX_ZERO];
    if (receiveDataCallback_ == nullptr) {
        HILOG_ERROR("uiWindow_ is nullptr");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
        return engine.CreateUndefined();
    }
    receiveDataCallback_->ResetCallback(std::shared_ptr<NativeReference>(engine.CreateReference(callback, 1)));
    return engine.CreateUndefined();
}

NativeValue *JsUIExtensionContentSession::OnLoadContent(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    std::string contextPath;
    if (info.argc < ARGC_ONE || !ConvertFromJsValue(engine, info.argv[INDEX_ZERO], contextPath)) {
        HILOG_ERROR("invalid param");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    HILOG_DEBUG("contextPath: %{public}s", contextPath.c_str());
    NativeValue* storage = nullptr;
    if (info.argc > ARGC_ONE && info.argv[INDEX_ONE]->TypeOf() == NATIVE_OBJECT) {
        storage = info.argv[INDEX_ONE];
    }
    if (uiWindow_ == nullptr) {
        HILOG_ERROR("uiWindow_ is nullptr");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
        return engine.CreateUndefined();
    }
    Rosen::WMError ret = uiWindow_->SetUIContent(contextPath, &engine, storage);
    if (ret == Rosen::WMError::WM_OK) {
        HILOG_DEBUG("SetUIContent success");
    } else {
        HILOG_ERROR("SetUIContent failed, ret=%{public}d", ret);
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return engine.CreateUndefined();
}

NativeValue *JsUIExtensionContentSession::OnSetWindowBackgroundColor(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    std::string color;
    if (info.argc < ARGC_ONE || !ConvertFromJsValue(engine, info.argv[INDEX_ZERO], color)) {
        HILOG_ERROR("invalid param");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    if (uiWindow_ == nullptr) {
        HILOG_ERROR("uiWindow_ is nullptr");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
        return engine.CreateUndefined();
    }
    Rosen::WMError ret = uiWindow_->SetBackgroundColor(color);
    if (ret == Rosen::WMError::WM_OK) {
        HILOG_DEBUG("SetBackgroundColor success");
    } else {
        HILOG_ERROR("SetBackgroundColor failed, ret=%{public}d", ret);
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return engine.CreateUndefined();
}

NativeValue *JsUIExtensionContentSession::OnSetWindowPrivacyMode(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    bool isPrivacyMode = false;
    if (info.argc < ARGC_ONE || !ConvertFromJsValue(engine, info.argv[INDEX_ZERO], isPrivacyMode)) {
        HILOG_ERROR("invalid param");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    int ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(selfToken, PERMISSION_PRIVACY_WINDOW);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        ThrowNoPermissionError(engine, PERMISSION_PRIVACY_WINDOW);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [uiWindow = uiWindow_, isPrivacyMode](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (uiWindow == nullptr) {
                HILOG_ERROR("uiWindow is nullptr");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto ret = uiWindow->SetPrivacyMode(isPrivacyMode);
            if (ret == Rosen::WMError::WM_OK) {
                task.ResolveWithNoError(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
            }
        };
    NativeValue* lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsUIExtensionContentSession::OnSetWindowPrivacyMode",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsUIExtensionContentSession::CreateJsUIExtensionContentSession(NativeEngine& engine,
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow,
    std::weak_ptr<AbilityRuntime::Context> context,
    std::shared_ptr<AbilityResultListeners>& abilityResultListeners)
{
    HILOG_DEBUG("begin");
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr");
        return engine.CreateUndefined();
    }

    std::unique_ptr<JsUIExtensionContentSession> jsSession =
        std::make_unique<JsUIExtensionContentSession>(engine, sessionInfo, uiWindow, context, abilityResultListeners);
    object->SetNativePointer(jsSession.release(), Finalizer, nullptr);

    const char *moduleName = "JsUIExtensionContentSession";
    BindNativeFunction(engine, *object, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(engine, *object, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(engine, *object, "sendData", moduleName, SendData);
    BindNativeFunction(engine, *object, "setReceiveDataCallback", moduleName, SetReceiveDataCallback);
    BindNativeFunction(engine, *object, "loadContent", moduleName, LoadContent);
    BindNativeFunction(engine, *object, "setWindowBackgroundColor", moduleName, SetWindowBackgroundColor);
    BindNativeFunction(engine, *object, "setWindowPrivacyMode", moduleName, SetWindowPrivacyMode);
    BindNativeFunction(engine, *object, "startAbility", moduleName, StartAbility);
    BindNativeFunction(engine, *object, "startAbilityForResult", moduleName, StartAbilityForResult);
    return objValue;
}

NativeValue *JsUIExtensionContentSession::CreateJsUIExtensionContentSession(NativeEngine& engine,
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow)
{
    HILOG_DEBUG("begin");
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr");
        return engine.CreateUndefined();
    }

    std::unique_ptr<JsUIExtensionContentSession> jsSession =
        std::make_unique<JsUIExtensionContentSession>(engine, sessionInfo, uiWindow);
    object->SetNativePointer(jsSession.release(), Finalizer, nullptr);

    const char *moduleName = "JsUIExtensionContentSession";
    BindNativeFunction(engine, *object, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(engine, *object, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(engine, *object, "sendData", moduleName, SendData);
    BindNativeFunction(engine, *object, "setReceiveDataCallback", moduleName, SetReceiveDataCallback);
    BindNativeFunction(engine, *object, "loadContent", moduleName, LoadContent);
    BindNativeFunction(engine, *object, "setWindowBackgroundColor", moduleName, SetWindowBackgroundColor);
    BindNativeFunction(engine, *object, "setWindowPrivacyMode", moduleName, SetWindowPrivacyMode);
    BindNativeFunction(engine, *object, "startAbility", moduleName, StartAbility);
    BindNativeFunction(engine, *object, "startAbilityForResult", moduleName, StartAbilityForResult);
    return objValue;
}

void JsUIExtensionContentSession::CallReceiveDataCallback(NativeEngine& engine,
    std::weak_ptr<CallbackWrapper> weakCallback, const AAFwk::WantParams& wantParams)
{
    auto cbWrapper = weakCallback.lock();
    if (cbWrapper == nullptr) {
        HILOG_WARN("cbWrapper is nullptr");
        return;
    }
    auto callback = cbWrapper->GetCallback();
    if (callback == nullptr) {
        HILOG_WARN("callback is nullptr");
        return;
    }
    NativeValue* method = callback->Get();
    if (method == nullptr) {
        HILOG_WARN("method is nullptr");
        return;
    }
    HandleScope handleScope(engine);
    NativeValue* nativeWantParams = AppExecFwk::CreateJsWantParams(engine, wantParams);
    if (nativeWantParams == nullptr) {
        HILOG_ERROR("nativeWantParams is nullptr");
        return;
    }
    NativeValue* argv[] = {nativeWantParams};
    engine.CallFunction(engine.GetGlobal(), method, argv, ARGC_ONE);
}

bool JsUIExtensionContentSession::UnWrapAbilityResult(NativeEngine& engine, NativeValue* argv, int& resultCode,
    AAFwk::Want& want)
{
    if (argv == nullptr) {
        HILOG_WARN("UnWrapAbilityResult argv == nullptr!");
        return false;
    }
    if (argv->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_WARN("UnWrapAbilityResult invalid type of abilityResult!");
        return false;
    }
    NativeObject* jObj = ConvertNativeValueTo<NativeObject>(argv);
    NativeValue* jResultCode = jObj->GetProperty("resultCode");
    if (jResultCode == nullptr) {
        HILOG_WARN("UnWrapAbilityResult jResultCode == nullptr!");
        return false;
    }
    if (jResultCode->TypeOf() != NativeValueType::NATIVE_NUMBER) {
        HILOG_WARN("UnWrapAbilityResult invalid type of resultCode!");
        return false;
    }
    resultCode = int64_t(*ConvertNativeValueTo<NativeNumber>(jObj->GetProperty("resultCode")));
    NativeValue* jWant = jObj->GetProperty("want");
    if (jWant == nullptr) {
        HILOG_WARN("UnWrapAbilityResult jWant == nullptr!");
        return false;
    }
    if (jWant->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_WARN("UnWrapAbilityResult invalid type of want!");
        return false;
    }
    return AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(jWant), want);
}

NativeValue* JsUIExtensionContentSession::WrapAbilityResult(NativeEngine& engine,
    const int& resultCode, const AAFwk::Want& want)
{
    NativeValue* jAbilityResult = engine.CreateObject();
    NativeObject* abilityResult = ConvertNativeValueTo<NativeObject>(jAbilityResult);
    if (abilityResult == nullptr) {
        HILOG_ERROR("abilityResult is nullptr");
        return engine.CreateUndefined();
    }
    abilityResult->SetProperty("resultCode", engine.CreateNumber(resultCode));
    abilityResult->SetProperty("want", WrapWant(engine, want));
    return jAbilityResult;
}

NativeValue* JsUIExtensionContentSession::WrapWant(NativeEngine& engine, const AAFwk::Want& want)
{
    return reinterpret_cast<NativeValue*>(AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&engine), want));
}

bool JsUIExtensionContentSession::UnWrapWant(NativeEngine& engine, NativeValue* argv, AAFwk::Want& want)
{
    if (argv == nullptr) {
        HILOG_WARN("argv == nullptr");
        return false;
    }
    return AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(argv), want);
}

void JsUIExtensionContentSession::AddFreeInstallObserver(NativeEngine& engine,
    const AAFwk::Want &want, NativeValue* callback, bool isAbilityResult)
{
    // adapter free install async return install and start result
    int ret = 0;
    if (freeInstallObserver_ == nullptr) {
        freeInstallObserver_ = new JsFreeInstallObserver(engine);
        ret = AAFwk::AbilityManagerClient::GetInstance()->AddFreeInstallObserver(freeInstallObserver_);
    }

    if (ret != ERR_OK) {
        HILOG_ERROR("AddFreeInstallObserver failed.");
    } else {
        HILOG_INFO("AddJsObserverObject");
        // build a callback observer with last param
        std::string bundleName = want.GetElement().GetBundleName();
        std::string abilityName = want.GetElement().GetAbilityName();
        std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
        freeInstallObserver_->AddJsObserverObject(bundleName, abilityName, startTime, callback, isAbilityResult);
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS