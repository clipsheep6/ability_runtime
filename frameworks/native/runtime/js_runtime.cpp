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
#include <fstream>
#include <regex>

#include <atomic>
#include <sys/epoll.h>
#include <unistd.h>

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
#include "iservice_registry.h"
#include "js_environment.h"
#include "js_module_reader.h"
#include "js_module_searcher.h"
#include "js_quickfix_callback.h"
#include "js_runtime_utils.h"
#include "js_utils.h"
#include "js_worker.h"
#include "module_checker_delegate.h"
#include "napi/native_api.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "native_engine/native_engine.h"
#include "ohos_js_env_logger.h"
#include "ohos_js_environment_impl.h"
#include "parameters.h"
#include "extractor.h"
#include "system_ability_definition.h"
#include "systemcapability.h"
#include "source_map.h"
#include "source_map_operator.h"

#ifdef SUPPORT_GRAPHICS
#include "ace_forward_compatibility.h"
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
constexpr int32_t API8 = 8;
const std::string SANDBOX_ARK_CACHE_PATH = "/data/storage/ark-cache/";
const std::string SANDBOX_ARK_PROIFILE_PATH = "/data/storage/ark-profile";
const std::string DEBUGGER = "@Debugger";
#ifdef APP_USE_ARM
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib/libark_debugger.z.so";
#elif defined(APP_USE_X86_64)
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib64/libark_debugger.z.so";
#else
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib64/libark_debugger.z.so";
#endif

constexpr char MERGE_ABC_PATH[] = "/ets/modules.abc";
constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";
constexpr const char* PERMISSION_RUN_ANY_CODE = "ohos.permission.RUN_ANY_CODE";

const std::string SYSTEM_KITS_CONFIG_PATH = "/system/etc/system_kits_config.json";

const std::string SYSTEM_KITS = "systemkits";
const std::string NAMESPACE = "namespace";
const std::string TARGET_OHM = "targetohm";
const std::string SINCE_VERSION = "sinceVersion";

static auto PermissionCheckFunc = []() {
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();

    int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, PERMISSION_RUN_ANY_CODE);
    if (result == Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        return true;
    } else {
        return false;
    }
};

napi_value CanIUse(napi_env env, napi_callback_info info)
{
    if (env == nullptr || info == nullptr) {
        HILOG_ERROR("get syscap failed since env or callback info is nullptr.");
        return nullptr;
    }
    napi_value undefined = CreateJsUndefined(env);

    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        HILOG_ERROR("Get syscap failed with invalid parameter.");
        return undefined;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_string) {
        HILOG_INFO("%{public}s called. Params is invalid.", __func__);
        return undefined;
    }

    char syscap[SYSCAP_MAX_SIZE] = { 0 };

    size_t strLen = 0;
    napi_get_value_string_utf8(env, argv[0], syscap, sizeof(syscap), &strLen);

    bool ret = HasSystemCapability(syscap);
    return CreateJsValue(env, ret);
}

void InitSyscapModule(napi_env env, napi_value globalObject)
{
    const char *moduleName = "JsRuntime";
    BindNativeFunction(env, globalObject, "canIUse", moduleName, CanIUse);
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
    HILOG_INFO("JsRuntime destructor.");
    Deinitialize();
    StopDebugMode();
}

std::unique_ptr<JsRuntime> JsRuntime::Create(const Options& options)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    std::unique_ptr<JsRuntime> instance;

    if (!options.preload && options.isStageModel) {
        auto preloadedInstance = Runtime::GetPreloaded();

#ifdef SUPPORT_GRAPHICS
        // reload ace if compatible mode changes
        if (Ace::AceForwardCompatibility::PipelineChanged() && preloadedInstance) {
            preloadedInstance.reset();
        }
#endif
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

void JsRuntime::StartDebugMode(bool needBreakPoint, const std::string &processName, bool isDebugApp)
{
    CHECK_POINTER(jsEnv_);
    if (jsEnv_->GetDebugMode()) {
        HILOG_INFO("Already in debug mode");
        return;
    }
    // Set instance id to tid after the first instance.
    if (JsRuntime::hasInstance.exchange(true, std::memory_order_relaxed)) {
        instanceId_ = static_cast<uint32_t>(gettid());
    }

    HILOG_INFO("Ark VM is starting debug mode [%{public}s]", needBreakPoint ? "break" : "normal");
    StartDebuggerInWorkerModule();
    SetDebuggerApp(isDebugApp);
    const std::string bundleName = bundleName_;
    uint32_t instanceId = instanceId_;
    auto weak = jsEnv_;
    std::string inputProcessName = "";
    if (bundleName_ != processName) {
        inputProcessName = processName;
    }
    HdcRegister::Get().StartHdcRegister(bundleName_, inputProcessName, isDebugApp,
        [bundleName, needBreakPoint, instanceId, weak](int socketFd, std::string option) {
        HILOG_INFO("HdcRegister callback is call, socket fd is %{public}d, option is %{public}s.",
            socketFd, option.c_str());
        if (weak == nullptr) {
            HILOG_ERROR("jsEnv is nullptr in hdc register callback");
            return;
        }
        if (option.find(DEBUGGER) == std::string::npos) {
            ConnectServerManager::Get().StopConnectServer(false);
            ConnectServerManager::Get().StartConnectServer(bundleName, socketFd, false);
        } else {
            weak->StopDebugger(option);
            weak->StartDebugger(option, ARK_DEBUGGER_LIB_PATH, socketFd, needBreakPoint, instanceId);
        }
    });
    ConnectServerManager::Get().StartConnectServer(bundleName_, -1, true);
    ConnectServerManager::Get().AddInstance(instanceId_);
    jsEnv_->NotifyDebugMode(gettid(), ARK_DEBUGGER_LIB_PATH, instanceId_, isDebugApp, needBreakPoint);
}

void JsRuntime::StopDebugMode()
{
    CHECK_POINTER(jsEnv_);
    if (jsEnv_->GetDebugMode()) {
        ConnectServerManager::Get().RemoveInstance(instanceId_);
        StopDebugger();
    }
}

void JsRuntime::InitConsoleModule()
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->InitConsoleModule();
}

bool JsRuntime::StartDebugger(bool needBreakPoint, uint32_t instanceId)
{
    HILOG_DEBUG("StartDebugger called.");
    return true;
}

void JsRuntime::StopDebugger()
{
    CHECK_POINTER(jsEnv_);
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

void JsRuntime::StartProfiler(
    const std::string &perfCmd, bool needBreakPoint, const std::string &processName, bool isDebugApp)
{
    CHECK_POINTER(jsEnv_);
    if (JsRuntime::hasInstance.exchange(true, std::memory_order_relaxed)) {
        instanceId_ = static_cast<uint32_t>(gettid());
    }

    StartDebuggerInWorkerModule();
    SetDebuggerApp(isDebugApp);
    const std::string bundleName = bundleName_;
    auto weak = jsEnv_;
    uint32_t instanceId = instanceId_;
    std::string inputProcessName = "";
    if (bundleName_ != processName) {
        inputProcessName = processName;
    }
    HdcRegister::Get().StartHdcRegister(bundleName_, inputProcessName, isDebugApp,
        [bundleName, needBreakPoint, instanceId, weak](int socketFd, std::string option) {
        HILOG_INFO("HdcRegister callback is call, socket fd is %{public}d, option is %{public}s.",
            socketFd, option.c_str());
        if (weak == nullptr) {
            HILOG_ERROR("jsEnv is nullptr in hdc register callback");
            return;
        }
        if (option.find(DEBUGGER) == std::string::npos) {
            ConnectServerManager::Get().StopConnectServer(false);
            ConnectServerManager::Get().StartConnectServer(bundleName, socketFd, false);
        } else {
            weak->StopDebugger(option);
            weak->StartDebugger(option, ARK_DEBUGGER_LIB_PATH, socketFd, needBreakPoint, instanceId);
        }
    });
    ConnectServerManager::Get().StartConnectServer(bundleName_, 0, true);
    ConnectServerManager::Get().AddInstance(instanceId_);

    JsEnv::JsEnvironment::PROFILERTYPE profiler = JsEnv::JsEnvironment::PROFILERTYPE::PROFILERTYPE_HEAP;
    int32_t interval = 0;
    const std::string profilerCommand("profile");
    if (perfCmd.find(profilerCommand) != std::string::npos) {
        profiler = JsEnv::JsEnvironment::PROFILERTYPE::PROFILERTYPE_CPU;
        interval = JsperfProfilerCommandParse(perfCmd, DEFAULT_INTER_VAL);
    }

    HILOG_DEBUG("profiler:%{public}d interval:%{public}d.", profiler, interval);
    jsEnv_->StartProfiler(ARK_DEBUGGER_LIB_PATH, instanceId_, profiler, interval, gettid());
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

bool JsRuntime::LoadScript(const std::string& path, uint8_t* buffer, size_t len, bool isBundle)
{
    HILOG_DEBUG("function called.");
    CHECK_POINTER_AND_RETURN(jsEnv_, false);
    return jsEnv_->LoadScript(path, buffer, len, isBundle);
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModuleByEngine(
    napi_env env, const std::string& moduleName, const napi_value* argv, size_t argc)
{
    HILOG_DEBUG("JsRuntime::LoadSystemModule(%{public}s)", moduleName.c_str());
    if (env == nullptr) {
        HILOG_INFO("JsRuntime::LoadSystemModule: invalid engine.");
        return std::unique_ptr<NativeReference>();
    }

    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    napi_value propertyValue = nullptr;
    napi_get_named_property(env, globalObj, "requireNapi", &propertyValue);

    std::unique_ptr<NativeReference> methodRequireNapiRef_;
    napi_ref tmpRef = nullptr;
    napi_create_reference(env, propertyValue, 1, &tmpRef);
    methodRequireNapiRef_.reset(reinterpret_cast<NativeReference*>(tmpRef));
    if (!methodRequireNapiRef_) {
        HILOG_ERROR("Failed to create reference for global.requireNapi");
        return nullptr;
    }

    napi_value className = nullptr;
    napi_create_string_utf8(env, moduleName.c_str(), moduleName.length(), &className);
    auto refValue = methodRequireNapiRef_->GetNapiValue();
    napi_value args[1] = { className };
    napi_value classValue = nullptr;
    napi_call_function(env, globalObj, refValue, 1, args, &classValue);
    napi_value instanceValue = nullptr;
    napi_new_instance(env, classValue, argc, argv, &instanceValue);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    napi_ref resultRef = nullptr;
    napi_create_reference(env, instanceValue, 1, &resultRef);
    return std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(resultRef));
}

void JsRuntime::FinishPreload()
{
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    panda::JSNApi::PreFork(vm);
}

void JsRuntime::PostPreload(const Options& options)
{
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    auto env = GetNapiEnv();
    CHECK_POINTER(env);
    panda::RuntimeOption postOption;
    postOption.SetBundleName(options.bundleName);
    if (!options.arkNativeFilePath.empty()) {
        std::string sandBoxAnFilePath = SANDBOX_ARK_CACHE_PATH + options.arkNativeFilePath;
        postOption.SetAnDir(sandBoxAnFilePath);
    }
    bool profileEnabled = OHOS::system::GetBoolParameter("ark.profile", false);
    postOption.SetEnableProfile(profileEnabled);
    panda::JSNApi::PostFork(vm, postOption);
    reinterpret_cast<NativeEngine*>(env)->ReinitUVLoop();
    uv_loop_s* loop = nullptr;
    napi_get_uv_event_loop(env, &loop);
    panda::JSNApi::SetLoop(vm, loop);
}

void JsRuntime::LoadAotFile(const Options& options)
{
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    if (options.hapPath.empty()) {
        return;
    }

    bool newCreate = false;
    std::string loadPath = ExtractorUtil::GetLoadFilePath(options.hapPath);
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate, true);
    if (extractor != nullptr && newCreate) {
        panda::JSNApi::LoadAotFile(vm, options.moduleName);
    }
}

bool JsRuntime::Initialize(const Options& options)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
#ifdef SUPPORT_GRAPHICS
    if (Ace::AceForwardCompatibility::PipelineChanged()) {
        preloaded_ = false;
    }
#endif
    if (!preloaded_) {
        if (!CreateJsEnv(options)) {
            HILOG_ERROR("Create js environment failed.");
            return false;
        }
    }
    apiTargetVersion_ = options.apiTargetVersion;
    HILOG_INFO("Initialize: %{public}d.", apiTargetVersion_);
    bool isModular = false;
    if (IsUseAbilityRuntime(options)) {
        auto env = GetNapiEnv();
        auto nativeEngine = reinterpret_cast<NativeEngine*>(env);
        CHECK_POINTER_AND_RETURN(nativeEngine, false);

        auto vm = GetEcmaVm();
        CHECK_POINTER_AND_RETURN(vm, false);

        if (preloaded_) {
            PostPreload(options);
        }
        HandleScope handleScope(*this);
        napi_value globalObj = nullptr;
        napi_get_global(env, &globalObj);
        CHECK_POINTER_AND_RETURN(globalObj, false);

        if (!preloaded_) {
            InitSyscapModule(env, globalObj);

            // Simple hook function 'isSystemplugin'
            const char* moduleName = "JsRuntime";
            BindNativeFunction(env, globalObj, "isSystemplugin", moduleName,
                [](napi_env env, napi_callback_info cbinfo) -> napi_value {
                    return CreateJsUndefined(env);
                });

            napi_value propertyValue = nullptr;
            napi_get_named_property(env, globalObj, "requireNapi", &propertyValue);
            napi_ref tmpRef = nullptr;
            napi_create_reference(env, propertyValue, 1, &tmpRef);
            methodRequireNapiRef_.reset(reinterpret_cast<NativeReference*>(tmpRef));
            if (!methodRequireNapiRef_) {
                HILOG_ERROR("Failed to create reference for global.requireNapi");
                return false;
            }
            HILOG_INFO("PreloadAce start.");
            PreloadAce(options);
            HILOG_INFO("PreloadAce end.");
            nativeEngine->RegisterPermissionCheck(PermissionCheckFunc);
        }

        if (!options.preload) {
            isBundle_ = options.isBundle;
            bundleName_ = options.bundleName;
            codePath_ = options.codePath;
            ReInitJsEnvImpl(options);
            LoadAotFile(options);

            panda::JSNApi::SetBundle(vm, options.isBundle);
            panda::JSNApi::SetBundleName(vm, options.bundleName);
            panda::JSNApi::SetHostResolveBufferTracker(
                vm, JsModuleReader(options.bundleName, options.hapPath, options.isUnique));
            isModular = !panda::JSNApi::IsBundle(vm);
            std::vector<panda::HmsMap> systemKitsMap = GetSystemKitsMap(apiTargetVersion_);
            panda::JSNApi::SetHmsModuleList(vm, systemKitsMap);
        }
    }

    if (!preloaded_) {
        InitConsoleModule();
    }

    if (!options.preload) {
        auto operatorObj = std::make_shared<JsEnv::SourceMapOperator>(options.bundleName, isModular);
        InitSourceMap(operatorObj);

        if (options.isUnique) {
            HILOG_INFO("Not supported TimerModule when form render");
        } else {
            InitTimerModule();
        }

        InitWorkerModule(options);
        SetModuleLoadChecker(options.moduleCheckerDelegate);
        SetRequestAotCallback();

        if (!InitLoop()) {
            HILOG_ERROR("Initialize loop failed.");
            return false;
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
    jsEnv_ = std::make_shared<JsEnv::JsEnvironment>(std::make_unique<OHOSJsEnvironmentImpl>(options.eventRunner));
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

bool JsRuntime::InitLoop()
{
    CHECK_POINTER_AND_RETURN(jsEnv_, false);
    return jsEnv_->InitLoop();
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
}

void JsRuntime::InitSourceMap(const std::shared_ptr<JsEnv::SourceMapOperator> operatorObj)
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->InitSourceMap(operatorObj);
    JsEnv::SourceMap::RegisterReadSourceMapCallback(JsRuntime::ReadSourceMapData);
    JsEnv::SourceMap::RegisterGetHapPathCallback(JsModuleReader::GetHapPathList);
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

napi_value JsRuntime::LoadJsBundle(const std::string& path, const std::string& hapPath, bool useCommonChunk)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto env = GetNapiEnv();
    CHECK_POINTER_AND_RETURN(env, nullptr);
    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    napi_value exports = nullptr;
    napi_create_object(env, &exports);
    napi_set_named_property(env, globalObj, "exports", exports);

    if (!RunScript(path, hapPath, useCommonChunk)) {
        HILOG_ERROR("Failed to run script: %{private}s", path.c_str());
        return nullptr;
    }

    napi_value exportsObj = nullptr;
    napi_get_named_property(env, globalObj, "exports", &exportsObj);
    if (exportsObj == nullptr) {
        HILOG_ERROR("Failed to get exports objcect: %{private}s", path.c_str());
        return nullptr;
    }

    napi_value exportObj = nullptr;
    napi_get_named_property(env, exportsObj, "default", &exportObj);
    if (exportObj == nullptr) {
        HILOG_ERROR("Failed to get default objcect: %{private}s", path.c_str());
        return nullptr;
    }

    return exportObj;
}

napi_value JsRuntime::LoadJsModule(const std::string& path, const std::string& hapPath)
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

    auto env = GetNapiEnv();
    CHECK_POINTER_AND_RETURN(env, nullptr);
    return ArkNativeEngine::ArkValueToNapiValue(env, exportObj);
}

std::unique_ptr<NativeReference> JsRuntime::LoadModule(const std::string& moduleName, const std::string& modulePath,
    const std::string& hapPath, bool esmodule, bool useCommonChunk)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Load module(%{public}s, %{private}s, %{private}s, %{public}s)",
        moduleName.c_str(), modulePath.c_str(), hapPath.c_str(), esmodule ? "true" : "false");
    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, std::unique_ptr<NativeReference>());
    // use for debugger, js engine need to know load module to handle debug event
    panda::JSNApi::NotifyLoadModule(vm);
    auto env = GetNapiEnv();
    CHECK_POINTER_AND_RETURN(env, std::unique_ptr<NativeReference>());

    HandleScope handleScope(*this);

    std::string path = moduleName;
    auto pos = path.find("::");
    if (pos != std::string::npos) {
        path.erase(pos, path.size() - pos);
        moduleName_ = path;
    }

    napi_value classValue = nullptr;

    auto it = modules_.find(modulePath);
    if (it != modules_.end()) {
        classValue = it->second->GetNapiValue();
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

        napi_ref tmpRef = nullptr;
        napi_create_reference(env, classValue, 1, &tmpRef);
        modules_.emplace(modulePath, reinterpret_cast<NativeReference*>(tmpRef));
    }

    napi_value instanceValue = nullptr;
    napi_new_instance(env, classValue, 0, nullptr, &instanceValue);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    napi_ref resultRef = nullptr;
    napi_create_reference(env, instanceValue, 1, &resultRef);
    return std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(resultRef));
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModule(
    const std::string& moduleName, const napi_value* argv, size_t argc)
{
    HILOG_INFO("JsRuntime::LoadSystemModule(%{public}s)", moduleName.c_str());
    napi_env env = GetNapiEnv();
    CHECK_POINTER_AND_RETURN(env, std::unique_ptr<NativeReference>());

    HandleScope handleScope(*this);

    napi_value className = nullptr;
    napi_create_string_utf8(env, moduleName.c_str(), moduleName.length(), &className);
    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    napi_value refValue = methodRequireNapiRef_->GetNapiValue();
    napi_value args[1] = { className };
    napi_value classValue = nullptr;
    napi_call_function(env, globalObj, refValue, 1, args, &classValue);
    napi_value instanceValue = nullptr;
    napi_new_instance(env, classValue, argc, argv, &instanceValue);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    napi_ref resultRef = nullptr;
    napi_create_reference(env, instanceValue, 1, &resultRef);
    return std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(resultRef));
}

bool JsRuntime::RunScript(const std::string& srcPath, const std::string& hapPath, bool useCommonChunk)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
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
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate, true);
    if (!extractor) {
        HILOG_ERROR("Get extractor failed. hapPath[%{private}s]", hapPath.c_str());
        return false;
    }
    if (newCreate) {
        panda::JSNApi::LoadAotFile(vm, moduleName_);
        auto resourceManager = AbilityBase::ExtractResourceManager::GetExtractResourceManager().GetGlobalObject();
        if (resourceManager) {
            resourceManager->AddResource(loadPath.c_str());
        }
    }

    auto func = [&](std::string modulePath, const std::string abcPath) {
        bool useSafeMempry = apiTargetVersion_ == 0 || apiTargetVersion_ > API8;
        if (!extractor->IsHapCompress(modulePath) && useSafeMempry) {
            auto safeData = extractor->GetSafeData(modulePath);
            if (!safeData) {
                HILOG_ERROR("Get abc file failed.");
                return false;
            }
            return LoadScript(abcPath, safeData->GetDataPtr(), safeData->GetDataLen(), isBundle_);
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

void JsRuntime::PostSyncTask(const std::function<void()>& task, const std::string& name)
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->PostSyncTask(task, name);
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

void JsRuntime::DestroyHeapProfiler()
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->DestroyHeapProfiler();
}

void JsRuntime::ForceFullGC()
{
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    panda::JSNApi::TriggerGC(vm, panda::JSNApi::TRIGGER_GC_TYPE::FULL_GC);
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

bool JsRuntime::SuspendVM(uint32_t tid)
{
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER_AND_RETURN(nativeEngine, false);
    return nativeEngine->SuspendVMById(tid);
}

void JsRuntime::ResumeVM(uint32_t tid)
{
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
    nativeEngine->ResumeVMById(tid);
}

void JsRuntime::PreloadSystemModule(const std::string& moduleName)
{
    HandleScope handleScope(*this);
    auto env = GetNapiEnv();
    CHECK_POINTER(env);
    napi_value className = nullptr;
    napi_create_string_utf8(env, moduleName.c_str(), moduleName.length(), &className);
    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    napi_value refValue = methodRequireNapiRef_->GetNapiValue();
    napi_value args[1] = { className };
    napi_call_function(env, globalObj, refValue, 1, args, nullptr);
}

NativeEngine& JsRuntime::GetNativeEngine() const
{
    return *GetNativeEnginePointer();
}

napi_env JsRuntime::GetNapiEnv() const
{
    return reinterpret_cast<napi_env>(GetNativeEnginePointer());
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

void JsRuntime::RegisterUncaughtExceptionHandler(const JsEnv::UncaughtExceptionInfo& uncaughtExceptionInfo)
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->RegisterUncaughtExceptionHandler(uncaughtExceptionInfo);
}

void JsRuntime::RegisterQuickFixQueryFunc(const std::map<std::string, std::string>& moduleAndPath)
{
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    panda::JSNApi::RegisterQuickFixQueryFunc(vm, JsQuickfixCallback(moduleAndPath));
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
    content.assign(dataPtr.get(), dataPtr.get() + len);
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
    workerInfo->moduleName = options.moduleName;
    if (options.isJsFramework) {
        SetJsFramework();
    }
    jsEnv_->InitWorkerModule(workerInfo);
}

void JsRuntime::ReInitJsEnvImpl(const Options& options)
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->ReInitJsEnvImpl(std::make_unique<OHOSJsEnvironmentImpl>(options.eventRunner));
}

void JsRuntime::SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate>& moduleCheckerDelegate) const
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->SetModuleLoadChecker(moduleCheckerDelegate);
}

void JsRuntime::SetRequestAotCallback()
{
    CHECK_POINTER(jsEnv_);
    auto callback = [](const std::string& bundleName, const std::string& moduleName, int32_t triggerMode) -> int32_t {
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityMgr == nullptr) {
            HILOG_ERROR("Failed to get system ability manager.");
            return ERR_INVALID_VALUE;
        }

        auto remoteObj = systemAbilityMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (remoteObj == nullptr) {
            HILOG_ERROR("Remote object is nullptr.");
            return ERR_INVALID_VALUE;
        }

        auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObj);
        if (bundleMgr == nullptr) {
            HILOG_ERROR("Failed to get bundle manager.");
            return ERR_INVALID_VALUE;
        }

        HILOG_DEBUG("Reset compile status, bundleName: %{public}s, moduleName: %{public}s, triggerMode: %{public}d.",
            bundleName.c_str(), moduleName.c_str(), triggerMode);
        return bundleMgr->ResetAOTCompileStatus(bundleName, moduleName, triggerMode);
    };

    jsEnv_->SetRequestAotCallback(callback);
}

void JsRuntime::SetDeviceDisconnectCallback(const std::function<bool()> &cb)
{
    HILOG_DEBUG("Start.");
    CHECK_POINTER(jsEnv_);
    jsEnv_->SetDeviceDisconnectCallback(cb);
}

std::vector<panda::HmsMap> JsRuntime::GetSystemKitsMap(uint32_t version)
{
    std::vector<panda::HmsMap> systemKitsMap;
    nlohmann::json jsonBuf;
    if (access(SYSTEM_KITS_CONFIG_PATH.c_str(), F_OK) != 0) {
        return systemKitsMap;
    }

    std::fstream in;
    char errBuf[256];
    errBuf[0] = '\0';
    in.open(SYSTEM_KITS_CONFIG_PATH, std::ios_base::in);
    if (!in.is_open()) {
        strerror_r(errno, errBuf, sizeof(errBuf));
        return systemKitsMap;
    }

    in.seekg(0, std::ios::end);
    int64_t size = in.tellg();
    if (size <= 0) {
        HILOG_ERROR("the file is an empty file");
        in.close();
        return systemKitsMap;
    }

    in.seekg(0, std::ios::beg);
    jsonBuf = nlohmann::json::parse(in, nullptr, false);
    in.close();
    if (jsonBuf.is_discarded()) {
        HILOG_ERROR("bad profile file");
        return systemKitsMap;
    }

    if (!jsonBuf.contains(SYSTEM_KITS)) {
        HILOG_ERROR("json config doesn't contain systemkits.");
        return systemKitsMap;
    }
    for (auto &item : jsonBuf.at(SYSTEM_KITS).items()) {
        nlohmann::json& jsonObject = item.value();
        if (!jsonObject.contains(NAMESPACE) || !jsonObject.at(NAMESPACE).is_string() ||
            !jsonObject.contains(TARGET_OHM) || !jsonObject.at(TARGET_OHM).is_string() ||
            !jsonObject.contains(SINCE_VERSION) || !jsonObject.at(SINCE_VERSION).is_number()) {
            continue;
        }
        uint32_t sinceVersion = jsonObject.at(SINCE_VERSION).get<uint32_t>();
        if (version >= sinceVersion) {
            panda::HmsMap hmsMap = {
                .originalPath = jsonObject.at(NAMESPACE).get<std::string>(),
                .targetPath = jsonObject.at(TARGET_OHM).get<std::string>(),
                .sinceVersion = sinceVersion
            };
            systemKitsMap.emplace_back(hmsMap);
        }
    }
    HILOG_DEBUG("The size of the map is %{public}zu", systemKitsMap.size());
    return systemKitsMap;
}
} // namespace AbilityRuntime
} // namespace OHOS
