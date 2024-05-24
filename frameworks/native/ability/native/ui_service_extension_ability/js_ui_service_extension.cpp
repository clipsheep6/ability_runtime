/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "js_ui_service_extension.h"

#include <regex>

#include "ability_business_error.h"
#include "ability_handler.h"
#include "ability_info.h"
#include "ability.h"
#include "ability_manager_client.h"
#include "configuration_utils.h"
#include "hitrace_meter.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "insight_intent_executor_mgr.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_service_extension_context.h"
#include "js_ui_service_host_proxy.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "scene_board_judgement.h"
#include "wm_common.h"
#include "window.h"
#ifdef SUPPORT_GRAPHICS
#include "iservice_registry.h"
#include "system_ability_definition.h"
#endif


namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
}
using namespace OHOS::AppExecFwk;

#ifdef SUPPORT_GRAPHICS
void JsUIServiceExtension::OnSceneWillCreated(std::shared_ptr<Rosen::WindowStageConfig> windowStageConfig)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "OnSceneWillCreated call");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    auto jsAppWindowStageConfig = CreateJsWindowStageConfig(env, windowStageConfig);
    if (jsAppWindowStageConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to create jsWindowSatgeConfig object.");
        return;
    }
    napi_value argv[] = {jsAppWindowStageConfig};
    CallObjectMethod("onWindowStageWillCreate", argv, ArraySize(argv));

    TAG_LOGI(AAFwkTag::UISERVC_EXT, "End OnSceneWillCreated.");
}

void JsUIServiceExtension::OnSceneDidCreated()
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "OnSceneDidCreated call");
    HandleScope handleScope(jsRuntime_);
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "JsAppWindowStage is nullptr.");
        return;
    }
    UpdateJsWindowStage(jsAppWindowStage->GetNapiValue());
    napi_value argv[] = {jsAppWindowStage->GetNapiValue()};
    CallObjectMethod("onWindowStageDidCreate", argv, ArraySize(argv)); //load page
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "Call onWindowStageDidCreate finish.");
    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());

    auto window = scene_->GetMainWindow();
    if (window != nullptr && securityFlag_) {
        window->SetSystemPrivacyMode(true);
    }
    scene_->GoForeground(UIServiceExtension::sceneFlag_); //display window
    scene_->GoBackground(UIServiceExtension::sceneFlag_); //hide window
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "End OnSceneDidCreated.");
}

std::unique_ptr<NativeReference> JsUIServiceExtension::CreateAppWindowStage()
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsWindowStage = Rosen::CreateJsWindowStage(env, GetScene());
    if (jsWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to create jsWindowSatge object.");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(env, "application.WindowStage", &jsWindowStage, 1);
}

void JsUIServiceExtension::UpdateJsWindowStage(napi_value windowStage)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Called.");
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "shellContextRef_ is nullptr.");
        return;
    }
    napi_value contextObj = shellContextRef_->GetNapiValue();
    napi_env env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get context native object.");
        return;
    }
    if (windowStage == nullptr) {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "Set context windowStage is undefined.");
        napi_set_named_property(env, contextObj, "windowStage", CreateJsUndefined(env));
        return;
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Set context windowStage object.");
    napi_set_named_property(env, contextObj, "windowStage", windowStage);
}
#endif

JsUIServiceExtension::JsUIServiceExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {
}

JsUIServiceExtension::~JsUIServiceExtension()
{
//    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Js UIservice extension destructor.");
    if (extensionStub_) {
        extensionStub_.reset();
    }
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
}


void JsUIServiceExtension::OnStart(const AAFwk::Want &want)
{
    Extension::OnStart(want);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "call");

    auto context = GetContext();
    if (context != nullptr) {
        int32_t  displayId = static_cast<int32_t>(Rosen::DisplayManager::GetInstance().GetDefaultDisplayId());
        displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, displayId);
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "displayId %{public}d", displayId);
        auto configUtils = std::make_shared<ConfigurationUtils>();
        configUtils->InitDisplayConfig(displayId, context->GetConfiguration(), context->GetResourceManager());
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // display config has changed, need update context.config
    if (context != nullptr) {
        JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, context->GetConfiguration());
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    CallObjectMethod("onCreate", argv, ARGC_ONE);
#ifdef SUPPORT_GRAPHICS
    std::shared_ptr<Rosen::WindowStageConfig> windowStageConfig = std::make_shared<Rosen::WindowStageConfig>();
    OnSceneWillCreated(windowStageConfig);
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "windowStageConfig.windowStageAttribute: %{public}d",
        windowStageConfig->windowStageAttribute);
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "windowStageConfig.rect.posX_: %{public}d",
        windowStageConfig->rect.posX_);
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "windowStageConfig.rect.posY_: %{public}d",
        windowStageConfig->rect.posY_);
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "windowStageConfig.rect.width_: %{public}d",
        windowStageConfig->rect.width_);
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "windowStageConfig.rect.height_: %{public}d",
            windowStageConfig->rect.height_);
    scene_ = std::make_shared<Rosen::WindowScene>();
    int32_t displayId = static_cast<int32_t>(Rosen::DisplayManager::GetInstance().GetDefaultDisplayId());
    auto option = GetWindowOption(want, windowStageConfig);
    Rosen::WMError ret = Rosen::WMError::WM_OK;
    ret = scene_->Init(displayId, abilityContext_, sceneListener_, option);
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to init window scene.");
        return;
    }
    OnSceneDidCreated();

#endif	
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "ok");
}



napi_value JsUIServiceExtension::CallObjectMethod(const char* name, napi_value const* argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "name:%{public}s", name);

    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "Not found ServiceExtension.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get ServiceExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get '%{public}s' from ServiceExtension object", name);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "CallFunction(%{public}s) ok", name);
    napi_value result = nullptr;
    napi_call_function(env, obj, method, argc, argv, &result);
    return result;
}


#endif
} // AbilityRuntime
} // OHOS
