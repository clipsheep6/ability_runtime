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
NativeValue *AttachAbilityStageContext(NativeEngine *engine, void *value, void *)
{
    HILOG_DEBUG("AttachAbilityStageContext");
    if (engine == nullptr || value == nullptr) {
        HILOG_WARN("invalid parameter, engine or value is nullptr.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityContext> *>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }
    NativeValue *object = CreateJsAbilityStageContext(*engine, ptr, nullptr, nullptr);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(engine, "application.AbilityStageContext", &object, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        return nullptr;
    }
    auto contextObj = systemModule->Get();
    NativeObject *nObject = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nObject == nullptr) {
        HILOG_WARN("LoadSystemModuleByEngine or ConvertNativeValueTo failed.");
        return nullptr;
    }
    nObject->ConvertToNativeBindingObject(engine, DetachCallbackFunc, AttachAbilityStageContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(ptr);
    nObject->SetNativePointer(workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr ability stage context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context> *>(data);
        }, nullptr);
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

JsAbilityStage::~JsAbilityStage() = default;

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
    auto& engine = jsRuntime_.GetNativeEngine();

    NativeObject* obj = ConvertNativeValueTo<NativeObject>(jsAbilityStageObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return;
    }

    NativeValue* contextObj = CreateJsAbilityStageContext(engine, context, nullptr, nullptr);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(&engine, "application.AbilityStageContext", &contextObj, 1);
    if (shellContextRef_ == nullptr) {
        HILOG_ERROR("Failed to get LoadSystemModuleByEngine");
        return;
    }
    contextObj = shellContextRef_->Get();
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(context);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachAbilityStageContext,
        workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());
    obj->SetProperty("context", contextObj);
    HILOG_DEBUG("Set ability stage context");

    nativeObj->SetNativePointer(workContext,
        [](NativeEngine*, void* data, void*) {
            HILOG_DEBUG("Finalizer for weak_ptr ability stage context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context>*>(data);
        }, nullptr);
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
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityStageObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("OnCreate, Failed to get AbilityStage object");
        return;
    }

    NativeValue* methodOnCreate = obj->GetProperty("onCreate");
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get 'onCreate' from AbilityStage object");
        return;
    }
    nativeEngine.CallFunction(value, methodOnCreate, nullptr, 0);

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
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityStageObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return "";
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&nativeEngine), want);
    NativeValue* jsWant = reinterpret_cast<NativeValue*>(napiWant);

    NativeValue* methodOnAcceptWant = obj->GetProperty("onAcceptWant");
    if (methodOnAcceptWant == nullptr) {
        HILOG_ERROR("Failed to get 'OnAcceptWant' from AbilityStage object");
        return "";
    }

    NativeValue* argv[] = { jsWant };
    NativeValue* flagNative = nativeEngine.CallFunction(value, methodOnAcceptWant, argv, 1);
    return AppExecFwk::UnwrapStringFromJS(
        reinterpret_cast<napi_env>(&nativeEngine), reinterpret_cast<napi_value>(flagNative));
}

void JsAbilityStage::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    AbilityStage::OnConfigurationUpdated(configuration);

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    // Notify Ability stage context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }
    JsAbilityStageContext::ConfigurationUpdated(&nativeEngine, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(
        reinterpret_cast<napi_env>(&nativeEngine), *fullConfig);
    NativeValue* jsConfiguration = reinterpret_cast<NativeValue*>(napiConfiguration);
    CallObjectMethod("onConfigurationUpdated", &jsConfiguration, 1);
    CallObjectMethod("onConfigurationUpdate", &jsConfiguration, 1);
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
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityStageObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("OnMemoryLevel, Failed to get AbilityStage object");
        return;
    }

    NativeValue *jsLevel = CreateJsValue(nativeEngine, level);
    NativeValue *argv[] = { jsLevel };
    CallObjectMethod("onMemoryLevel", argv, ArraySize(argv));
    HILOG_DEBUG("%{public}s end.", __func__);
}

NativeValue* JsAbilityStage::CallObjectMethod(const char* name, NativeValue * const * argv, size_t argc)
{
    HILOG_DEBUG("JsAbilityStage::CallObjectMethod %{public}s", name);
    if (!jsAbilityStageObj_) {
        HILOG_WARN("Not found AbilityStage.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityStageObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return nullptr;
    }

    NativeValue* method = obj->GetProperty(name);
    if (method == nullptr || method->TypeOf() != NATIVE_FUNCTION) {
        HILOG_ERROR("Failed to get '%{public}s' from AbilityStage object", name);
        return nullptr;
    }

    return nativeEngine.CallFunction(value, method, argv, argc);
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
