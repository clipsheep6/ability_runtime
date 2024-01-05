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
#include "js_extension_window.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_context.h"
#include "string_wrapper.h"
#include "napi_common_start_options.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "native_engine.h"
#include "native_value.h"
#include "tokenid_kit.h"
#include "ui_content.h"
#include "want.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t INDEX_THREE = 3;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_THREE = 3;
constexpr const char* PERMISSION_PRIVACY_WINDOW = "ohos.permission.PRIVACY_WINDOW";
const std::string UIEXTENSION_TARGET_TYPE_KEY = "ability.want.params.uiExtensionTargetType";
const std::string FLAG_AUTH_READ_URI_PERMISSION = "ability.want.params.uriPermissionFlag";
} // namespace

#define CHECK_IS_SYSTEM_APP                                                             \
do {                                                                                    \
    auto selfToken = IPCSkeleton::GetSelfTokenID();                                     \
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {      \
        HILOG_ERROR("This application is not system-app, can not use system-api");      \
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);                   \
        return CreateJsUndefined(env);                                                  \
    }                                                                                   \
} while(0)

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
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow,
    std::weak_ptr<AbilityRuntime::Context> &context,
    std::shared_ptr<AbilityResultListeners>& abilityResultListeners)
    : sessionInfo_(sessionInfo), uiWindow_(uiWindow), context_(context)
{
    listener_ = std::make_shared<UISessionAbilityResultListener>();
    if (abilityResultListeners == nullptr) {
        HILOG_ERROR("abilityResultListeners is nullptr");
    } else if (sessionInfo == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr");
    } else {
        abilityResultListeners->AddListener(sessionInfo->uiExtensionComponentId, listener_);
    }
}

JsUIExtensionContentSession::JsUIExtensionContentSession(
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow)
    : sessionInfo_(sessionInfo), uiWindow_(uiWindow) {}

void JsUIExtensionContentSession::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_DEBUG("JsUIExtensionContentSession Finalizer is called");
    std::unique_ptr<JsUIExtensionContentSession>(static_cast<JsUIExtensionContentSession*>(data));
}

napi_value JsUIExtensionContentSession::StartAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnStartAbility);
}

napi_value JsUIExtensionContentSession::StartAbilityAsCaller(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnStartAbilityAsCaller);
}

napi_value JsUIExtensionContentSession::GetUIExtensionHostWindowProxy(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnGetUIExtensionHostWindowProxy);
}

napi_value JsUIExtensionContentSession::StartAbilityForResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnStartAbilityForResult);
}

napi_value JsUIExtensionContentSession::TerminateSelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnTerminateSelf);
}

napi_value JsUIExtensionContentSession::TerminateSelfWithResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnTerminateSelfWithResult);
}

napi_value JsUIExtensionContentSession::SendData(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnSendData);
}

napi_value JsUIExtensionContentSession::SetReceiveDataCallback(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnSetReceiveDataCallback);
}

napi_value JsUIExtensionContentSession::SetReceiveDataForResultCallback(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnSetReceiveDataForResultCallback);
}

napi_value JsUIExtensionContentSession::LoadContent(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnLoadContent);
}

napi_value JsUIExtensionContentSession::SetWindowBackgroundColor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnSetWindowBackgroundColor);
}

napi_value JsUIExtensionContentSession::SetWindowPrivacyMode(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnSetWindowPrivacyMode);
}

napi_value JsUIExtensionContentSession::StartAbilityByType(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnStartAbilityByType);
}

napi_value JsUIExtensionContentSession::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("OnStartAbility is called");
    CHECK_IS_SYSTEM_APP;

    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    size_t unwrapArgc = 1;
    if (!OHOS::AppExecFwk::UnwrapWant(env, info.argv[0], want)) {
        HILOG_ERROR("Failed to parse want!");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    if (!want.HasParameter(Want::PARAM_BACK_TO_OTHER_MISSION_STACK)) {
        want.SetParam(Want::PARAM_BACK_TO_OTHER_MISSION_STACK, true);
    }
    HILOG_INFO("StartAbility, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    auto innerErrorCode = std::make_shared<int>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = StartAbilityExecuteCallback(
        want, unwrapArgc, env, info, innerErrorCode);

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*innerErrorCode == 0) {
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
        }
    };

    napi_value lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        AddFreeInstallObserver(env, want, lastParam);
        NapiAsyncTask::Schedule("JsUIExtensionContentSession::OnStartAbility", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, &result));
    } else {
        NapiAsyncTask::Schedule("JsUIExtensionContentSession::OnStartAbility", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    }
    HILOG_DEBUG("OnStartAbility is called end");
    return result;
}

