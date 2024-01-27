/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "native_runtime_impl.h"

#include "bundle_mgr_interface.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "js_environment.h"
#include "js_module_reader.h"
#include "js_worker.h"
#include "ohos_js_env_logger.h"
#include "ohos_js_environment_impl.h"
#include "parameters.h"
#include "system_ability_definition.h"

using Extractor = OHOS::AbilityBase::Extractor;
using ExtractorUtil = OHOS::AbilityBase::ExtractorUtil;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int64_t DEFAULT_GC_POOL_SIZE = 0x10000000; // 256MB
constexpr size_t MAX_ENV_COUNT = 16;
const std::string SANDBOX_ARK_PROIFILE_PATH = "/data/storage/ark-profile";
int32_t PrintVmLog(int32_t, int32_t, const char*, const char*, const char* message)
{
    HILOG_INFO("ArkLog: %{public}s", message);
    return 0;
}
}
NativeRuntimeImpl::NativeRuntimeImpl()
{}

NativeRuntimeImpl::~NativeRuntimeImpl()
{
    std::lock_guard<std::mutex> lock(envMutex_);
    for (auto it : envMap_) {
        it.second.reset();
        it.second = nullptr;
    }
    envMap_.clear();
    threadIds_.clear();
}

NativeRuntimeImpl& NativeRuntimeImpl::GetNativeRuntimeImpl()
{
    static NativeRuntimeImpl nativeRuntimeImpl;
    return nativeRuntimeImpl;
}

int32_t NativeRuntimeImpl::CreateJsEnv(const Options& options, std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    HILOG_DEBUG("called");
    panda::RuntimeOption pandaOption;
    int arkProperties = OHOS::system::GetIntParameter<int>("persist.ark.properties", -1);
    std::string bundleName = OHOS::system::GetParameter("persist.ark.arkbundlename", "");
    size_t gcThreadNum = OHOS::system::GetUintParameter<size_t>("persist.ark.gcthreads", 7);
    size_t longPauseTime = OHOS::system::GetUintParameter<size_t>("persist.ark.longpausetime", 40);
    pandaOption.SetArkProperties(arkProperties);
    pandaOption.SetArkBundleName(bundleName);
    pandaOption.SetGcThreadNum(gcThreadNum);
    pandaOption.SetLongPauseTime(longPauseTime);
    HILOG_INFO("NativeRuntimeImpl::Initialize ark properties = %{public}d bundlename = %{public}s",
        arkProperties, bundleName.c_str());
    pandaOption.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    pandaOption.SetGcPoolSize(DEFAULT_GC_POOL_SIZE);
    pandaOption.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::FOLLOW);
    pandaOption.SetLogBufPrint(PrintVmLog);

    bool asmInterpreterEnabled = OHOS::system::GetBoolParameter("persist.ark.asminterpreter", true);
    std::string asmOpcodeDisableRange = OHOS::system::GetParameter("persist.ark.asmopcodedisablerange", "");
    pandaOption.SetEnableAsmInterpreter(asmInterpreterEnabled);
    pandaOption.SetAsmOpcodeDisableRange(asmOpcodeDisableRange);

    bool useAbilityRuntime = (options.isStageModel) || (options.isTestFramework);
    if (useAbilityRuntime) {
        bool aotEnabled = OHOS::system::GetBoolParameter("persist.ark.aot", true);
        pandaOption.SetEnableAOT(aotEnabled);
        pandaOption.SetProfileDir(SANDBOX_ARK_PROIFILE_PATH);
    }

    OHOSJsEnvLogger::RegisterJsEnvLogger();
    // options eventRunner is nullptr
    jsEnv = std::make_shared<JsEnv::JsEnvironment>(std::make_unique<OHOSJsEnvironmentImpl>(options.eventRunner));
    if (jsEnv == nullptr || !jsEnv->Initialize(pandaOption, static_cast<void*>(this))) {
        HILOG_ERROR("Initialize js environment failed.");
        return NATIVE_RUNTIME_INNER_ERROR;
    }

    return AddEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()), jsEnv);
}

int32_t NativeRuntimeImpl::Init(const Options& options, napi_env env)
{
    auto jsEnv = GetJsEnv(env);
    if (jsEnv == nullptr) {
        HILOG_ERROR("jsEnv is nullptr.");
        return NATIVE_RUNTIME_INNER_ERROR;
    }

    auto vm = GetEcmaVm(jsEnv);
    if (!vm) {
        HILOG_ERROR("vm is nullptr.");
        return NATIVE_RUNTIME_INNER_ERROR;
    }

    bool isModular = false;
    if (!options.preload) {
        LoadAotFile(options, jsEnv);
        panda::JSNApi::SetBundle(vm, options.isBundle);
        panda::JSNApi::SetBundleName(vm, options.bundleName);
        panda::JSNApi::SetHostResolveBufferTracker(
            vm, JsModuleReader(options.bundleName, options.hapPath, options.isUnique));
        isModular = !panda::JSNApi::IsBundle(vm);
        panda::JSNApi::SetSearchHapPathTracker(
            vm, [options](const std::string moduleName, std::string &hapPath) -> bool {
                if (options.hapModulePath.find(moduleName) == options.hapModulePath.end()) {
                    return false;
                }
                hapPath = options.hapModulePath.find(moduleName)->second;
                return true;
            });
    }

    if (!preloaded_) {
        InitConsoleModule(jsEnv);
    }

    if (!options.preload) {
        auto operatorObj = std::make_shared<JsEnv::SourceMapOperator>(options.bundleName, isModular);
        InitSourceMap(operatorObj, jsEnv);
        if (!options.isUnique) {
            InitTimerModule(jsEnv);
        }
        InitWorkerModule(options, jsEnv);
        SetModuleLoadChecker(options.moduleCheckerDelegate, jsEnv);
        SetRequestAotCallback(jsEnv);

        if (!InitLoop(jsEnv)) {
            HILOG_ERROR("Initialize loop failed.");
            return NATIVE_RUNTIME_INNER_ERROR;
        }
    }

    preloaded_ = options.preload;
    return NATIVE_RUNTIME_ERR_OK;
}

