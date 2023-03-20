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

#include "accesstoken_kit.h"
#include "constants.h"
#include "connect_server_manager.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "event_handler.h"
#include "extract_resource_manager.h"
#include "file_path_utils.h"
#include "hdc_register.h"
#include "hilog_wrapper.h"
#include "hot_reloader.h"
#include "ipc_skeleton.h"
#include "js_console_log.h"
#include "js_environment.h"
#include "js_module_reader.h"
#include "js_module_searcher.h"
#include "js_runtime_utils.h"
#include "js_timer.h"
#include "js_utils.h"
#include "js_worker.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "ohos_js_env_logger.h"
#include "ohos_js_environment_impl.h"
#include "parameters.h"
#include "extractor.h"
#include "systemcapability.h"

#ifdef SUPPORT_GRAPHICS
#include "declarative_module_preloader.h"
#endif

using namespace OHOS::AbilityBase;
using Extractor = OHOS::AbilityBase::Extractor;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr uint8_t SYSCAP_MAX_SIZE = 64;
constexpr int64_t DEFAULT_GC_POOL_SIZE = 0x10000000; // 256MB
const std::string SANDBOX_ARK_CACHE_PATH = "/data/storage/ark-cache/";
const std::string SANDBOX_ARK_PROIFILE_PATH = "/data/storage/ark-profile";
#ifdef APP_USE_ARM
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib/libark_debugger.z.so";
#else
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib64/libark_debugger.z.so";
#endif

constexpr char TIMER_TASK[] = "uv_timer_task";
constexpr char MERGE_ABC_PATH[] = "/ets/modules.abc";
constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";
constexpr const char* PERMISSION_RUN_ANY_CODE = "ohos.permission.RUN_ANY_CODE";

static auto PermissionCheckFunc = []() {
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();

    int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, PERMISSION_RUN_ANY_CODE);
    if (result == Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        return true;
    } else {
        return false;
    }
};

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

int32_t PrintVmLog(int32_t, int32_t, const char*, const char*, const char* message)
{
    HILOG_INFO("ArkLog: %{public}s", message);
    return 0;
}
} // namespace

std::atomic<bool> JsRuntime::hasInstance(false);

JsRuntime::JsRuntime()
{
    HILOG_DEBUG("JsRuntime costructor.");
}

JsRuntime::~JsRuntime()
{
    HILOG_DEBUG("JsRuntime destructor.");
    Deinitialize();

    auto vm = GetEcmaVm();
    if (vm != nullptr) {
        if (debugMode_) {
            ConnectServerManager::Get().RemoveInstance(instanceId_);
            panda::JSNApi::StopDebugger(vm);
        }
    }
}

std::unique_ptr<Runtime> JsRuntime::Create(const Options& options)
{
    std::unique_ptr<JsRuntime> instance;

    if (!options.preload && options.isStageModel) {
        auto preloadedInstance = Runtime::GetPreloaded();
        if (preloadedInstance && preloadedInstance->GetLanguage() == Runtime::Language::JS) {
            instance.reset(static_cast<JsRuntime*>(preloadedInstance.release()));
        } else {
            instance = std::make_unique<JsRuntime>();
        }
    } else {
        instance = std::make_unique<JsRuntime>();
    }

    if (!instance->Initialize(options)) {
        return std::unique_ptr<Runtime>();
    }
    return instance;
}

void JsRuntime::StartDebugMode(bool needBreakPoint)
{
    if (debugMode_) {
        HILOG_INFO("Already in debug mode");
        return;
    }

    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);

    // Set instance id to tid after the first instance.
    if (JsRuntime::hasInstance.exchange(true, std::memory_order_relaxed)) {
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
    panda::JSNApi::StartDebugger(ARK_DEBUGGER_LIB_PATH, vm, needBreakPoint, instanceId_, debuggerPostTask);

    debugMode_ = true;
}