napi_value JsUIExtensionContentSession::OnGetUIExtensionHostWindowProxy(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("OnGetUIExtensionHostWindowProxy is called");
    CHECK_IS_SYSTEM_APP;
    napi_value jsExtensionWindow = Rosen::JsExtensionWindow::CreateJsExtensionWindow(env, uiWindow_);
    if (jsExtensionWindow == nullptr) {
        HILOG_ERROR("Failed to create jsExtensionWindow object.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    auto value = JsRuntime::LoadSystemModuleByEngine(env, "application.extensionWindow", &jsExtensionWindow, 1);
    if (value == nullptr) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    return value->GetNapiValue();
}

napi_value JsUIExtensionContentSession::OnStartAbilityAsCaller(napi_env env, NapiCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    bool unWrapWantFlag = OHOS::AppExecFwk::UnwrapWant(env, info.argv[0], want);
    if (!unWrapWantFlag) {
        ThrowTooFewParametersError(env);
    }
    decltype(info.argc) unwrapArgc = 1;
    HILOG_INFO("StartAbilityAsCaller, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        HILOG_DEBUG("OnStartAbilityAsCaller start options is used.");
        bool unWrapStartOptionsFlag = AppExecFwk::UnwrapStartOptions(env, info.argv[INDEX_ONE], startOptions);
        if (!unWrapStartOptionsFlag) {
            ThrowTooFewParametersError(env);
        }
        unwrapArgc++;
    }
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, startOptions, unwrapArgc, sessionInfo = sessionInfo_]
        (napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            if (sessionInfo == nullptr) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto innerErrorCode = (unwrapArgc == 1) ?
                AAFwk::AbilityManagerClient::GetInstance()->
                StartAbilityAsCaller(want, context->GetToken(), sessionInfo->callerToken) :
                AAFwk::AbilityManagerClient::GetInstance()->
                StartAbilityAsCaller(want, startOptions, context->GetToken(), sessionInfo->callerToken);
            if (innerErrorCode == 0) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };
    napi_value lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUIExtensionContentSession::OnStartAbilityAsCaller",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NapiAsyncTask::ExecuteCallback JsUIExtensionContentSession::StartAbilityExecuteCallback(AAFwk::Want& want,
    size_t& unwrapArgc, napi_env env, NapiCallbackInfo& info, std::shared_ptr<int> &innerErrorCode)
{
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[1], napi_object)) {
        HILOG_DEBUG("OnStartAbility start options is used.");
        AppExecFwk::UnwrapStartOptions(env, info.argv[1], startOptions);
        unwrapArgc++;
    }

    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    }
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, startOptions, unwrapArgc,
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

napi_value JsUIExtensionContentSession::OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info)
{
    CHECK_IS_SYSTEM_APP;
    if (info.argc == ARGC_ZERO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[0], want)) {
        HILOG_ERROR("Error to parse want!");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    if (!want.HasParameter(Want::PARAM_BACK_TO_OTHER_MISSION_STACK)) {
        want.SetParam(Want::PARAM_BACK_TO_OTHER_MISSION_STACK, true);
    }
    size_t unwrapArgc = 1;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[1], napi_object)) {
        HILOG_DEBUG("OnStartAbilityForResult start options is used.");
        AppExecFwk::UnwrapStartOptions(env, info.argv[1], startOptions);
        unwrapArgc++;
    }

    napi_value lastParam = info.argc > unwrapArgc ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, lastParam, true);
    }
    std::unique_ptr<NapiAsyncTask> uasyncTask =
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    if (asyncTask == nullptr) {
        HILOG_ERROR("asyncTask is nullptr");
        return CreateJsUndefined(env);
    }
    if (listener_ == nullptr) {
        HILOG_ERROR("listener_ is nullptr");
        return CreateJsUndefined(env);
    }
    StartAbilityForResultRuntimeTask(env, want, asyncTask, unwrapArgc, startOptions);
    return result;
}