int32_t NativeRuntimeImpl::AddEnv(napi_env env, std::shared_ptr<JsEnv::JsEnvironment> jsEnv)
{
    std::lock_guard<std::mutex> lock(envMutex_);
    pid_t threadId = gettid();
    if (threadIds_.find(threadId) != threadIds_.end()) {
        HILOG_ERROR("already created!");
        return NATIVE_RUNTIME_THREAD_ONLY_ONE_RUNENV;
    }
    if (envMap_.size() >= MAX_ENV_COUNT) {
        HILOG_ERROR("the maximum number of runtime environments that can be created is 16!");
        return NATIVE_RUNTIME_THREAD_COUNT_OVERLOAD;
    }
    threadIds_.insert(threadId);
    HILOG_DEBUG("add threadId %{public}zu", threadId);
    auto it = envMap_.find(env);
    if (it == envMap_.end()) {
        envMap_[env] = jsEnv;
        return NATIVE_RUNTIME_ERR_OK;
    }
    return NATIVE_RUNTIME_INNER_ERROR;
}

int32_t NativeRuntimeImpl::RemoveJsEnv(napi_env env)
{
    std::lock_guard<std::mutex> lock(envMutex_);
    pid_t threadId = gettid();
    HILOG_DEBUG("remove threadId %{public}zu", threadId);
    threadIds_.erase(threadId);
    auto it = envMap_.find(env);
    if (it != envMap_.end()) {
        it->second.reset();
        it->second = nullptr;
        envMap_.erase(env);
        return NATIVE_RUNTIME_ERR_OK;
    }
    return NATIVE_RUNTIME_DESTROY_FAILED;
}

panda::ecmascript::EcmaVM* NativeRuntimeImpl::GetEcmaVm(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv) const
{
    if (jsEnv == nullptr) {
        HILOG_ERROR("jsEnv is nullptr.");
        return nullptr;
    }
    return jsEnv->GetVM();
}

std::shared_ptr<JsEnv::JsEnvironment> NativeRuntimeImpl::GetJsEnv(napi_env env)
{
    std::lock_guard<std::mutex> lock(envMutex_);
    auto jsEnv = envMap_.find(env);
    if (jsEnv != envMap_.end()) {
        return jsEnv->second;
    }
    return nullptr;
}

void NativeRuntimeImpl::LoadAotFile(const Options& options, const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    auto vm = GetEcmaVm(jsEnv);
    if (!vm || options.hapPath.empty()) {
        return;
    }

    bool newCreate = false;
    std::string loadPath = ExtractorUtil::GetLoadFilePath(options.hapPath);
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate, true);
    if (extractor != nullptr && newCreate) {
        panda::JSNApi::LoadAotFile(vm, options.moduleName);
    }
}

void NativeRuntimeImpl::InitConsoleModule(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    if (jsEnv == nullptr) {
        HILOG_ERROR("jsEnv is nullptr.");
        return;
    }
    jsEnv->InitConsoleModule();
}

void NativeRuntimeImpl::InitSourceMap(const std::shared_ptr<JsEnv::SourceMapOperator> operatorObj,
    const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    if (jsEnv == nullptr) {
        HILOG_ERROR("jsEnv is nullptr.");
        return;
    }
    jsEnv->InitSourceMap(operatorObj);
    JsEnv::SourceMap::RegisterReadSourceMapCallback(JsRuntime::ReadSourceMapData);
    JsEnv::SourceMap::RegisterGetHapPathCallback(JsModuleReader::GetHapPathList);
}

void NativeRuntimeImpl::InitTimerModule(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    if (jsEnv == nullptr) {
        HILOG_ERROR("jsEnv is nullptr.");
        return;
    }
    jsEnv->InitTimerModule();
}

void NativeRuntimeImpl::SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate>& moduleCheckerDelegate,
    const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    if (jsEnv == nullptr) {
        HILOG_ERROR("jsEnv is nullptr.");
        return;
    }
    jsEnv->SetModuleLoadChecker(moduleCheckerDelegate);
}

void NativeRuntimeImpl::SetRequestAotCallback(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    if (jsEnv == nullptr) {
        HILOG_ERROR("jsEnv is nullptr.");
        return;
    }
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

    jsEnv->SetRequestAotCallback(callback);
}

bool NativeRuntimeImpl::InitLoop(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    if (jsEnv == nullptr) {
        HILOG_ERROR("jsEnv is nullptr.");
        return false;
    }
    return jsEnv->InitLoop();
}

void NativeRuntimeImpl::InitWorkerModule(const Options& options, const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    if (jsEnv == nullptr) {
        HILOG_ERROR("jsEnv is nullptr.");
        return;
    }

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
    jsEnv->InitWorkerModule(workerInfo);
}
}
}