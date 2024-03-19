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
#include "connection_manager.h"
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
const int ARGC_TWO = 2;

napi_value AttachFormExtensionContext(napi_env env, void* value, void*)
{
    HILOG_INFO("call");
    if (value == nullptr) {
        HILOG_WARN("invalid parameter");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<FormExtensionContext>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context");
        return nullptr;
    }
    napi_value object = CreateJsFormExtensionContext(env, ptr);
    auto sysModule = JsRuntime::LoadSystemModuleByEngine(env,
        "application.FormExtensionContext", &object, 1);
    if (sysModule == nullptr) {
        HILOG_WARN("load module failed");
        return nullptr;
    }
    auto contextObj = sysModule->GetNapiValue();
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachFormExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<FormExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void * data, void *) {
            HILOG_INFO("Finalizer for weak_ptr form extension context is called");
            delete static_cast<std::weak_ptr<FormExtensionContext> *>(data);
        },
        nullptr, nullptr);
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
    HILOG_DEBUG("call");
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
    auto env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        HILOG_ERROR("Error to get jsObj_");
        return;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Error to get JsFormExtension object");
        return;
    }

    BindContext(env, obj);
}

void JsFormExtension::BindContext(napi_env env, napi_value obj)
{
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    HILOG_DEBUG("call");
    napi_value contextObj = CreateJsFormExtensionContext(env, context);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.FormExtensionContext", &contextObj, 1);
    if (shellContextRef_ == nullptr) {
        HILOG_ERROR("Failed to load module");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_ERROR("Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<FormExtensionContext>(context);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachFormExtensionContext, workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);

    napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            HILOG_INFO("Finalizer for weak_ptr form extension context is called");
            delete static_cast<std::weak_ptr<FormExtensionContext>*>(data);
        },
        nullptr, nullptr);

    HILOG_DEBUG("ok");
}

OHOS::AppExecFwk::FormProviderInfo JsFormExtension::OnCreate(const OHOS::AAFwk::Want& want)
{
    HILOG_INFO("call");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    napi_value nativeResult = CallObjectMethod("onAddForm", "onCreate", argv, 1);

    OHOS::AppExecFwk::FormProviderInfo formProviderInfo;
    if (!CheckTypeForNapiValue(env, nativeResult, napi_object)) {
        HILOG_ERROR("nativeResult is nullptr.");
        return formProviderInfo;
    }

    napi_value nativeDataValue = nullptr;
    napi_get_named_property(env, nativeResult, "data", &nativeDataValue);
    if (nativeDataValue == nullptr) {
        HILOG_ERROR("nativeResult get data is nullptr.");
        return formProviderInfo;
    }
    std::string formDataStr;
    if (!ConvertFromJsValue(env, nativeDataValue, formDataStr)) {
        HILOG_ERROR("convert formDataStr failed.");
        return formProviderInfo;
    }
    AppExecFwk::FormProviderData formData = AppExecFwk::FormProviderData(formDataStr);
    formProviderInfo.SetFormData(formData);

    napi_value nativeProxies = nullptr;
    napi_get_named_property(env, nativeResult, "proxies", &nativeProxies);
    std::vector<FormDataProxy> formDataProxies;
    if (nativeProxies != nullptr && !ConvertFromDataProxies(env, nativeProxies, formDataProxies)) {
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
    napi_env env = jsRuntime_.GetNapiEnv();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(env, std::to_string(formId).c_str(), NAPI_AUTO_LENGTH,
        &napiFormId);
    napi_value argv[] = {napiFormId};
    CallObjectMethod("onRemoveForm", "onDestroy", argv, 1);
}

void JsFormExtension::OnStop()
{
    HILOG_INFO("call");
    CallObjectMethod("onStop", nullptr, nullptr, 0);
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        HILOG_INFO("The service extension connection is not disconnected.");
    }
}

void JsFormExtension::OnEvent(const int64_t formId, const std::string& message)
{
    HILOG_INFO("OnEvent, formId: %{public}" PRId64 ".", formId);
    FormExtension::OnEvent(formId, message);

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(env, std::to_string(formId).c_str(),
        NAPI_AUTO_LENGTH, &napiFormId);
    // wrap message
    napi_value napiMessage = nullptr;
    napi_create_string_utf8(env, message.c_str(), NAPI_AUTO_LENGTH, &napiMessage);
    napi_value argv[] = {napiFormId, napiMessage};
    CallObjectMethod("onFormEvent", "onEvent", argv, ON_EVENT_PARAMS_SIZE);
}

