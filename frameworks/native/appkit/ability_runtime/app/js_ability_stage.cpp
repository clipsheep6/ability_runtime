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

#include "js_ability_stage.h"

#include "ability_delegator_registry.h"
#include "hilog_wrapper.h"
#include "js_ability_stage_context.h"
#include "js_context_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_startup_task.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "startup_manager.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value AttachAbilityStageContext(napi_env env, void *value, void *)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "AttachAbilityStageContext");
    if (env == nullptr || value == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "invalid parameter, env or value is nullptr.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "invalid context.");
        return nullptr;
    }
    napi_value object = CreateJsAbilityStageContext(env, ptr, nullptr, nullptr);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityStageContext", &object, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "invalid systemModule.");
        return nullptr;
    }
    auto contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGW(AAFwkTag::UIABILITY, "LoadSystemModuleByEngine or ConvertNativeValueTo failed.");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAbilityStageContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::UIABILITY, "Finalizer for weak_ptr ability stage context is called");
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
                TAG_LOGW(AAFwkTag::UIABILITY, "USE_COMMON_CHUNK = %s{public}s", md.value.c_str());
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
        TAG_LOGW(AAFwkTag::UIABILITY, "invalid parameter, runtime is nullptr.");
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
        TAG_LOGD(AAFwkTag::UIABILITY, "srcPath is %{public}s", srcPath.c_str());
    }
    return std::make_shared<JsAbilityStage>(jsRuntime, std::move(moduleObj));
}

JsAbilityStage::JsAbilityStage(JsRuntime& jsRuntime, std::unique_ptr<NativeReference>&& jsAbilityStageObj)
    : jsRuntime_(jsRuntime), jsAbilityStageObj_(std::move(jsAbilityStageObj))
{}

JsAbilityStage::~JsAbilityStage()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
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
        TAG_LOGE(AAFwkTag::UIABILITY, "context is nullptr");
        return;
    }

    if (!jsAbilityStageObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "stage is nullptr");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get AbilityStage object");
        return;
    }

    napi_value contextObj = CreateJsAbilityStageContext(env, context, nullptr, nullptr);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityStageContext", &contextObj, 1);
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get LoadSystemModuleByEngine");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(context);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAbilityStageContext, workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    TAG_LOGD(AAFwkTag::UIABILITY, "Set ability stage context");
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            TAG_LOGD(AAFwkTag::UIABILITY, "Finalizer for weak_ptr ability stage context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context>*>(data);
        },
        nullptr, nullptr);
}

void JsAbilityStage::OnCreate(const AAFwk::Want &want) const
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    AbilityStage::OnCreate(want);

    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::UIABILITY, "Not found AbilityStage.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get AbilityStage object");
        return;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, "onCreate", &methodOnCreate);
    if (methodOnCreate == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get 'onCreate' from AbilityStage object");
        return;
    }
    napi_call_function(env, obj, methodOnCreate, 0, nullptr, nullptr);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call PostPerformStageStart");
        delegator->PostPerformStageStart(CreateStageProperty());
    }
}

std::string JsAbilityStage::OnAcceptWant(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    AbilityStage::OnAcceptWant(want);

    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::UIABILITY, "Not found AbilityStage.js");
        return "";
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get AbilityStage object");
        return "";
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value methodOnAcceptWant = nullptr;
    napi_get_named_property(env, obj, "onAcceptWant", &methodOnAcceptWant);
    if (methodOnAcceptWant == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get 'OnAcceptWant' from AbilityStage object");
        return "";
    }

    napi_value argv[] = { napiWant };
    napi_value flagNative = nullptr;
    napi_call_function(env, obj, methodOnAcceptWant, 1, argv, &flagNative);
    return AppExecFwk::UnwrapStringFromJS(env, flagNative);
}


std::string JsAbilityStage::OnNewProcessRequest(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    AbilityStage::OnNewProcessRequest(want);

    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::UIABILITY, "Not found AbilityStage.js");
        return "";
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get AbilityStage object");
        return "";
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value methodOnNewProcessRequest = nullptr;
    napi_get_named_property(env, obj, "onNewProcessRequest", &methodOnNewProcessRequest);
    if (methodOnNewProcessRequest == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get 'onNewProcessRequest' from AbilityStage object");
        return "";
    }

    napi_value argv[] = { napiWant };
    napi_value flagNative = nullptr;
    napi_call_function(env, obj, methodOnNewProcessRequest, 1, argv, &flagNative);
    return AppExecFwk::UnwrapStringFromJS(env, flagNative);
}

void JsAbilityStage::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    AbilityStage::OnConfigurationUpdated(configuration);

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    // Notify Ability stage context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::UIABILITY, "configuration is nullptr.");
        return;
    }
    JsAbilityStageContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    CallObjectMethod("onConfigurationUpdated", &napiConfiguration, 1);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, 1);
}

void JsAbilityStage::OnMemoryLevel(int32_t level)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    AbilityStage::OnMemoryLevel(level);

    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::UIABILITY, "Not found stage");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "OnMemoryLevel, Failed to get AbilityStage object");
        return;
    }

    napi_value jsLevel = CreateJsValue(env, level);
    napi_value argv[] = { jsLevel };
    CallObjectMethod("onMemoryLevel", argv, ArraySize(argv));
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

int32_t JsAbilityStage::RunAutoStartupTask(bool &waitingForStartup)
{
    HILOG_DEBUG("called");
    waitingForStartup = false;
    return ERR_OK;
}

int32_t JsAbilityStage::RunAutoStartupTaskInner(bool &waitingForStartup)
{
    int32_t result = RegisterStartupTaskFromProfile();
    if (result != ERR_OK) {
        waitingForStartup = false;
        return result;
    }
    std::shared_ptr<StartupTaskManager> startupTaskManager = nullptr;
    result = DelayedSingleton<StartupManager>::GetInstance()->BuildAutoStartupTaskManager(startupTaskManager);
    if (result != ERR_OK) {
        waitingForStartup = false;
        return result;
    }
    startupTaskManager->Prepare();
    startupTaskManager->Run();
    waitingForStartup = true;
    return ERR_OK;
}

int32_t JsAbilityStage::RegisterStartupTaskFromProfile()
{
    // GetProfileInfoFromResourceManager
    // AnalyzeProfileInfoAndRegisterStartupTask
    // register test
    std::shared_ptr<NativeReference> jsStartupTaskObj = nullptr;
    std::shared_ptr<JsStartupTask> task = std::make_shared<JsStartupTask>("test", jsRuntime_, jsStartupTaskObj,
        shellContextRef_);
    task->Init();
    DelayedSingleton<StartupManager>::GetInstance()->RegisterStartupTask("test", task);
    return ERR_OK;
}

napi_value JsAbilityStage::CallObjectMethod(const char* name, napi_value const * argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "call %{public}s", name);
    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::UIABILITY, "Not found AbilityStage.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get AbilityStage object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get '%{public}s' from AbilityStage object", name);
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
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get context");
        return std::string();
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get hapModuleInfo");
        return std::string();
    }
    if (propName.compare("name") == 0) {
        return hapModuleInfo->name;
    }
    if (propName.compare("srcEntrance") == 0) {
        return hapModuleInfo->srcEntrance;
    }
    TAG_LOGE(AAFwkTag::UIABILITY, "name = %{public}s", propName.c_str());
    return std::string();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
