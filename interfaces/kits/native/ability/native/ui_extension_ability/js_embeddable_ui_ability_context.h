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

#ifndef OHOS_ABILITY_RUNTIME_JS_EMBEDDABLE_UI_ABILITY_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_JS_EMBEDDABLE_UI_ABILITY_CONTEXT_H

#include <algorithm>
#include <memory>
#include <native_engine/native_value.h>

#include "ability_connect_callback.h"
#include "foundation/ability/ability_runtime/interfaces/kits/native/ability/ability_runtime/ability_context.h"
#include "js_free_install_observer.h"
#include "js_runtime.h"
#include "js_ui_extension_context.h"
#include "event_handler.h"
#include "js_ability_context.h"

namespace OHOS {
namespace AbilityRuntime {
class JsEmbeddableUIAbilityContext final {
public:
    explicit JsEmbeddableUIAbilityContext(const std::shared_ptr<AbilityContext>& uiContext,
        const std::shared_ptr<UIExtensionContext>& uiExtContext, int32_t screenMode);
    virtual ~JsEmbeddableUIAbilityContext() = default;
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value StartAbility(napi_env env, napi_callback_info info);
    static napi_value StartAbilityForResult(napi_env env, napi_callback_info info);
    static napi_value ConnectAbility(napi_env env, napi_callback_info info);
    static napi_value DisconnectAbility(napi_env env, napi_callback_info info);
    static napi_value TerminateSelf(napi_env env, napi_callback_info info);
    static napi_value TerminateSelfWithResult(napi_env env, napi_callback_info info);
    static napi_value CreateJsEmbeddableUIAbilityContext(napi_env env, std::shared_ptr<AbilityContext> uiContext,
        std::shared_ptr<UIExtensionContext> uiExtContext, int32_t screenMode);
    // error
    static napi_value StartAbilityAsCaller(napi_env env, napi_callback_info info);
    static napi_value StartAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value StartAbilityByCall(napi_env env, napi_callback_info info);
    static napi_value StartAbilityForResultWithAccount(napi_env env, napi_callback_info info);
    static napi_value StartServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value StartServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value StopServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value StopServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value ConnectAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value RestoreWindowStage(napi_env env, napi_callback_info info);
    static napi_value IsTerminating(napi_env env, napi_callback_info info);
    static napi_value StartRecentAbility(napi_env env, napi_callback_info info);
    static napi_value RequestDialogService(napi_env env, napi_callback_info info);
    static napi_value ReportDrawnCompleted(napi_env env, napi_callback_info info);
    static napi_value SetMissionContinueState(napi_env env, napi_callback_info info);
    static napi_value StartAbilityByType(napi_env env, napi_callback_info info);

private:
    static void WrapJsUIAbilityContext(napi_env env, std::shared_ptr<AbilityContext> uiContext,
        napi_value &objValue, int32_t screenMode);
    static void WrapJsUIExtensionContext(napi_env env, std::shared_ptr<UIExtensionContext> uiExtContext,
        napi_value &objValue, int32_t screenMode);
    napi_value OnStartAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info);
    napi_value OnConnectAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnDisconnectAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnTerminateSelf(napi_env env, NapiCallbackInfo& info);
    napi_value OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info);
    // error
    napi_value OnStartAbilityAsCaller(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityByCall(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityForResultWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartExtensionAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnStopExtensionAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStopExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnConnectAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnRestoreWindowStage(napi_env env, NapiCallbackInfo& info);
    napi_value OnIsTerminating(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartRecentAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnRequestDialogService(napi_env env, NapiCallbackInfo& info);
    napi_value OnReportDrawnCompleted(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetMissionContinueState(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityByType(napi_env env, NapiCallbackInfo& info);

#ifdef SUPPORT_GRAPHICS
public:
    static napi_value SetMissionLabel(napi_env env, napi_callback_info info);
    static napi_value SetMissionIcon(napi_env env, napi_callback_info info);
private:
    napi_value OnSetMissionLabel(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetMissionIcon(napi_env env, NapiCallbackInfo& info);
#endif

private:
    std::shared_ptr<JsAbilityContext> jsAbilityContext_;
    std::shared_ptr<JsUIExtensionContext> jsUIExtensionContext_;
    int32_t screenMode_ = AAFwk::IDLE_SCREEN_MODE;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_EMBEDDABLE_UI_ABILITY_CONTEXT_H