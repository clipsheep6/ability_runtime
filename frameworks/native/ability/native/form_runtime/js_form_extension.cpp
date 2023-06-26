/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "form_runtime/js_form_extension.h"

#include "ability_info.h"
#include "form_provider_data.h"
#include "form_runtime/form_extension_provider_client.h"
#include "form_runtime/js_form_extension_context.h"
#include "hilog_wrapper.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include <type_traits>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
const int ON_EVENT_PARAMS_SIZE = 2;

NativeValue* AttachFormExtensionContext(NativeEngine* engine, void* value, void*)
{
    HILOG_INFO("call");
    if (value == nullptr) {
        HILOG_WARN("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<FormExtensionContext>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }
    NativeValue* object = CreateJsFormExtensionContext(*engine, ptr);
    auto contextObj = JsRuntime::LoadSystemModuleByEngine(engine,
        "application.FormExtensionContext", &object, 1)->Get();
    NativeObject *nObject = ConvertNativeValueTo<NativeObject>(contextObj);
    nObject->ConvertToNativeBindingObject(engine, DetachCallbackFunc, AttachFormExtensionContext,
        value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<FormExtensionContext>(ptr);
    nObject->SetNativePointer(workContext,
        [](NativeEngine *, void * data, void *) {
            HILOG_INFO("Finalizer for weak_ptr form extension context is called");
            delete static_cast<std::weak_ptr<FormExtensionContext> *>(data);
        }, nullptr);
    return contextObj;
}

JsFormExtension* JsFormExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    HILOG_INFO("call");
    return new JsFormExtension(static_cast<JsRuntime&>(*runtime));
}

JsFormExtension::JsFormExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsFormExtension::~JsFormExtension()
{
    HILOG_DEBUG("Js form extension destructor.");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
}

void JsFormExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("call");
    FormExtension::Init(record, application, handler, token);
    std::string srcPath;
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        HILOG_ERROR("Failed to get srcPath");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HILOG_DEBUG("JsFormExtension::Init moduleName:%{public}s,srcPath:%{public}s, compileMode :%{public}d.",
        moduleName.c_str(), srcPath.c_str(), abilityInfo_->compileMode);
    HandleScope handleScope(jsRuntime_);
    auto& engine = jsRuntime_.GetNativeEngine();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        HILOG_ERROR("Failed to get jsObj_");
        return;
    }

    NativeObject* obj = ConvertNativeValueTo<NativeObject>(jsObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get JsFormExtension object");
        return;
    }

    BindContext(engine, obj);
}

void JsFormExtension::BindContext(NativeEngine& engine, NativeObject* obj)
{
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    HILOG_INFO("call");
    NativeValue* contextObj = CreateJsFormExtensionContext(engine, context);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(&engine, "application.FormExtensionContext", &contextObj, 1);
    contextObj = shellContextRef_->Get();
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<FormExtensionContext>(context);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachFormExtensionContext,
        workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());
    obj->SetProperty("context", contextObj);

    nativeObj->SetNativePointer(workContext,
        [](NativeEngine*, void* data, void*) {
            HILOG_INFO("Finalizer for weak_ptr form extension context is called");
            delete static_cast<std::weak_ptr<FormExtensionContext>*>(data);
        }, nullptr);

    HILOG_INFO("ok");
}

OHOS::AppExecFwk::FormProviderInfo JsFormExtension::OnCreate(const OHOS::AAFwk::Want& want)
{
    HILOG_INFO("call");
    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
    NativeValue* nativeWant = reinterpret_cast<NativeValue*>(napiWant);
    NativeValue* argv[] = { nativeWant };
    NativeValue* nativeResult = CallObjectMethod("onAddForm", "onCreate", argv, 1);
    NativeObject* nativeObject = ConvertNativeValueTo<NativeObject>(nativeResult);

    OHOS::AppExecFwk::FormProviderInfo formProviderInfo;
    if (nativeObject == nullptr) {
        HILOG_ERROR("nativeObject is nullptr.");
        return formProviderInfo;
    }

    NativeValue* nativeDataValue = nativeObject->GetProperty("data");
    if (nativeDataValue == nullptr) {
        HILOG_ERROR("nativeObject get data is nullptr.");
        return formProviderInfo;
    }
    std::string formDataStr;
    if (!ConvertFromJsValue(*nativeEngine, nativeDataValue, formDataStr)) {
        HILOG_ERROR("convert formDataStr failed.");
        return formProviderInfo;
    }
    AppExecFwk::FormProviderData formData = AppExecFwk::FormProviderData(formDataStr);
    formProviderInfo.SetFormData(formData);

    NativeValue* nativeProxies = nativeObject->GetProperty("proxies");
    std::vector<FormDataProxy> formDataProxies;
    if (nativeProxies != nullptr && !ConvertFromDataProxies(*nativeEngine, nativeProxies, formDataProxies)) {
        HILOG_WARN("convert formDataProxies failed.");
        return formProviderInfo;
    }
    formProviderInfo.SetFormDataProxies(formDataProxies);
    HILOG_INFO("ok");
    return formProviderInfo;
}

