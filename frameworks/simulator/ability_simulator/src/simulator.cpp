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

#include "EventHandler.h"
#include "StageContext.h"
#include "ability_context.h"
#include "ability_stage_context.h"
#include "bundle_container.h"
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
#error "Unsupported platform"
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
private:
    bool OnInit();
    void Run();
    NativeValue *LoadScript(const std::string &srcPath);
    void InitResourceMgr();
    void InitJsAbilityContext(NativeValue *instanceValue);
    void DispatchStartLifecycle(NativeValue *instanceValue);
    std::unique_ptr<NativeReference> CreateJsWindowStage(const std::shared_ptr<Rosen::WindowScene> &windowScene);
    NativeValue *CreateJsWant(NativeEngine &engine);
    bool LoadAbilityStage(uint8_t *buffer, size_t len);
    void InitJsAbilityStageContext(NativeValue *instanceValue);
    NativeValue *CreateJsLaunchParam(NativeEngine &engine);
    bool ParseBundleAndModuleInfo();
    bool ParseAbilityInfo(const std::string &abilitySrcPath);
    bool LoadRuntimeEnv(NativeEngine &nativeEngine, NativeObject &globalObject);

    panda::ecmascript::EcmaVM *CreateJSVM();
    Options options_;
    std::string abilityPath_;
    panda::ecmascript::EcmaVM *vm_ = nullptr;
    DebuggerTask debuggerTask_;
    std::unique_ptr<NativeEngine> nativeEngine_;

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
        uv_loop_t *uvLoop = nativeEngine_->GetUVLoop();
        if (uvLoop != nullptr) {
            uv_work_t work;
            uv_queue_work(uvLoop, &work, [](uv_work_t*) {}, [](uv_work_t *work, int32_t status) {
                HILOG_DEBUG("Simulator stop uv loop");
                uv_stop(work->loop);
            });
        }
    }

    panda::JSNApi::StopDebugger(vm_);

    abilities_.clear();
    nativeEngine_.reset();
    panda::JSNApi::DestroyJSVM(vm_);
    vm_ = nullptr;
}

bool SimulatorImpl::Initialize(const Options &options)
{
    if (nativeEngine_) {
        HILOG_DEBUG("Simulator is already initialized");
        return true;
    }

    options_ = options;
    if (!OnInit()) {
        return false;
    }

    uv_loop_t *uvLoop = nativeEngine_->GetUVLoop();
    if (uvLoop == nullptr) {
        return false;
    }

    uv_async_init(uvLoop, &debuggerTask_.onPostTaskSignal,
        reinterpret_cast<uv_async_cb>(DebuggerTask::HandleTask));

    Run();
    return true;
}

void CallObjectMethod(NativeEngine &engine, NativeValue *value, const char *name, NativeValue *const *argv, size_t argc)
{
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("%{public}s, Failed to get Ability object", __func__);
        return;
    }

    NativeValue *methodOnCreate = obj->GetProperty(name);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from Ability object", name);
        return;
    }
    engine.CallFunction(value, methodOnCreate, argv, argc);
}

NativeValue *SimulatorImpl::LoadScript(const std::string &srcPath)
{
    panda::Local<panda::ObjectRef> objRef = panda::JSNApi::GetExportObject(vm_, srcPath, "default");
    if (objRef->IsNull()) {
        HILOG_ERROR("Get export object failed");
        return nullptr;
    }

    auto obj = ArkNativeEngine::ArkValueToNativeValue(static_cast<ArkNativeEngine*>(nativeEngine_.get()), objRef);
    return nativeEngine_->CreateInstance(obj, nullptr, 0);
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
    options_.enablePartialUpdate = false;

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
    if (!nativeEngine_->RunScriptBuffer(abilityPath_, buf, len, false)) {
        HILOG_ERROR("Failed to run script: %{public}s", abilityPath_.c_str());
        return -1;
    }

    NativeValue *instanceValue = LoadScript(abilityPath_);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return -1;
    }

    ++currentId_;
    InitResourceMgr();
    InitJsAbilityContext(instanceValue);
    DispatchStartLifecycle(instanceValue);
    abilities_.emplace(currentId_, nativeEngine_->CreateReference(instanceValue, 1));
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
    if (!nativeEngine_->RunScriptBuffer(moduleSrcPath, buffer, len, false)) {
        HILOG_ERROR("Failed to run ability stage script: %{public}s", moduleSrcPath.c_str());
        return false;
    }

    NativeValue *instanceValue = LoadScript(moduleSrcPath);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create ability stage instance");
        return false;
    }

    InitJsAbilityStageContext(instanceValue);

    CallObjectMethod(*nativeEngine_, instanceValue, "onCreate", nullptr, 0);
    NativeValue *wantArgv[] = {
        CreateJsWant(*nativeEngine_)
    };
    CallObjectMethod(*nativeEngine_, instanceValue, "onAcceptWant", wantArgv, ArraySize(wantArgv));

    abilityStage_ = std::shared_ptr<NativeReference>(nativeEngine_->CreateReference(instanceValue, 1));
    return true;
}