void JsFormExtension::OnUpdate(const int64_t formId, const AAFwk::WantParams &wantParams)
{
    HILOG_INFO("OnUpdate, formId: %{public}" PRId64 ".", formId);
    FormExtension::OnUpdate(formId, wantParams);

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(env, std::to_string(formId).c_str(),
        NAPI_AUTO_LENGTH, &napiFormId);
    // wrap wantParams
    napi_value nativeObj = WrapWantParams(env, wantParams);
    napi_value argv[] = {napiFormId, nativeObj};
    CallObjectMethod("onUpdateForm", "onUpdate", argv, ARGC_TWO);
}

void JsFormExtension::OnCastToNormal(const int64_t formId)
{
    HILOG_INFO("OnCastToNormal, formId: %{public}" PRId64 ".", formId);
    FormExtension::OnCastToNormal(formId);

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(env, std::to_string(formId).c_str(), NAPI_AUTO_LENGTH,
        &napiFormId);
    napi_value argv[] = {napiFormId};
    CallObjectMethod("onCastToNormalForm", "onCastToNormal", argv, 1);
}

void JsFormExtension::OnVisibilityChange(const std::map<int64_t, int32_t>& formEventsMap)
{
    HILOG_INFO("call");
    FormExtension::OnVisibilityChange(formEventsMap);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value nativeObj = nullptr;
    napi_create_object(env, &nativeObj);
    for (auto item = formEventsMap.begin(); item != formEventsMap.end(); item++) {
        napi_set_named_property(env, nativeObj, std::to_string(item->first).c_str(), CreateJsValue(env, item->second));
    }
    napi_value argv[] = {nativeObj};
    CallObjectMethod("onChangeFormVisibility", "onVisibilityChange", argv, 1);
}

sptr<IRemoteObject> JsFormExtension::OnConnect(const OHOS::AAFwk::Want& want)
{
    HILOG_DEBUG("call");
    Extension::OnConnect(want);
    if (providerRemoteObject_ == nullptr) {
        HILOG_DEBUG("providerRemoteObject_ is nullptr, need init.");
        sptr<FormExtensionProviderClient> providerClient = new (std::nothrow) FormExtensionProviderClient();
        std::shared_ptr<JsFormExtension> formExtension = std::static_pointer_cast<JsFormExtension>(shared_from_this());
        providerClient->SetOwner(formExtension);
        providerRemoteObject_ = providerClient->AsObject();
    }
    HILOG_DEBUG("ok");
    return providerRemoteObject_;
}

napi_value JsFormExtension::CallObjectMethod(const char* name, const char *bakName, napi_value const* argv,
    size_t argc)
{
    HILOG_DEBUG("CallObjectMethod(%{public}s)", name);
    if (!jsObj_) {
        HILOG_WARN("jsObj_ is nullptr");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get FormExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        HILOG_ERROR("Failed to get '%{public}s' from FormExtension object", name);
        if (bakName == nullptr) {
            return nullptr;
        }
        method = nullptr;
        napi_get_named_property(env, obj, bakName, &method);
        if (!CheckTypeForNapiValue(env, method, napi_function)) {
            HILOG_ERROR("Failed to get '%{public}s' from FormExtension object", bakName);
            return nullptr;
        }
    }
    HILOG_DEBUG("CallFunction(%{public}s), ok", name);
    napi_value result = nullptr;
    napi_call_function(env, obj, method, argc, argv, &result);
    return handleEscape.Escape(result);
}

void JsFormExtension::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    FormExtension::OnConfigurationUpdated(configuration);
    HILOG_INFO("call");

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // Notify extension context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    CallObjectMethod("onConfigurationUpdate", "onConfigurationUpdated", &napiConfiguration, 1);
}

FormState JsFormExtension::OnAcquireFormState(const Want &want)
{
    HILOG_INFO("call");
    auto state = static_cast<int32_t>(FormState::DEFAULT);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);

    napi_value argv[] = { napiWant };
    napi_value nativeResult = CallObjectMethod("onAcquireFormState", nullptr, argv, 1);
    if (nativeResult == nullptr) {
        HILOG_INFO("function onAcquireFormState not found.");
        return FormState::DEFAULT;
    }

    if (!ConvertFromJsValue(env, nativeResult, state)) {
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
    HILOG_DEBUG("end");
}