void JsFormExtension::OnDestroy(const int64_t formId)
{
    HILOG_INFO("OnDestroy, formId: %{public}" PRId64 ".", formId);
    FormExtension::OnDestroy(formId);

    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(reinterpret_cast<napi_env>(nativeEngine), std::to_string(formId).c_str(), NAPI_AUTO_LENGTH,
        &napiFormId);
    NativeValue* nativeFormId = reinterpret_cast<NativeValue*>(napiFormId);
    NativeValue* argv[] = {nativeFormId};
    CallObjectMethod("onRemoveForm", "onDestroy", argv, 1);
}

void JsFormExtension::OnEvent(const int64_t formId, const std::string& message)
{
    HILOG_INFO("OnEvent, formId: %{public}" PRId64 ".", formId);
    FormExtension::OnEvent(formId, message);

    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(reinterpret_cast<napi_env>(nativeEngine), std::to_string(formId).c_str(),
        NAPI_AUTO_LENGTH, &napiFormId);
    NativeValue* nativeFormId = reinterpret_cast<NativeValue*>(napiFormId);
    // wrap message
    napi_value napiMessage = nullptr;
    napi_create_string_utf8(reinterpret_cast<napi_env>(nativeEngine), message.c_str(), NAPI_AUTO_LENGTH, &napiMessage);
    NativeValue* nativeMessage = reinterpret_cast<NativeValue*>(napiMessage);
    NativeValue* argv[] = {nativeFormId, nativeMessage};
    CallObjectMethod("onFormEvent", "onEvent", argv, ON_EVENT_PARAMS_SIZE);
}

void JsFormExtension::OnUpdate(const int64_t formId)
{
    HILOG_INFO("OnUpdate, formId: %{public}" PRId64 ".", formId);
    FormExtension::OnUpdate(formId);

    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(reinterpret_cast<napi_env>(nativeEngine), std::to_string(formId).c_str(),
        NAPI_AUTO_LENGTH, &napiFormId);
    NativeValue* nativeFormId = reinterpret_cast<NativeValue*>(napiFormId);
    NativeValue* argv[] = {nativeFormId};
    CallObjectMethod("onUpdateForm", "onUpdate", argv, 1);
}

void JsFormExtension::OnCastToNormal(const int64_t formId)
{
    HILOG_INFO("OnCastToNormal, formId: %{public}" PRId64 ".", formId);
    FormExtension::OnCastToNormal(formId);

    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(reinterpret_cast<napi_env>(nativeEngine), std::to_string(formId).c_str(), NAPI_AUTO_LENGTH,
        &napiFormId);
    NativeValue* nativeFormId = reinterpret_cast<NativeValue*>(napiFormId);
    NativeValue* argv[] = {nativeFormId};
    CallObjectMethod("onCastToNormalForm", "onCastToNormal", argv, 1);
}

void JsFormExtension::OnVisibilityChange(const std::map<int64_t, int32_t>& formEventsMap)
{
    HILOG_INFO("call");
    FormExtension::OnVisibilityChange(formEventsMap);
    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    NativeValue* nativeFormEventsMap = nativeEngine->CreateObject();
    NativeObject* nativeObj = ConvertNativeValueTo<NativeObject>(nativeFormEventsMap);
    for (auto item = formEventsMap.begin(); item != formEventsMap.end(); item++) {
        nativeObj->SetProperty(std::to_string(item->first).c_str(), CreateJsValue(*nativeEngine, item->second));
    }
    NativeValue* argv[] = {nativeFormEventsMap};
    CallObjectMethod("onChangeFormVisibility", "onVisibilityChange", argv, 1);
}

