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
#define CHECK_POINTER_RETURN(object)                 \
    if (object == nullptr) {                                   \
        HILOG_ERROR("jsAbilityContext_ is nullptr"); \
        return nullptr;                              \
    }

JsEmbeddableUIAbilityContext::JsEmbeddableUIAbilityContext(const std::shared_ptr<AbilityContext>& uiContext,
    const std::shared_ptr<UIExtensionContext>& uiExtContext, int32_t screenMode)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    jsAbilityContext_ = std::make_shared<JsAbilityContext>(uiContext);
    jsUIExtensionContext_ = std::make_shared<JsUIExtensionContext>(uiExtContext);
    screenMode_ = screenMode;
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

napi_value JsEmbeddableUIAbilityContext::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::HALF_SCREEN_MODE) {
        HILOG_INFO("OnStartAbility Half screen mode");
        return jsUIExtensionContext_->OnStartAbility(env, info);
    }
    CHECK_POINTER_RETURN(jsAbilityContext_);
    return jsAbilityContext_->OnStartAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::HALF_SCREEN_MODE) {
        HILOG_INFO("OnStartAbilityForResult Half screen mode");
        return jsUIExtensionContext_->OnStartAbilityForResult(env, info);
    }
    CHECK_POINTER_RETURN(jsAbilityContext_);
    return jsAbilityContext_->OnStartAbilityForResult(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnConnectAbility(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::HALF_SCREEN_MODE) {
        HILOG_INFO("OnConnectAbility Half screen mode");
        return jsUIExtensionContext_->OnConnectAbility(env, info);
    }
    CHECK_POINTER_RETURN(jsAbilityContext_);
    return jsAbilityContext_->OnConnectAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnDisconnectAbility(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::HALF_SCREEN_MODE) {
        HILOG_INFO("OnDisconnectAbility Half screen mode");
        return jsUIExtensionContext_->OnDisconnectAbility(env, info);
    }
    CHECK_POINTER_RETURN(jsAbilityContext_);
    return jsAbilityContext_->OnDisconnectAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnTerminateSelf(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::HALF_SCREEN_MODE) {
        HILOG_INFO("OnTerminateSelf Half screen mode");
        return jsUIExtensionContext_->OnTerminateSelf(env, info);
    }
    CHECK_POINTER_RETURN(jsAbilityContext_);
    return jsAbilityContext_->OnTerminateSelf(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info)
{
    if (screenMode_ == AAFwk::HALF_SCREEN_MODE) {
        HILOG_INFO("OnTerminateSelfWithResult Half screen mode");
        return jsUIExtensionContext_->OnTerminateSelfWithResult(env, info);
    }
    CHECK_POINTER_RETURN(jsAbilityContext_);
    return jsAbilityContext_->OnTerminateSelfWithResult(env, info);
}

void JsEmbeddableUIAbilityContext::WrapJsUIAbilityContext(napi_env env,
    std::shared_ptr<AbilityContext> uiContext, napi_value &objValue, int32_t screenMode)
{
    objValue = CreateJsBaseContext(env, uiContext);
    std::unique_ptr<JsEmbeddableUIAbilityContext> jsContext = std::make_unique<JsEmbeddableUIAbilityContext>(
        uiContext, nullptr, screenMode);
    napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);

    auto abilityInfo = uiContext->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        napi_set_named_property(env, objValue, "abilityInfo", CreateJsAbilityInfo(env, *abilityInfo));
    }

    auto configuration = uiContext->GetConfiguration();
    if (configuration != nullptr) {
        napi_set_named_property(env, objValue, "config", CreateJsConfiguration(env, *configuration));
    }
}

void JsEmbeddableUIAbilityContext::WrapJsUIExtensionContext(napi_env env,
    std::shared_ptr<UIExtensionContext> uiExtContext, napi_value &objValue, int32_t screenMode)
{
    objValue = CreateJsBaseContext(env, uiExtContext);
    std::unique_ptr<JsEmbeddableUIAbilityContext> jsContext = std::make_unique<JsEmbeddableUIAbilityContext>(
        nullptr , uiExtContext, screenMode);
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
    std::shared_ptr<AbilityContext> uiContext, std::shared_ptr<UIExtensionContext> uiExtContext, int screenMode)
{
    HILOG_DEBUG("CreateJsEmbeddableUIAbilityContext begin");
    if (uiContext == nullptr && uiExtContext == nullptr) {
        HILOG_ERROR("Contexts are both null.");
        return nullptr;
    }
    napi_value objValue = nullptr;
    if (screenMode == AAFwk::FULL_SCREEN_MODE) {
        WrapJsUIAbilityContext(env, uiContext, objValue, screenMode);
    } else if (screenMode == AAFwk::HALF_SCREEN_MODE) {
        WrapJsUIExtensionContext(env, uiExtContext, objValue, screenMode);
    }

    const char *moduleName = "JsEmbeddableUIAbilityContext";
    BindNativeFunction(env, objValue, "startAbility", moduleName, StartAbility);
    BindNativeFunction(env, objValue, "startAbilityForResult", moduleName, StartAbilityForResult);
    BindNativeFunction(env, objValue, "connectServiceExtensionAbility", moduleName, ConnectAbility);
    BindNativeFunction(env, objValue, "disconnectServiceExtensionAbility", moduleName, DisconnectAbility);
    BindNativeFunction(env, objValue, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(env, objValue, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);

    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS