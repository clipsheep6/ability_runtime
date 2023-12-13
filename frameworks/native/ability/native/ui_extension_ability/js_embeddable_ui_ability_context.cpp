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

#include "js_embeddable_ui_ability_context.h"

#include <chrono>
#include <cstdint>

#include "ability_manager_client.h"
#include "event_handler.h"
#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_common_start_options.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {

JsEmbeddableUIAbilityContext::JsEmbeddableUIAbilityContext(const std::shared_ptr<AbilityContext>& uiContext,
    const std::shared_ptr<UIExtensionContext>& uiExtContext, bool isHalfScreenMode)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    jsAbilityContext_ = std::make_shared<JsAbilityContext>(uiContext);
    jsUIExtensionContext_ = std::make_shared<JsUIExtensionContext>(uiExtContext);
    isHalfScreenMode_ = isHalfScreenMode;
}

void JsEmbeddableUIAbilityContext::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    std::unique_ptr<JsEmbeddableUIAbilityContext>(static_cast<JsEmbeddableUIAbilityContext*>(data));
}

napi_value JsEmbeddableUIAbilityContext::StartAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
     GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbility);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityForResult(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityForResult);
}

napi_value JsEmbeddableUIAbilityContext::ConnectAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnConnectAbility);
}

napi_value JsEmbeddableUIAbilityContext::DisconnectAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnDisconnectAbility);
}

napi_value JsEmbeddableUIAbilityContext::TerminateSelf(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnTerminateSelf);
}

napi_value JsEmbeddableUIAbilityContext::TerminateSelfWithResult(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnTerminateSelfWithResult);
}
//  不支持以返回errcode形式处理
napi_value JsEmbeddableUIAbilityContext::StartAbilityAsCaller(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityAsCaller);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityWithAccount(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityByCall(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityByCall);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityForResultWithAccount(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityForResultWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::StartServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartExtensionAbility);
}

napi_value JsEmbeddableUIAbilityContext::StartServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartExtensionAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::StopServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStopExtensionAbility);
}

napi_value JsEmbeddableUIAbilityContext::StopServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStopExtensionAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::ConnectAbilityWithAccount(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnConnectAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::RestoreWindowStage(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnRestoreWindowStage);
}

napi_value JsEmbeddableUIAbilityContext::IsTerminating(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnIsTerminating);
}

napi_value JsEmbeddableUIAbilityContext::StartRecentAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartRecentAbility);
}

napi_value JsEmbeddableUIAbilityContext::RequestDialogService(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnRequestDialogService);
}

napi_value JsEmbeddableUIAbilityContext::ReportDrawnCompleted(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnReportDrawnCompleted);
}

napi_value JsEmbeddableUIAbilityContext::SetMissionContinueState(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnSetMissionContinueState);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityByType(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityByType);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        HILOG_INFO("Half Screen StartMode");
        return jsUIExtensionContext_->OnStartAbility(env, info);
    } else {
        return jsAbilityContext_->OnStartAbility(env, info);
    }
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        HILOG_INFO("Half Screen StartMode");
        return nullptr;
        return jsUIExtensionContext_->OnStartAbilityForResult(env, info);
    } else {
        return jsAbilityContext_->OnStartAbilityForResult(env, info);
    }
}

napi_value JsEmbeddableUIAbilityContext::OnConnectAbility(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        HILOG_INFO("Half Screen StartMode");
        return nullptr;
        return jsUIExtensionContext_->OnConnectAbility(env, info);
    } else {
        return jsAbilityContext_->OnConnectAbility(env, info);
    }
}

napi_value JsEmbeddableUIAbilityContext::OnDisconnectAbility(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        HILOG_INFO("Half Screen StartMode");
        return nullptr;
        return jsUIExtensionContext_->OnDisconnectAbility(env, info);
    } else {
        return jsAbilityContext_->OnDisconnectAbility(env, info);
    }
}

napi_value JsEmbeddableUIAbilityContext::OnTerminateSelf(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        HILOG_INFO("Half Screen StartMode");
        return nullptr;
        // TODO:半屏
        // return jsUIExtensionContext_->OnTerminateSelf(env, info);
    } else {
        return jsAbilityContext_->OnTerminateSelf(env, info);
    }
}

napi_value JsEmbeddableUIAbilityContext::OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        HILOG_INFO("Half Screen StartMode");
        return nullptr;
        // TODO:半屏
        // jsUIExtensionContext_->OnTerminateSelfWithResult(env, info);
    } else {
        return jsAbilityContext_->OnTerminateSelfWithResult(env, info);
    }
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityAsCaller(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnStartAbilityAsCaller(env, info);
    }
}
napi_value JsEmbeddableUIAbilityContext::OnStartAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnStartAbilityWithAccount(env, info);
    }
}
napi_value JsEmbeddableUIAbilityContext::OnStartAbilityByCall(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnStartAbilityByCall(env, info);
    }
}
napi_value JsEmbeddableUIAbilityContext::OnStartAbilityForResultWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnStartAbilityForResultWithAccount(env, info);
    }
}
napi_value JsEmbeddableUIAbilityContext::OnStartExtensionAbility(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnStartExtensionAbility(env, info);
    }
}
napi_value JsEmbeddableUIAbilityContext::OnStartExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnStartExtensionAbilityWithAccount(env, info);
    }
}
napi_value JsEmbeddableUIAbilityContext::OnStopExtensionAbility(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnStopExtensionAbility(env, info);
    }
}
napi_value JsEmbeddableUIAbilityContext::OnStopExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnStopExtensionAbilityWithAccount(env, info);
    }
}

napi_value JsEmbeddableUIAbilityContext::OnConnectAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnConnectAbilityWithAccount(env, info);
    }
}

napi_value JsEmbeddableUIAbilityContext::OnRestoreWindowStage(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnRestoreWindowStage(env, info);
    }
}
napi_value JsEmbeddableUIAbilityContext::OnIsTerminating(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnIsTerminating(env, info);
    }
}
napi_value JsEmbeddableUIAbilityContext::OnStartRecentAbility(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnStartRecentAbility(env, info);
    }
}
napi_value JsEmbeddableUIAbilityContext::OnRequestDialogService(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnRequestDialogService(env, info);
    }
}
napi_value JsEmbeddableUIAbilityContext::OnReportDrawnCompleted(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnReportDrawnCompleted(env, info);
    }
}
napi_value JsEmbeddableUIAbilityContext::OnSetMissionContinueState(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnSetMissionContinueState(env, info);
    }
}
napi_value JsEmbeddableUIAbilityContext::OnStartAbilityByType(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO:errcode
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnStartAbilityByType(env, info);
    }
}

#ifdef SUPPORT_GRAPHICS
napi_value JsEmbeddableUIAbilityContext::SetMissionLabel(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnSetMissionLabel);
}

napi_value JsEmbeddableUIAbilityContext::SetMissionIcon(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnSetMissionIcon);
}

napi_value JsEmbeddableUIAbilityContext::OnSetMissionLabel(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO : Errcode处理
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnSetMissionLabel(env, info);
    }
}

napi_value JsEmbeddableUIAbilityContext::OnSetMissionIcon(napi_env env, NapiCallbackInfo& info)
{
    if (isHalfScreenMode_) {
        return nullptr;
        //TODO : Errcode处理
        //return CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        return jsAbilityContext_->OnSetMissionIcon(env, info);
    }
}
#endif