sptr<IRemoteObject> JsFormExtension::OnConnect(const OHOS::AAFwk::Want& want)
{
    HILOG_INFO("call");
    Extension::OnConnect(want);
    if (providerRemoteObject_ == nullptr) {
        HILOG_INFO("providerRemoteObject_ is nullptr, need init.");
        sptr<FormExtensionProviderClient> providerClient = new (std::nothrow) FormExtensionProviderClient();
        std::shared_ptr<JsFormExtension> formExtension = std::static_pointer_cast<JsFormExtension>(shared_from_this());
        providerClient->SetOwner(formExtension);
        providerRemoteObject_ = providerClient->AsObject();
    }
    HILOG_INFO("ok");
    return providerRemoteObject_;
}

NativeValue* JsFormExtension::CallObjectMethod(const char* name, const char *bakName, NativeValue* const* argv,
    size_t argc)
{
    HILOG_INFO("CallObjectMethod(%{public}s)", name);
    if (!jsObj_) {
        HILOG_WARN("jsObj_ is nullptr");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get FormExtension object");
        return nullptr;
    }

    NativeValue* method = obj->GetProperty(name);
    if (method == nullptr || method->TypeOf() != NATIVE_FUNCTION) {
        HILOG_ERROR("Failed to get '%{public}s' from FormExtension object", name);
        if (bakName == nullptr) {
            return nullptr;
        }
        method = obj->GetProperty(bakName);
        if (method == nullptr || method->TypeOf() != NATIVE_FUNCTION) {
            HILOG_ERROR("Failed to get '%{public}s' from FormExtension object", bakName);
            return nullptr;
        }
    }
    HILOG_INFO("CallFunction(%{public}s), ok", name);
    return handleEscape.Escape(nativeEngine.CallFunction(value, method, argv, argc));
}

void JsFormExtension::GetSrcPath(std::string &srcPath)
{
    if (!Extension::abilityInfo_->isModuleJson) {
        /* temporary compatibility api8 + config.json */
        srcPath.append(Extension::abilityInfo_->package);
        srcPath.append("/assets/js/");
        if (!Extension::abilityInfo_->srcPath.empty()) {
            srcPath.append(Extension::abilityInfo_->srcPath);
        }
        srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");
        return;
    }

    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}

void JsFormExtension::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    FormExtension::OnConfigurationUpdated(configuration);
    HILOG_INFO("call");

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    // Notify extension context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }
    JsExtensionContext::ConfigurationUpdated(&nativeEngine, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(
        reinterpret_cast<napi_env>(&nativeEngine), *fullConfig);
    NativeValue* jsConfiguration = reinterpret_cast<NativeValue*>(napiConfiguration);
    CallObjectMethod("onConfigurationUpdate", "onConfigurationUpdated", &jsConfiguration, 1);
}

FormState JsFormExtension::OnAcquireFormState(const Want &want)
{
    HILOG_INFO("call");
    auto state = static_cast<int32_t>(FormState::DEFAULT);
    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);

    NativeValue* nativeWant = reinterpret_cast<NativeValue*>(napiWant);
    NativeValue* argv[] = { nativeWant };
    NativeValue* nativeResult = CallObjectMethod("onAcquireFormState", nullptr, argv, 1);
    if (nativeResult == nullptr) {
        HILOG_INFO("function onAcquireFormState not found.");
        return FormState::DEFAULT;
    }

    if (!ConvertFromJsValue(*nativeEngine, nativeResult, state)) {
        HILOG_ERROR("convert form state failed.");
        return FormState::UNKNOWN;
    }

    HILOG_INFO("state: %{public}d", state);
    if (state <= static_cast<int32_t>(AppExecFwk::FormState::UNKNOWN) ||
        state > static_cast<int32_t>(AppExecFwk::FormState::READY)) {
        return AppExecFwk::FormState::UNKNOWN;
    } else {
        return static_cast<AppExecFwk::FormState>(state);
    }
}

