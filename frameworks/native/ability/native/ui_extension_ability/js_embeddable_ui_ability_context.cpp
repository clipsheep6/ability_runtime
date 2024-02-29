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
#include <string>

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
#define CHECK_POINTER_RETURN(env, object)            \
    if (!(object)) {                                 \
        HILOG_ERROR("Context is nullptr");           \
        return CreateJsUndefined(env);               \
    }

namespace {
const std::string ERR_MSG_NOT_SUPPORT = "Not support the interface in embedded screen mode of atomic service.";
}

JsEmbeddableUIAbilityContext::JsEmbeddableUIAbilityContext(const std::shared_ptr<AbilityContext>& uiAbiContext,
    const std::shared_ptr<UIExtensionContext>& uiExtContext, int32_t screenMode)
{
    jsAbilityContext_ = std::make_shared<JsAbilityContext>(uiAbiContext);
    jsUIExtensionContext_ = std::make_shared<JsUIExtensionContext>(uiExtContext);
    screenMode_ = screenMode;
}

void JsEmbeddableUIAbilityContext::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_DEBUG("The Finalizer of embeddable UI ability context is called.");
    std::unique_ptr<JsEmbeddableUIAbilityContext>(static_cast<JsEmbeddableUIAbilityContext*>(data));
}

napi_value JsEmbeddableUIAbilityContext::StartAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbility);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityForResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityForResult);
}

napi_value JsEmbeddableUIAbilityContext::ConnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnConnectAbility);
}

napi_value JsEmbeddableUIAbilityContext::DisconnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnDisconnectAbility);
}

napi_value JsEmbeddableUIAbilityContext::TerminateSelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnTerminateSelf);
}

napi_value JsEmbeddableUIAbilityContext::TerminateSelfWithResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnTerminateSelfWithResult);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityAsCaller(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityAsCaller);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityByCall(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityByCall);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityForResultWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityForResultWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::StartServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartExtensionAbility);
}

napi_value JsEmbeddableUIAbilityContext::StartServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartExtensionAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::StopServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStopExtensionAbility);
}

napi_value JsEmbeddableUIAbilityContext::StopServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStopExtensionAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::ConnectAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnConnectAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::RestoreWindowStage(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnRestoreWindowStage);
}

napi_value JsEmbeddableUIAbilityContext::IsTerminating(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnIsTerminating);
}

napi_value JsEmbeddableUIAbilityContext::StartRecentAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartRecentAbility);
}

napi_value JsEmbeddableUIAbilityContext::RequestDialogService(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnRequestDialogService);
}

napi_value JsEmbeddableUIAbilityContext::ReportDrawnCompleted(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnReportDrawnCompleted);
}

