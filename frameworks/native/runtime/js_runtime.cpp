/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include <dlfcn.h>

#include "accesstoken_kit.h"
#include "constants.h"
#include "connect_server_manager.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "extract_resource_manager.h"
#include "file_path_utils.h"
#include "hdc_register.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "hot_reloader.h"
#include "ipc_skeleton.h"
#include "js_environment.h"
#include "js_module_reader.h"
#include "js_module_searcher.h"
#include "js_quickfix_callback.h"
#include "js_runtime_utils.h"
#include "js_utils.h"
#include "js_worker.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "ohos_js_env_logger.h"
#include "ohos_js_environment_impl.h"
#include "parameters.h"
#include "extractor.h"
#include "systemcapability.h"
#include "source_map.h"
#include "source_map_operator.h"

#ifdef SUPPORT_GRAPHICS
#include "declarative_module_preloader.h"
#endif

using namespace OHOS::AbilityBase;
using Extractor = OHOS::AbilityBase::Extractor;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t PARAM_TWO = 2;
constexpr uint8_t SYSCAP_MAX_SIZE = 64;
constexpr int64_t DEFAULT_GC_POOL_SIZE = 0x10000000; // 256MB
constexpr int32_t DEFAULT_INTER_VAL = 500;
constexpr int32_t TRIGGER_GC_AFTER_CLEAR_STAGE_MS = 3000;
const std::string SANDBOX_ARK_CACHE_PATH = "/data/storage/ark-cache/";
const std::string SANDBOX_ARK_PROIFILE_PATH = "/data/storage/ark-profile";
#ifdef APP_USE_ARM
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib/libark_debugger.z.so";
#else
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib64/libark_debugger.z.so";
#endif

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
    StopDebugMode();
}

std::unique_ptr<JsRuntime> JsRuntime::Create(const Options& options)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
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
        return std::unique_ptr<JsRuntime>();
    }
    return instance;
}

void JsRuntime::StartDebugMode(bool needBreakPoint)
{
    if (debugMode_) {
        HILOG_INFO("Already in debug mode");
        return;
    }
    CHECK_POINTER(jsEnv_);
    // Set instance id to tid after the first instance.
    if (JsRuntime::hasInstance.exchange(true, std::memory_order_relaxed)) {
        instanceId_ = static_cast<uint32_t>(gettid());
    }

    HILOG_INFO("Ark VM is starting debug mode [%{public}s]", needBreakPoint ? "break" : "normal");
    auto debuggerPostTask = [jsEnv = jsEnv_](std::function<void()>&& task) {
        jsEnv->PostTask(task);
    };
    StartDebuggerInWorkerModule();
    HdcRegister::Get().StartHdcRegister(bundleName_);
    ConnectServerManager::Get().StartConnectServer(bundleName_);
    ConnectServerManager::Get().AddInstance(instanceId_);
    debugMode_ = StartDebugger(needBreakPoint, instanceId_, debuggerPostTask);
}

void JsRuntime::StopDebugMode()
{
    if (debugMode_) {
        ConnectServerManager::Get().RemoveInstance(instanceId_);
        StopDebugger();
    }
}

void JsRuntime::InitConsoleModule()
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->InitConsoleModule();
}

bool JsRuntime::StartDebugger(bool needBreakPoint, const DebuggerPostTask& debuggerPostTask)
{
    return StartDebugger(needBreakPoint, gettid(), debuggerPostTask);
}

bool JsRuntime::StartDebugger(bool needBreakPoint, uint32_t instanceId, const DebuggerPostTask& debuggerPostTask)
{
    CHECK_POINTER_AND_RETURN(jsEnv_, false);
    return jsEnv_->StartDebugger(ARK_DEBUGGER_LIB_PATH, needBreakPoint, instanceId, debuggerPostTask);
}

void JsRuntime::StopDebugger()
{
    jsEnv_->StopDebugger();
}

