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

#include "js_runtime.h"

#include <atomic>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <sys/epoll.h>
#include <unistd.h>

#include "connect_server_manager.h"
#include "event_handler.h"
#include "hdc_register.h"
#include "hilog_wrapper.h"
#include "js_console_log.h"
#include "js_module_searcher.h"
#include "js_module_reader.h"
#include "js_runtime_utils.h"
#include "js_timer.h"
#include "js_worker.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "parameters.h"
#include "runtime_extractor.h"
#include "systemcapability.h"

#ifdef SUPPORT_GRAPHICS
#include "declarative_module_preloader.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr uint8_t SYSCAP_MAX_SIZE = 64;
constexpr int64_t DEFAULT_GC_POOL_SIZE = 0x10000000; // 256MB
#if defined(_ARM64_)
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib64/libark_debugger.z.so";
#else
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib/libark_debugger.z.so";
#endif

constexpr char TIMER_TASK[] = "uv_timer_task";

class ArkJsRuntime : public JsRuntime {
public:
    ArkJsRuntime()
    {
        isArkEngine_ = true;
    }

    ~ArkJsRuntime() override
    {
        Deinitialize();

        if (vm_ != nullptr) {
            if (debugMode_) {
                ConnectServerManager::Get().RemoveInstance(instanceId_);
                panda::JSNApi::StopDebugger(vm_);
            }

            panda::JSNApi::DestroyJSVM(vm_);
            vm_ = nullptr;
        }
    }

    void StartDebugMode(bool needBreakPoint) override
    {
        if (vm_ == nullptr) {
            HILOG_ERROR("virtual machine does not exist");
            return;
        }

        if (debugMode_) {
            HILOG_INFO("Already in debug mode");
            return;
        }

        // Set instance id to tid after the first instance.
        if (ArkJsRuntime::hasInstance.exchange(true, std::memory_order_relaxed)) {
            instanceId_ = static_cast<uint32_t>(gettid());
        }

        HILOG_INFO("Ark VM is starting debug mode [%{public}s]", needBreakPoint ? "break" : "normal");

        HdcRegister::Get().StartHdcRegister(bundleName_);
        ConnectServerManager::Get().StartConnectServer(bundleName_);
        ConnectServerManager::Get().AddInstance(instanceId_);
        StartDebuggerInWorkerModule();

        auto debuggerPostTask = [eventHandler = eventHandler_](std::function<void()>&& task) {
            eventHandler->PostTask(task);
        };
        panda::JSNApi::StartDebugger(ARK_DEBUGGER_LIB_PATH, vm_, needBreakPoint, instanceId_, debuggerPostTask);

        debugMode_ = true;
    }

    bool RunScript(const std::string& path, const std::string& hapPath) override
    {
        bool result = false;
        if (!hapPath.empty()) {
            std::ostringstream outStream;
            if (runtimeExtractor_ == nullptr) {
                runtimeExtractor_ = InitRuntimeExtractor(hapPath);
            }
            if (!GetFileBuffer(runtimeExtractor_, path, outStream)) {
                HILOG_ERROR("Get abc file failed");
                return result;
            }

            const auto& outStr = outStream.str();
            std::vector<uint8_t> buffer;
            buffer.assign(outStr.begin(), outStr.end());

            result = nativeEngine_->RunScriptBuffer(path.c_str(), buffer) != nullptr;
        } else {
            result = nativeEngine_->RunScriptPath(path.c_str()) != nullptr;
        }
        return result;
    }

    NativeValue* LoadJsModule(const std::string& path, const std::string& hapPath) override
    {
        if (!RunScript(path, hapPath)) {
            HILOG_ERROR("Failed to run script: %{public}s", path.c_str());
            return nullptr;
        }

        panda::Local<panda::ObjectRef> exportObj = panda::JSNApi::GetExportObject(vm_, path, "default");
        if (exportObj->IsNull()) {
            HILOG_ERROR("Get export object failed");
            return nullptr;
        }

        return ArkNativeEngine::ArkValueToNativeValue(
            static_cast<ArkNativeEngine*>(nativeEngine_.get()), exportObj);
    }

private:
    static int32_t PrintVmLog(int32_t, int32_t, const char*, const char*, const char* message)
    {
        HILOG_INFO("ArkLog: %{public}s", message);
        return 0;
    }

    void FinishPreload() override
    {
        panda::JSNApi::preFork(vm_);
    }

    bool Initialize(const Runtime::Options& options) override
    {
        if (preloaded_) {
            panda::JSNApi::postFork(vm_);
            nativeEngine_->ReinitUVLoop();
        } else {
            panda::RuntimeOption pandaOption;
            int arkProperties = OHOS::system::GetIntParameter<int>("persist.ark.properties", -1);
            size_t gcThreadNum = OHOS::system::GetUintParameter<size_t>("persist.ark.gcthreads", 7);
            size_t longPauseTime = OHOS::system::GetUintParameter<size_t>("persist.ark.longpausetime", 40);
            pandaOption.SetArkProperties(arkProperties);
            pandaOption.SetGcThreadNum(gcThreadNum);
            pandaOption.SetLongPauseTime(longPauseTime);
            HILOG_INFO("ArkJSRuntime::Initialize ark properties = %{public}d", arkProperties);
            pandaOption.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
            pandaOption.SetGcPoolSize(DEFAULT_GC_POOL_SIZE);
            pandaOption.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::INFO);
            pandaOption.SetLogBufPrint(PrintVmLog);
            // Fix a problem that if vm will crash if preloaded
            if (options.preload) {
                pandaOption.SetEnableAsmInterpreter(false);
            } else {
                bool asmInterpreterEnabled = OHOS::system::GetBoolParameter("persist.ark.asminterpreter", true);
                std::string asmOpcodeDisableRange = OHOS::system::GetParameter("persist.ark.asmopcodedisablerange", "");
                pandaOption.SetEnableAsmInterpreter(asmInterpreterEnabled);
                pandaOption.SetAsmOpcodeDisableRange(asmOpcodeDisableRange);
            }
            vm_ = panda::JSNApi::CreateJSVM(pandaOption);
            if (vm_ == nullptr) {
                return false;
            }

            nativeEngine_ = std::make_unique<ArkNativeEngine>(vm_, static_cast<JsRuntime*>(this));
        }

        if (!options.preload) {
            bundleName_ = options.bundleName;
            runtimeExtractor_ = InitRuntimeExtractor(options.hapPath);
            panda::JSNApi::SetHostResolvePathTracker(vm_, JsModuleSearcher(options.bundleName));
            panda::JSNApi::SetHostResolveBufferTracker(
                vm_, JsModuleReader(options.bundleName, options.hapPath, runtimeExtractor_));
        }
        return JsRuntime::Initialize(options);
    }

    std::string bundleName_;
    panda::ecmascript::EcmaVM* vm_ = nullptr;
    uint32_t instanceId_ = 0;

    static std::atomic<bool> hasInstance;
};

std::atomic<bool> ArkJsRuntime::hasInstance(false);

NativeValue* CanIUse(NativeEngine* engine, NativeCallbackInfo* info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("get syscap failed since engine or callback info is nullptr.");
        return nullptr;
    }

    if (info->argc != 1 || info->argv[0]->TypeOf() != NATIVE_STRING) {
        HILOG_ERROR("Get syscap failed with invalid parameter.");
        return engine->CreateUndefined();
    }

    char syscap[SYSCAP_MAX_SIZE] = { 0 };

    NativeString* str = ConvertNativeValueTo<NativeString>(info->argv[0]);
    if (str == nullptr) {
        HILOG_ERROR("Convert to NativeString failed.");
        return engine->CreateUndefined();
    }
    size_t bufferLen = str->GetLength();
    size_t strLen = 0;
    str->GetCString(syscap, bufferLen + 1, &strLen);

    bool ret = HasSystemCapability(syscap);
    return engine->CreateBoolean(ret);
}

void InitSyscapModule(NativeEngine& engine, NativeObject& globalObject)
{
    const char *moduleName = "JsRuntime";
    BindNativeFunction(engine, globalObject, "canIUse", moduleName, CanIUse);
}
class UvLoopHandler : public AppExecFwk::FileDescriptorListener, public std::enable_shared_from_this<UvLoopHandler> {
public:
    explicit UvLoopHandler(uv_loop_t* uvLoop) : uvLoop_(uvLoop) {}

    void OnReadable(int32_t) override
    {
        HILOG_DEBUG("UvLoopHandler::OnReadable is triggered");
        OnTriggered();
    }

    void OnWritable(int32_t) override
    {
        HILOG_DEBUG("UvLoopHandler::OnWritable is triggered");
        OnTriggered();
    }

private:
    void OnTriggered()
    {
        HILOG_DEBUG("UvLoopHandler::OnTriggered is triggered");

        auto fd = uv_backend_fd(uvLoop_);
        struct epoll_event ev;
        do {
            uv_run(uvLoop_, UV_RUN_NOWAIT);
        } while (epoll_wait(fd, &ev, 1, 0) > 0);

        auto eventHandler = GetOwner();
        if (!eventHandler) {
            return;
        }

        int32_t timeout = uv_backend_timeout(uvLoop_);
        if (timeout < 0) {
            if (haveTimerTask_) {
                eventHandler->RemoveTask(TIMER_TASK);
            }
            return;
        }

        int64_t timeStamp = static_cast<int64_t>(uv_now(uvLoop_)) + timeout;
        if (timeStamp == lastTimeStamp_) {
            return;
        }

        if (haveTimerTask_) {
            eventHandler->RemoveTask(TIMER_TASK);
        }

        auto callback = [wp = weak_from_this()] {
            auto sp = wp.lock();
            if (sp) {
                // Timer task is triggered, so there is no timer task now.
                sp->haveTimerTask_ = false;
                sp->OnTriggered();
            }
        };
        eventHandler->PostTask(callback, TIMER_TASK, timeout);
        lastTimeStamp_ = timeStamp;
        haveTimerTask_ = true;
    }

    uv_loop_t* uvLoop_ = nullptr;
    int64_t lastTimeStamp_ = 0;
    bool haveTimerTask_ = false;
};
} // namespace

std::unique_ptr<Runtime> JsRuntime::Create(const Runtime::Options& options)
{
    std::unique_ptr<JsRuntime> instance;

    if (!options.preload) {
        auto preloadedInstance = Runtime::GetPreloaded();
        if (preloadedInstance && preloadedInstance->GetLanguage() == Runtime::Language::JS) {
            instance.reset(static_cast<JsRuntime*>(preloadedInstance.release()));
        } else {
            instance = std::make_unique<ArkJsRuntime>();
        }
    } else {
        instance = std::make_unique<ArkJsRuntime>();
    }

    if (!instance->Initialize(options)) {
        return std::unique_ptr<Runtime>();
    }
    return instance;
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModuleByEngine(NativeEngine* engine,
    const std::string& moduleName, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("JsRuntime::LoadSystemModule(%{public}s)", moduleName.c_str());
    if (engine == nullptr) {
        HILOG_INFO("JsRuntime::LoadSystemModule: invalid engine.");
        return std::unique_ptr<NativeReference>();
    }

    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(engine->GetGlobal());
    std::unique_ptr<NativeReference> methodRequireNapiRef_;
    methodRequireNapiRef_.reset(engine->CreateReference(globalObj->GetProperty("requireNapi"), 1));
    if (!methodRequireNapiRef_) {
        HILOG_ERROR("Failed to create reference for global.requireNapi");
        return nullptr;
    }
    NativeValue* className = engine->CreateString(moduleName.c_str(), moduleName.length());
    NativeValue* classValue =
        engine->CallFunction(engine->GetGlobal(), methodRequireNapiRef_->Get(), &className, 1);
    NativeValue* instanceValue = engine->CreateInstance(classValue, argv, argc);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    return std::unique_ptr<NativeReference>(engine->CreateReference(instanceValue, 1));
}

void *DetachCallbackFunc(NativeEngine *engine, void *value, void *)
{
    return value;
}

bool JsRuntime::Initialize(const Options& options)
{
    HandleScope handleScope(*this);

    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine_->GetGlobal());
    if (globalObj == nullptr) {
        HILOG_ERROR("Failed to get global object");
        return false;
    }

    if (!preloaded_) {
        InitConsoleLogModule(*nativeEngine_, *globalObj);
        InitSyscapModule(*nativeEngine_, *globalObj);

        // Simple hook function 'isSystemplugin'
        const char *moduleName = "JsRuntime";
        BindNativeFunction(*nativeEngine_, *globalObj, "isSystemplugin", moduleName,
            [](NativeEngine* engine, NativeCallbackInfo* info) -> NativeValue* {
                return engine->CreateUndefined();
            });

        methodRequireNapiRef_.reset(nativeEngine_->CreateReference(globalObj->GetProperty("requireNapi"), 1));
        if (!methodRequireNapiRef_) {
            HILOG_ERROR("Failed to create reference for global.requireNapi");
            return false;
        }
#ifdef SUPPORT_GRAPHICS
        if (options.loadAce) {
            OHOS::Ace::DeclarativeModulePreloader::Preload(*nativeEngine_);
        }
#endif
    }

    if (!options.preload) {
        // Create event handler for runtime
        eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(options.eventRunner);

        auto uvLoop = nativeEngine_->GetUVLoop();
        auto fd = uvLoop != nullptr ? uv_backend_fd(uvLoop) : -1;
        if (fd < 0) {
            HILOG_ERROR("Failed to get backend fd from uv loop");
            return false;
        }

        // MUST run uv loop once before we listen its backend fd.
        uv_run(uvLoop, UV_RUN_NOWAIT);

        uint32_t events = AppExecFwk::FILE_DESCRIPTOR_INPUT_EVENT | AppExecFwk::FILE_DESCRIPTOR_OUTPUT_EVENT;
        eventHandler_->AddFileDescriptorListener(fd, events, std::make_shared<UvLoopHandler>(uvLoop));

        codePath_ = options.codePath;

        auto moduleManager = NativeModuleManager::GetInstance();
        std::string packagePath = options.packagePath;
        if (moduleManager && !packagePath.empty()) {
            moduleManager->SetAppLibPath(packagePath.c_str());
        }

        InitTimerModule(*nativeEngine_, *globalObj);
        InitWorkerModule(*nativeEngine_, codePath_);
    }

    preloaded_ = options.preload;
    return true;
}