bool JsRuntime::GetFileBuffer(const std::string& filePath, std::string& fileFullName, std::vector<uint8_t>& buffer)
{
    Extractor extractor(filePath);
    if (!extractor.Init()) {
        HILOG_ERROR("GetFileBuffer, Extractor of %{private}s init failed.", filePath.c_str());
        return false;
    }

    std::vector<std::string> fileNames;
    extractor.GetSpecifiedTypeFiles(fileNames, ".abc");
    if (fileNames.empty()) {
        HILOG_WARN("GetFileBuffer, There's no abc file in hap or hqf %{private}s.", filePath.c_str());
        return true;
    }

    std::string fileName = fileNames.front();
    fileFullName = filePath + "/" + fileName;
    std::ostringstream outStream;
    if (!extractor.ExtractByName(fileName, outStream)) {
        HILOG_ERROR("GetFileBuffer, Extract %{public}s failed.", fileFullName.c_str());
        return false;
    }

    const auto &outStr = outStream.str();
    buffer.assign(outStr.begin(), outStr.end());
    return true;
}

bool JsRuntime::LoadRepairPatch(const std::string& hqfFile, const std::string& hapPath)
{
    HILOG_DEBUG("LoadRepairPatch function called.");
    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, false);

    std::string patchFile;
    std::vector<uint8_t> patchBuffer;
    if (!GetFileBuffer(hqfFile, patchFile, patchBuffer)) {
        HILOG_ERROR("LoadRepairPatch, get patch file buffer failed.");
        return false;
    }

    std::string baseFile;
    std::vector<uint8_t> baseBuffer;
    if (!GetFileBuffer(hapPath, baseFile, baseBuffer)) {
        HILOG_ERROR("LoadRepairPatch, get base file buffer failed.");
        return false;
    }

    std::string resolvedHapPath;
    auto position = hapPath.find(".hap");
    if (position != std::string::npos) {
        resolvedHapPath = hapPath.substr(0, position) + MERGE_ABC_PATH;
    }

    HILOG_DEBUG("LoadRepairPatch, LoadPatch, patchFile: %{private}s, baseFile: %{private}s.",
        patchFile.c_str(), resolvedHapPath.c_str());
    auto ret = panda::JSNApi::LoadPatch(vm, patchFile, patchBuffer.data(), patchBuffer.size(),
        resolvedHapPath, baseBuffer.data(), baseBuffer.size());
    if (ret != panda::JSNApi::PatchErrorCode::SUCCESS) {
        HILOG_ERROR("LoadPatch failed with %{public}d.", static_cast<int32_t>(ret));
        return false;
    }

    HILOG_DEBUG("LoadRepairPatch, Load patch %{private}s succeed.", patchFile.c_str());
    return true;
}

bool JsRuntime::UnLoadRepairPatch(const std::string& hqfFile)
{
    HILOG_DEBUG("UnLoadRepairPatch function called.");
    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, false);

    Extractor extractor(hqfFile);
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
        auto ret = panda::JSNApi::UnloadPatch(vm, patchFile);
        if (ret != panda::JSNApi::PatchErrorCode::SUCCESS) {
            HILOG_WARN("UnLoadPatch failed with %{public}d.", static_cast<int32_t>(ret));
        }
        HILOG_DEBUG("UnLoadRepairPatch, UnLoad patch %{private}s succeed.", patchFile.c_str());
    }

    return true;
}