napi_value JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(napi_env env,
    std::shared_ptr<AbilityContext> uiContext, std::shared_ptr<UIExtensionContext> uiExtContext, int isHalfScreenMode)
{
    HILOG_DEBUG("CreateJsEmbeddableUIAbilityContext begin");
    if (uiContext == nullptr && uiExtContext == nullptr) {
        HILOG_ERROR("Contexts are both null.");
        return nullptr;
    }
    napi_value objValue = nullptr;
    //TODO: 判断有点挫, 待改进
    if (isHalfScreenMode == 0) {
        objValue = CreateJsBaseContext(env, uiContext);
        std::unique_ptr<JsEmbeddableUIAbilityContext> jsContext = std::make_unique<JsEmbeddableUIAbilityContext>(
            uiContext, nullptr, isHalfScreenMode);
        napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);

        auto abilityInfo = uiContext->GetAbilityInfo();
        if (abilityInfo != nullptr) {
            napi_set_named_property(env, objValue, "abilityInfo", CreateJsAbilityInfo(env, *abilityInfo));
        }

        auto configuration = uiContext->GetConfiguration();
        if (configuration != nullptr) {
            napi_set_named_property(env, objValue, "config", CreateJsConfiguration(env, *configuration));
        }
    } else if (isHalfScreenMode == 1) {
        objValue = CreateJsBaseContext(env, uiExtContext);
        std::unique_ptr<JsEmbeddableUIAbilityContext> jsContext = std::make_unique<JsEmbeddableUIAbilityContext>(
            nullptr , uiExtContext, isHalfScreenMode);
        napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);

        auto abilityInfo = uiExtContext->GetAbilityInfo();
        if (abilityInfo != nullptr) {
            napi_set_named_property(env, objValue, "abilityInfo", CreateJsAbilityInfo(env, *abilityInfo));
        }

        auto configuration = uiExtContext->GetConfiguration();
        if (configuration != nullptr) {
            napi_set_named_property(env, objValue, "config", CreateJsConfiguration(env, *configuration));
        }
    }

    const char *moduleName = "JsEmbeddableUIAbilityContext";
    BindNativeFunction(env, objValue, "startAbility", moduleName, StartAbility);
    BindNativeFunction(env, objValue, "startAbilityForResult", moduleName, StartAbilityForResult);
    BindNativeFunction(env, objValue, "connectServiceExtensionAbility", moduleName, ConnectAbility);
    BindNativeFunction(env, objValue, "disconnectServiceExtensionAbility", moduleName, DisconnectAbility);
    BindNativeFunction(env, objValue, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(env, objValue, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    // 返回errcode形式处理
    BindNativeFunction(env, objValue, "startAbilityAsCaller", moduleName, StartAbilityAsCaller);
    BindNativeFunction(env, objValue, "startAbilityWithAccount", moduleName, StartAbilityWithAccount);
    BindNativeFunction(env, objValue, "startAbilityByCall", moduleName, StartAbilityByCall);
    BindNativeFunction(env, objValue, "startAbilityForResultWithAccount", moduleName, StartAbilityForResultWithAccount);
    BindNativeFunction(env, objValue, "startServiceExtensionAbility", moduleName, StartServiceExtensionAbility);
    BindNativeFunction(env, objValue, "startServiceExtensionAbilityWithAccount", moduleName,
        StartServiceExtensionAbilityWithAccount);
    BindNativeFunction(env, objValue, "stopServiceExtensionAbility", moduleName, StopServiceExtensionAbility);
    BindNativeFunction(env, objValue, "stopServiceExtensionAbilityWithAccount", moduleName,
        StopServiceExtensionAbilityWithAccount);
    BindNativeFunction(env, objValue, "connectServiceExtensionAbilityWithAccount", moduleName, ConnectAbilityWithAccount);
    BindNativeFunction(env, objValue, "restoreWindowStage", moduleName, RestoreWindowStage);
    BindNativeFunction(env, objValue, "isTerminating", moduleName, IsTerminating);
    BindNativeFunction(env, objValue, "startRecentAbility", moduleName, StartRecentAbility);
    BindNativeFunction(env, objValue, "requestDialogService", moduleName, RequestDialogService);
    BindNativeFunction(env, objValue, "reportDrawnCompleted", moduleName, ReportDrawnCompleted);
    BindNativeFunction(env, objValue, "setMissionContinueState", moduleName, SetMissionContinueState);
    BindNativeFunction(env, objValue, "startAbilityByType", moduleName, StartAbilityByType);

#ifdef SUPPORT_GRAPHICS
    BindNativeFunction(env, objValue, "setMissionLabel", moduleName, SetMissionLabel);
    BindNativeFunction(env, objValue, "setMissionIcon", moduleName, SetMissionIcon);
#endif
    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS