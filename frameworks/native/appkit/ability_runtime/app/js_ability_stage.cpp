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
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_ability_stage_context.h"
#include "js_context_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_startup_config.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "startup_manager.h"
#include <algorithm>
#include <cstring>
#include <exception>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
constexpr const char* PROFILE_FILE_PREFIX = "$profile:";
constexpr const char* STARTUP_TASKS = "startupTasks";
constexpr const char* NAME = "name";
constexpr const char* SRC_ENTRY = "srcEntry";
constexpr const char* DEPENDENCIES = "dependencies";
constexpr const char* EXCLUDE_FROM_AUTO_START = "excludeFromAutoStart";
constexpr const char* RUN_ON_THREAD = "runOnThread";
constexpr const char* WAIT_ON_MAIN_THREAD = "waitOnMainThread";
constexpr const char* CONFIG_ENTRY = "configEntry";
    
napi_value AttachAbilityStageContext(napi_env env, void *value, void *)
{
    TAG_LOGD(AAFwkTag::APPKIT, "AttachAbilityStageContext");
    if (env == nullptr || value == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid parameter, env or value is nullptr.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid context.");
        return nullptr;
    }
    napi_value object = CreateJsAbilityStageContext(env, ptr, nullptr, nullptr);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityStageContext", &object, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid systemModule.");
        return nullptr;
    }
    auto contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGW(AAFwkTag::APPKIT, "LoadSystemModuleByEngine or ConvertNativeValueTo failed.");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAbilityStageContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr ability stage context is called");
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
                TAG_LOGW(AAFwkTag::APPKIT, "USE_COMMON_CHUNK = %s{public}s", md.value.c_str());
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
        TAG_LOGW(AAFwkTag::APPKIT, "invalid parameter, runtime is nullptr.");
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
        TAG_LOGD(AAFwkTag::APPKIT, "srcPath is %{public}s", srcPath.c_str());
    }
    return std::make_shared<JsAbilityStage>(jsRuntime, std::move(moduleObj));
}

JsAbilityStage::JsAbilityStage(JsRuntime& jsRuntime, std::unique_ptr<NativeReference>&& jsAbilityStageObj)
    : jsRuntime_(jsRuntime), jsAbilityStageObj_(std::move(jsAbilityStageObj))
{}

JsAbilityStage::~JsAbilityStage()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
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
        TAG_LOGE(AAFwkTag::APPKIT, "context is nullptr");
        return;
    }

    if (!jsAbilityStageObj_) {
        TAG_LOGE(AAFwkTag::APPKIT, "stage is nullptr");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get AbilityStage object");
        return;
    }

    napi_value contextObj = CreateJsAbilityStageContext(env, context, nullptr, nullptr);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityStageContext", &contextObj, 1);
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get LoadSystemModuleByEngine");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(context);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAbilityStageContext, workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    TAG_LOGD(AAFwkTag::APPKIT, "Set ability stage context");
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr ability stage context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context>*>(data);
        },
        nullptr, nullptr);
}

void JsAbilityStage::OnCreate(const AAFwk::Want &want) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    AbilityStage::OnCreate(want);

    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::APPKIT, "Not found AbilityStage.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get AbilityStage object");
        return;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, "onCreate", &methodOnCreate);
    if (methodOnCreate == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get 'onCreate' from AbilityStage object");
        return;
    }
    napi_call_function(env, obj, methodOnCreate, 0, nullptr, nullptr);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::APPKIT, "Call PostPerformStageStart");
        delegator->PostPerformStageStart(CreateStageProperty());
    }
}

void JsAbilityStage::OnDestroy() const
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called");
    AbilityStage::OnDestroy();

    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::APPKIT, "Not found AbilityStage.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get AbilityStage object");
        return;
    }

    napi_value methodOnDestroy = nullptr;
    napi_get_named_property(env, obj, "onDestroy", &methodOnDestroy);
    if (methodOnDestroy == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get 'onDestroy' from AbilityStage object");
        return;
    }
    napi_call_function(env, obj, methodOnDestroy, 0, nullptr, nullptr);
}

