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

#include "js_photo_editor_extension_impl.h"
#include "hilog_tag_wrapper.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "js_photo_editor_extension_context.h"
#include "js_ui_extension_content_session.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_THREE = 3;
} // namespace

napi_value AttachUIExtensionContext(napi_env env, void *value, void *)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid param value");
        return nullptr;
    }

    auto ptr = reinterpret_cast<std::weak_ptr<PhotoEditorExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid context");
        return nullptr;
    }
    napi_value object = JsPhotoEditorExtensionContext::CreateJsPhotoEditorExtensionContext(env, ptr);
    auto contextRef =
        JsRuntime::LoadSystemModuleByEngine(env, "application.PhotoEditorExtensionContext", &object, ARGC_ONE);
    if (contextRef == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Fail to load module");
        return nullptr;
    }
    auto contextObj = contextRef->GetNapiValue();
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Load context error");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachUIExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<PhotoEditorExtensionContext>(ptr);
    napi_wrap(
        env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Finalizer called");
            if (data == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "Finalizer for weak_ptr is nullptr");
                return;
            }
            delete static_cast<std::weak_ptr<PhotoEditorExtensionContext> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

JsPhotoEditorExtensionImpl::JsPhotoEditorExtensionImpl(const std::unique_ptr<Runtime> &runtime)
    : JsUIExtensionBase(runtime)
{}

void JsPhotoEditorExtensionImpl::BindContext()
{
    HandleScope handleScope(jsRuntime_);
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "jsobj_ is nullptr");
        return;
    }

    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "obj is not object");
        return;
    }
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context is nullptr");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "BindContext CreateJsPhotoEditorExtensionContext");
    napi_value contextObj = JsPhotoEditorExtensionContext::CreateJsPhotoEditorExtensionContext(env, context_);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Create js ui extension context error");
        return;
    }

    shellContextRef_ =
        JsRuntime::LoadSystemModuleByEngine(env, "application.PhotoEditorExtensionContext", &contextObj, ARGC_ONE);
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Fail to get loadSystemModuleByEngine");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Fail to get context native object");
        return;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<PhotoEditorExtensionContext>(context_);
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachUIExtensionContext, workContext,
                                         nullptr);
    context_->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Finalizer called");
            if (data == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "Finalizer for weak_ptr is nullptr");
                return;
            }
            delete static_cast<std::weak_ptr<PhotoEditorExtensionContext> *>(data);
        },
        nullptr, nullptr);
    TAG_LOGD(AAFwkTag::UI_EXT, "Bind context end");
}

void JsPhotoEditorExtensionImpl::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    JsUIExtensionBase::OnForeground(want, sessionInfo);
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiExtensionComponentIdSet_.find(componentId) == uiExtensionComponentIdSet_.end()) {
        OnStartContentEditing(want, sessionInfo);
        uiExtensionComponentIdSet_.emplace(componentId);
    }
}

void JsPhotoEditorExtensionImpl::OnStartContentEditing(const AAFwk::Want &want,
                                                       const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "JsPhotoEditorExtension want: (%{public}s), begin", want.ToUri().c_str());

    std::string imageUri = want.GetStringParam("ability.params.stream");
    if (imageUri.empty()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "OnStartContentEditing failed, imageUri is empty");
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "JsPhotoEditorExtension imageUri: (%{public}s), begin", imageUri.c_str());
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "PhotoEditorExtension context is nullptr");
        return;
    }
    context_->SetWant(std::make_shared<AAFwk::Want>(want));

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Env is nullptr");
        return;
    }
    napi_value jsWant = AppExecFwk::WrapWant(env, want);
    if (jsWant == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Fail to get want");
        return;
    }
    napi_value jsImageUri;
    napi_create_string_utf8(env, imageUri.c_str(), imageUri.size(), &jsImageUri);
    napi_value argv[] = {jsImageUri, jsWant, contentSessions_[sessionInfo->uiExtensionComponentId]->GetNapiValue()};

    CallObjectMethod("onStartContentEditing", argv, ARGC_THREE);
}

} // namespace AbilityRuntime
} // namespace OHOS