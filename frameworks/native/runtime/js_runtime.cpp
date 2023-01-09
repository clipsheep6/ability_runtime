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

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <regex>

#include <atomic>
#include <sys/epoll.h>
#include <unistd.h>

#include "ability_constants.h"
#include "connect_server_manager.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "event_handler.h"
#include "file_path_utils.h"
#include "hdc_register.h"
#include "hilog_wrapper.h"
#include "hot_reloader.h"
#include "js_console_log.h"
#include "js_module_reader.h"
#include "js_module_searcher.h"
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
const std::string SANDBOX_ARK_CACHE_PATH = "/data/storage/ark-cache/";
const std::string SANDBOX_ARK_PROIFILE_PATH = "/data/storage/ark-profile";
#if defined(_ARM64_)
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib64/libark_debugger.z.so";
#else
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib/libark_debugger.z.so";
#endif

constexpr char TIMER_TASK[] = "uv_timer_task";
constexpr char MERGE_ABC_PATH[] = "/ets/modules.abc";
constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";

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
            HILOG_ERROR("Virtual machine does not exist");
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

    bool RunScript(const std::string& srcPath, const std::string& hapPath, bool useCommonChunk) override
    {
        std::string commonsPath = std::string(Constants::LOCAL_CODE_PATH) + "/" + moduleName_ + "/ets/commons.abc";
        std::string vendorsPath = std::string(Constants::LOCAL_CODE_PATH) + "/" + moduleName_ + "/ets/vendors.abc";

        if (hapPath.empty()) {
            if (useCommonChunk) {
                (void)nativeEngine_->RunScriptPath(commonsPath.c_str());
                (void)nativeEngine_->RunScriptPath(vendorsPath.c_str());
            }
            return nativeEngine_->RunScriptPath(srcPath.c_str()) != nullptr;
        }

        std::shared_ptr<RuntimeExtractor> runtimeExtractor;
        if (runtimeExtractorMap_.find(hapPath) == runtimeExtractorMap_.end()) {
            runtimeExtractor = RuntimeExtractor::Create(hapPath);
            if (runtimeExtractor == nullptr) {
                return false;
            }
            runtimeExtractor->SetRuntimeFlag(true);
            runtimeExtractorMap_.insert(make_pair(hapPath, runtimeExtractor));
        } else {
            runtimeExtractor = runtimeExtractorMap_.at(hapPath);
        }

        auto func = [&](std::string modulePath, std::string abcPath) {
            std::ostringstream outStream;
            if (!runtimeExtractor->GetFileBuffer(modulePath, outStream)) {
                HILOG_ERROR("Get Module abc file failed");
                return false;
            }

            const auto& outStr = outStream.str();
            std::vector<uint8_t> buffer;
            buffer.assign(outStr.begin(), outStr.end());

            return nativeEngine_->RunScriptBuffer(abcPath.c_str(), buffer, isBundle_) != nullptr;
        };

        if (useCommonChunk) {
            (void)func(commonsPath, commonsPath);
            (void)func(vendorsPath, vendorsPath);
        }

        std::string path = srcPath;
        if (!isBundle_) {
            if (!vm_ || moduleName_.empty()) {
                HILOG_ERROR("vm is nullptr or moduleName is hole");
                return false;
            }
            path = BUNDLE_INSTALL_PATH + moduleName_ + MERGE_ABC_PATH;
            panda::JSNApi::SetAssetPath(vm_, path);
        }
        return func(path, srcPath);
    }

    NativeValue* LoadJsModule(const std::string& path, const std::string& hapPath) override
    {
        if (!RunScript(path, hapPath, false)) {
            HILOG_ERROR("Failed to run script: %{private}s", path.c_str());
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

    bool LoadRepairPatch(const std::string& hqfFile, const std::string& hapPath) override
    {
        HILOG_DEBUG("LoadRepairPatch function called.");
        if (vm_ == nullptr) {
            HILOG_ERROR("LoadRepairPatch, vm is nullptr.");
            return false;
        }

        AbilityRuntime::RuntimeExtractor extractor(hqfFile);
        if (!extractor.Init()) {
            HILOG_ERROR("LoadRepairPatch, Extractor of %{private}s init failed.", hqfFile.c_str());
            return false;
        }

        std::vector<std::string> fileNames;
        extractor.GetSpecifiedTypeFiles(fileNames, ".abc");
        if (fileNames.empty()) {
            HILOG_WARN("LoadRepairPatch, There's no abc file in hqf %{private}s.", hqfFile.c_str());
            return true;
        }

        for (const auto &fileName : fileNames) {
            std::string patchFile = hqfFile + "/" + fileName;
            std::string baseFile = hapPath + "/" + fileName;
            std::ostringstream outStream;
            if (!extractor.ExtractByName(fileName, outStream)) {
                HILOG_ERROR("LoadRepairPatch, Extract %{public}s failed.", patchFile.c_str());
                return false;
            }

            const auto &outStr = outStream.str();
            std::vector<uint8_t> buffer;
            buffer.assign(outStr.begin(), outStr.end());
            HILOG_DEBUG("LoadRepairPatch, LoadPatch, patchFile: %{private}s, baseFile: %{private}s.",
                patchFile.c_str(), baseFile.c_str());
            bool ret = panda::JSNApi::LoadPatch(vm_, patchFile, buffer.data(), buffer.size(), baseFile);
            if (!ret) {
                HILOG_ERROR("LoadRepairPatch, LoadPatch failed.");
                return false;
            }
            HILOG_DEBUG("LoadRepairPatch, Load patch %{private}s succeed.", patchFile.c_str());
        }

        return true;
    }

    bool UnLoadRepairPatch(const std::string& hqfFile) override
    {
        HILOG_DEBUG("UnLoadRepairPatch function called.");
        if (vm_ == nullptr) {
            HILOG_ERROR("UnLoadRepairPatch vm is nullptr.");
            return false;
        }

        AbilityRuntime::RuntimeExtractor extractor(hqfFile);
        if (!extractor.Init()) {
            HILOG_ERROR("UnLoadRepairPatch, Extractor of %{private}s init failed.", hqfFile.c_str());
            return false;
        }

        std::vector<std::string> fileNames;
        extractor.GetSpecifiedTypeFiles(fileNames, ".abc");
        if (fileNames.empty()) {
            HILOG_WARN("UnLoadRepairPatch, There's no abc file in hqf %{private}s.", hqfFile.c_str());
            return true;
        }

        for (const auto &fileName : fileNames) {
            std::string patchFile = hqfFile + "/" + fileName;
            HILOG_DEBUG("UnLoadRepairPatch, UnloadPatch, patchFile: %{private}s.", patchFile.c_str());
            bool ret = panda::JSNApi::UnloadPatch(vm_, patchFile);
            if (!ret) {
                HILOG_ERROR("UnLoadRepairPatch, UnLoadPatch failed.");
                return false;
            }
            HILOG_DEBUG("UnLoadRepairPatch, UnLoad patch %{private}s succeed.", patchFile.c_str());
        }

        return true;
    }

    bool NotifyHotReloadPage() override
    {
        HILOG_DEBUG("function called.");
        Ace::HotReloader::HotReload();
        return true;
    }

private:
    static int32_t PrintVmLog(int32_t, int32_t, const char*, const char*, const char* message)
    {
        HILOG_INFO("ArkLog: %{public}s", message);
        return 0;
    }

    void FinishPreload() override
    {
        panda::JSNApi::PreFork(vm_);
    }

    bool Initialize(const Runtime::Options& options) override
    {
        if (preloaded_) {
            panda::RuntimeOption postOption;
            postOption.SetBundleName(options.bundleName);
            if (!options.arkNativeFilePath.empty()) {
                std::string sandBoxAnFilePath = SANDBOX_ARK_CACHE_PATH + options.arkNativeFilePath;
                postOption.SetAnDir(sandBoxAnFilePath);
            }
            panda::JSNApi::PostFork(vm_, postOption);
            nativeEngine_->ReinitUVLoop();
            panda::JSNApi::SetLoop(vm_, nativeEngine_->GetUVLoop());
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

            bool asmInterpreterEnabled = OHOS::system::GetBoolParameter("persist.ark.asminterpreter", true);
            std::string asmOpcodeDisableRange = OHOS::system::GetParameter("persist.ark.asmopcodedisablerange", "");
            pandaOption.SetEnableAsmInterpreter(asmInterpreterEnabled);
            pandaOption.SetAsmOpcodeDisableRange(asmOpcodeDisableRange);

            // aot related
            bool aotEnabled = OHOS::system::GetBoolParameter("persist.ark.aot", true);
            pandaOption.SetEnableAOT(aotEnabled);
            bool profileEnabled = OHOS::system::GetBoolParameter("persist.ark.profile", false);
            pandaOption.SetEnableProfile(profileEnabled);
            pandaOption.SetProfileDir(SANDBOX_ARK_PROIFILE_PATH);
            HILOG_DEBUG("JSRuntime::Initialize ArkNative file path = %{public}s", options.arkNativeFilePath.c_str());
            vm_ = panda::JSNApi::CreateJSVM(pandaOption);
            if (vm_ == nullptr) {
                return false;
            }

            nativeEngine_ = std::make_unique<ArkNativeEngine>(vm_, static_cast<JsRuntime*>(this));
        }

        if (!options.preload) {
            bundleName_ = options.bundleName;
            panda::JSNApi::SetHostResolvePathTracker(vm_, JsModuleSearcher(options.bundleName));
            std::shared_ptr<RuntimeExtractor> runtimeExtractor = RuntimeExtractor::Create(options.hapPath);
            if (runtimeExtractor == nullptr) {
                return false;
            }
            runtimeExtractor->SetRuntimeFlag(true);
            runtimeExtractorMap_.insert(make_pair(options.hapPath, runtimeExtractor));
            panda::JSNApi::SetHostResolveBufferTracker(
                vm_, JsModuleReader(options.bundleName, options.hapPath, runtimeExtractor));
        }
        isBundle_ = options.isBundle;
        panda::JSNApi::SetBundle(vm_, options.isBundle);
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
    HILOG_DEBUG("JsRuntime::LoadSystemModule(%{public}s)", moduleName.c_str());
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
    }

    auto moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager != nullptr) {
        for (const auto &appLibPath : options.appLibPaths) {
            moduleManager->SetAppLibPath(appLibPath.first, appLibPath.second);
        }
    }

    if (!options.preload) {
        InitTimerModule(*nativeEngine_, *globalObj);
        InitWorkerModule(*nativeEngine_, codePath_, options.isDebugVersion);
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
    if (fd >= 0 && eventHandler_ != nullptr) {
        eventHandler_->RemoveFileDescriptorListener(fd);
    }
    RemoveTask(TIMER_TASK);

    nativeEngine_.reset();
}

NativeValue* JsRuntime::LoadJsBundle(const std::string& path, const std::string& hapPath, bool useCommonChunk)
{
    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine_->GetGlobal());
    NativeValue* exports = nativeEngine_->CreateObject();
    globalObj->SetProperty("exports", exports);

    if (!RunScript(path, hapPath, useCommonChunk)) {
        HILOG_ERROR("Failed to run script: %{private}s", path.c_str());
        return nullptr;
    }

    NativeObject* exportsObj = ConvertNativeValueTo<NativeObject>(globalObj->GetProperty("exports"));
    if (exportsObj == nullptr) {
        HILOG_ERROR("Failed to get exports objcect: %{private}s", path.c_str());
        return nullptr;
    }

    NativeValue* exportObj = exportsObj->GetProperty("default");
    if (exportObj == nullptr) {
        HILOG_ERROR("Failed to get default objcect: %{private}s", path.c_str());
        return nullptr;
    }

    return exportObj;
}