void JsUIExtensionContentSession::StartAbilityForResultRuntimeTask(napi_env env,
    AAFwk::Want &want, std::shared_ptr<NapiAsyncTask> asyncTask, size_t& unwrapArgc,
    AAFwk::StartOptions startOptions)
{
    if (asyncTask == nullptr) {
        HILOG_ERROR("asyncTask is nullptr");
        return;
    }
    RuntimeTask task = [env, asyncTask, &observer = freeInstallObserver_](int resultCode,
        const AAFwk::Want& want, bool isInner) {
        HILOG_DEBUG("OnStartAbilityForResult async callback is enter");
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            HILOG_WARN("wrap abilityResult wrong");
            asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
        } else {
            if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
                resultCode != 0 && observer != nullptr) {
                std::string bundleName = want.GetElement().GetBundleName();
                std::string abilityName = want.GetElement().GetAbilityName();
                std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
                observer->OnInstallFinished(bundleName, abilityName, startTime,
                    static_cast<int>(GetJsErrorCodeByNativeError(resultCode)));
            } else if (isInner) {
                asyncTask->Reject(env, CreateJsErrorByNativeErr(env, resultCode));
            } else {
                asyncTask->ResolveWithNoError(env, abilityResult);
            }
        }
    };
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_WARN("context is released");
        asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    } else {
        want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
        int curRequestCode = reinterpret_cast<UIExtensionContext*>(context.get())->GenerateCurRequestCode();
        if (listener_ == nullptr) {
            HILOG_ERROR("listener_ is nullptr");
            return;
        }
        listener_->SaveResultCallbacks(curRequestCode, std::move(task));
        ErrCode err = (unwrapArgc == 1) ?
            AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByUIContentSession(want,
                context->GetToken(), sessionInfo_, curRequestCode, -1) :
            AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByUIContentSession(want,
                startOptions, context->GetToken(), sessionInfo_, curRequestCode, -1);
        if (err != ERR_OK && err != AAFwk::START_ABILITY_WAITING) {
            HILOG_ERROR("StartAbilityForResult. ret:%{public}d", err);
            listener_->OnAbilityResultInner(curRequestCode, err, want);
        }
    }
}

