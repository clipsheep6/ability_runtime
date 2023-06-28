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
#include "EventHandler.h"
#include "hilog_wrapper.h"
#include "js_ability_context.h"
#include "js_console_log.h"
#include "js_module_searcher.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_timer.h"
#include "js_window_stage.h"
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

int32_t PrintVmLog(int32_t, int32_t, const char*, const char*, const char* message)
{
    HILOG_INFO("ArkLog: %{public}s", message);
    return 0;
}

template<typename T, size_t N>
inline constexpr size_t ArraySize(T (&)[N]) noexcept
{
    return N;
}

struct DebuggerTask {
    void OnPostTask(std::function<void()>&& task);

    static void HandleTask(const uv_async_t* req);

    uv_async_t onPostTaskSignal {};
    std::function<void()> func;
};

class SimulatorImpl : public Simulator {
public:
    SimulatorImpl() = default;
    ~SimulatorImpl();

    bool Initialize(const Options& options);

    int64_t StartAbility(const std::string& abilityName, TerminateCallback callback) override;
    void TerminateAbility(int64_t abilityId) override;
private:
    bool OnInit();
    void Run();
    NativeValue* LoadScript();
    void InitResourceMgr();
    void InitJsAbilityContext(NativeValue* instanceValue);
    void DispatchStartLifecycle(NativeValue* instanceValue);
    std::unique_ptr<NativeReference> CreateJsWindowStage(const std::shared_ptr<Rosen::WindowScene>& windowScene);
    NativeValue* CreateJsWant(NativeEngine &engine);

    panda::ecmascript::EcmaVM* CreateJSVM();
    Options options_;
    std::string abilityPath_;
    panda::ecmascript::EcmaVM* vm_ = nullptr;
    DebuggerTask debuggerTask_;
    std::unique_ptr<NativeEngine> nativeEngine_;

    int64_t currentId_ = 0;
    std::unordered_map<int64_t, std::shared_ptr<NativeReference>> abilities_;
    std::unordered_map<int64_t, std::shared_ptr<Rosen::WindowScene>> windowScenes_;
    std::unordered_map<int64_t, std::shared_ptr<NativeReference>> jsWindowStages_;
    std::unordered_map<int64_t, std::shared_ptr<NativeReference>> jsContexts_;
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr_;
    std::shared_ptr<AbilityContext> context_;
};

void DebuggerTask::HandleTask(const uv_async_t* req)
{
    auto* debuggerTask = reinterpret_cast<DebuggerTask*>(req->data);
    if (debuggerTask == nullptr) {
        HILOG_ERROR("HandleTask debuggerTask is null");
        return;
    }
    debuggerTask->func();
}