std::unique_ptr<NativeReference> JsRuntime::LoadModule(const std::string& moduleName, const std::string& modulePath,
    const std::string& hapPath, bool esmodule, bool useCommonChunk)
{
    HILOG_DEBUG("JsRuntime::LoadModule(%{public}s, %{private}s, %{private}s, %{public}s)",
        moduleName.c_str(), modulePath.c_str(), hapPath.c_str(), esmodule ? "true" : "false");
    HandleScope handleScope(*this);

    std::string path = moduleName;
    auto pos = path.find("::");
    if (pos != std::string::npos) {
        path.erase(pos, path.size() - pos);
        moduleName_ = path;
    }

    NativeValue* classValue = nullptr;

    auto it = modules_.find(modulePath);
    if (it != modules_.end()) {
        classValue = it->second->Get();
    } else {
        std::string fileName;
        if (!hapPath.empty()) {
            fileName.append(codePath_).append(Constants::FILE_SEPARATOR).append(modulePath);
            std::regex pattern(std::string(Constants::FILE_DOT) + std::string(Constants::FILE_SEPARATOR));
            fileName = std::regex_replace(fileName, pattern, "");
        } else {
            if (!MakeFilePath(codePath_, modulePath, fileName)) {
                HILOG_ERROR("Failed to make module file path: %{private}s", modulePath.c_str());
                return std::unique_ptr<NativeReference>();
            }
        }
        HILOG_ERROR("Failed to make module file path: %{public}s", fileName.c_str());
        classValue = esmodule ? LoadJsModule(fileName, hapPath) : LoadJsBundle(fileName, hapPath, useCommonChunk);
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

bool JsRuntime::RunScript(const std::string& srcPath, const std::string& hapPath, bool useCommonChunk)
{
    bool result = false;
    if (!hapPath.empty()) {
        std::ostringstream outStream;
        std::shared_ptr<RuntimeExtractor> runtimeExtractor;
        if (runtimeExtractorMap_.find(hapPath) == runtimeExtractorMap_.end()) {
            runtimeExtractor = RuntimeExtractor::Create(hapPath);
            if (runtimeExtractor == nullptr) {
                return result;
            }
            runtimeExtractor->SetRuntimeFlag(true);
            runtimeExtractorMap_.insert(make_pair(hapPath, runtimeExtractor));
        } else {
            runtimeExtractor = runtimeExtractorMap_.at(hapPath);
        }
        if (isBundle_) {
            if (!runtimeExtractor->GetFileBuffer(srcPath, outStream)) {
                HILOG_ERROR("Get abc file failed");
                return result;
            }
        } else {
            std::string mergeAbcPath = BUNDLE_INSTALL_PATH + moduleName_ + MERGE_ABC_PATH;
            if (!runtimeExtractor->GetFileBuffer(mergeAbcPath, outStream)) {
                HILOG_ERROR("Get Module abc file failed");
                return result;
            }
        }

        const auto& outStr = outStream.str();
        std::vector<uint8_t> buffer;
        buffer.assign(outStr.begin(), outStr.end());

        result = nativeEngine_->RunScriptBuffer(srcPath.c_str(), buffer, isBundle_) != nullptr;
    } else {
        result = nativeEngine_->RunScript(srcPath.c_str()) != nullptr;
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
    if (eventHandler_ != nullptr) {
        eventHandler_->PostTask(task, name, delayTime);
    }
}

void JsRuntime::RemoveTask(const std::string& name)
{
    if (eventHandler_ != nullptr) {
        eventHandler_->RemoveTask(name);
    }
}

void JsRuntime::DumpHeapSnapshot(bool isPrivate)
{
    nativeEngine_->DumpHeapSnapshot(true, DumpFormat::JSON, isPrivate);
}

bool JsRuntime::BuildJsStackInfoList(uint32_t tid, std::vector<JsFrames>& jsFrames)
{
    std::vector<JsFrameInfo> jsFrameInfo;
    bool ret = nativeEngine_->BuildJsStackInfoList(tid, jsFrameInfo);
    if (!ret) {
        return ret;
    }
    for (auto jf : jsFrameInfo) {
        struct JsFrames jsFrame;
        jsFrame.functionName = jf.functionName;
        jsFrame.fileName = jf.fileName;
        jsFrame.pos = jf.pos;
        jsFrame.nativePointer = jf.nativePointer;
        jsFrames.emplace_back(jsFrame);
    }
    return ret;
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