int32_t JsRuntime::JsperfProfilerCommandParse(const std::string &command, int32_t defaultValue)
{
    HILOG_DEBUG("profiler command parse %{public}s", command.c_str());
    auto findPos = command.find("jsperf");
    if (findPos == std::string::npos) {
        // jsperf command not found, so not to do, return zero.
        HILOG_DEBUG("jsperf command not found");
        return 0;
    }

    // match jsperf command
    auto jsPerfStr = command.substr(findPos, command.length() - findPos);
    const std::regex regexJsperf(R"(^jsperf($|\s+($|\d*\s*($|nativeperf.*))))");
    std::match_results<std::string::const_iterator> matchResults;
    if (!std::regex_match(jsPerfStr, matchResults, regexJsperf)) {
        HILOG_DEBUG("the order not match");
        return defaultValue;
    }

    // get match resuflt
    std::string jsperfResuflt;
    constexpr size_t matchResultIndex = 1;
    if (matchResults.size() < PARAM_TWO) {
        HILOG_ERROR("no results need to be matched");
        return defaultValue;
    }

    jsperfResuflt = matchResults[matchResultIndex].str();
    // match number result
    const std::regex regexJsperfNum(R"(^\s*(\d+).*)");
    std::match_results<std::string::const_iterator> jsperfMatchResults;
    if (!std::regex_match(jsperfResuflt, jsperfMatchResults, regexJsperfNum)) {
        HILOG_DEBUG("the jsperf results not match");
        return defaultValue;
    }

    // get match result
    std::string interval;
    constexpr size_t matchNumResultIndex = 1;
    if (jsperfMatchResults.size() < PARAM_TWO) {
        HILOG_ERROR("no results need to be matched");
        return defaultValue;
    }

    interval = jsperfMatchResults[matchNumResultIndex].str();
    if (interval.empty()) {
        HILOG_DEBUG("match order result error");
        return defaultValue;
    }

    return std::stoi(interval);
}

void JsRuntime::StartProfiler(const std::string &perfCmd)
{
    CHECK_POINTER(jsEnv_);
    if (JsRuntime::hasInstance.exchange(true, std::memory_order_relaxed)) {
        instanceId_ = static_cast<uint32_t>(gettid());
    }

    auto debuggerPostTask = [jsEnv = jsEnv_](std::function<void()>&& task) {
        jsEnv->PostTask(task);
    };

    StartDebuggerInWorkerModule();
    HdcRegister::Get().StartHdcRegister(bundleName_);
    ConnectServerManager::Get().StartConnectServer(bundleName_);
    ConnectServerManager::Get().AddInstance(instanceId_);
    JsEnv::JsEnvironment::PROFILERTYPE profiler = JsEnv::JsEnvironment::PROFILERTYPE::PROFILERTYPE_HEAP;
    int32_t interval = 0;
    const std::string profilerCommand("profile");
    if (perfCmd.find(profilerCommand) != std::string::npos) {
        profiler = JsEnv::JsEnvironment::PROFILERTYPE::PROFILERTYPE_CPU;
        interval = JsperfProfilerCommandParse(perfCmd, DEFAULT_INTER_VAL);
    }

    HILOG_DEBUG("profiler:%{public}d interval:%{public}d.", profiler, interval);
    jsEnv_->StartProfiler(ARK_DEBUGGER_LIB_PATH, instanceId_, profiler, interval, debuggerPostTask);
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

    auto hspPosition = hapPath.find(".hsp");
    if (hspPosition != std::string::npos) {
        resolvedHapPath = hapPath.substr(0, hspPosition) + MERGE_ABC_PATH;
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

bool JsRuntime::LoadScript(const std::string& path, std::vector<uint8_t>* buffer, bool isBundle)
{
    HILOG_DEBUG("function called.");
    CHECK_POINTER_AND_RETURN(jsEnv_, false);
    return jsEnv_->LoadScript(path, buffer, isBundle);
}

bool JsRuntime::LoadScript(const std::string& path, uint8_t *buffer, size_t len, bool isBundle)
{
    HILOG_DEBUG("function called.");
    CHECK_POINTER_AND_RETURN(jsEnv_, false);
    return jsEnv_->LoadScript(path, buffer, len, isBundle);
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
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!preloaded_) {
        if (!CreateJsEnv(options)) {
            HILOG_ERROR("Create js environment failed.");
            return false;
        }
    }

    bool isModular = false;
    if (IsUseAbilityRuntime(options)) {
        HandleScope handleScope(*this);
        auto nativeEngine = GetNativeEnginePointer();
        CHECK_POINTER_AND_RETURN(nativeEngine, false);

        auto vm = GetEcmaVm();
        CHECK_POINTER_AND_RETURN(vm, false);

        if (preloaded_) {
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

        NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine->GetGlobal());
        CHECK_POINTER_AND_RETURN(globalObj, false);

        if (!preloaded_) {
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
                    panda::JSNApi::LoadAotFile(vm, options.moduleName);
                }
            }

            panda::JSNApi::SetBundle(vm, options.isBundle);
            panda::JSNApi::SetBundleName(vm, options.bundleName);
            panda::JSNApi::SetHostResolveBufferTracker(vm, JsModuleReader(options.bundleName));
            isModular = !panda::JSNApi::IsBundle(vm);

            if (!InitLoop(options.eventRunner)) {
                HILOG_ERROR("Initialize loop failed.");
                return false;
            }
        }
    }

    if (!preloaded_) {
        InitConsoleModule();
    }

    if (!options.preload) {
        auto operatorObj = std::make_shared<JsEnv::SourceMapOperator>(options.hapPath, isModular);
        InitSourceMap(operatorObj);

        if (options.isUnique) {
            HILOG_INFO("Not supported TimerModule when form render");
        } else {
            InitTimerModule();
        }

        InitWorkerModule(options);
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
    pandaOption.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::FOLLOW);
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
    jsEnv_ = std::make_shared<JsEnv::JsEnvironment>(std::make_unique<OHOSJsEnvironmentImpl>());
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
            OHOS::Ace::DeclarativeModulePreloader::PreloadCard(*nativeEngine, options.bundleName);
        } else {
            OHOS::Ace::DeclarativeModulePreloader::Preload(*nativeEngine);
        }
        // ArkTsCard end
    }
#endif
}

void JsRuntime::ReloadFormComponent()
{
    HILOG_DEBUG("Call.");
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
    // ArkTsCard update condition, need to reload new component
    OHOS::Ace::DeclarativeModulePreloader::ReloadCard(*nativeEngine, bundleName_);
}

void JsRuntime::DoCleanWorkAfterStageCleaned()
{
    // Force gc. If the jsRuntime is destroyed, this task should not be executed.
    HILOG_DEBUG("DoCleanWorkAfterStageCleaned begin");
    RemoveTask("ability_destruct_gc");
    auto gcTask = [this]() {
        panda::JSNApi::TriggerGC(GetEcmaVm(), panda::JSNApi::TRIGGER_GC_TYPE::FULL_GC);
    };
    PostTask(gcTask, "ability_destruct_gc", TRIGGER_GC_AFTER_CLEAR_STAGE_MS);
}

bool JsRuntime::InitLoop(const std::shared_ptr<AppExecFwk::EventRunner>& eventRunner)
{
    CHECK_POINTER_AND_RETURN(jsEnv_, false);
    return jsEnv_->InitLoop(eventRunner);
}

void JsRuntime::SetAppLibPath(const AppLibPathMap& appLibPaths, const bool& isSystemApp)
{
    HILOG_DEBUG("Set library path.");

    if (appLibPaths.size() == 0) {
        HILOG_WARN("There's no library path need to set.");
        return;
    }

    auto moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager == nullptr) {
        HILOG_ERROR("Get module manager failed.");
        return;
    }

    for (const auto &appLibPath : appLibPaths) {
        moduleManager->SetAppLibPath(appLibPath.first, appLibPath.second, isSystemApp);
    }

    if (!isSystemApp) {
        HILOG_ERROR("------------------into disable\r\n");
        dlns_disable();
    } else {
        HILOG_ERROR("------------------into un disable\r\n");
    }
}

void JsRuntime::InitSourceMap(const std::shared_ptr<JsEnv::SourceMapOperator> operatorObj)
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->InitSourceMap(operatorObj);
    JsEnv::SourceMap::RegisterReadSourceMapCallback(JsRuntime::ReadSourceMapData);
}

void JsRuntime::Deinitialize()
{
    HILOG_DEBUG("JsRuntime deinitialize.");
    for (auto it = modules_.begin(); it != modules_.end(); it = modules_.erase(it)) {
        delete it->second;
        it->second = nullptr;
    }

    methodRequireNapiRef_.reset();

    CHECK_POINTER(jsEnv_);
    jsEnv_->DeInitLoop();
}

NativeValue* JsRuntime::LoadJsBundle(const std::string& path, const std::string& hapPath, bool useCommonChunk)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
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
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
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
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
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
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER_AND_RETURN(nativeEngine, false);
    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, false);

    std::string commonsPath = std::string(Constants::LOCAL_CODE_PATH) + "/" + moduleName_ + "/ets/commons.abc";
    std::string vendorsPath = std::string(Constants::LOCAL_CODE_PATH) + "/" + moduleName_ + "/ets/vendors.abc";
    if (hapPath.empty()) {
        if (useCommonChunk) {
            (void)LoadScript(commonsPath);
            (void)LoadScript(vendorsPath);
        }
        return LoadScript(srcPath);
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
        panda::JSNApi::LoadAotFile(vm, moduleName_);
        auto resourceManager = AbilityBase::ExtractResourceManager::GetExtractResourceManager().GetGlobalObject();
        if (resourceManager) {
            resourceManager->AddResource(loadPath.c_str());
        }
    }

    auto func = [&](std::string modulePath, const std::string abcPath) {
        if (!extractor->IsHapCompress(modulePath)) {
            std::unique_ptr<uint8_t[]> dataPtr = nullptr;
            size_t len = 0;
            if (!extractor->ExtractToBufByName(modulePath, dataPtr, len, true)) {
                HILOG_ERROR("Get abc file failed.");
                return false;
            }
            return LoadScript(abcPath, dataPtr.release(), len, isBundle_);
        } else {
            std::ostringstream outStream;
            if (!extractor->GetFileBuffer(modulePath, outStream)) {
                HILOG_ERROR("Get abc file failed");
                return false;
            }
            const auto& outStr = outStream.str();
            std::vector<uint8_t> buffer;
            buffer.assign(outStr.begin(), outStr.end());

            return LoadScript(abcPath, &buffer, isBundle_);
        }
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
    CHECK_POINTER(jsEnv_);
    jsEnv_->PostTask(task, name, delayTime);
}

void JsRuntime::RemoveTask(const std::string& name)
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->RemoveTask(name);
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

void JsRuntime::UpdateModuleNameAndAssetPath(const std::string& moduleName)
{
    if (isBundle_) {
        return;
    }

    auto vm = GetEcmaVm();
    if (!vm || moduleName.empty()) {
        HILOG_ERROR("vm is nullptr or moduleName is empty");
        return;
    }

    moduleName_ = moduleName;
    std::string path = BUNDLE_INSTALL_PATH + moduleName_ + MERGE_ABC_PATH;
    panda::JSNApi::SetAssetPath(vm, path);
    panda::JSNApi::SetModuleName(vm, moduleName_);
}

void JsRuntime::RegisterUncaughtExceptionHandler(JsEnv::UncaughtExceptionInfo uncaughtExceptionInfo)
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->RegisterUncaughtExceptionHandler(uncaughtExceptionInfo);
}

void JsRuntime::RegisterQuickFixQueryFunc(const std::map<std::string, std::string>& moduleAndPath)
{
    auto vm = GetEcmaVm();
    if (vm != nullptr) {
        panda::JSNApi::RegisterQuickFixQueryFunc(vm, JsQuickfixCallback(moduleAndPath));
    }
}

