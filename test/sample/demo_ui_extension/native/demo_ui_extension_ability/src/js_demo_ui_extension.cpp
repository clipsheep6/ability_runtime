/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "js_demo_ui_extension.h"

#include "hilog_wrapper.h"
#include "js_ui_extension_base.h"
#include "js_demo_ui_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
} // namespace
JsDemoUIExtension *JsDemoUIExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    TAG_LOGD(AAFwkTag::TEST, "Create js demo uiextension.");
    return new JsDemoUIExtension(runtime);
}

JsDemoUIExtension::JsDemoUIExtension(const std::unique_ptr<Runtime> &runtime) : JsUIExtensionBase(runtime)
{
    SetUIExtensionBaseImpl(std::shared_ptr<JsDemoUIExtension>(this));
}

JsDemoUIExtension::~JsDemoUIExtension()
{
    TAG_LOGD(AAFwkTag::TEST, "Js demo uiextension destructor.");
}

void JsDemoUIExtension::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    TAG_LOGE(AAFwkTag::UI_EXT, "OnForeground");

    ForegroundWindow(want, sessionInfo);
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onForeground");
    CallObjectMethod("onTest");
}

napi_value AttachUIExtensionBaseContext(napi_env env, void *value, void*)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid parameter.");
        return nullptr;
    }

    auto ptr = reinterpret_cast<std::weak_ptr<UIExtensionContext>*>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid context.");
        return nullptr;
    }
    napi_value object = JsDemoUIExtensionContext::CreateJsDemoUIExtensionContext(env, ptr);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "create context error.");
        return nullptr;
    }
    auto contextRef = JsRuntime::LoadSystemModuleByEngine(
        env, "application.UIExtensionContext", &object, 1);
    if (contextRef == nullptr) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Failed to get LoadSystemModuleByEngine");
        return nullptr;
    }
    auto contextObj = contextRef->GetNapiValue();
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "load context error.");
        return nullptr;
    }
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not object.");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachUIExtensionBaseContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void*) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Finalizer for weak_ptr ui extension context is called");
            if (data == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "Finalizer for weak_ptr is nullptr");
                return;
            }
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

void JsDemoUIExtension::BindContext()
{
    HandleScope handleScope(jsRuntime_);
    std::shared_ptr<UIExtensionContext> context = JsUIExtensionBase::context_;
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "jsObj_ is nullptr");
        return;
    }
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "obj is not object");
        return;
    }
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "BindContext CreateJsDemoUIExtensionContext.");
    napi_value contextObj = JsDemoUIExtensionContext::CreateJsDemoUIExtensionContext(env, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Create js ui extension context error.");
        return;
    }
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(
        env, "application.UIExtensionContext", &contextObj, ARGC_ONE);
    if (shellContextRef_ == nullptr) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Failed to get LoadSystemModuleByEngine");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(context);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachUIExtensionBaseContext, workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void*) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Finalizer for weak_ptr ui extension context is called");
            if (data == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "Finalizer for weak_ptr is nullptr");
                return;
            }
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
        },
        nullptr, nullptr);
}
} // namespace AbilityRuntime
} // namespace OHOS