void SimulatorImpl::InitJsAbilityStageContext(NativeValue *instanceValue)
{
    NativeValue *contextObj = CreateJsAbilityStageContext(*nativeEngine_, stageContext_);
    if (contextObj == nullptr) {
        HILOG_ERROR("contextObj is nullptr");
        return;
    }

    jsStageContext_ = std::shared_ptr<NativeReference>(
        JsRuntime::LoadSystemModuleByEngine(nativeEngine_.get(), "application.AbilityStageContext", &contextObj, 1));
    if (jsStageContext_ == nullptr) {
        HILOG_ERROR("Failed to get LoadSystemModuleByEngine");
        return;
    }

    contextObj = jsStageContext_->Get();
    if (contextObj == nullptr) {
        HILOG_ERROR("contextObj is nullptr.");
        return;
    }

    NativeObject *obj = ConvertNativeValueTo<NativeObject>(instanceValue);
    if (obj == nullptr) {
        HILOG_ERROR("obj is nullptr");
        return;
    }
    obj->SetProperty("context", contextObj);
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

    auto instanceValue = ref->Get();
    if (instanceValue == nullptr) {
        return;
    }

    CallObjectMethod(*nativeEngine_, instanceValue, "onBackground", nullptr, 0);
    CallObjectMethod(*nativeEngine_, instanceValue, "onWindowStageDestroy", nullptr, 0);
    CallObjectMethod(*nativeEngine_, instanceValue, "onDestroy", nullptr, 0);

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

    NativeValue *configArgv[] = {
        CreateJsConfiguration(*nativeEngine_, config)
    };

    auto abilityStage = abilityStage_->Get();
    if (abilityStage == nullptr) {
        HILOG_ERROR("abilityStage is nullptr");
        return;
    }
    CallObjectMethod(*nativeEngine_, abilityStage, "onConfigurationUpdated", configArgv, ArraySize(configArgv));
    CallObjectMethod(*nativeEngine_, abilityStage, "onConfigurationUpdate", configArgv, ArraySize(configArgv));
    JsAbilityStageContext::ConfigurationUpdated(nativeEngine_.get(), jsStageContext_, configuration);

    for (auto iter = abilities_.begin(); iter != abilities_.end(); iter++) {
        auto ability = iter->second->Get();
        if (ability == nullptr) {
            HILOG_ERROR("ability is nullptr");
            continue;
        }

        CallObjectMethod(*nativeEngine_, ability, "onConfigurationUpdated", configArgv, ArraySize(configArgv));
        CallObjectMethod(*nativeEngine_, ability, "onConfigurationUpdate", configArgv, ArraySize(configArgv));
        JsAbilityContext::ConfigurationUpdated(nativeEngine_.get(), iter->second, configuration);
    }
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

void SimulatorImpl::InitJsAbilityContext(NativeValue *instanceValue)
{
    if (context_ == nullptr) {
        context_ = std::make_shared<AbilityContext>();
        context_->SetSimulator(static_cast<Simulator*>(this));
        context_->SetOptions(options_);
        context_->SetAbilityStageContext(stageContext_);
        context_->SetResourceManager(resourceMgr_);
        context_->SetAbilityInfo(abilityInfo_);
    }
    NativeValue *contextObj = CreateJsAbilityContext(*nativeEngine_, context_);
    auto systemModule = std::shared_ptr<NativeReference>(
        JsRuntime::LoadSystemModuleByEngine(nativeEngine_.get(), "application.AbilityContext", &contextObj, 1));
    if (systemModule == nullptr) {
        HILOG_ERROR("systemModule is nullptr.");
        return;
    }

    contextObj = systemModule->Get();
    if (contextObj == nullptr) {
        HILOG_ERROR("contextObj is nullptr.");
        return;
    }

    NativeObject *obj = ConvertNativeValueTo<NativeObject>(instanceValue);
    if (obj == nullptr) {
        HILOG_ERROR("obj is nullptr");
        return;
    }
    obj->SetProperty("context", contextObj);
    jsContexts_.emplace(currentId_, systemModule);
}

NativeValue *SimulatorImpl::CreateJsWant(NativeEngine &engine)
{
    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    object->SetProperty("deviceId", CreateJsValue(engine, ""));
    object->SetProperty("bundleName", CreateJsValue(engine, options_.bundleName));
    if (abilityInfo_) {
        object->SetProperty("abilityName", CreateJsValue(engine, abilityInfo_->name));
    }
    object->SetProperty("moduleName", CreateJsValue(engine, options_.moduleName));
    object->SetProperty("uri", CreateJsValue(engine, ""));
    object->SetProperty("type", CreateJsValue(engine, ""));
    object->SetProperty("flags", CreateJsValue(engine, 0));
    object->SetProperty("action", CreateJsValue(engine, ""));
    object->SetProperty("parameters", engine.CreateObject());
    object->SetProperty("entities", engine.CreateArray(0));
    return objValue;
}

NativeValue *SimulatorImpl::CreateJsLaunchParam(NativeEngine &engine)
{
    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("launchReason", CreateJsValue(engine, AAFwk::LAUNCHREASON_UNKNOWN));
    object->SetProperty("lastExitReason", CreateJsValue(engine, AAFwk::LASTEXITREASON_UNKNOWN));
    return objValue;
}

void SimulatorImpl::DispatchStartLifecycle(NativeValue *instanceValue)
{
    NativeValue *wantArgv[] = {
        CreateJsWant(*nativeEngine_),
        CreateJsLaunchParam(*nativeEngine_)
    };
    CallObjectMethod(*nativeEngine_, instanceValue, "onCreate", wantArgv, ArraySize(wantArgv));

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
    NativeValue *argv[] = { jsWindowStage->Get() };
    CallObjectMethod(*nativeEngine_, instanceValue, "onWindowStageCreate", argv, ArraySize(argv));

    CallObjectMethod(*nativeEngine_, instanceValue, "onForeground", nullptr, 0);

    windowScenes_.emplace(currentId_, windowScene);
    jsWindowStages_.emplace(currentId_, std::shared_ptr<NativeReference>(jsWindowStage.release()));
}

std::unique_ptr<NativeReference> SimulatorImpl::CreateJsWindowStage(
    const std::shared_ptr<Rosen::WindowScene> &windowScene)
{
    NativeValue *jsWindowStage = Rosen::CreateJsWindowStage(*nativeEngine_, windowScene);
    if (jsWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsWindowSatge object");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(nativeEngine_.get(), "application.WindowStage", &jsWindowStage, 1);
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

    panda::JSNApi::SetHostResolveBufferTracker(vm_,
        [](const std::string &inputPath, uint8_t **buff, size_t *buffSize) -> bool {
            if (inputPath.empty() || buff == nullptr || buffSize == nullptr) {
                HILOG_ERROR("Param invalid.");
                return false;
            }

            HILOG_DEBUG("Get module buffer, input path: %{public}s.", inputPath.c_str());
            auto data = Ide::StageContext::GetInstance().GetModuleBuffer(inputPath);
            if (data == nullptr) {
                HILOG_ERROR("Get module buffer failed, input path: %{public}s.", inputPath.c_str());
                return false;
            }

            *buff = data->data();
            *buffSize = data->size();
            return true;
        });
    panda::JSNApi::DebugOption debugOption = {ARK_DEBUGGER_LIB_PATH, (options_.debugPort != 0), options_.debugPort};
    panda::JSNApi::StartDebugger(vm_, debugOption, 0,
        std::bind(&DebuggerTask::OnPostTask, &debuggerTask_, std::placeholders::_1));

    auto nativeEngine = std::make_unique<ArkNativeEngine>(vm_, nullptr);
    if (nativeEngine == nullptr) {
        HILOG_ERROR("nativeEngine is nullptr");
        return false;
    }
    NativeObject *globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine->GetGlobal());
    if (globalObj == nullptr) {
        HILOG_ERROR("Failed to get global object");
        return false;
    }

    if (!LoadRuntimeEnv(*nativeEngine, *globalObj)) {
        HILOG_ERROR("Load runtime env failed.");
        return false;
    }

    panda::JSNApi::SetBundle(vm_, false);
    panda::JSNApi::SetBundleName(vm_, options_.bundleName);
    panda::JSNApi::SetModuleName(vm_, options_.moduleName);
    panda::JSNApi::SetAssetPath(vm_, options_.modulePath);

    nativeEngine_ = std::move(nativeEngine);
    return true;
}

bool SimulatorImpl::LoadRuntimeEnv(NativeEngine &nativeEngine, NativeObject &globalObj)
{
    InitConsoleLogModule(nativeEngine, globalObj);
    InitTimer(nativeEngine, globalObj);
    globalObj.SetProperty("group", nativeEngine.CreateObject());

    uintptr_t bufferStart = reinterpret_cast<uintptr_t>(_binary_jsMockSystemPlugin_abc_start);
    uintptr_t bufferEnd = reinterpret_cast<uintptr_t>(_binary_jsMockSystemPlugin_abc_end);
    const uint8_t *buffer = reinterpret_cast<const uint8_t*>(bufferStart);
    size_t size = bufferEnd - bufferStart;
    panda::JSNApi::Execute(vm_, buffer, size, "_GLOBAL::func_main_0");

    NativeValue *mockRequireNapi = globalObj.GetProperty("requireNapi");
    globalObj.SetProperty("mockRequireNapi", mockRequireNapi);

    auto* moduleManager = nativeEngine.GetModuleManager();
    if (moduleManager != nullptr) {
        HILOG_DEBUG("moduleManager SetPreviewSearchPath: %{public}s", options_.containerSdkPath.c_str());
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
    BindNativeFunction(nativeEngine, globalObj, "requireNapi", moduleName,
        [](NativeEngine *engine, NativeCallbackInfo *info) {
        NativeObject *globalObj = ConvertNativeValueTo<NativeObject>(engine->GetGlobal());
        NativeValue *requireNapi = globalObj->GetProperty("requireNapiPreview");

        NativeValue *result = engine->CallFunction(engine->CreateUndefined(), requireNapi, info->argv, info->argc);
        if (result->TypeOf() != NATIVE_UNDEFINED) {
            return result;
        }

        NativeValue *mockRequireNapi = globalObj->GetProperty("mockRequireNapi");
        return engine->CallFunction(engine->CreateUndefined(), mockRequireNapi, info->argv, info->argc);
    });
    return true;
}

void SimulatorImpl::Run()
{
    uv_loop_t *uvLoop = nativeEngine_->GetUVLoop();
    if (uvLoop != nullptr) {
        uv_run(uvLoop, UV_RUN_NOWAIT);
    }

    AppExecFwk::EventHandler::PostTask([this]() {
        Run();
    });
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
} // namespace AbilityRuntime
} // namespace OHOS