bool JsRuntime::NotifyHotReloadPage()
{
    HILOG_DEBUG("function called.");
    Ace::HotReloader::HotReload();
    return true;
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

void JsRuntime::FinishPreload()
{
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    panda::JSNApi::PreFork(vm);
}

bool JsRuntime::Initialize(const Options& options)
{
    if (!preloaded_) {
        if (!CreateJsEnv(options)) {
            HILOG_ERROR("Create js environment failed.");
            return false;
        }
    }

    HandleScope handleScope(*this);
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER_AND_RETURN(nativeEngine, false);

    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, false);

    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine->GetGlobal());
    CHECK_POINTER_AND_RETURN(globalObj, false);

    if (IsUseAbilityRuntime(options)) {
        if (!preloaded_) {
            InitConsoleLogModule(*nativeEngine, *globalObj);
            InitSyscapModule(*nativeEngine, *globalObj);

            // Simple hook function 'isSystemplugin'
            const char* moduleName = "JsRuntime";
            BindNativeFunction(*nativeEngine, *globalObj, "isSystemplugin", moduleName,
                [](NativeEngine* engine, NativeCallbackInfo* info) -> NativeValue* {
                    return engine->CreateUndefined();
                });

            methodRequireNapiRef_.reset(nativeEngine->CreateReference(globalObj->GetProperty("requireNapi"), 1));
            if (!methodRequireNapiRef_) {
                HILOG_ERROR("Failed to create reference for global.requireNapi");
                return false;
            }

            PreloadAce(options);
            nativeEngine->RegisterPermissionCheck(PermissionCheckFunc);
        } else {
            panda::RuntimeOption postOption;
            postOption.SetBundleName(options.bundleName);
            if (!options.arkNativeFilePath.empty()) {
                std::string sandBoxAnFilePath = SANDBOX_ARK_CACHE_PATH + options.arkNativeFilePath;
                postOption.SetAnDir(sandBoxAnFilePath);
            }
            bool profileEnabled = OHOS::system::GetBoolParameter("ark.profile", false);
            postOption.SetEnableProfile(profileEnabled);
            panda::JSNApi::PostFork(vm, postOption);
            nativeEngine->ReinitUVLoop();
            panda::JSNApi::SetLoop(vm, nativeEngine->GetUVLoop());
        }

        if (!options.preload) {
            isBundle_ = options.isBundle;
            bundleName_ = options.bundleName;
            codePath_ = options.codePath;

            if (!options.hapPath.empty()) {
                bool newCreate = false;
                std::string loadPath = ExtractorUtil::GetLoadFilePath(options.hapPath);
                std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate);
                if (!extractor) {
                    HILOG_ERROR("Get extractor failed. hapPath[%{private}s]", options.hapPath.c_str());
                    return false;
                }
                if (newCreate) {
                    ExtractorUtil::AddExtractor(loadPath, extractor);
                    extractor->SetRuntimeFlag(true);
                    panda::JSNApi::LoadAotFile(vm, options.hapPath);
                }
            }

            panda::JSNApi::SetBundle(vm, options.isBundle);
            panda::JSNApi::SetBundleName(vm, options.bundleName);
            panda::JSNApi::SetHostResolveBufferTracker(vm, JsModuleReader(options.bundleName));

            if (!InitLoop(options.eventRunner)) {
                HILOG_ERROR("Initialize loop failed.");
                return false;
            }

            SetAppLibPath(options.appLibPaths);
            InitSourceMap(options);

            if (options.isUnique) {
                HILOG_INFO("Not supported TimerModule when form render");
            } else {
                InitTimerModule(*nativeEngine, *globalObj);
            }

            InitWorkerModule(*nativeEngine, codePath_, options.isDebugVersion, options.isBundle);
        }
    }

    preloaded_ = options.preload;
    return true;
}

