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
        HILOG_DEBUG("srcPath is %{public}s", srcPath.c_str());
    }
    return std::make_shared<JsAbilityStage>(jsRuntime, std::move(moduleObj));
}

JsAbilityStage::JsAbilityStage(JsRuntime& jsRuntime, std::unique_ptr<NativeReference>&& jsAbilityStageObj)
    : jsRuntime_(jsRuntime), jsAbilityStageObj_(std::move(jsAbilityStageObj))
{}

JsAbilityStage::~JsAbilityStage()
{
    HILOG_DEBUG("called");
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
        HILOG_ERROR("stage is nullptr");
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
    HILOG_DEBUG("called");
    AbilityStage::OnCreate(want);

    if (!jsAbilityStageObj_) {
        HILOG_WARN("Not found AbilityStage.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get AbilityStage object");
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
        HILOG_DEBUG("Call PostPerformStageStart");
        delegator->PostPerformStageStart(CreateStageProperty());
    }
}

std::string JsAbilityStage::OnAcceptWant(const AAFwk::Want &want)
{
    HILOG_DEBUG("called");
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
    HILOG_DEBUG("called");
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
    HILOG_DEBUG("called");
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
    HILOG_DEBUG("called");
    AbilityStage::OnMemoryLevel(level);

    if (!jsAbilityStageObj_) {
        HILOG_WARN("Not found stage");
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
    HILOG_DEBUG("end");
}

int32_t JsAbilityStage::RunAutoStartupTask(bool &waitingForStartup)
{
    HILOG_DEBUG("called.");
    waitingForStartup = false;
    return ERR_OK;
}

int32_t JsAbilityStage::RunAutoStartupTaskInner(bool &waitingForStartup)
{
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
    std::shared_ptr<StartupTaskManager> startupTaskManager = nullptr;
    result = DelayedSingleton<StartupManager>::GetInstance()->BuildAutoStartupTaskManager(startupTaskManager);
    if (result != ERR_OK) {
        return result;
    }
    startupTaskManager->Prepare();
    startupTaskManager->Run();
    waitingForStartup = true;
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
    
    if (!AnalyzeProfileInfoAndRegisterStartupTask(profileInfo, jsStartupTasks)) {
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

std::unique_ptr<NativeReference> JsAbilityStage::LoadJsStartupTask(const std::string &srcEntry)
{
    HILOG_DEBUG("LoadJsStartupTask called.");
    if (srcEntry.empty()) {
        HILOG_ERROR("srcEntry invalid.");
        return std::unique_ptr<NativeReference>();
    }
    auto context = GetContext();
    if (!context) {
        HILOG_ERROR("context is nullptr.");
        return std::unique_ptr<NativeReference>();
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        HILOG_ERROR("hapModuleInfo is nullptr.");
        return std::unique_ptr<NativeReference>();
    }

    bool esmodule = hapModuleInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    std::string moduleName(hapModuleInfo->moduleName);
    std::string srcPath(moduleName + "/" + srcEntry);
    
    auto pos = srcPath.rfind('.');
    if (pos == std::string::npos) {
        return std::unique_ptr<NativeReference>();
    }
    srcPath.erase(pos);
    srcPath.append(".abc");
    
    std::unique_ptr<NativeReference> jsCode(
        jsRuntime_.LoadModule(moduleName, srcPath, hapModuleInfo->hapPath, esmodule));
    
    return jsCode;
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
        jsStartupTask.SetIsAutoStartup(module.at(EXCLUDE_FROM_AUTO_START).get<bool>());
    } else {
        jsStartupTask.SetIsAutoStartup(true);
    }
    
    // always true
    jsStartupTask.SetIsAutoStartup(true);
    
    if (module.contains(WAIT_ON_MAIN_THREAD) && module[WAIT_ON_MAIN_THREAD].is_boolean()) {
        jsStartupTask.SetWaitOnMainThread(module.at(WAIT_ON_MAIN_THREAD).get<bool>());
    } else {
        jsStartupTask.SetWaitOnMainThread(false);
    }
}

bool JsAbilityStage::AnalyzeProfileInfoAndRegisterStartupTask(
    std::vector<std::string> &profileInfo,
    std::vector<JsStartupTask> &jsStartupTasks)
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
    
    if (!(startupInfoJson.contains(STARTUP_TASKS) && startupInfoJson[STARTUP_TASKS].is_array())) {
        HILOG_ERROR("startupTasks invalid.");
        return false;
    }
    for (const auto& module : startupInfoJson.at(STARTUP_TASKS).get<nlohmann::json>()) {
        if (!module.contains(SRC_ENTRY) || !module[SRC_ENTRY].is_string() ||
        !module.contains(NAME) || !module[NAME].is_string()) {
            HILOG_ERROR("Invalid module data.");
            return false;
        }
        
        std::shared_ptr<NativeReference> startupJsRef = LoadJsStartupTask(module.at(SRC_ENTRY).get<std::string>());
        if (startupJsRef == nullptr) {
            HILOG_ERROR("load js appStartup tasks failed.");
            return false;
        }
        
        std::shared_ptr<NativeReference> contextJsRef = nullptr;
        JsStartupTask jsStartupTask = JsStartupTask(
            module.at(NAME).get<std::string>(),
            jsRuntime_,
            startupJsRef,
            contextJsRef);
        SetOptionalParameters(module, jsStartupTask);
        jsStartupTasks.push_back(jsStartupTask);
    }
    return true;
}

napi_value JsAbilityStage::CallObjectMethod(const char* name, napi_value const * argv, size_t argc)
{
    HILOG_DEBUG("call %{public}s", name);
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
    HILOG_ERROR("name = %{public}s", propName.c_str());
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