std::string JsAbilityStage::OnAcceptWant(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    AbilityStage::OnAcceptWant(want);

    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::APPKIT, "Not found AbilityStage.js");
        return "";
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get AbilityStage object");
        return "";
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value methodOnAcceptWant = nullptr;
    napi_get_named_property(env, obj, "onAcceptWant", &methodOnAcceptWant);
    if (methodOnAcceptWant == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get 'OnAcceptWant' from AbilityStage object");
        return "";
    }

    napi_value argv[] = { napiWant };
    napi_value flagNative = nullptr;
    napi_call_function(env, obj, methodOnAcceptWant, 1, argv, &flagNative);
    return AppExecFwk::UnwrapStringFromJS(env, flagNative);
}


std::string JsAbilityStage::OnNewProcessRequest(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    AbilityStage::OnNewProcessRequest(want);

    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::APPKIT, "Not found AbilityStage.js");
        return "";
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get AbilityStage object");
        return "";
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value methodOnNewProcessRequest = nullptr;
    napi_get_named_property(env, obj, "onNewProcessRequest", &methodOnNewProcessRequest);
    if (methodOnNewProcessRequest == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get 'onNewProcessRequest' from AbilityStage object");
        return "";
    }

    napi_value argv[] = { napiWant };
    napi_value flagNative = nullptr;
    napi_call_function(env, obj, methodOnNewProcessRequest, 1, argv, &flagNative);
    return AppExecFwk::UnwrapStringFromJS(env, flagNative);
}

void JsAbilityStage::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    AbilityStage::OnConfigurationUpdated(configuration);

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    // Notify Ability stage context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::APPKIT, "configuration is nullptr.");
        return;
    }
    JsAbilityStageContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    CallObjectMethod("onConfigurationUpdated", &napiConfiguration, 1);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, 1);
}

void JsAbilityStage::OnMemoryLevel(int32_t level)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    AbilityStage::OnMemoryLevel(level);

    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::APPKIT, "Not found stage");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "OnMemoryLevel, Failed to get AbilityStage object");
        return;
    }

    napi_value jsLevel = CreateJsValue(env, level);
    napi_value argv[] = { jsLevel };
    CallObjectMethod("onMemoryLevel", argv, ArraySize(argv));
    TAG_LOGD(AAFwkTag::APPKIT, "end");
}

int32_t JsAbilityStage::RunAutoStartupTask(const std::function<void()> &callback, bool &isAsyncCallback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    isAsyncCallback = false;
    auto context = GetContext();
    if (!context) {
        HILOG_ERROR("context invalid.");
        return ERR_INVALID_VALUE;
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        HILOG_ERROR("hapModuleInfo invalid.");
        return ERR_INVALID_VALUE;
    }
    if (hapModuleInfo->moduleType != AppExecFwk::ModuleType::ENTRY ||
    hapModuleInfo->appStartup.empty()) {
        HILOG_DEBUG("not entry module or appStartup not exist.");
        return ERR_INVALID_VALUE;
    }

    std::vector<JsStartupTask> jsStartupTasks;
    int32_t result = RegisterStartupTaskFromProfile(jsStartupTasks);
    if (result != ERR_OK) {
        return result;
    }
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        HILOG_ERROR("failed to get startupManager.");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<StartupTaskManager> startupTaskManager = nullptr;
    result = startupManager->BuildAutoStartupTaskManager(startupTaskManager);
    if (result != ERR_OK) {
        return result;
    }
    result = startupTaskManager->Prepare();
    if (result != ERR_OK) {
        return result;
    }
    auto runAutoStartupCallback = std::make_shared<OnCompletedCallback>(
        [callback](const std::shared_ptr<StartupTaskResult> &) {
            HILOG_INFO("RunAutoStartupCallback");
            callback();
        });
    result = startupTaskManager->Run(runAutoStartupCallback);
    if (result != ERR_OK) {
        isAsyncCallback = runAutoStartupCallback->IsCalled();
        return result;
    }
    isAsyncCallback = true;
    return ERR_OK;
}