napi_value JsUIExtensionContentSession::OnTerminateSelf(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("OnTerminateSelf called");
    NapiAsyncTask::CompleteCallback complete =
        [sessionInfo = sessionInfo_](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (sessionInfo == nullptr) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo);
            if (errorCode == 0) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errorCode));
            }
        };

    napi_value lastParam = (info.argc > ARGC_ZERO) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUIExtensionContentSession::OnTerminateSelf",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsUIExtensionContentSession::OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("invalid param");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    int resultCode = 0;
    AAFwk::Want want;
    if (!AppExecFwk::UnWrapAbilityResult(env, info.argv[INDEX_ZERO], resultCode, want)) {
        HILOG_ERROR("OnTerminateSelfWithResult Failed to parse ability result!");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [uiWindow = uiWindow_, sessionInfo = sessionInfo_, want, resultCode](napi_env env,
            NapiAsyncTask& task, int32_t status) {
            if (uiWindow == nullptr) {
                HILOG_ERROR("uiWindow is nullptr.");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto ret = uiWindow->TransferAbilityResult(resultCode, want);
            if (ret != Rosen::WMError::WM_OK) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo);
            if (errorCode == 0) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errorCode));
            }
        };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUIExtensionContentSession::OnTerminateSelfWithResult",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsUIExtensionContentSession::OnSendData(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called");
    CHECK_IS_SYSTEM_APP;
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("invalid param");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    AAFwk::WantParams params;
    if (!AppExecFwk::UnwrapWantParams(env, info.argv[INDEX_ZERO], params)) {
        HILOG_ERROR("OnSendData Failed to parse param!");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    if (uiWindow_ == nullptr) {
        HILOG_ERROR("uiWindow_ is nullptr");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    Rosen::WMError ret = uiWindow_->TransferExtensionData(params);
    if (ret == Rosen::WMError::WM_OK) {
        HILOG_DEBUG("TransferExtensionData success");
    } else {
        HILOG_ERROR("TransferExtensionData failed, ret=%{public}d", ret);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return CreateJsUndefined(env);
}

napi_value JsUIExtensionContentSession::OnSetReceiveDataCallback(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called");
    CHECK_IS_SYSTEM_APP;
    if (info.argc < ARGC_ONE || !CheckTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_function)) {
        HILOG_ERROR("invalid param");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    if (!isRegistered) {
        if (uiWindow_ == nullptr) {
            HILOG_ERROR("uiWindow_ is nullptr");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        receiveDataCallback_ = std::make_shared<CallbackWrapper>();
        std::weak_ptr<CallbackWrapper> weakCallback(receiveDataCallback_);
        auto handler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
        uiWindow_->RegisterTransferComponentDataListener([env, handler, weakCallback](
            const AAFwk::WantParams& wantParams) {
            if (handler) {
                handler->PostTask([env, weakCallback, wantParams]() {
                    JsUIExtensionContentSession::CallReceiveDataCallback(env, weakCallback, wantParams);
                    }, "JsUIExtensionContentSession:OnSetReceiveDataCallback");
            }
        });
        isRegistered = true;
    }

    napi_value callback = info.argv[INDEX_ZERO];
    if (receiveDataCallback_ == nullptr) {
        HILOG_ERROR("uiWindow_ is nullptr");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    napi_ref ref = nullptr;
    napi_create_reference(env, callback, 1, &ref);
    receiveDataCallback_->ResetCallback(std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    return CreateJsUndefined(env);
}

napi_value JsUIExtensionContentSession::OnSetReceiveDataForResultCallback(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called");
    CHECK_IS_SYSTEM_APP;
    if (info.argc < ARGC_ONE || !CheckTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_function)) {
        HILOG_ERROR("invalid param");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    if (!isSyncRegistered) {
        if (uiWindow_ == nullptr) {
            HILOG_ERROR("uiWindow_ is nullptr");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        receiveDataForResultCallback_ = std::make_shared<CallbackWrapper>();
        std::weak_ptr<CallbackWrapper> weakCallback(receiveDataForResultCallback_);
        auto handler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
        uiWindow_->RegisterTransferComponentDataForResultListener([env, handler, weakCallback] (
            const AAFwk::WantParams& wantParams) -> AAFwk::WantParams {
                AAFwk::WantParams retWantParams;
                if (handler) {
                    handler->PostSyncTask([env, weakCallback, wantParams, &retWantParams]() {
                        JsUIExtensionContentSession::CallReceiveDataCallbackForResult(env, weakCallback,
                            wantParams, retWantParams);
                        }, "JsUIExtensionContentSession:OnSetReceiveDataForResultCallback");
                }
                return retWantParams;
        });
        isSyncRegistered = true;
    }
    napi_value callback = info.argv[INDEX_ZERO];
    if (receiveDataForResultCallback_ == nullptr) {
        HILOG_ERROR("receiveDataForResultCallback_ is nullptr");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    napi_ref ref = nullptr;
    napi_create_reference(env, callback, 1, &ref);
    receiveDataForResultCallback_->ResetCallback(
        std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));

    return CreateJsUndefined(env);
}

napi_value JsUIExtensionContentSession::OnLoadContent(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called");
    std::string contextPath;
    if (info.argc < ARGC_ONE || !ConvertFromJsValue(env, info.argv[INDEX_ZERO], contextPath)) {
        HILOG_ERROR("invalid param");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    HILOG_DEBUG("contextPath: %{public}s", contextPath.c_str());
    napi_value storage = nullptr;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        storage = info.argv[INDEX_ONE];
    }
    if (uiWindow_ == nullptr) {
        HILOG_ERROR("uiWindow_ is nullptr");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    sptr<IRemoteObject> parentToken = sessionInfo_->parentToken;
    Rosen::WMError ret = uiWindow_->NapiSetUIContent(contextPath, env, storage, false, parentToken);
    if (ret == Rosen::WMError::WM_OK) {
        HILOG_DEBUG("NapiSetUIContent success");
    } else {
        HILOG_ERROR("NapiSetUIContent failed, ret=%{public}d", ret);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return CreateJsUndefined(env);
}

napi_value JsUIExtensionContentSession::OnSetWindowBackgroundColor(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called");
    CHECK_IS_SYSTEM_APP;
    std::string color;
    if (info.argc < ARGC_ONE || !ConvertFromJsValue(env, info.argv[INDEX_ZERO], color)) {
        HILOG_ERROR("invalid param");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    if (uiWindow_ == nullptr) {
        HILOG_ERROR("uiWindow_ is nullptr");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    Rosen::WMError ret = uiWindow_->SetBackgroundColor(color);
    if (ret == Rosen::WMError::WM_OK) {
        HILOG_DEBUG("SetBackgroundColor success");
    } else {
        HILOG_ERROR("SetBackgroundColor failed, ret=%{public}d", ret);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return CreateJsUndefined(env);
}

napi_value JsUIExtensionContentSession::OnSetWindowPrivacyMode(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called");
    bool isPrivacyMode = false;
    if (info.argc < ARGC_ONE || !ConvertFromJsValue(env, info.argv[INDEX_ZERO], isPrivacyMode)) {
        HILOG_ERROR("invalid param");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    int ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(selfToken, PERMISSION_PRIVACY_WINDOW);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        ThrowNoPermissionError(env, PERMISSION_PRIVACY_WINDOW);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [uiWindow = uiWindow_, isPrivacyMode](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (uiWindow == nullptr) {
                HILOG_ERROR("uiWindow is null");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto ret = uiWindow->SetPrivacyMode(isPrivacyMode);
            if (ret == Rosen::WMError::WM_OK) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            }
        };
    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsUIExtensionContentSession::OnSetWindowPrivacyMode",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsUIExtensionContentSession::OnStartAbilityByType(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("called");
    if (info.argc < ARGC_THREE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string type;
    AAFwk::WantParams wantParam;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], type) ||
        !AppExecFwk::UnwrapWantParams(env, info.argv[INDEX_ONE], wantParam)) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    wantParam.SetParam(UIEXTENSION_TARGET_TYPE_KEY, AAFwk::String::Box(type));
    AAFwk::Want want;
    want.SetParams(wantParam);
    if (wantParam.HasParam(FLAG_AUTH_READ_URI_PERMISSION)) {
        int32_t flag = wantParam.GetIntParam(FLAG_AUTH_READ_URI_PERMISSION, 0);
        want.SetFlags(flag);
        wantParam.Remove(FLAG_AUTH_READ_URI_PERMISSION);
    }
    std::shared_ptr<JsUIExtensionCallback> uiExtensionCallback = std::make_shared<JsUIExtensionCallback>(env);
    uiExtensionCallback->SetJsCallbackObject(info.argv[INDEX_TWO]);
    NapiAsyncTask::CompleteCallback complete = [uiWindow = uiWindow_, type, want, uiExtensionCallback]
        (napi_env env, NapiAsyncTask& task, int32_t status) {
            if (uiWindow == nullptr || uiWindow->GetUIContent() == nullptr) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            Ace::ModalUIExtensionCallbacks callback;
            callback.onError = std::bind(&JsUIExtensionCallback::OnError, uiExtensionCallback, std::placeholders::_1);
            callback.onRelease = std::bind(&JsUIExtensionCallback::OnRelease,
                uiExtensionCallback, std::placeholders::_1);
            Ace::ModalUIExtensionConfig config;
            auto uiContent = uiWindow->GetUIContent();
            int32_t sessionId = uiContent->CreateModalUIExtension(want, callback, config);
            if (sessionId == 0) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            } else {
                uiExtensionCallback->SetUIContent(uiContent);
                uiExtensionCallback->SetSessionId(sessionId);
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            }
        };

    napi_value lastParam = (info.argc > ARGC_THREE) ? info.argv[INDEX_THREE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUIExtensionContentSession::OnStartAbilityByType",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsUIExtensionContentSession::CreateJsUIExtensionContentSession(napi_env env,
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow,
    std::weak_ptr<AbilityRuntime::Context> context,
    std::shared_ptr<AbilityResultListeners>& abilityResultListeners)
{
    HILOG_DEBUG("start");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("object is null");
        return CreateJsUndefined(env);
    }

    std::unique_ptr<JsUIExtensionContentSession> jsSession =
        std::make_unique<JsUIExtensionContentSession>(sessionInfo, uiWindow, context, abilityResultListeners);
    napi_wrap(env, object, jsSession.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsUIExtensionContentSession";
    BindNativeFunction(env, object, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(env, object, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(env, object, "sendData", moduleName, SendData);
    BindNativeFunction(env, object, "setReceiveDataCallback", moduleName, SetReceiveDataCallback);
    BindNativeFunction(env, object, "setReceiveDataForResultCallback", moduleName, SetReceiveDataForResultCallback);
    BindNativeFunction(env, object, "loadContent", moduleName, LoadContent);
    BindNativeFunction(env, object, "setWindowBackgroundColor", moduleName, SetWindowBackgroundColor);
    BindNativeFunction(env, object, "setWindowPrivacyMode", moduleName, SetWindowPrivacyMode);
    BindNativeFunction(env, object, "startAbility", moduleName, StartAbility);
    BindNativeFunction(env, object, "startAbilityForResult", moduleName, StartAbilityForResult);
    BindNativeFunction(env, object, "startAbilityByType", moduleName, StartAbilityByType);
    BindNativeFunction(env, object, "startAbilityAsCaller", moduleName, StartAbilityAsCaller);
    BindNativeFunction(env, object, "getUIExtensionHostWindowProxy", moduleName, GetUIExtensionHostWindowProxy);
    return object;
}

napi_value JsUIExtensionContentSession::CreateJsUIExtensionContentSession(napi_env env,
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow)
{
    HILOG_DEBUG("begin");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr");
        return CreateJsUndefined(env);
    }

    std::unique_ptr<JsUIExtensionContentSession> jsSession =
        std::make_unique<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    napi_wrap(env, object, jsSession.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsUIExtensionContentSession";
    BindNativeFunction(env, object, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(env, object, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(env, object, "sendData", moduleName, SendData);
    BindNativeFunction(env, object, "setReceiveDataCallback", moduleName, SetReceiveDataCallback);
    BindNativeFunction(env, object, "setReceiveDataForResultCallback", moduleName, SetReceiveDataForResultCallback);
    BindNativeFunction(env, object, "loadContent", moduleName, LoadContent);
    BindNativeFunction(env, object, "setWindowBackgroundColor", moduleName, SetWindowBackgroundColor);
    BindNativeFunction(env, object, "setWindowPrivacyMode", moduleName, SetWindowPrivacyMode);
    BindNativeFunction(env, object, "startAbility", moduleName, StartAbility);
    BindNativeFunction(env, object, "startAbilityForResult", moduleName, StartAbilityForResult);
    BindNativeFunction(env, object, "startAbilityByType", moduleName, StartAbilityByType);
    BindNativeFunction(env, object, "startAbilityAsCaller", moduleName, StartAbilityAsCaller);
    BindNativeFunction(env, object, "getUIExtensionHostWindowProxy", moduleName, GetUIExtensionHostWindowProxy);
    return object;
}

void JsUIExtensionContentSession::CallReceiveDataCallback(napi_env env,
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
    napi_value method = callback->GetNapiValue();
    if (method == nullptr) {
        HILOG_WARN("method is nullptr");
        return;
    }
    HandleScope handleScope(env);
    napi_value napiWantParams = AppExecFwk::WrapWantParams(env, wantParams);
    if (napiWantParams == nullptr) {
        HILOG_ERROR("napiWantParams is nullptr");
        return;
    }
    napi_value argv[] = {napiWantParams};
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_call_function(env, global, method, ARGC_ONE, argv, nullptr);
}

void JsUIExtensionContentSession::CallReceiveDataCallbackForResult(napi_env env,
    std::weak_ptr<CallbackWrapper> weakCallback, const AAFwk::WantParams& wantParams, AAFwk::WantParams& retWantParams)
{
    auto cbWrapper = weakCallback.lock();
    if (cbWrapper == nullptr) {
        HILOG_ERROR("cbWrapper is nullptr");
        return;
    }
    auto callback = cbWrapper->GetCallback();
    if (callback == nullptr) {
        HILOG_ERROR("callback is nullptr");
        return;
    }
    napi_value method = reinterpret_cast<napi_value>(callback->Get());
    if (method == nullptr) {
        HILOG_ERROR("method is nullptr");
        return;
    }
    HandleScope handleScope(env);
    napi_value napiWantParams = AppExecFwk::WrapWantParams(env, wantParams);
    if (napiWantParams == nullptr) {
        HILOG_ERROR("napiWantParams is nullptr");
        return;
    }
    napi_value argv[] = {napiWantParams};
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_value ret = nullptr;
    napi_call_function(env, global, method, ARGC_ONE, argv, &ret);
    if (ret == nullptr) {
        HILOG_ERROR("ret is nullptr");
        return;
    }

    if (!AppExecFwk::UnwrapWantParams(env, ret, retWantParams)) {
        HILOG_ERROR("Failed to parse param!");
        return;
    }
}

void JsUIExtensionContentSession::AddFreeInstallObserver(napi_env env,
    const AAFwk::Want &want, napi_value callback, bool isAbilityResult)
{
    // adapter free install async return install and start result
    int ret = 0;
    if (freeInstallObserver_ == nullptr) {
        freeInstallObserver_ = new JsFreeInstallObserver(env);
        ret = AAFwk::AbilityManagerClient::GetInstance()->AddFreeInstallObserver(freeInstallObserver_);
    }

    if (ret != ERR_OK) {
        HILOG_ERROR("AddFreeInstallObserver failed");
    } else {
        HILOG_INFO("AddJsObserverObject.");
        // build a callback observer with last param
        std::string bundleName = want.GetElement().GetBundleName();
        std::string abilityName = want.GetElement().GetAbilityName();
        std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
        freeInstallObserver_->AddJsObserverObject(
            bundleName, abilityName, startTime, callback, isAbilityResult);
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS
