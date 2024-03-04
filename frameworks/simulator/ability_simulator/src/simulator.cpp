/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "simulator.h"

#include <condition_variable>
#include <fstream>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "ability_context.h"
#include "ability_stage_context.h"
#include "bundle_container.h"
#include "commonlibrary/ets_utils/js_sys_module/timer/timer.h"
#include "commonlibrary/ets_utils/js_sys_module/console/console.h"
#include "hilog_wrapper.h"
#include "js_ability_context.h"
#include "js_ability_stage_context.h"
#include "js_console_log.h"
#include "js_data_converter.h"
#include "js_module_searcher.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_timer.h"
#include "js_window_stage.h"
#include "json_serializer.h"
#include "launch_param.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "resource_manager.h"
#include "window_scene.h"

extern const char _binary_jsMockSystemPlugin_abc_start[];
extern const char _binary_jsMockSystemPlugin_abc_end[];

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int64_t DEFAULT_GC_POOL_SIZE = 0x10000000; // 256MB
constexpr int32_t DEFAULT_ARK_PROPERTIES = -1;
constexpr size_t DEFAULT_GC_THREAD_NUM = 7;
constexpr size_t DEFAULT_LONG_PAUSE_TIME = 40;

constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";

#if defined(WINDOWS_PLATFORM)
constexpr char ARK_DEBUGGER_LIB_PATH[] = "libark_debugger.dll";
#elif defined(MAC_PLATFORM)
constexpr char ARK_DEBUGGER_LIB_PATH[] = "libark_debugger.dylib";
#else
constexpr char ARK_DEBUGGER_LIB_PATH[] = "libark_debugger.so";
#endif

int32_t PrintVmLog(int32_t, int32_t, const char*, const char*, const char *message)
{
    HILOG_DEBUG("ArkLog: %{public}s", message);
    return 0;
}

template<typename T, size_t N>
inline constexpr size_t ArraySize(T (&)[N]) noexcept
{
    return N;
}

struct DebuggerTask {
    void OnPostTask(std::function<void()> &&task);

    static void HandleTask(const uv_async_t *req);

    uv_async_t onPostTaskSignal {};
    std::function<void()> func;
};

class SimulatorImpl : public Simulator {
public:
    SimulatorImpl() = default;
    ~SimulatorImpl();

    bool Initialize(const Options &options);

    int64_t StartAbility(const std::string &abilityName, TerminateCallback callback) override;
    void TerminateAbility(int64_t abilityId) override;
    void UpdateConfiguration(const AppExecFwk::Configuration &config) override;
    void SetMockList(const std::map<std::string, std::string> &mockList) override;
    void SetHostResolveBufferTracker(ResolveBufferTrackerCallback cb) override;
private:
    bool OnInit();
    void Run();
    napi_value LoadScript(const std::string &srcPath);
    void InitResourceMgr();
    void InitJsAbilityContext(napi_env env, napi_value instanceValue);
    void DispatchStartLifecycle(napi_value instanceValue);
    std::unique_ptr<NativeReference> CreateJsWindowStage(const std::shared_ptr<Rosen::WindowScene> &windowScene);
    napi_value CreateJsWant(napi_env env);
    bool LoadAbilityStage(uint8_t *buffer, size_t len);
    void InitJsAbilityStageContext(napi_value instanceValue);
    napi_value CreateJsLaunchParam(napi_env env);
    bool ParseBundleAndModuleInfo();
    bool ParseAbilityInfo(const std::string &abilitySrcPath);
    bool LoadRuntimeEnv(napi_env env, napi_value globalObject);
    static napi_value RequireNapi(napi_env env, napi_callback_info info);
    inline void SetHostResolveBufferTracker();

    panda::ecmascript::EcmaVM *CreateJSVM();
    Options options_;
    std::string abilityPath_;
    panda::ecmascript::EcmaVM *vm_ = nullptr;
    DebuggerTask debuggerTask_;
    napi_env nativeEngine_ = nullptr;

    int64_t currentId_ = 0;
    std::unordered_map<int64_t, std::shared_ptr<NativeReference>> abilities_;
    std::unordered_map<int64_t, std::shared_ptr<Rosen::WindowScene>> windowScenes_;
    std::unordered_map<int64_t, std::shared_ptr<NativeReference>> jsWindowStages_;
    std::unordered_map<int64_t, std::shared_ptr<NativeReference>> jsContexts_;
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr_;
    std::shared_ptr<AbilityContext> context_;
    std::shared_ptr<NativeReference> abilityStage_;
    std::shared_ptr<AbilityStageContext> stageContext_;
    std::shared_ptr<NativeReference> jsStageContext_;
    std::shared_ptr<AppExecFwk::ApplicationInfo> appInfo_;
    std::shared_ptr<AppExecFwk::HapModuleInfo> moduleInfo_;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo_;
    CallbackTypePostTask postTask_ = nullptr;
};

void DebuggerTask::HandleTask(const uv_async_t *req)
{
    auto *debuggerTask = reinterpret_cast<DebuggerTask*>(req->data);
    if (debuggerTask == nullptr) {
        HILOG_ERROR("HandleTask debuggerTask is null");
        return;
    }
    debuggerTask->func();
}

void DebuggerTask::OnPostTask(std::function<void()> &&task)
{
    if (uv_is_active((uv_handle_t*)&onPostTaskSignal)) {
        func = std::move(task);
        onPostTaskSignal.data = static_cast<void*>(this);
        uv_async_send(&onPostTaskSignal);
    }
}

SimulatorImpl::~SimulatorImpl()
{
    if (nativeEngine_) {
        uv_close(reinterpret_cast<uv_handle_t*>(&debuggerTask_.onPostTaskSignal), nullptr);
        uv_loop_t* uvLoop = nullptr;
        napi_get_uv_event_loop(nativeEngine_, &uvLoop);
        if (uvLoop != nullptr) {
            uv_work_t work;
            uv_queue_work(uvLoop, &work, [](uv_work_t*) {}, [](uv_work_t *work, int32_t status) {
                HILOG_ERROR("Simulator stop uv loop");
                uv_stop(work->loop);
            });
        }
    }

    panda::JSNApi::StopDebugger(vm_);

    abilities_.clear();
    nativeEngine_ = nullptr;
    panda::JSNApi::DestroyJSVM(vm_);
    vm_ = nullptr;
}

bool SimulatorImpl::Initialize(const Options &options)
{
    if (nativeEngine_) {
        HILOG_ERROR("Simulator is already initialized");
        return true;
    }

    options_ = options;
    postTask_ = options.postTask;
    if (!OnInit()) {
        return false;
    }

    uv_loop_t* uvLoop = nullptr;
    napi_get_uv_event_loop(nativeEngine_, &uvLoop);
    if (uvLoop == nullptr) {
        return false;
    }

    uv_async_init(uvLoop, &debuggerTask_.onPostTaskSignal,
        reinterpret_cast<uv_async_cb>(DebuggerTask::HandleTask));

    Run();
    return true;
}

void CallObjectMethod(napi_env env, napi_value obj, const char *name, napi_value const *argv, size_t argc)
{
    if (obj == nullptr) {
        HILOG_ERROR("%{public}s, Failed to get Ability object", __func__);
        return;
    }
    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, name, &methodOnCreate);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from Ability object", name);
        return;
    }
    napi_status status = napi_call_function(env, obj, methodOnCreate, argc, argv, nullptr);
    if (status != napi_ok) {
        HILOG_ERROR("Failed to napi call function");
    }
}

napi_value SimulatorImpl::LoadScript(const std::string &srcPath)
{
    panda::Local<panda::ObjectRef> objRef = panda::JSNApi::GetExportObject(vm_, srcPath, "default");
    if (objRef->IsNull()) {
        HILOG_ERROR("Get export object failed");
        return nullptr;
    }

    auto obj = ArkNativeEngine::ArkValueToNapiValue(nativeEngine_, objRef);
    napi_value instanceValue = nullptr;
    napi_new_instance(nativeEngine_, obj, 0, nullptr, &instanceValue);
    return instanceValue;
}

bool SimulatorImpl::ParseBundleAndModuleInfo()
{
    AppExecFwk::BundleContainer::GetInstance().LoadBundleInfos(options_.moduleJsonBuffer);
    appInfo_ = AppExecFwk::BundleContainer::GetInstance().GetApplicationInfo();
    if (appInfo_ == nullptr) {
        HILOG_ERROR("appinfo parse failed.");
        return false;
    }
    nlohmann::json appInfoJson;
    to_json(appInfoJson, *appInfo_);
    std::cout << "appinfo : " << appInfoJson.dump() << std::endl;

    options_.bundleName = appInfo_->bundleName;
    options_.compatibleVersion = appInfo_->apiCompatibleVersion;
    options_.installationFree = (appInfo_->bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE ? true : false);
    options_.targetVersion = appInfo_->apiTargetVersion;
    options_.releaseType = appInfo_->apiReleaseType;
    options_.compileMode = "esmodule";

    if (appInfo_->moduleInfos.empty()) {
        HILOG_ERROR("module name is not exist");
        return false;
    }
    options_.moduleName = appInfo_->moduleInfos[0].moduleName;
    std::cout << "module name is " << options_.moduleName << std::endl;

    moduleInfo_ = AppExecFwk::BundleContainer::GetInstance().GetHapModuleInfo(options_.moduleName);
    if (moduleInfo_ == nullptr) {
        HILOG_ERROR("module info parse failed.");
        return false;
    }
    nlohmann::json moduleInfoJson;
    to_json(moduleInfoJson, *moduleInfo_);
    std::cout << "moduleInfo : " << moduleInfoJson.dump() << std::endl;

    options_.pageProfile = moduleInfo_->pages;
    options_.enablePartialUpdate = true;
    for (auto iter : moduleInfo_->metadata) {
        if (iter.name == "ArkTSPartialUpdate" && iter.value == "false") {
            options_.enablePartialUpdate = false;
            break;
        }
    }
    return true;
}

bool SimulatorImpl::ParseAbilityInfo(const std::string &abilitySrcPath)
{
    auto path = abilitySrcPath;
    path.erase(path.rfind("."));
    auto abilityName = path.substr(path.rfind('/') + 1, path.length());
    abilityInfo_ = AppExecFwk::BundleContainer::GetInstance().GetAbilityInfo(options_.moduleName, abilityName);
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("ability info parse failed.");
        return false;
    }
    nlohmann::json json;
    to_json(json, *abilityInfo_);
    std::cout << "abilityInfo : " << json.dump() << std::endl;

    options_.labelId = abilityInfo_->labelId;
    return true;
}

int64_t SimulatorImpl::StartAbility(const std::string &abilitySrcPath, TerminateCallback callback)
{
    if (!ParseAbilityInfo(abilitySrcPath)) {
        return -1;
    }

    if (stageContext_ == nullptr) {
        stageContext_ = std::make_shared<AbilityStageContext>();
        stageContext_->SetOptions(options_);
        stageContext_->SetConfiguration(options_.configuration);
        stageContext_->SetApplicationInfo(appInfo_);
        stageContext_->SetHapModuleInfo(moduleInfo_);
    }

    std::ifstream stream(options_.modulePath, std::ios::ate | std::ios::binary);
    if (!stream.is_open()) {
        HILOG_ERROR("Failed to open: %{public}s", options_.modulePath.c_str());
        return -1;
    }

    size_t len = stream.tellg();
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(len);
    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(buffer.get()), len);
    stream.close();

    auto buf = buffer.release();
    if (!LoadAbilityStage(buf, len)) {
        HILOG_ERROR("Load ability stage failed.");
        return -1;
    }

    abilityPath_ = BUNDLE_INSTALL_PATH + options_.moduleName + "/" + abilitySrcPath;
    if (!reinterpret_cast<NativeEngine*>(nativeEngine_)->RunScriptBuffer(abilityPath_, buf, len, false)) {
        HILOG_ERROR("Failed to run script: %{public}s", abilityPath_.c_str());
        return -1;
    }

    napi_value instanceValue = LoadScript(abilityPath_);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return -1;
    }

    ++currentId_;
    InitResourceMgr();
    InitJsAbilityContext(nativeEngine_, instanceValue);
    DispatchStartLifecycle(instanceValue);
    napi_ref ref = nullptr;
    napi_create_reference(nativeEngine_, instanceValue, 1, &ref);
    abilities_.emplace(currentId_, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    return currentId_;
}

bool SimulatorImpl::LoadAbilityStage(uint8_t *buffer, size_t len)
{
    if (moduleInfo_ == nullptr) {
        HILOG_ERROR("moduleInfo is nullptr");
        return false;
    }

    if (moduleInfo_->srcEntrance.empty()) {
        HILOG_DEBUG("module src path is empty.");
        return true;
    }

    if (nativeEngine_ == nullptr) {
        HILOG_ERROR("nativeEngine_ is nullptr");
        return false;
    }
    std::string srcEntrance = moduleInfo_->srcEntrance;
    srcEntrance.erase(srcEntrance.rfind("."));
    srcEntrance.append(".abc");
    srcEntrance = srcEntrance.substr(srcEntrance.find('/') + 1, srcEntrance.length());

    auto moduleSrcPath = BUNDLE_INSTALL_PATH + options_.moduleName + "/" + srcEntrance;
    HILOG_DEBUG("moduleSrcPath is %{public}s", moduleSrcPath.c_str());
    if (!reinterpret_cast<NativeEngine*>(nativeEngine_)->RunScriptBuffer(moduleSrcPath, buffer, len, false)) {
        HILOG_ERROR("Failed to run ability stage script: %{public}s", moduleSrcPath.c_str());
        return false;
    }

    napi_value instanceValue = LoadScript(moduleSrcPath);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create ability stage instance");
        return false;
    }

    InitJsAbilityStageContext(instanceValue);
    CallObjectMethod(nativeEngine_, instanceValue, "onCreate", nullptr, 0);

    napi_value wantArgv[] = {
        CreateJsWant(nativeEngine_)
    };
    CallObjectMethod(nativeEngine_, instanceValue, "onAcceptWant", wantArgv, ArraySize(wantArgv));
    napi_ref ref = nullptr;
    napi_create_reference(nativeEngine_, instanceValue, 1, &ref);
    abilityStage_ = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
    return true;
}

void SimulatorImpl::InitJsAbilityStageContext(napi_value obj)
{
    napi_value contextObj = CreateJsAbilityStageContext(nativeEngine_, stageContext_);
    if (contextObj == nullptr) {
        HILOG_ERROR("contextObj is nullptr");
        return;
    }

    jsStageContext_ = std::shared_ptr<NativeReference>(
        JsRuntime::LoadSystemModuleByEngine(nativeEngine_, "application.AbilityStageContext", &contextObj, 1));
    if (jsStageContext_ == nullptr) {
        HILOG_ERROR("Failed to get LoadSystemModuleByEngine");
        return;
    }

    contextObj = jsStageContext_->GetNapiValue();
    if (contextObj == nullptr) {
        HILOG_ERROR("contextObj is nullptr.");
        return;
    }

    if (obj == nullptr) {
        HILOG_ERROR("obj is nullptr");
        return;
    }
    napi_set_named_property(nativeEngine_, obj, "context", contextObj);
}

void SimulatorImpl::TerminateAbility(int64_t abilityId)
{
    if (abilityId == 0 && abilities_.begin() != abilities_.end()) {
        TerminateAbility(abilities_.begin()->first);
        return;
    }

    auto it = abilities_.find(abilityId);
    if (it == abilities_.end()) {
        return;
    }

    std::shared_ptr<NativeReference> ref = it->second;
    abilities_.erase(it);

    auto instanceValue = ref->GetNapiValue();
    if (instanceValue == nullptr) {
        return;
    }

    CallObjectMethod(nativeEngine_, instanceValue, "onBackground", nullptr, 0);
    CallObjectMethod(nativeEngine_, instanceValue, "onWindowStageDestroy", nullptr, 0);
    CallObjectMethod(nativeEngine_, instanceValue, "onDestroy", nullptr, 0);

    auto windowSceneIter = windowScenes_.find(abilityId);
    if (windowSceneIter != windowScenes_.end()) {
        windowScenes_.erase(windowSceneIter);
    }

    auto windowStageIter = jsWindowStages_.find(abilityId);
    if (windowStageIter != jsWindowStages_.end()) {
        jsWindowStages_.erase(windowStageIter);
    }

    auto jsContextIter = jsContexts_.find(abilityId);
    if (jsContextIter != jsContexts_.end()) {
        jsContexts_.erase(jsContextIter);
    }
}

void SimulatorImpl::UpdateConfiguration(const AppExecFwk::Configuration &config)
{
    HILOG_DEBUG("called.");
    if (abilityStage_ == nullptr) {
        HILOG_ERROR("abilityStage_ is nullptr");
        return;
    }

    auto configuration = std::make_shared<AppExecFwk::Configuration>(config);
    if (configuration == nullptr) {
        return;
    }

    if (stageContext_) {
        stageContext_->SetConfiguration(configuration);
    }

    napi_value configArgv[] = {
        CreateJsConfiguration(nativeEngine_, config)
    };

    auto abilityStage = abilityStage_->GetNapiValue();
    if (abilityStage == nullptr) {
        HILOG_ERROR("abilityStage is nullptr");
        return;
    }
    CallObjectMethod(nativeEngine_, abilityStage, "onConfigurationUpdated", configArgv, ArraySize(configArgv));
    CallObjectMethod(nativeEngine_, abilityStage, "onConfigurationUpdate", configArgv, ArraySize(configArgv));
    JsAbilityStageContext::ConfigurationUpdated(nativeEngine_, jsStageContext_, configuration);

    for (auto iter = abilities_.begin(); iter != abilities_.end(); iter++) {
        auto ability = iter->second->GetNapiValue();
        if (ability == nullptr) {
            HILOG_ERROR("ability is nullptr");
            continue;
        }

        CallObjectMethod(nativeEngine_, ability, "onConfigurationUpdated", configArgv, ArraySize(configArgv));
        CallObjectMethod(nativeEngine_, ability, "onConfigurationUpdate", configArgv, ArraySize(configArgv));
        JsAbilityContext::ConfigurationUpdated(nativeEngine_, iter->second, configuration);
    }
}

void SimulatorImpl::SetMockList(const std::map<std::string, std::string> &mockList)
{
    HILOG_DEBUG("called. mockList size: %{public}zu", mockList.size());
    panda::JSNApi::SetMockModuleList(vm_, mockList);
}

void SimulatorImpl::InitResourceMgr()
{
    HILOG_DEBUG("called.");
    resourceMgr_ = std::shared_ptr<Global::Resource::ResourceManager>(Global::Resource::CreateResourceManager());
    if (resourceMgr_ == nullptr) {
        HILOG_ERROR("resourceMgr is nullptr");
        return;
    }

    if (!resourceMgr_->AddResource(options_.resourcePath.c_str())) {
        HILOG_ERROR("Add resource failed.");
    }
    HILOG_DEBUG("Add resource success.");
}

void SimulatorImpl::InitJsAbilityContext(napi_env env, napi_value obj)
{
    if (context_ == nullptr) {
        context_ = std::make_shared<AbilityContext>();
        context_->SetSimulator(static_cast<Simulator*>(this));
        context_->SetOptions(options_);
        context_->SetAbilityStageContext(stageContext_);
        context_->SetResourceManager(resourceMgr_);
        context_->SetAbilityInfo(abilityInfo_);
    }
    napi_value contextObj = CreateJsAbilityContext(nativeEngine_, context_);
    auto systemModule = std::shared_ptr<NativeReference>(
        JsRuntime::LoadSystemModuleByEngine(nativeEngine_, "application.AbilityContext", &contextObj, 1));
    if (systemModule == nullptr) {
        HILOG_ERROR("systemModule is nullptr.");
        return;
    }
    contextObj = systemModule->GetNapiValue();
    if (contextObj == nullptr) {
        HILOG_ERROR("contextObj is nullptr.");
        return;
    }

    if (obj == nullptr) {
        HILOG_ERROR("obj is nullptr");
        return;
    }
    napi_set_named_property(env, obj, "context", contextObj);
    jsContexts_.emplace(currentId_, systemModule);
}

napi_value SimulatorImpl::CreateJsWant(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "deviceId", CreateJsValue(env, std::string("")));
    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, options_.bundleName));
    if (abilityInfo_) {
        napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, abilityInfo_->name));
    }
    napi_set_named_property(env, objValue, "moduleName", CreateJsValue(env, options_.moduleName));

    napi_set_named_property(env, objValue, "uri", CreateJsValue(env, std::string("")));
    napi_set_named_property(env, objValue, "type", CreateJsValue(env, std::string("")));
    napi_set_named_property(env, objValue, "flags", CreateJsValue(env, 0));
    napi_set_named_property(env, objValue, "type", CreateJsValue(env, std::string("")));
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_set_named_property(env, objValue, "parameters", object);
    napi_value array = nullptr;
    napi_create_array_with_length(env, 0, &array);
    napi_set_named_property(env, objValue, "entities", array);
    return objValue;
}

napi_value SimulatorImpl::CreateJsLaunchParam(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "launchReason", CreateJsValue(env, AAFwk::LAUNCHREASON_UNKNOWN));
    napi_set_named_property(env, objValue, "lastExitReason", CreateJsValue(env, AAFwk::LASTEXITREASON_UNKNOWN));
    napi_set_named_property(env, objValue, "lastExitMessage", CreateJsValue(env, std::string("")));
    return objValue;
}

void SimulatorImpl::DispatchStartLifecycle(napi_value instanceValue)
{
    napi_value wantArgv[] = {
        CreateJsWant(nativeEngine_),
        CreateJsLaunchParam(nativeEngine_)
    };
    CallObjectMethod(nativeEngine_, instanceValue, "onCreate", wantArgv, ArraySize(wantArgv));
    auto windowScene = std::make_shared<Rosen::WindowScene>();
    if (windowScene == nullptr) {
        return;
    }
    sptr<Rosen::IWindowLifeCycle> listener = nullptr;
    windowScene->Init(-1, context_, listener);
    auto jsWindowStage = CreateJsWindowStage(windowScene);
    if (jsWindowStage == nullptr) {
        return;
    }
    napi_value argv[] = { jsWindowStage->GetNapiValue() };
    CallObjectMethod(nativeEngine_, instanceValue, "onWindowStageCreate", argv, ArraySize(argv));

    CallObjectMethod(nativeEngine_, instanceValue, "onForeground", nullptr, 0);

    windowScenes_.emplace(currentId_, windowScene);
    jsWindowStages_.emplace(currentId_, std::shared_ptr<NativeReference>(jsWindowStage.release()));
}

std::unique_ptr<NativeReference> SimulatorImpl::CreateJsWindowStage(
    const std::shared_ptr<Rosen::WindowScene> &windowScene)
{
    napi_value jsWindowStage = Rosen::CreateJsWindowStage(nativeEngine_, windowScene);
    if (jsWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsWindowSatge object");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(nativeEngine_, "application.WindowStage", &jsWindowStage, 1);
}

panda::ecmascript::EcmaVM *SimulatorImpl::CreateJSVM()
{
    panda::RuntimeOption pandaOption;
    pandaOption.SetArkProperties(DEFAULT_ARK_PROPERTIES);
    pandaOption.SetGcThreadNum(DEFAULT_GC_THREAD_NUM);
    pandaOption.SetLongPauseTime(DEFAULT_LONG_PAUSE_TIME);
    pandaOption.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    pandaOption.SetGcPoolSize(DEFAULT_GC_POOL_SIZE);
    pandaOption.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::FOLLOW);
    pandaOption.SetLogBufPrint(PrintVmLog);
    pandaOption.SetEnableAsmInterpreter(true);
    pandaOption.SetAsmOpcodeDisableRange("");
    return panda::JSNApi::CreateJSVM(pandaOption);
}

bool SimulatorImpl::OnInit()
{
    if (!ParseBundleAndModuleInfo()) {
        HILOG_ERROR("parse bundle and module info failed.");
        return false;
    }

    vm_ = CreateJSVM();
    if (vm_ == nullptr) {
        return false;
    }

    panda::JSNApi::DebugOption debugOption = {ARK_DEBUGGER_LIB_PATH, (options_.debugPort != 0), options_.debugPort};
    panda::JSNApi::StartDebugger(vm_, debugOption, 0,
        std::bind(&DebuggerTask::OnPostTask, &debuggerTask_, std::placeholders::_1));

    auto nativeEngine = new (std::nothrow) ArkNativeEngine(vm_, nullptr);
    if (nativeEngine == nullptr) {
        HILOG_ERROR("nativeEngine is nullptr");
        return false;
    }
    napi_env env = reinterpret_cast<napi_env>(nativeEngine);

    napi_value globalObj;
    napi_get_global(env, &globalObj);
    if (globalObj == nullptr) {
        delete nativeEngine;
        HILOG_ERROR("Failed to get global object");
        return false;
    }

    if (!LoadRuntimeEnv(env, globalObj)) {
        delete nativeEngine;
        HILOG_ERROR("Load runtime env failed.");
        return false;
    }

    panda::JSNApi::SetBundle(vm_, false);
    panda::JSNApi::SetBundleName(vm_, options_.bundleName);
    panda::JSNApi::SetModuleName(vm_, options_.moduleName);
    panda::JSNApi::SetAssetPath(vm_, options_.modulePath);

    nativeEngine_ = env;
    return true;
}

napi_value SimulatorImpl::RequireNapi(napi_env env, napi_callback_info info)
{
    napi_value globalObj;
    napi_get_global(env, &globalObj);
    napi_value requireNapi = nullptr;
    napi_get_named_property(env, globalObj, "requireNapiPreview", &requireNapi);
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    napi_value result = nullptr;
    napi_call_function(env, CreateJsUndefined(env), requireNapi, argc, argv, &result);
    if (!CheckTypeForNapiValue(env, result, napi_undefined)) {
        return result;
    }
    napi_value mockRequireNapi = nullptr;
    napi_get_named_property(env, globalObj, "mockRequireNapi", &mockRequireNapi);
    napi_call_function(env, CreateJsUndefined(env), mockRequireNapi, argc, argv, &result);
    return result;
}

bool SimulatorImpl::LoadRuntimeEnv(napi_env env, napi_value globalObj)
{
    JsSysModule::Console::InitConsoleModule(env);
    auto ret = JsSysModule::Timer::RegisterTime(env);
    if (!ret) {
        HILOG_ERROR("Register timer failed");
    }
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_set_named_property(env, globalObj, "group", object);

    uintptr_t bufferStart = reinterpret_cast<uintptr_t>(_binary_jsMockSystemPlugin_abc_start);
    uintptr_t bufferEnd = reinterpret_cast<uintptr_t>(_binary_jsMockSystemPlugin_abc_end);
    const uint8_t *buffer = reinterpret_cast<const uint8_t*>(bufferStart);
    size_t size = bufferEnd - bufferStart;
    panda::JSNApi::Execute(vm_, buffer, size, "_GLOBAL::func_main_0");

    napi_value mockRequireNapi = nullptr;
    napi_get_named_property(env, globalObj, "requireNapi", &mockRequireNapi);
    napi_set_named_property(env, globalObj, "mockRequireNapi", mockRequireNapi);
    auto* moduleManager = reinterpret_cast<NativeEngine*>(env)->GetModuleManager();
    if (moduleManager != nullptr) {
        HILOG_ERROR("moduleManager SetPreviewSearchPath: %{public}s", options_.containerSdkPath.c_str());
        moduleManager->SetPreviewSearchPath(options_.containerSdkPath);
    }

    std::string fileSeparator = "/";
    auto pos = options_.containerSdkPath.find(fileSeparator);
    if (pos == std::string::npos) {
        fileSeparator = "\\";
    }

    std::string fileName = options_.containerSdkPath + fileSeparator + "apiMock" + fileSeparator + "jsMockHmos.abc";
    HILOG_DEBUG("file name: %{public}s", fileName.c_str());
    if (!fileName.empty() && AbilityStageContext::Access(fileName)) {
        panda::JSNApi::Execute(vm_, fileName, "_GLOBAL::func_main_0");
    }

    const char *moduleName = "SimulatorImpl";
    BindNativeFunction(env, globalObj, "requireNapi", moduleName, SimulatorImpl::RequireNapi);
    return true;
}

void SimulatorImpl::Run()
{
    uv_loop_t* uvLoop = nullptr;
    napi_get_uv_event_loop(nativeEngine_, &uvLoop);
    if (uvLoop != nullptr) {
        uv_run(uvLoop, UV_RUN_NOWAIT);
    }

    if (postTask_ != nullptr) {
        postTask_([this]() { Run(); }, 0);
    }
}
}

std::unique_ptr<Simulator> Simulator::Create(const Options &options)
{
    auto simulator = std::make_unique<SimulatorImpl>();
    if (simulator->Initialize(options)) {
        return simulator;
    }
    return nullptr;
}

void SimulatorImpl::SetHostResolveBufferTracker(ResolveBufferTrackerCallback cb)
{
    if (vm_ == nullptr || cb == nullptr) {
        HILOG_ERROR("Params invalid.");
        return;
    }
    panda::JSNApi::SetHostResolveBufferTracker(vm_, cb);
}
} // namespace AbilityRuntime
} // namespace OHOS