bool JsFormExtension::OnShare(int64_t formId, AAFwk::WantParams &wantParams)
{
    HILOG_DEBUG("OnShare, formId: %{public}" PRId64 ".", formId);
    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    if (nativeEngine == nullptr) {
        HILOG_ERROR("NativeEngine is nullptr.");
        return false;
    }

    auto formIdStr = std::to_string(formId);
    NativeValue* argv[] = { nativeEngine->CreateString(formIdStr.c_str(), formIdStr.length()) };
    NativeValue* nativeResult = CallObjectMethod("onShareForm", "onShare", argv, 1);
    if (nativeResult == nullptr) {
        HILOG_ERROR("OnShare return value is nullptr.");
        return false;
    }

    if (nativeResult->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("OnShare return value`s type is %{public}d.", static_cast<int32_t>(nativeResult->TypeOf()));
        return false;
    }

    if (!OHOS::AppExecFwk::UnwrapWantParams(reinterpret_cast<napi_env>(nativeEngine),
        reinterpret_cast<napi_value>(nativeResult), wantParams)) {
        HILOG_ERROR("Unwrap want params failed.");
        return false;
    }

    HILOG_DEBUG("called end.");
    return true;
}

bool JsFormExtension::OnAcquireData(int64_t formId, AAFwk::WantParams &wantParams)
{
    HILOG_DEBUG("OnAcquireData, formId: %{public}" PRId64 ".", formId);
    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    if (nativeEngine == nullptr) {
        HILOG_ERROR("NativeEngine is nullptr.");
        return false;
    }

    auto formIdStr = std::to_string(formId);
    NativeValue* argv[] = { nativeEngine->CreateString(formIdStr.c_str(), formIdStr.length()) };
    NativeValue* nativeResult = CallObjectMethod("onAcquireFormData", "OnAcquireData", argv, 1);
    if (nativeResult == nullptr) {
        HILOG_ERROR("return value is nullptr.");
        return false;
    }

    if (nativeResult->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("return value`s type is %{public}d.", static_cast<int32_t>(nativeResult->TypeOf()));
        return false;
    }

    if (!OHOS::AppExecFwk::UnwrapWantParams(reinterpret_cast<napi_env>(nativeEngine),
        reinterpret_cast<napi_value>(nativeResult), wantParams)) {
        HILOG_ERROR("Unwrap want params failed.");
        return false;
    }

    HILOG_DEBUG("called end.");
    return true;
}

bool JsFormExtension::ConvertFromDataProxies(NativeEngine& engine, NativeValue* jsValue,
    std::vector<FormDataProxy> &formDataProxies)
{
    if (jsValue == nullptr || !jsValue->IsArray()) {
        HILOG_ERROR("%{public}s, jsValue is nullptr not array", __func__);
        return false;
    }

    auto array = ConvertNativeValueTo<NativeArray>(jsValue);
    if (array == nullptr) {
        HILOG_ERROR("%{public}s, convert array failed", __func__);
        return false;
    }

    for (uint32_t i = 0; i < array->GetLength(); i++) {
        FormDataProxy formDataProxy("", "");
        if (!ConvertFormDataProxy(engine, array->GetElement(i), formDataProxy)) {
            HILOG_ERROR("GetElement from array [%{public}u] error", i);
            continue;
        }
        formDataProxies.push_back(formDataProxy);
    }
    return true;
}

bool JsFormExtension::ConvertFormDataProxy(NativeEngine& engine, NativeValue* jsValue, FormDataProxy &formDataProxy)
{
    if (jsValue == nullptr || jsValue->TypeOf() != NATIVE_OBJECT) {
        HILOG_ERROR("%{public}s, jsValue is nullptr not object", __func__);
        return false;
    }

    NativeObject *jsObject = ConvertNativeValueTo<NativeObject>(jsValue);
    if (jsObject == nullptr) {
        HILOG_ERROR("%{public}s called, jsObject is nullptr.", __func__);
        return false;
    }

    NativeValue* key = jsObject->GetProperty("key");
    if (!ConvertFromJsValue(engine, key, formDataProxy.key)) {
        HILOG_ERROR("Parse key failed");
        return false;
    }
    NativeValue* subscribeId = jsObject->GetProperty("subscriberId");
    if (subscribeId != nullptr && !ConvertFromJsValue(engine, subscribeId, formDataProxy.subscribeId)) {
        HILOG_WARN("Parse subscriberId failed, use empty as default value.");
        formDataProxy.subscribeId = "";
    }
    HILOG_INFO("key is %{public}s, subscriberId is %{public}s", formDataProxy.key.c_str(),
        formDataProxy.subscribeId.c_str());
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
