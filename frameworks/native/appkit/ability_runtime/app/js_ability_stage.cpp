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

#include "js_ability_stage.h"

#include "ability_delegator_registry.h"
#include "hilog_wrapper.h"
#include "js_ability_stage_context.h"
#include "js_context_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value AttachAbilityStageContext(napi_env env, void *value, void *)
{
    HILOG_DEBUG("AttachAbilityStageContext");
    if (env == nullptr || value == nullptr) {
        HILOG_WARN("invalid parameter, env or value is nullptr.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityContext> *>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }
    napi_value object = CreateJsAbilityStageContext(env, ptr, nullptr, nullptr);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityStageContext", &object, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        return nullptr;
    }
    auto contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_WARN("LoadSystemModuleByEngine or ConvertNativeValueTo failed.");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAbilityStageContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr ability stage context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

bool JsAbilityStage::UseCommonChunk(const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    for (auto &md: hapModuleInfo.metadata) {
        if (md.name == "USE_COMMON_CHUNK") {
            if (md.value != "true") {
                HILOG_WARN("USE_COMMON_CHUNK = %s{public}s", md.value.c_str());
                return false;
            }
            return true;
        }
    }
    return false;
}

std::shared_ptr<AbilityStage> JsAbilityStage::Create(
    const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    if (runtime == nullptr) {
        HILOG_WARN("invalid parameter, runtime is nullptr.");
        return nullptr;
    }
    auto& jsRuntime = static_cast<JsRuntime&>(*runtime);
    std::string srcPath(hapModuleInfo.name);
    std::string moduleName(hapModuleInfo.moduleName);
    moduleName.append("::").append("AbilityStage");
    bool commonChunkFlag = UseCommonChunk(hapModuleInfo);
    /* temporary compatibility api8 + config.json */
    if (!hapModuleInfo.isModuleJson) {
        srcPath.append("/assets/js/");
        if (hapModuleInfo.srcPath.empty()) {
            srcPath.append("AbilityStage.abc");
        } else {
            srcPath.append(hapModuleInfo.srcPath);
            srcPath.append("/AbilityStage.abc");
        }
        auto moduleObj = jsRuntime.LoadModule(moduleName, srcPath, hapModuleInfo.hapPath,
            hapModuleInfo.compileMode == AppExecFwk::CompileMode::ES_MODULE, commonChunkFlag);
        return std::make_shared<JsAbilityStage>(jsRuntime, std::move(moduleObj));
    }

    std::unique_ptr<NativeReference> moduleObj;
    srcPath.append("/");
    if (!hapModuleInfo.srcEntrance.empty()) {
        srcPath.append(hapModuleInfo.srcEntrance);
        srcPath.erase(srcPath.rfind("."));
        srcPath.append(".abc");
        moduleObj = jsRuntime.LoadModule(moduleName, srcPath, hapModuleInfo.hapPath,
            hapModuleInfo.compileMode == AppExecFwk::CompileMode::ES_MODULE, commonChunkFlag);
        HILOG_INFO("JsAbilityStage srcPath is %{public}s", srcPath.c_str());
    }
    return std::make_shared<JsAbilityStage>(jsRuntime, std::move(moduleObj));
}

JsAbilityStage::JsAbilityStage(JsRuntime& jsRuntime, std::unique_ptr<NativeReference>&& jsAbilityStageObj)
    : jsRuntime_(jsRuntime), jsAbilityStageObj_(std::move(jsAbilityStageObj))
{}

JsAbilityStage::~JsAbilityStage()
{
    HILOG_DEBUG("Js ability stage destructor.");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsAbilityStageObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
}

void JsAbilityStage::Init(const std::shared_ptr<Context> &context)
{
    AbilityStage::Init(context);

    if (!context) {
        HILOG_ERROR("context is nullptr");
        return;
    }

    if (!jsAbilityStageObj_) {
        HILOG_ERROR("AbilityStageObj is nullptr");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return;
    }

    napi_value contextObj = CreateJsAbilityStageContext(env, context, nullptr, nullptr);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityStageContext", &contextObj, 1);
    if (shellContextRef_ == nullptr) {
        HILOG_ERROR("Failed to get LoadSystemModuleByEngine");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_ERROR("Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(context);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAbilityStageContext, workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    HILOG_DEBUG("Set ability stage context");
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            HILOG_DEBUG("Finalizer for weak_ptr ability stage context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context>*>(data);
        },
        nullptr, nullptr);
}

void JsAbilityStage::OnCreate(const AAFwk::Want &want) const
{
    HILOG_DEBUG("JsAbilityStage::OnCreate come");
    AbilityStage::OnCreate(want);

    if (!jsAbilityStageObj_) {
        HILOG_WARN("Not found AbilityStage.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("OnCreate, Failed to get AbilityStage object");
        return;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, "onCreate", &methodOnCreate);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get 'onCreate' from AbilityStage object");
        return;
    }
    napi_call_function(env, obj, methodOnCreate, 0, nullptr, nullptr);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call AbilityDelegator::PostPerformStageStart");
        delegator->PostPerformStageStart(CreateStageProperty());
    }
}

std::string JsAbilityStage::OnAcceptWant(const AAFwk::Want &want)
{
    HILOG_DEBUG("JsAbilityStage::OnAcceptWant come");
    AbilityStage::OnAcceptWant(want);

    if (!jsAbilityStageObj_) {
        HILOG_WARN("Not found AbilityStage.js");
        return "";
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return "";
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value methodOnAcceptWant = nullptr;
    napi_get_named_property(env, obj, "onAcceptWant", &methodOnAcceptWant);
    if (methodOnAcceptWant == nullptr) {
        HILOG_ERROR("Failed to get 'OnAcceptWant' from AbilityStage object");
        return "";
    }

    napi_value argv[] = { napiWant };
    napi_value flagNative = nullptr;
    napi_call_function(env, obj, methodOnAcceptWant, 1, argv, &flagNative);
    return AppExecFwk::UnwrapStringFromJS(env, flagNative);
}


std::string JsAbilityStage::OnNewProcessRequest(const AAFwk::Want &want)
{
    HILOG_DEBUG("JsAbilityStage::OnNewProcessRequest come");
    AbilityStage::OnNewProcessRequest(want);

    if (!jsAbilityStageObj_) {
        HILOG_WARN("Not found AbilityStage.js");
        return "";
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return "";
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value methodOnNewProcessRequest = nullptr;
    napi_get_named_property(env, obj, "onNewProcessRequest", &methodOnNewProcessRequest);
    if (methodOnNewProcessRequest == nullptr) {
        HILOG_ERROR("Failed to get 'onNewProcessRequest' from AbilityStage object");
        return "";
    }

    napi_value argv[] = { napiWant };
    napi_value flagNative = nullptr;
    napi_call_function(env, obj, methodOnNewProcessRequest, 1, argv, &flagNative);
    return AppExecFwk::UnwrapStringFromJS(env, flagNative);
}

void JsAbilityStage::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    AbilityStage::OnConfigurationUpdated(configuration);

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    // Notify Ability stage context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }
    JsAbilityStageContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    CallObjectMethod("onConfigurationUpdated", &napiConfiguration, 1);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, 1);
}

void JsAbilityStage::OnMemoryLevel(int32_t level)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    AbilityStage::OnMemoryLevel(level);

    if (!jsAbilityStageObj_) {
        HILOG_WARN("Not found AbilityStage.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("OnMemoryLevel, Failed to get AbilityStage object");
        return;
    }

    napi_value jsLevel = CreateJsValue(env, level);
    napi_value argv[] = { jsLevel };
    CallObjectMethod("onMemoryLevel", argv, ArraySize(argv));
    HILOG_DEBUG("%{public}s end.", __func__);
}

napi_value JsAbilityStage::CallObjectMethod(const char* name, napi_value const * argv, size_t argc)
{
    HILOG_DEBUG("JsAbilityStage::CallObjectMethod %{public}s", name);
    if (!jsAbilityStageObj_) {
        HILOG_WARN("Not found AbilityStage.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        HILOG_ERROR("Failed to get '%{public}s' from AbilityStage object", name);
        return nullptr;
    }

    napi_value result = nullptr;
    napi_call_function(env, obj, method, argc, argv, &result);
    return result;
}

std::shared_ptr<AppExecFwk::DelegatorAbilityStageProperty> JsAbilityStage::CreateStageProperty() const
{
    auto property = std::make_shared<AppExecFwk::DelegatorAbilityStageProperty>();
    property->moduleName_ = GetHapModuleProp("name");
    property->srcEntrance_ = GetHapModuleProp("srcEntrance");
    property->object_ = jsAbilityStageObj_;
    return property;
}

std::string JsAbilityStage::GetHapModuleProp(const std::string &propName) const
{
    auto context = GetContext();
    if (!context) {
        HILOG_ERROR("Failed to get context");
        return std::string();
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        HILOG_ERROR("Failed to get hapModuleInfo");
        return std::string();
    }
    if (propName.compare("name") == 0) {
        return hapModuleInfo->name;
    }
    if (propName.compare("srcEntrance") == 0) {
        return hapModuleInfo->srcEntrance;
    }
    HILOG_ERROR("Failed to GetHapModuleProp name = %{public}s", propName.c_str());
    return std::string();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