napi_value JsEmbeddableUIAbilityContext::SetMissionContinueState(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnSetMissionContinueState);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityByType(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityByType);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_INFO("Start ability in embedded screen mode.");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnStartAbility(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_INFO("Start ability for result in embedded screen mode.");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnStartAbilityForResult(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartAbilityForResult(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnConnectAbility(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_INFO("Connect ability in embedded screen mode.");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnConnectAbility(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnConnectAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnDisconnectAbility(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_INFO("Disconnect ability in embedded screen mode.");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnDisconnectAbility(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnDisconnectAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnTerminateSelf(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_INFO("TerminateSelf in embedded screen mode.");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnTerminateSelf(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnTerminateSelf(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_INFO("TerminateSelfWithResult ability in embedded screen mode.");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnTerminateSelfWithResult(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnTerminateSelfWithResult(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityAsCaller(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Start ability as caller in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartAbilityAsCaller(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Start ability with account in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartAbilityWithAccount(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityByCall(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Start ability by caller in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartAbilityByCall(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityForResultWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Start ability for result in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartAbilityForResultWithAccount(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartExtensionAbility(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Start extension in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartExtensionAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Start extensionin with account in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartExtensionAbilityWithAccount(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStopExtensionAbility(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Stop extensionin in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStopExtensionAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStopExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Stop extensionin with account in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStopExtensionAbilityWithAccount(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnConnectAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Connect ability with account in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnConnectAbilityWithAccount(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnRestoreWindowStage(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Restore window stage with account in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnRestoreWindowStage(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnIsTerminating(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Get terminating state in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnIsTerminating(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartRecentAbility(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Start recent ability in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartRecentAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnRequestDialogService(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Request dialog service in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnRequestDialogService(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnReportDrawnCompleted(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_INFO("Report Drawn Completed in half screen mode.");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnReportDrawnCompleted(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnReportDrawnCompleted(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnSetMissionContinueState(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Set mission continue state in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnSetMissionContinueState(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityByType(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Start ability by type in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartAbilityByType(env, info);
}

#ifdef SUPPORT_GRAPHICS
napi_value JsEmbeddableUIAbilityContext::SetMissionLabel(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Set mission label is called.");
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnSetMissionLabel);
}

napi_value JsEmbeddableUIAbilityContext::SetMissionIcon(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Set mission icon is called.");
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnSetMissionIcon);
}

napi_value JsEmbeddableUIAbilityContext::OnSetMissionLabel(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Set mission label in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnSetMissionLabel(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnSetMissionIcon(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        HILOG_ERROR("Set mission icon in embedded screen mode.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnSetMissionIcon(env, info);
}
#endif

void JsEmbeddableUIAbilityContext::WrapJsUIAbilityContext(napi_env env,
    std::shared_ptr<AbilityContext> uiAbiContext, napi_value &objValue, int32_t screenMode)
{
    if (uiAbiContext == nullptr) {
        HILOG_ERROR("UI ability context is nullptr");
        return;
    }
    objValue = CreateJsBaseContext(env, uiAbiContext);
    std::unique_ptr<JsEmbeddableUIAbilityContext> jsContext = std::make_unique<JsEmbeddableUIAbilityContext>(
        uiAbiContext, nullptr, screenMode);
    napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);

    auto abilityInfo = uiAbiContext->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        napi_set_named_property(env, objValue, "abilityInfo", CreateJsAbilityInfo(env, *abilityInfo));
    }

    auto configuration = uiAbiContext->GetConfiguration();
    if (configuration != nullptr) {
        napi_set_named_property(env, objValue, "config", CreateJsConfiguration(env, *configuration));
    }
}

void JsEmbeddableUIAbilityContext::WrapJsUIExtensionContext(napi_env env,
    std::shared_ptr<UIExtensionContext> uiExtContext, napi_value &objValue, int32_t screenMode)
{
    if (uiExtContext == nullptr) {
        HILOG_ERROR("UI extension context is nullptr");
        return;
    }
    objValue = CreateJsBaseContext(env, uiExtContext);
    std::unique_ptr<JsEmbeddableUIAbilityContext> jsContext = std::make_unique<JsEmbeddableUIAbilityContext>(
        nullptr, uiExtContext, screenMode);
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

napi_value JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(napi_env env,
    std::shared_ptr<AbilityContext> uiAbiContext, std::shared_ptr<UIExtensionContext> uiExtContext, int32_t screenMode)
{
    HILOG_DEBUG("Create JS embeddable UIAbility context begin.");
    napi_value objValue = nullptr;
    if (screenMode == AAFwk::JUMP_SCREEN_MODE) {
        WrapJsUIAbilityContext(env, uiAbiContext, objValue, screenMode);
    } else if (screenMode == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        WrapJsUIExtensionContext(env, uiExtContext, objValue, screenMode);
    }

    const char* moduleName = "JsEmbeddableUIAbilityContext";
    BindNativeFunction(env, objValue, "startAbility", moduleName, StartAbility);
    BindNativeFunction(env, objValue, "startAbilityForResult", moduleName, StartAbilityForResult);
    BindNativeFunction(env, objValue, "connectServiceExtensionAbility", moduleName, ConnectAbility);
    BindNativeFunction(env, objValue, "disconnectServiceExtensionAbility", moduleName, DisconnectAbility);
    BindNativeFunction(env, objValue, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(env, objValue, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(env, objValue, "startAbilityAsCaller", moduleName, StartAbilityAsCaller);
    BindNativeFunction(env, objValue, "startAbilityWithAccount", moduleName, StartAbilityWithAccount);
    BindNativeFunction(env, objValue, "startAbilityByCall", moduleName, StartAbilityByCall);
    BindNativeFunction(env, objValue, "startAbilityForResultWithAccount", moduleName,
        StartAbilityForResultWithAccount);
    BindNativeFunction(env, objValue, "startServiceExtensionAbility", moduleName, StartServiceExtensionAbility);
    BindNativeFunction(env, objValue, "startServiceExtensionAbilityWithAccount", moduleName,
        StartServiceExtensionAbilityWithAccount);
    BindNativeFunction(env, objValue, "stopServiceExtensionAbility", moduleName, StopServiceExtensionAbility);
    BindNativeFunction(env, objValue, "stopServiceExtensionAbilityWithAccount", moduleName,
        StopServiceExtensionAbilityWithAccount);
    BindNativeFunction(env, objValue, "connectServiceExtensionAbilityWithAccount", moduleName,
        ConnectAbilityWithAccount);
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