void JsRuntime::Deinitialize()
{
    for (auto it = modules_.begin(); it != modules_.end(); it = modules_.erase(it)) {
        delete it->second;
        it->second = nullptr;
    }

    methodRequireNapiRef_.reset();

    auto uvLoop = nativeEngine_->GetUVLoop();
    auto fd = uvLoop != nullptr ? uv_backend_fd(uvLoop) : -1;
    if (fd >= 0) {
        eventHandler_->RemoveFileDescriptorListener(fd);
    }
    RemoveTask(TIMER_TASK);

    nativeEngine_.reset();
}

NativeValue* JsRuntime::LoadJsBundle(const std::string& path, const std::string& hapPath)
{
    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine_->GetGlobal());
    NativeValue* exports = nativeEngine_->CreateObject();
    globalObj->SetProperty("exports", exports);

    if (!RunScript(path, hapPath)) {
        HILOG_ERROR("Failed to run script: %{public}s", path.c_str());
        return nullptr;
    }

    NativeObject* exportsObj = ConvertNativeValueTo<NativeObject>(globalObj->GetProperty("exports"));
    if (exportsObj == nullptr) {
        HILOG_ERROR("Failed to get exports objcect: %{public}s", path.c_str());
        return nullptr;
    }

    NativeValue* exportObj = exportsObj->GetProperty("default");
    if (exportObj == nullptr) {
        HILOG_ERROR("Failed to get default objcect: %{public}s", path.c_str());
        return nullptr;
    }

    return exportObj;
}