bool JsRuntime::ReadSourceMapData(const std::string& hapPath, const std::string& sourceMapPath, std::string& content)
{
    // Source map relative path, FA: "/assets/js", Stage: "/ets"
    if (hapPath.empty()) {
        HILOG_ERROR("hapPath is empty");
        return false;
    }
    bool newCreate = false;
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(
        ExtractorUtil::GetLoadFilePath(hapPath), newCreate);
    if (extractor == nullptr) {
        HILOG_ERROR("hap's path: %{public}s, get extractor failed", hapPath.c_str());
        return false;
    }
    std::unique_ptr<uint8_t[]> dataPtr = nullptr;
    size_t len = 0;
    if (!extractor->ExtractToBufByName(sourceMapPath, dataPtr, len)) {
        HILOG_DEBUG("can't find source map, and switch to stage model.");
        std::string tempPath = std::regex_replace(sourceMapPath, std::regex("ets"), "assets/js");
        if (!extractor->ExtractToBufByName(tempPath, dataPtr, len)) {
            HILOG_ERROR("get mergeSourceMapData fileBuffer failed, map path: %{private}s", tempPath.c_str());
            return false;
        }
    }
    content = reinterpret_cast<char*>(dataPtr.get());
    return true;
}

void JsRuntime::FreeNativeReference(std::unique_ptr<NativeReference> reference)
{
    FreeNativeReference(std::move(reference), nullptr);
}

void JsRuntime::FreeNativeReference(std::shared_ptr<NativeReference>&& reference)
{
    FreeNativeReference(nullptr, std::move(reference));
}

struct JsNativeReferenceDeleterObject {
    std::unique_ptr<NativeReference> uniqueNativeRef_ = nullptr;
    std::shared_ptr<NativeReference> sharedNativeRef_ = nullptr;
};

void JsRuntime::FreeNativeReference(std::unique_ptr<NativeReference> uniqueNativeRef,
    std::shared_ptr<NativeReference>&& sharedNativeRef)
{
    if (uniqueNativeRef == nullptr && sharedNativeRef == nullptr) {
        HILOG_WARN("native reference is invalid.");
        return;
    }

    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
    auto uvLoop = nativeEngine->GetUVLoop();
    CHECK_POINTER(uvLoop);

    auto work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("new uv work failed.");
        return;
    }

    auto cb = new (std::nothrow) JsNativeReferenceDeleterObject();
    if (cb == nullptr) {
        HILOG_ERROR("new deleter object failed.");
        delete work;
        work = nullptr;
        return;
    }

    if (uniqueNativeRef != nullptr) {
        cb->uniqueNativeRef_ = std::move(uniqueNativeRef);
    }
    if (sharedNativeRef != nullptr) {
        cb->sharedNativeRef_ = std::move(sharedNativeRef);
    }
    work->data = reinterpret_cast<void*>(cb);
    int ret = uv_queue_work(uvLoop, work, [](uv_work_t *work) {},
    [](uv_work_t *work, int status) {
        if (work != nullptr) {
            if (work->data != nullptr) {
                delete reinterpret_cast<JsNativeReferenceDeleterObject*>(work->data);
                work->data = nullptr;
            }
            delete work;
            work = nullptr;
        }
    });
    if (ret != 0) {
        delete reinterpret_cast<JsNativeReferenceDeleterObject*>(work->data);
        work->data = nullptr;
        delete work;
        work = nullptr;
    }
}

void JsRuntime::InitTimerModule()
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->InitTimerModule();
}

void JsRuntime::InitWorkerModule(const Options& options)
{
    CHECK_POINTER(jsEnv_);
    std::shared_ptr<JsEnv::WorkerInfo> workerInfo = std::make_shared<JsEnv::WorkerInfo>();
    workerInfo->codePath = options.codePath;
    workerInfo->isDebugVersion = options.isDebugVersion;
    workerInfo->isBundle = options.isBundle;
    workerInfo->packagePathStr = options.packagePathStr;
    workerInfo->assetBasePathStr = options.assetBasePathStr;
    workerInfo->hapPath = options.hapPath;
    workerInfo->isStageModel = options.isStageModel;
    jsEnv_->InitWorkerModule(workerInfo);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