bool JsFormExtension::OnShare(int64_t formId, AAFwk::WantParams &wantParams)
{
    HILOG_DEBUG("OnShare, formId: %{public}" PRId64 ".", formId);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (env == nullptr) {
        HILOG_ERROR("env is null.");
        return false;
    }

    auto formIdStr = std::to_string(formId);
    napi_value argv[] = { CreateJsValue(env, formIdStr) };
    napi_value nativeResult = CallObjectMethod("onShareForm", "onShare", argv, 1);
    if (nativeResult == nullptr) {
        HILOG_ERROR("OnShare return value is nullptr.");
        return false;
    }

    if (!CheckTypeForNapiValue(env, nativeResult, napi_object)) {
        HILOG_ERROR("OnShare return value`s type is not object.");
        return false;
    }

    if (!OHOS::AppExecFwk::UnwrapWantParams(env, nativeResult, wantParams)) {
        HILOG_ERROR("Unwrap want params error.");
        return false;
    }

    HILOG_DEBUG("OnShare called end.");
    return true;
}

bool JsFormExtension::OnAcquireData(int64_t formId, AAFwk::WantParams &wantParams)
{
    HILOG_DEBUG("OnAcquireData, formId: %{public}" PRId64 ".", formId);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (env == nullptr) {
        HILOG_ERROR("env is nullptr.");
        return false;
    }

    auto formIdStr = std::to_string(formId);
    napi_value argv[] = { CreateJsValue(env, formIdStr) };
    napi_value nativeResult = CallObjectMethod("onAcquireFormData", "OnAcquireData", argv, 1);
    if (nativeResult == nullptr) {
        HILOG_ERROR("return value is nullptr.");
        return false;
    }

    if (!CheckTypeForNapiValue(env, nativeResult, napi_object)) {
        HILOG_ERROR("return value`s type is not object.");
        return false;
    }

    if (!OHOS::AppExecFwk::UnwrapWantParams(env, nativeResult, wantParams)) {
        HILOG_ERROR("Unwrap want params failed.");
        return false;
    }

    HILOG_DEBUG("OnAcquireData called end.");
    return true;
}

bool JsFormExtension::ConvertFromDataProxies(napi_env env, napi_value jsValue,
    std::vector<FormDataProxy> &formDataProxies)
{
    if (jsValue == nullptr) {
        HILOG_ERROR("%{public}s, jsValue is nullptr not array", __func__);
        return false;
    }

    uint32_t len = 0;
    NAPI_CALL_NO_THROW(napi_get_array_length(env, jsValue, &len), false);
    for (uint32_t i = 0; i < len; i++) {
        FormDataProxy formDataProxy("", "");
        napi_value element = nullptr;
        napi_get_element(env, jsValue, i, &element);
        if (!ConvertFormDataProxy(env, element, formDataProxy)) {
            HILOG_ERROR("GetElement from array [%{public}u] error", i);
            continue;
        }
        formDataProxies.push_back(formDataProxy);
    }
    return true;
}

bool JsFormExtension::ConvertFormDataProxy(napi_env env, napi_value jsValue, FormDataProxy &formDataProxy)
{
    if (!CheckTypeForNapiValue(env, jsValue, napi_object)) {
        HILOG_ERROR("%{public}s, jsValue is nullptr not object", __func__);
        return false;
    }

    napi_value key = nullptr;
    napi_get_named_property(env, jsValue, "key", &key);
    if (!ConvertFromJsValue(env, key, formDataProxy.key)) {
        HILOG_ERROR("Parse key failed");
        return false;
    }
    napi_value subscribeId = nullptr;
    napi_get_named_property(env, jsValue, "subscriberId", &subscribeId);
    if (subscribeId != nullptr && !ConvertFromJsValue(env, subscribeId, formDataProxy.subscribeId)) {
        HILOG_WARN("Parse subscriberId failed, use empty as default value.");
        formDataProxy.subscribeId = "";
    }
    HILOG_INFO("key is %{public}s, subscriberId is %{public}s", formDataProxy.key.c_str(),
        formDataProxy.subscribeId.c_str());
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