void DebuggerTask::OnPostTask(std::function<void()>&& task)
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
        uv_loop_t* uvLoop = nativeEngine_->GetUVLoop();
        if (uvLoop != nullptr) {
            uv_work_t work;
            uv_queue_work(uvLoop, &work, [](uv_work_t*) {}, [](uv_work_t* work, int32_t status) {
                HILOG_INFO("Simulator stop uv loop");
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

bool SimulatorImpl::Initialize(const Options& options)
{
    if (nativeEngine_) {
        HILOG_INFO("Simulator is already initialized");
        return true;
    }

    options_ = options;
    bool initResult = OnInit();
    if (!initResult) {
        return false;
    }

    uv_loop_t* uvLoop = nativeEngine_->GetUVLoop();
    if (uvLoop == nullptr) {
        return false;
    }

    uv_async_init(uvLoop, &debuggerTask_.onPostTaskSignal,
        reinterpret_cast<uv_async_cb>(DebuggerTask::HandleTask));

    Run();
    return true;
}

void CallObjectMethod(NativeEngine& engine, NativeValue* value, const char *name, NativeValue *const *argv, size_t argc)
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

NativeValue* SimulatorImpl::LoadScript()
{
    panda::Local<panda::ObjectRef> objRef = panda::JSNApi::GetExportObject(vm_, abilityPath_, "default");
    if (objRef->IsNull()) {
        HILOG_ERROR("Get export object failed");
        return nullptr;
    }

    auto obj = ArkNativeEngine::ArkValueToNativeValue(static_cast<ArkNativeEngine*>(nativeEngine_.get()), objRef);
    return nativeEngine_->CreateInstance(obj, nullptr, 0);
}

int64_t SimulatorImpl::StartAbility(const std::string& abilitySrcPath, TerminateCallback callback)
{
    abilityPath_ = BUNDLE_INSTALL_PATH + options_.moduleName + "/" + abilitySrcPath;

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
    if (!nativeEngine_->RunScriptBuffer(abilityPath_, buffer.release(), len, false)) {
        HILOG_ERROR("Failed to run script: %{public}s", abilityPath_.c_str());
        return -1;
    }

    NativeValue* instanceValue = LoadScript();
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

void SimulatorImpl::TerminateAbility(int64_t abilityId)
{
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

void SimulatorImpl::InitResourceMgr()
{
    HILOG_INFO("called.");
    resourceMgr_ = std::shared_ptr<Global::Resource::ResourceManager>(Global::Resource::CreateResourceManager());
    if (resourceMgr_ == nullptr) {
        HILOG_ERROR("resourceMgr is nullptr");
        return;
    }

    if (!resourceMgr_->AddResource(options_.resourcePath.c_str())) {
        HILOG_ERROR("Add resource failed.");
    }
    HILOG_INFO("Add resource success.");
}

void SimulatorImpl::InitJsAbilityContext(NativeValue* instanceValue)
{
    if (context_ == nullptr) {
        context_ = std::make_shared<AbilityContext>();
        context_->SetOptions(options_);
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

NativeValue* SimulatorImpl::CreateJsWant(NativeEngine &engine)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    object->SetProperty("deviceId", engine.CreateUndefined());
    object->SetProperty("bundleName", engine.CreateUndefined());
    object->SetProperty("abilityName", engine.CreateUndefined());
    object->SetProperty("moduleName", engine.CreateUndefined());
    object->SetProperty("uri", engine.CreateUndefined());
    object->SetProperty("type", engine.CreateUndefined());
    object->SetProperty("flags", engine.CreateUndefined());
    object->SetProperty("action", engine.CreateUndefined());
    object->SetProperty("parameters", engine.CreateUndefined());
    object->SetProperty("entities", engine.CreateUndefined());
    return objValue;
}

void SimulatorImpl::DispatchStartLifecycle(NativeValue* instanceValue)
{
    NativeValue *wantArgv[] = {
        CreateJsWant(*nativeEngine_)
    };
    CallObjectMethod(*nativeEngine_, instanceValue, "onCreate", wantArgv, ArraySize(wantArgv));

    auto windowScene = std::make_shared<Rosen::WindowScene>();
    if (windowScene == nullptr) {
        return;
    }
    sptr<Rosen::IWindowLifeCycle> listener = nullptr;
    windowScene->Init(-1, context_, listener);
    auto jsWindowStage = CreateJsWindowStage(windowScene);
    if(jsWindowStage == nullptr) {
        return;
    }
    NativeValue *argv[] = { jsWindowStage->Get() };
    CallObjectMethod(*nativeEngine_, instanceValue, "onWindowStageCreate", argv, ArraySize(argv));

    CallObjectMethod(*nativeEngine_, instanceValue, "onForeground", nullptr, 0);

    windowScenes_.emplace(currentId_, windowScene);
    jsWindowStages_.emplace(currentId_, std::shared_ptr<NativeReference>(jsWindowStage.release()));
}

std::unique_ptr<NativeReference> SimulatorImpl::CreateJsWindowStage(
    const std::shared_ptr<Rosen::WindowScene>& windowScene)
{
    NativeValue *jsWindowStage = Rosen::CreateJsWindowStage(*nativeEngine_, windowScene);
    if (jsWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsWindowSatge object");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(nativeEngine_.get(), "application.WindowStage", &jsWindowStage, 1);
}

panda::ecmascript::EcmaVM* SimulatorImpl::CreateJSVM()
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
    vm_ = CreateJSVM();
    if (vm_ == nullptr) {
        return false;
    }

    panda::JSNApi::DebugOption debugOption = {ARK_DEBUGGER_LIB_PATH, true, options_.debugPort};
    panda::JSNApi::StartDebugger(vm_, debugOption, 0,
        std::bind(&DebuggerTask::OnPostTask, &debuggerTask_, std::placeholders::_1));

    auto nativeEngine = std::make_unique<ArkNativeEngine>(vm_, nullptr);

    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine->GetGlobal());
    if (globalObj == nullptr) {
        HILOG_ERROR("Failed to get global object");
        return false;
    }

    InitConsoleLogModule(*nativeEngine, *globalObj);
    InitTimer(*nativeEngine, *globalObj);

    globalObj->SetProperty("group", nativeEngine->CreateObject());

    uintptr_t bufferStart = reinterpret_cast<uintptr_t>(_binary_jsMockSystemPlugin_abc_start);
    uintptr_t bufferEnd = reinterpret_cast<uintptr_t>(_binary_jsMockSystemPlugin_abc_end);
    const uint8_t* buffer = reinterpret_cast<const uint8_t*>(bufferStart);
    size_t size = bufferEnd - bufferStart;

    panda::JSNApi::Execute(vm_, buffer, size, "_GLOBAL::func_main_0");

    NativeValue* mockRequireNapi = globalObj->GetProperty("requireNapi");
    globalObj->SetProperty("mockRequireNapi", mockRequireNapi);

    const char *moduleName = "SimulatorImpl";
    BindNativeFunction(*nativeEngine, *globalObj, "requireNapi", moduleName,
        [](NativeEngine* engine, NativeCallbackInfo* info) {
        NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(engine->GetGlobal());
        NativeValue* requireNapi = globalObj->GetProperty("requireNapiPreview");

        NativeValue* result = engine->CallFunction(engine->CreateUndefined(), requireNapi, info->argv, info->argc);
        if (result->TypeOf() != NATIVE_UNDEFINED) {
            return result;
        }

        NativeValue* mockRequireNapi = globalObj->GetProperty("mockRequireNapi");
        return engine->CallFunction(engine->CreateUndefined(), mockRequireNapi, info->argv, info->argc);
    });

    panda::JSNApi::SetBundle(vm_, false);
    panda::JSNApi::SetBundleName(vm_, options_.bundleName);
    panda::JSNApi::SetModuleName(vm_, options_.moduleName);
    panda::JSNApi::SetAssetPath(vm_, options_.modulePath);

    nativeEngine_ = std::move(nativeEngine);
    return true;
}

void SimulatorImpl::Run()
{
    uv_loop_t* uvLoop = nativeEngine_->GetUVLoop();
    if (uvLoop != nullptr) {
        uv_run(uvLoop, UV_RUN_NOWAIT);
    }

    AppExecFwk::EventHandler::Current()->PostTask([this]() {
        Run();
    });
}
}

std::unique_ptr<Simulator> Simulator::Create(const Options& options)
{
    auto simulator = std::make_unique<SimulatorImpl>();
    if (simulator->Initialize(options)) {
        return simulator;
    }
    return nullptr;
}
} // namespace AbilityRuntime
} // namespace OHOS