bool JsRuntime::CreateJsEnv(const Options& options)
{
    panda::RuntimeOption pandaOption;
    int arkProperties = OHOS::system::GetIntParameter<int>("persist.ark.properties", -1);
    std::string bundleName = OHOS::system::GetParameter("persist.ark.arkbundlename", "");
    size_t gcThreadNum = OHOS::system::GetUintParameter<size_t>("persist.ark.gcthreads", 7);
    size_t longPauseTime = OHOS::system::GetUintParameter<size_t>("persist.ark.longpausetime", 40);
    pandaOption.SetArkProperties(arkProperties);
    pandaOption.SetArkBundleName(bundleName);
    pandaOption.SetGcThreadNum(gcThreadNum);
    pandaOption.SetLongPauseTime(longPauseTime);
    HILOG_INFO("JSRuntime::Initialize ark properties = %{public}d bundlename = %{public}s",
        arkProperties, bundleName.c_str());
    pandaOption.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    pandaOption.SetGcPoolSize(DEFAULT_GC_POOL_SIZE);
    pandaOption.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::INFO);
    pandaOption.SetLogBufPrint(PrintVmLog);

    bool asmInterpreterEnabled = OHOS::system::GetBoolParameter("persist.ark.asminterpreter", true);
    std::string asmOpcodeDisableRange = OHOS::system::GetParameter("persist.ark.asmopcodedisablerange", "");
    pandaOption.SetEnableAsmInterpreter(asmInterpreterEnabled);
    pandaOption.SetAsmOpcodeDisableRange(asmOpcodeDisableRange);

    if (IsUseAbilityRuntime(options)) {
        // aot related
        bool aotEnabled = OHOS::system::GetBoolParameter("persist.ark.aot", true);
        pandaOption.SetEnableAOT(aotEnabled);
        pandaOption.SetProfileDir(SANDBOX_ARK_PROIFILE_PATH);
    }

    OHOSJsEnvLogger::RegisterJsEnvLogger();
    auto jsEnvImpl = std::make_shared<OHOSJsEnvironmentImpl>();
    jsEnv_ = std::make_shared<JsEnv::JsEnvironment>(jsEnvImpl);
    if (jsEnv_ == nullptr || !jsEnv_->Initialize(pandaOption, static_cast<void*>(this))) {
        HILOG_ERROR("Initialize js environment failed.");
        return false;
    }

    return true;
}

void JsRuntime::PreloadAce(const Options& options)
{
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
#ifdef SUPPORT_GRAPHICS
    if (options.loadAce) {
        // ArkTsCard start
        if (options.isUnique) {
            OHOS::Ace::DeclarativeModulePreloader::PreloadCard(*nativeEngine);
        } else {
            OHOS::Ace::DeclarativeModulePreloader::Preload(*nativeEngine);
        }
        // ArkTsCard end
    }
#endif
}

bool JsRuntime::InitLoop(const std::shared_ptr<AppExecFwk::EventRunner>& eventRunner)
{
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER_AND_RETURN(nativeEngine, false);

    // Create event handler for runtime
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(eventRunner);

    auto uvLoop = nativeEngine->GetUVLoop();
    auto fd = uvLoop != nullptr ? uv_backend_fd(uvLoop) : -1;
    if (fd < 0) {
        HILOG_ERROR("Failed to get backend fd from uv loop");
        return false;
    }

    // MUST run uv loop once before we listen its backend fd.
    uv_run(uvLoop, UV_RUN_NOWAIT);

    uint32_t events = AppExecFwk::FILE_DESCRIPTOR_INPUT_EVENT | AppExecFwk::FILE_DESCRIPTOR_OUTPUT_EVENT;
    eventHandler_->AddFileDescriptorListener(fd, events, std::make_shared<UvLoopHandler>(uvLoop));
    return true;
}

void JsRuntime::SetAppLibPath(const std::map<std::string, std::vector<std::string>>& appLibPaths)
{
    auto moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager != nullptr) {
        for (const auto &appLibPath : appLibPaths) {
            moduleManager->SetAppLibPath(appLibPath.first, appLibPath.second);
        }
    }
}

void JsRuntime::InitSourceMap(const Options& options)
{
    bindSourceMaps_ = std::make_unique<ModSourceMap>(options.bundleCodeDir, options.isStageModel);
}

void JsRuntime::Deinitialize()
{
    HILOG_DEBUG("JsRuntime deinitialize.");
    for (auto it = modules_.begin(); it != modules_.end(); it = modules_.erase(it)) {
        delete it->second;
        it->second = nullptr;
    }

    methodRequireNapiRef_.reset();

    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
    auto uvLoop = nativeEngine->GetUVLoop();
    auto fd = uvLoop != nullptr ? uv_backend_fd(uvLoop) : -1;
    if (fd >= 0 && eventHandler_ != nullptr) {
        eventHandler_->RemoveFileDescriptorListener(fd);
    }
    RemoveTask(TIMER_TASK);
}

