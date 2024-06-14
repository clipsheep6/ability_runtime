/*
 * Copyright (c) s2024 Huawei Device Co., Ltd.
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

#include "ui_service_extension.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "ability_delegator_registry.h"
#include "js_ui_service_extension.h"
#include "runtime.h"
#include "js_runtime_utils.h"
#include "ui_service_extension_context.h"
#include "time_util.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
void SetFormInfoPropertyInt32(napi_env env, int32_t inValue, napi_value &result, const char *outName)
{
    napi_value temp = WrapInt32ToJS(env, inValue);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "%{public}s=%{public}d.", outName, inValue);
    napi_set_named_property(env, result, outName, temp);
}

#ifdef SUPPORT_GRAPHICS
napi_value CreateWindowStageConfig(napi_env env, WindowStageConfig &windowStageConfig)

{
    napi_value object = CreateJSObject(env);
    if (object == nullptr) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "invalid object.");
        return nullptr;
    }
    SetFormInfoPropertyInt32(env, windowStageConfig.windowStageAttribute, &object, "windowStageAttribute");
    napi_value rectValue = CreateJSObject(env);
    if (rectValue == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "invalid rectValue.");
        return nullptr;
    }
    SetFormInfoPropertyInt32(env, windowStageConfig.rect.left, &rectValue, "left");
    SetFormInfoPropertyInt32(env, windowStageConfig.rect.top, &rectValue, "top");
    SetFormInfoPropertyInt32(env, windowStageConfig.rect.width, &rectValue, "width");
    SetFormInfoPropertyInt32(env, windowStageConfig.rect.height, &rectValue, "height");

    if (!SetPropertyValueByPropertyName(env, object, "windowStageRect", &rectValue)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "set rectValue failed.");
        return nullptr;
    }
    return object;
}

bool UnwrapWindowStageConfig(napi_env env, napi_value configValue, WindowStageConfig &windowStageConfig)
{
    if (!IsTypeForNapiValue(env, configValue, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "%{public}s called. windowStageConfig is invalid.", __func__);
        return false;
    }

    int32_t windowStageAttribute = 0;
    if (UnwrapInt32ByPropertyName(env, configValue, "windowStageAttribute", windowStageAttribute)) {
        windowStageConfig.windowStageAttribute = windowStageAttribute;
    }

    napi_value rectValue = nullptr;
    if (napi_get_named_property(env, configValue, "windowStageRect", &rectValue) != napi_ok) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Get js value failed.");
        return false;
    }
    int32_t left = 0;
    if (UnwrapInt32ByPropertyName(env, rectValue, "left", left)) {
        windowStageConfig.rect.left = left;
    }

    int32_t top = 0;
    if (UnwrapInt32ByPropertyName(env, rectValue, "top", top)) {
        windowStageConfig.rect.top = top;
    }

    int32_t width = 0;
    if (UnwrapInt32ByPropertyName(env, rectValue, "width", width)) {
        windowStageConfig.rect.width = width;
    }

    int32_t height = 0;
    if (UnwrapInt32ByPropertyName(env, rectValue, "height", height)) {
        windowStageConfig.rect.height = height;
    }

    return true;
}
#endif
} //namespace
using namespace OHOS::AppExecFwk;
UIServiceExtension* UIServiceExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    if (!runtime) {
        return new UIServiceExtension();
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "UIServiceExtension Create runtime");
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsUIServiceExtension::Create(runtime);

        default:
            return new UIServiceExtension();
    }
}

void UIServiceExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "UIExtension begin init");
    ExtensionBase<UIExtensionContext>::Init(record, application, handler, token);
}

std::shared_ptr<UIServiceExtensionContext> UIServiceExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    std::shared_ptr<UIServiceExtensionContext> context =
        ExtensionBase<UIServiceExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "UIServiceExtension CreateAndInitContext context is nullptr");
        return context;
    }
    return context;
}

#ifdef SUPPORT_GRAPHICS
void UIServiceExtension::OnSceneWillCreated(WindowStageConfig &windowStageConfig)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Begin ability is %{public}s.", GetAbilityName().c_str());
    auto env = jsRuntime_.GetNapiEnv();
    auto jsAppWindowStageConfig = CreateWindowStageConfig(env, windowStageConfig);
    if (jsAppWindowStageConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "jsAppWindowStageConfig is nullptr.");
        return;
    }
    HandleScope handleScope(jsRuntime_);
    napi_value argv[] = {jsAppWindowStageConfig};
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "onWindowStageWillCreate");
        std::string methodName = "OnSceneWillCreated";
        AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
        CallObjectMethod("onWindowStageWillCreate", argv, ArraySize(argv));
        AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    }
    if (!UnwrapWindowStageConfig(env, jsAppWindowStageConfig, windowStageConfig)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "unwrap WindowStageConfig failed.");
        return;
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "End ability is %{public}s.", GetAbilityName().c_str());
}

void UIServiceExtension::OnSceneDidCreated()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Begin ability is %{public}s.", GetAbilityName().c_str());
    UIAbility::OnSceneCreated();
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "JsAppWindowStage is nullptr.");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    UpdateJsWindowStage(jsAppWindowStage->GetNapiValue());
    napi_value argv[] = {jsAppWindowStage->GetNapiValue()};
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "onWindowStageDidCreate");
        std::string methodName = "OnSceneDidCreated";
        AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
        CallObjectMethod("onWindowStageDidCreate", argv, ArraySize(argv));
        AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "Call PostPerformScenceCreated.");
        delegator->PostPerformScenceCreated(CreateADelegatorAbilityProperty());
    }

    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnWindowStageCreate(jsAbilityObj_, jsWindowStageObj_);
    }

    TAG_LOGD(AAFwkTag::UISERVC_EXT, "End ability is %{public}s.", GetAbilityName().c_str());
}

void UIServiceExtension::UpdateJsWindowStage(napi_value windowStage)
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

napi_value UIServiceExtension::CallObjectMethod(const char *name, napi_value const *argv, size_t argc, bool withResult,
    bool showMethodNotFoundLog)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Lifecycle: the begin of %{public}s", name);
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Not found Ability.js");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get Ability object.");
        return nullptr;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, name, &methodOnCreate);
    if (methodOnCreate == nullptr) {
        if (showMethodNotFoundLog) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get '%{public}s' from Ability object.", name);
        }
        return nullptr;
    }
    TryCatch tryCatch(env);
    if (withResult) {
        napi_value result = nullptr;
        napi_call_function(env, obj, methodOnCreate, argc, argv, &result);
        if (tryCatch.HasCaught()) {
            reinterpret_cast<NativeEngine*>(env)->HandleUncaughtException();
        }
        return handleEscape.Escape(result);
    }
    napi_call_function(env, obj, methodOnCreate, argc, argv, nullptr);
    if (tryCatch.HasCaught()) {
        reinterpret_cast<NativeEngine*>(env)->HandleUncaughtException();
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Lifecycle: the end of %{public}s", name);
    return nullptr;
}

std::unique_ptr<NativeReference> UIServiceExtension::CreateAppWindowStage()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsWindowStage = Rosen::CreateJsWindowStage(env, GetScene());
    if (jsWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to create jsWindowSatge object.");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(env, "application.WindowStage", &jsWindowStage, 1);
}

void UIServiceExtension::AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState state,
    const std::string &methodName) const
{
    FreezeUtil::LifecycleFlow flow = { AbilityContext::token_, state };
    auto entry = std::to_string(TimeUtil::SystemTimeMillisecond()) + "; JsUIAbility::" + methodName +
        "; the " + methodName + " begin.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
}

void UIServiceExtension::AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState state,
    const std::string &methodName) const
{
    FreezeUtil::LifecycleFlow flow = { AbilityContext::token_, state };
    auto entry = std::to_string(TimeUtil::SystemTimeMillisecond()) + "; JsUIAbility::" + methodName +
        "; the " + methodName + " end.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
}
}
}