std::unique_ptr<NativeReference> JsRuntime::LoadModule(
    const std::string& moduleName, const std::string& modulePath, const std::string& hapPath, bool esmodule)
{
    HILOG_DEBUG("JsRuntime::LoadModule(%{public}s, %{public}s, %{public}s, %{public}s)",
        moduleName.c_str(), modulePath.c_str(), hapPath.c_str(), esmodule ? "true" : "false");

    HandleScope handleScope(*this);

    NativeValue* classValue = nullptr;

    auto it = modules_.find(modulePath);
    if (it != modules_.end()) {
        classValue = it->second->Get();
    } else {
        std::string fileName;
        if (!MakeFilePath(codePath_, modulePath, fileName)) {
            HILOG_ERROR("Failed to make module file path: %{private}s", modulePath.c_str());
            return std::unique_ptr<NativeReference>();
        }

        classValue = esmodule ? LoadJsModule(fileName, hapPath) : LoadJsBundle(fileName, hapPath);
        if (classValue == nullptr) {
            return std::unique_ptr<NativeReference>();
        }

        modules_.emplace(modulePath, nativeEngine_->CreateReference(classValue, 1));
    }

    NativeValue* instanceValue = nativeEngine_->CreateInstance(classValue, nullptr, 0);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    return std::unique_ptr<NativeReference>(nativeEngine_->CreateReference(instanceValue, 1));
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModule(
    const std::string& moduleName, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("JsRuntime::LoadSystemModule(%{public}s)", moduleName.c_str());

    HandleScope handleScope(*this);

    NativeValue* className = nativeEngine_->CreateString(moduleName.c_str(), moduleName.length());
    NativeValue* classValue =
        nativeEngine_->CallFunction(nativeEngine_->GetGlobal(), methodRequireNapiRef_->Get(), &className, 1);
    NativeValue* instanceValue = nativeEngine_->CreateInstance(classValue, argv, argc);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    return std::unique_ptr<NativeReference>(nativeEngine_->CreateReference(instanceValue, 1));
}

bool JsRuntime::RunScript(const std::string& path, const std::string& hapPath)
{
    bool result = false;
    if (!hapPath.empty()) {
        std::ostringstream outStream;
        if (runtimeExtractor_ == nullptr) {
            runtimeExtractor_ = InitRuntimeExtractor(hapPath);
        }
        if (!GetFileBuffer(runtimeExtractor_, path, outStream)) {
            HILOG_ERROR("Get abc file failed");
            return result;
        }

        const auto& outStr = outStream.str();
        std::vector<uint8_t> buffer;
        buffer.assign(outStr.begin(), outStr.end());

        result = nativeEngine_->RunScriptBuffer(path.c_str(), buffer) != nullptr;
    } else {
        result = nativeEngine_->RunScript(path.c_str()) != nullptr;
    }
    return result;
}

bool JsRuntime::RunSandboxScript(const std::string& path, const std::string& hapPath)
{
    std::string fileName;
    if (!MakeFilePath(codePath_, path, fileName)) {
        HILOG_ERROR("Failed to make module file path: %{private}s", path.c_str());
        return false;
    }

    if (!RunScript(fileName, hapPath)) {
        HILOG_ERROR("Failed to run script: %{public}s", fileName.c_str());
        return false;
    }
    return true;
}

void JsRuntime::PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime)
{
    eventHandler_->PostTask(task, name, delayTime);
}

void JsRuntime::RemoveTask(const std::string& name)
{
    eventHandler_->RemoveTask(name);
}

void JsRuntime::DumpHeapSnapshot(bool isPrivate)
{
    nativeEngine_->DumpHeapSnapshot(true, DumpFormat::JSON, isPrivate);
}

std::string JsRuntime::BuildJsStackTrace()
{
    std::string straceStr = "";
    [[maybe_unused]]bool temp = nativeEngine_->BuildJsStackTrace(straceStr);
    return straceStr;
}

void JsRuntime::NotifyApplicationState(bool isBackground)
{
    if (nativeEngine_ == nullptr) {
        HILOG_INFO("NotifyApplicationState, nativeEngine_ is nullptr");
        return;
    }
    nativeEngine_->NotifyApplicationState(isBackground);
    HILOG_INFO("NotifyApplicationState, isBackground %{public}d.", isBackground);
}

void JsRuntime::PreloadSystemModule(const std::string& moduleName)
{
    HandleScope handleScope(*this);

    NativeValue* className = nativeEngine_->CreateString(moduleName.c_str(), moduleName.length());
    nativeEngine_->CallFunction(nativeEngine_->GetGlobal(), methodRequireNapiRef_->Get(), &className, 1);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