int32_t JsAbilityStage::RegisterStartupTaskFromProfile(std::vector<JsStartupTask> &jsStartupTasks)
{
    HILOG_DEBUG("RegisterStartupTaskFromProfile called.");
    std::vector<std::string> profileInfo;
    if (!GetProfileInfoFromResourceManager(profileInfo)) {
        HILOG_ERROR("appStartup config not exist.");
        return ERR_INVALID_VALUE;
    }
    
    if (!AnalyzeProfileInfoAndRegisterStartupTask(profileInfo)) {
        HILOG_ERROR("appStartup config not exist.");
        return ERR_INVALID_VALUE;
    }
    
    return ERR_OK;
}

bool JsAbilityStage::GetProfileInfoFromResourceManager(std::vector<std::string> &profileInfo)
{
    HILOG_DEBUG("GetProfileInfoFromResourceManager called.");
    auto context = GetContext();
    if (!context) {
        HILOG_ERROR("context is nullptr.");
        return false;
    }
    
    auto resMgr = context->GetResourceManager();
    if (!resMgr) {
        HILOG_ERROR("resMgr is nullptr.");
        return false;
    }
    
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        HILOG_ERROR("hapModuleInfo is nullptr.");
        return false;
    }
    
    jsRuntime_.UpdateModuleNameAndAssetPath(hapModuleInfo->moduleName);
    bool isCompressed = !hapModuleInfo->hapPath.empty();
    std::string appStartup = hapModuleInfo->appStartup;
    if (appStartup.empty()) {
        HILOG_ERROR("appStartup invalid.");
        return false;
    }
    
    GetResFromResMgr(appStartup, resMgr, isCompressed, profileInfo);
    if (profileInfo.empty()) {
        HILOG_ERROR("appStartup config not exist.");
        return false;
    }
    return true;
}

std::unique_ptr<NativeReference> JsAbilityStage::LoadJsSrcEntry(const std::string &srcEntry)
{
    HILOG_DEBUG("call.");
    if (srcEntry.empty()) {
        HILOG_ERROR("srcEntry invalid.");
        return nullptr;
    }
    auto context = GetContext();
    if (!context) {
        HILOG_ERROR("context is nullptr.");
        return nullptr;
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        HILOG_ERROR("hapModuleInfo is nullptr.");
        return nullptr;
    }

    bool esmodule = hapModuleInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    std::string moduleName(hapModuleInfo->moduleName);
    std::string srcPath(moduleName + "/" + srcEntry);
    
    auto pos = srcPath.rfind('.');
    if (pos == std::string::npos) {
        return nullptr;
    }
    srcPath.erase(pos);
    srcPath.append(".abc");
    
    std::unique_ptr<NativeReference> jsCode(
        jsRuntime_.LoadModule(moduleName, srcPath, hapModuleInfo->hapPath, esmodule));
    
    return jsCode;
}

bool JsAbilityStage::LoadJsStartupConfig(const std::string &srcEntry)
{
    std::unique_ptr<NativeReference> startupConfigEntry = LoadJsSrcEntry(srcEntry);
    if (startupConfigEntry == nullptr) {
        HILOG_ERROR("fail to load config src entry.");
        return false;
    }
    auto env = jsRuntime_.GetNapiEnv();
    std::shared_ptr<JsStartupConfig> startupConfig = std::make_shared<JsStartupConfig>(env);
    if (startupConfig == nullptr) {
        HILOG_ERROR("startupConfig is null.");
        return false;
    }
    if (startupConfig->Init(startupConfigEntry) != ERR_OK) {
        return false;
    }
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        HILOG_ERROR("failed to get startupManager.");
        return false;
    }
    startupManager->SetDefaultConfig(startupConfig);
    return true;
}

void JsAbilityStage::SetOptionalParameters(
    const nlohmann::json &module,
    JsStartupTask &jsStartupTask)
{
    HILOG_DEBUG("SetOptionalParameters called.");
    if (module.contains(DEPENDENCIES) && module[DEPENDENCIES].is_array()) {
        std::vector<std::string> dependencies;
        for (const auto& dependency : module.at(DEPENDENCIES)) {
            if (dependency.is_string()) {
                dependencies.push_back(dependency.get<std::string>());
            }
        }
        jsStartupTask.SetDependencies(dependencies);
    }

    if (module.contains(EXCLUDE_FROM_AUTO_START) && module[EXCLUDE_FROM_AUTO_START].is_boolean()) {
        jsStartupTask.SetIsExcludeFromAutoStart(module.at(EXCLUDE_FROM_AUTO_START).get<bool>());
    } else {
        jsStartupTask.SetIsExcludeFromAutoStart(false);
    }

    // always true
    jsStartupTask.SetCallCreateOnMainThread(true);
    
    if (module.contains(WAIT_ON_MAIN_THREAD) && module[WAIT_ON_MAIN_THREAD].is_boolean()) {
        jsStartupTask.SetWaitOnMainThread(module.at(WAIT_ON_MAIN_THREAD).get<bool>());
    } else {
        jsStartupTask.SetWaitOnMainThread(true);
    }
}

bool JsAbilityStage::AnalyzeProfileInfoAndRegisterStartupTask(const std::vector<std::string> &profileInfo)
{
    HILOG_DEBUG("AnalyzeProfileInfoAndRegisterStartupTask called.");
    std::string startupInfo;
    for (const std::string& info: profileInfo) {
        startupInfo.append(info);
    }
    if (startupInfo.empty()) {
        HILOG_ERROR("startupInfo invalid.");
        return false;
    }

    nlohmann::json startupInfoJson = nlohmann::json::parse(startupInfo, nullptr, false);
    if (startupInfoJson.is_discarded()) {
        HILOG_ERROR("Failed to parse json string.");
        return false;
    }

    if (!(startupInfoJson.contains(CONFIG_ENTRY) && startupInfoJson[CONFIG_ENTRY].is_string())) {
        HILOG_ERROR("no config entry.");
        return false;
    }
    if (!LoadJsStartupConfig(startupInfoJson.at(CONFIG_ENTRY).get<std::string>())) {
        HILOG_ERROR("failed to load config entry.");
        return false;
    }

    if (!(startupInfoJson.contains(STARTUP_TASKS) && startupInfoJson[STARTUP_TASKS].is_array())) {
        HILOG_ERROR("startupTasks invalid.");
        return false;
    }
    std::vector<std::shared_ptr<JsStartupTask>> jsStartupTasks;
    for (const auto& module : startupInfoJson.at(STARTUP_TASKS).get<nlohmann::json>()) {
        if (!module.contains(SRC_ENTRY) || !module[SRC_ENTRY].is_string() ||
        !module.contains(NAME) || !module[NAME].is_string()) {
            HILOG_ERROR("Invalid module data.");
            return false;
        }
        
        std::unique_ptr<NativeReference> startupJsRef = LoadJsSrcEntry(module.at(SRC_ENTRY).get<std::string>());
        if (startupJsRef == nullptr) {
            HILOG_ERROR("load js appStartup tasks failed.");
            return false;
        }

        auto jsStartupTask = std::make_shared<JsStartupTask>(
            module.at(NAME).get<std::string>(), jsRuntime_, startupJsRef, shellContextRef_);
        SetOptionalParameters(module, *jsStartupTask);
        jsStartupTasks.push_back(jsStartupTask);
    }
    for (auto &iter : jsStartupTasks) {
        DelayedSingleton<StartupManager>::GetInstance()->RegisterStartupTask(iter->GetName(), iter);
    }
    return true;
}

napi_value JsAbilityStage::CallObjectMethod(const char* name, napi_value const * argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::APPKIT, "call %{public}s", name);
    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::APPKIT, "Not found AbilityStage.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get AbilityStage object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get '%{public}s' from AbilityStage object", name);
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
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get context");
        return std::string();
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get hapModuleInfo");
        return std::string();
    }
    if (propName.compare("name") == 0) {
        return hapModuleInfo->name;
    }
    if (propName.compare("srcEntrance") == 0) {
        return hapModuleInfo->srcEntrance;
    }
    TAG_LOGE(AAFwkTag::APPKIT, "name = %{public}s", propName.c_str());
    return std::string();
}

bool JsAbilityStage::IsFileExisted(const std::string &filePath)
{
    if (filePath.empty()) {
        HILOG_ERROR("the file is not existed due to empty file path.");
        return false;
    }

    if (access(filePath.c_str(), F_OK) != 0) {
        HILOG_ERROR("can not access the file: %{private}s, errno:%{public}d.", filePath.c_str(), errno);
        return false;
    }
    return true;
}

bool JsAbilityStage::TransformFileToJsonString(const std::string &resPath, std::string &profile)
{
    if (!IsFileExisted(resPath)) {
        HILOG_ERROR("the file is not existed.");
        return false;
    }
    std::fstream in;
    in.open(resPath, std::ios_base::in | std::ios_base::binary);
    if (!in.is_open()) {
        HILOG_ERROR("the file cannot be open errno:%{public}d.", errno);
        return false;
    }
    in.seekg(0, std::ios::end);
    int64_t size = in.tellg();
    if (size <= 0) {
        HILOG_ERROR("the file is an empty file, errno:%{public}d.", errno);
        in.close();
        return false;
    }
    in.seekg(0, std::ios::beg);
    nlohmann::json profileJson = nlohmann::json::parse(in, nullptr, false);
    if (profileJson.is_discarded()) {
        HILOG_ERROR("bad profile file.");
        in.close();
        return false;
    }
    profile = profileJson.dump();
    in.close();
    return true;
}

bool JsAbilityStage::GetResFromResMgr(
    const std::string &resName,
    const std::shared_ptr<Global::Resource::ResourceManager> &resMgr,
    bool isCompressed, std::vector<std::string> &profileInfo)
{
    if (resName.empty()) {
        HILOG_ERROR("res name is empty.");
        return false;
    }
    
    size_t pos = resName.rfind(PROFILE_FILE_PREFIX);
    if ((pos == std::string::npos) || (pos == resName.length() - strlen(PROFILE_FILE_PREFIX))) {
        HILOG_ERROR("res name %{public}s is invalid.", resName.c_str());
        return false;
    }
    std::string profileName = resName.substr(pos + strlen(PROFILE_FILE_PREFIX));
        // hap is compressed status, get file content.
    if (isCompressed) {
        HILOG_DEBUG("compressed status.");
        std::unique_ptr<uint8_t[]> fileContentPtr = nullptr;
        size_t len = 0;
        if (resMgr->GetProfileDataByName(profileName.c_str(), len, fileContentPtr) != Global::Resource::SUCCESS) {
            HILOG_ERROR("GetProfileDataByName failed.");
            return false;
        }
        if (fileContentPtr == nullptr || len == 0) {
            HILOG_ERROR("invalid data.");
            return false;
        }
        std::string rawData(fileContentPtr.get(), fileContentPtr.get() + len);
        nlohmann::json profileJson = nlohmann::json::parse(rawData, nullptr, false);
        if (profileJson.is_discarded()) {
            HILOG_ERROR("bad profile file.");
            return false;
        }
        profileInfo.emplace_back(profileJson.dump());
        return true;
    }
    // hap is decompressed status, get file path then read file.
    std::string resPath;
    if (resMgr->GetProfileByName(profileName.c_str(), resPath) != Global::Resource::SUCCESS) {
        HILOG_DEBUG("profileName cannot be found.");
        return false;
    }
    HILOG_DEBUG("resPath is %{private}s.", resPath.c_str());
    std::string profile;
    if (!TransformFileToJsonString(resPath, profile)) {
        HILOG_ERROR("Transform file to json string filed.");
        return false;
    }
    profileInfo.emplace_back(profile);
    return true;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