NativeValue* JsRuntime::LoadJsBundle(const std::string& path, const std::string& hapPath, bool useCommonChunk)
{
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER_AND_RETURN(nativeEngine, nullptr);
    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine->GetGlobal());
    NativeValue* exports = nativeEngine->CreateObject();
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

NativeValue* JsRuntime::LoadJsModule(const std::string& path, const std::string& hapPath)
{
    if (!RunScript(path, hapPath, false)) {
        HILOG_ERROR("Failed to run script: %{private}s", path.c_str());
        return nullptr;
    }

    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, nullptr);
    panda::Local<panda::ObjectRef> exportObj = panda::JSNApi::GetExportObject(vm, path, "default");
    if (exportObj->IsNull()) {
        HILOG_ERROR("Get export object failed");
        return nullptr;
    }

    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER_AND_RETURN(nativeEngine, nullptr);
    return ArkNativeEngine::ArkValueToNativeValue(static_cast<ArkNativeEngine*>(nativeEngine), exportObj);
}

std::unique_ptr<NativeReference> JsRuntime::LoadModule(const std::string& moduleName, const std::string& modulePath,
    const std::string& hapPath, bool esmodule, bool useCommonChunk)
{
    HILOG_DEBUG("JsRuntime::LoadModule(%{public}s, %{private}s, %{private}s, %{public}s)",
        moduleName.c_str(), modulePath.c_str(), hapPath.c_str(), esmodule ? "true" : "false");
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER_AND_RETURN(nativeEngine, std::unique_ptr<NativeReference>());

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
        classValue = esmodule ? LoadJsModule(fileName, hapPath) : LoadJsBundle(fileName, hapPath, useCommonChunk);
        if (classValue == nullptr) {
            return std::unique_ptr<NativeReference>();
        }

        modules_.emplace(modulePath, nativeEngine->CreateReference(classValue, 1));
    }

    NativeValue* instanceValue = nativeEngine->CreateInstance(classValue, nullptr, 0);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    return std::unique_ptr<NativeReference>(nativeEngine->CreateReference(instanceValue, 1));
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModule(
    const std::string& moduleName, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("JsRuntime::LoadSystemModule(%{public}s)", moduleName.c_str());
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER_AND_RETURN(nativeEngine, std::unique_ptr<NativeReference>());

    HandleScope handleScope(*this);

    NativeValue* className = nativeEngine->CreateString(moduleName.c_str(), moduleName.length());
    NativeValue* classValue =
        nativeEngine->CallFunction(nativeEngine->GetGlobal(), methodRequireNapiRef_->Get(), &className, 1);
    NativeValue* instanceValue = nativeEngine->CreateInstance(classValue, argv, argc);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    return std::unique_ptr<NativeReference>(nativeEngine->CreateReference(instanceValue, 1));
}

bool JsRuntime::RunScript(const std::string& srcPath, const std::string& hapPath, bool useCommonChunk)
{
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER_AND_RETURN(nativeEngine, false);
    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, false);

    std::string commonsPath = std::string(Constants::LOCAL_CODE_PATH) + "/" + moduleName_ + "/ets/commons.abc";
    std::string vendorsPath = std::string(Constants::LOCAL_CODE_PATH) + "/" + moduleName_ + "/ets/vendors.abc";
    if (hapPath.empty()) {
        if (useCommonChunk) {
            (void)nativeEngine->RunScriptPath(commonsPath.c_str());
            (void)nativeEngine->RunScriptPath(vendorsPath.c_str());
        }
        return nativeEngine->RunScriptPath(srcPath.c_str()) != nullptr;
    }

    bool newCreate = false;
    std::string loadPath = ExtractorUtil::GetLoadFilePath(hapPath);
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate);
    if (!extractor) {
        HILOG_ERROR("Get extractor failed. hapPath[%{private}s]", hapPath.c_str());
        return false;
    }
    if (newCreate) {
        ExtractorUtil::AddExtractor(loadPath, extractor);
        extractor->SetRuntimeFlag(true);
        panda::JSNApi::LoadAotFile(vm, hapPath);
        auto resourceManager = AbilityBase::ExtractResourceManager::GetExtractResourceManager().GetGlobalObject();
        if (resourceManager) {
            resourceManager->AddResource(loadPath.c_str());
        }
    }

    auto func = [&](std::string modulePath, const std::string abcPath) {
        std::ostringstream outStream;
        if (!extractor->GetFileBuffer(modulePath, outStream)) {
            HILOG_ERROR("Get abc file failed");
            return false;
        }

        const auto& outStr = outStream.str();
        std::vector<uint8_t> buffer;
        buffer.assign(outStr.begin(), outStr.end());

        return nativeEngine->RunScriptBuffer(abcPath.c_str(), buffer, isBundle_) != nullptr;
    };

    if (useCommonChunk) {
        (void)func(commonsPath, commonsPath);
        (void)func(vendorsPath, vendorsPath);
    }

    std::string path = srcPath;
    if (!isBundle_) {
        if (moduleName_.empty()) {
            HILOG_ERROR("moduleName is hole");
            return false;
        }
        path = BUNDLE_INSTALL_PATH + moduleName_ + MERGE_ABC_PATH;
        panda::JSNApi::SetAssetPath(vm, path);
        panda::JSNApi::SetModuleName(vm, moduleName_);
    }
    return func(path, srcPath);
}

bool JsRuntime::RunSandboxScript(const std::string& path, const std::string& hapPath)
{
    std::string fileName;
    if (!hapPath.empty()) {
        fileName.append(codePath_).append(Constants::FILE_SEPARATOR).append(path);
        std::regex pattern(std::string(Constants::FILE_DOT) + std::string(Constants::FILE_SEPARATOR));
        fileName = std::regex_replace(fileName, pattern, "");
    } else {
        if (!MakeFilePath(codePath_, path, fileName)) {
            HILOG_ERROR("Failed to make module file path: %{private}s", path.c_str());
            return false;
        }
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
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
    nativeEngine->DumpHeapSnapshot(true, DumpFormat::JSON, isPrivate);
}

bool JsRuntime::BuildJsStackInfoList(uint32_t tid, std::vector<JsFrames>& jsFrames)
{
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER_AND_RETURN(nativeEngine, false);
    std::vector<JsFrameInfo> jsFrameInfo;
    bool ret = nativeEngine->BuildJsStackInfoList(tid, jsFrameInfo);
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
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
    nativeEngine->NotifyApplicationState(isBackground);
    HILOG_INFO("NotifyApplicationState, isBackground %{public}d.", isBackground);
}

void JsRuntime::PreloadSystemModule(const std::string& moduleName)
{
    HandleScope handleScope(*this);
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
    NativeValue* className = nativeEngine->CreateString(moduleName.c_str(), moduleName.length());
    nativeEngine->CallFunction(nativeEngine->GetGlobal(), methodRequireNapiRef_->Get(), &className, 1);
}

void JsRuntime::UpdateExtensionType(int32_t extensionType)
{
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
    NativeModuleManager* moduleManager = nativeEngine->GetModuleManager();
    if (moduleManager == nullptr) {
        HILOG_ERROR("UpdateExtensionType error, moduleManager is nullptr");
        return;
    }
    moduleManager->SetProcessExtensionType(extensionType);
}

NativeEngine& JsRuntime::GetNativeEngine() const
{
    return *GetNativeEnginePointer();
}

NativeEngine* JsRuntime::GetNativeEnginePointer() const
{
    CHECK_POINTER_AND_RETURN(jsEnv_, nullptr);
    return jsEnv_->GetNativeEngine();
}

panda::ecmascript::EcmaVM* JsRuntime::GetEcmaVm() const
{
    CHECK_POINTER_AND_RETURN(jsEnv_, nullptr);
    return jsEnv_->GetVM();
}

bool JsRuntime::IsUseAbilityRuntime(const Options& options) const
{
    return (options.isStageModel) || (options.isTestFramework);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
