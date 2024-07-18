/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "child_process_manager.h"

#include <csignal>
#include <filesystem>
#include <string>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "app_utils.h"
#include "application_info.h"
#include "app_mgr_interface.h"
#include "bundle_info.h"
#include "bundle_mgr_interface.h"
#include "child_process.h"
#include "native_child_ipc_process.h"
#include "child_process_manager_error_utils.h"
#include "child_process_request.h"
#include "child_process_start_info.h"
#include "constants.h"
#include "event_runner.h"
#include "errors.h"
#include "hap_module_info.h"
#include "hilog_tag_wrapper.h"
#include "parameters.h"
#include "runtime.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
    bool g_jitEnabled = false;
    AbilityRuntime::Runtime::DebugOption g_debugOption;
}
bool ChildProcessManager::signalRegistered_ = false;

ChildProcessManager &ChildProcessManager::GetInstance()
{
    static ChildProcessManager instance;
    return instance;
}

ChildProcessManager::ChildProcessManager()
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "ChildProcessManager constructor called");
}

ChildProcessManager::~ChildProcessManager()
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "ChildProcessManager deconstructor called");
}

ChildProcessManagerErrorCode ChildProcessManager::StartChildProcessBySelfFork(const std::string &srcEntry, pid_t &pid)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "called.");
    ChildProcessManagerErrorCode errorCode = PreCheck();
    if (errorCode != ChildProcessManagerErrorCode::ERR_OK) {
        return errorCode;
    }

    AppExecFwk::BundleInfo bundleInfo;
    if (!GetBundleInfo(bundleInfo)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "GetBundleInfo failed.");
        return ChildProcessManagerErrorCode::ERR_GET_BUNDLE_INFO_FAILED;
    }

    RegisterSignal();
    pid = fork();
    if (pid < 0) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Fork process failed");
        return ChildProcessManagerErrorCode::ERR_FORK_FAILED;
    }
    MakeProcessName(srcEntry); // set process name
    if (pid == 0) {
        const char *processName = g_debugOption.processName.c_str();
        if (prctl(PR_SET_NAME, processName) < 0) {
            TAG_LOGW(AAFwkTag::PROCESSMGR, "Set process name failed with %{public}d", errno);
        }
        HandleChildProcessBySelfFork(srcEntry, bundleInfo);
    }
    return ChildProcessManagerErrorCode::ERR_OK;
}

ChildProcessManagerErrorCode ChildProcessManager::StartChildProcessByAppSpawnFork(
    const std::string &srcEntry, pid_t &pid)
{
    AppExecFwk::ChildProcessArgs args;
    AppExecFwk::ChildProcessOptions options;
    return StartArkChildProcess(srcEntry, pid, AppExecFwk::CHILD_PROCESS_TYPE_JS, args, options);
}

ChildProcessManagerErrorCode ChildProcessManager::StartArkChildProcess(
    const std::string &srcEntry, pid_t &pid, int32_t childProcessType, const AppExecFwk::ChildProcessArgs &args,
    const AppExecFwk::ChildProcessOptions &options)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "StartChildProcessByAppSpawnFork, startWitDebug: %{public}d, processName:"
        "%{public}s, native:%{public}d, entryParams:%{private}s, fdsSize:%{public}zu, options.isolationMode:%{public}d",
        g_debugOption.isStartWithDebug, g_debugOption.processName.c_str(), g_debugOption.isStartWithNative,
        args.entryParams.c_str(), args.fds.size(), options.isolationMode);
    ChildProcessManagerErrorCode errorCode = PreCheck(childProcessType != AppExecFwk::CHILD_PROCESS_TYPE_JS);
    if (errorCode != ChildProcessManagerErrorCode::ERR_OK) {
        return errorCode;
    }
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "GetAppMgr failed.");
        return ChildProcessManagerErrorCode::ERR_GET_APP_MGR_FAILED;
    }
    AppExecFwk::ChildProcessRequest request;
    request.srcEntry = srcEntry;
    request.childProcessType = childProcessType;
    request.childProcessCount = childProcessCount_;
    request.isStartWithDebug = g_debugOption.isStartWithDebug;
    request.args = args;
    request.options = options;
    auto ret = appMgr->StartChildProcess(pid, request);
    childProcessCount_++;
    TAG_LOGD(AAFwkTag::PROCESSMGR, "AppMgr StartChildProcess ret:%{public}d", ret);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "AppMgr StartChildProcess failed, ret:%{public}d", ret);
        return ChildProcessManagerErrorCode::ERR_GET_APP_MGR_START_PROCESS_FAILED;
    }
    return ChildProcessManagerErrorCode::ERR_OK;
}

ChildProcessManagerErrorCode ChildProcessManager::StartNativeChildProcessByAppSpawnFork(
    const std::string &libName, const sptr<IRemoteObject> &callbackStub)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "called, libName:%{private}s", libName.c_str());
    ChildProcessManagerErrorCode errorCode = PreCheckNativeProcess();
    if (errorCode != ChildProcessManagerErrorCode::ERR_OK) {
        return errorCode;
    }

    sptr<AppExecFwk::IAppMgr> appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "GetAppMgr for native child process failed.");
        return ChildProcessManagerErrorCode::ERR_GET_APP_MGR_FAILED;
    }

    auto ret = appMgr->StartNativeChildProcess(libName, childProcessCount_, callbackStub);
    TAG_LOGD(AAFwkTag::PROCESSMGR, "AppMgr StartNativeChildProcess ret:%{public}d", ret);

    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "AppMgr StartNativeChildProcess failed, ret:%{public}d", ret);
        if (ret == ERR_OVERFLOW) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Max native child processes readched");
            return ChildProcessManagerErrorCode::ERR_MAX_NATIVE_CHILD_PROCESSES;
        }
        return ChildProcessManagerErrorCode::ERR_GET_APP_MGR_START_PROCESS_FAILED;
    }

    ++childProcessCount_;
    return ChildProcessManagerErrorCode::ERR_OK;
}

void ChildProcessManager::RegisterSignal()
{
    if (!signalRegistered_) {
        signalRegistered_ = true;
        TAG_LOGD(AAFwkTag::PROCESSMGR, "Register signal");
        signal(SIGCHLD, ChildProcessManager::HandleSigChild);
    }
}

void ChildProcessManager::HandleSigChild(int32_t signo)
{
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        continue;
    }
}

ChildProcessManagerErrorCode ChildProcessManager::PreCheck(bool useNewErrorCode)
{
    if (!AAFwk::AppUtils::GetInstance().IsMultiProcessModel()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Multi process model is not enabled");
        return useNewErrorCode ? ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED_NEW :
            ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED;
    }
    if (IsChildProcess()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
        return ChildProcessManagerErrorCode::ERR_ALREADY_IN_CHILD_PROCESS;
    }
    return ChildProcessManagerErrorCode::ERR_OK;
}

ChildProcessManagerErrorCode ChildProcessManager::PreCheckNativeProcess()
{
    ChildProcessManagerErrorCode errCode = PreCheck();
    if (errCode != ChildProcessManagerErrorCode::ERR_OK) {
        return errCode;
    }

    if (!AAFwk::AppUtils::GetInstance().IsSupportNativeChildProcess()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Unsupport native child process");
        return ChildProcessManagerErrorCode::ERR_UNSUPPORT_NATIVE_CHILD_PROCESS;
    }

    return ChildProcessManagerErrorCode::ERR_OK;
}

bool ChildProcessManager::IsChildProcess()
{
    return isChildProcessBySelfFork_ || HasChildProcessRecord();
}

void ChildProcessManager::HandleChildProcessBySelfFork(const std::string &srcEntry,
    const AppExecFwk::BundleInfo &bundleInfo)
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "HandleChildProcessBySelfFork start.");
    isChildProcessBySelfFork_ = true;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (eventRunner == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Get main eventRunner failed.");
        return;
    }
    eventRunner->Stop();
    
    AppExecFwk::HapModuleInfo hapModuleInfo;
    if (!GetEntryHapModuleInfo(bundleInfo, hapModuleInfo)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "GetHapModuleInfo failed.");
        return;
    }

    auto runtime = CreateRuntime(bundleInfo, hapModuleInfo, false, g_jitEnabled);
    if (!runtime) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Failed to create child process runtime");
        return;
    }
    TAG_LOGD(AAFwkTag::PROCESSMGR, "StartDebugMode, isStartWithDebug is %{public}d, processName is %{public}s, "
        "isDebugApp is %{public}d, isStartWithNative is %{public}d.", g_debugOption.isStartWithDebug,
        g_debugOption.processName.c_str(), g_debugOption.isDebugApp, g_debugOption.isStartWithNative);
    runtime->StartDebugMode(g_debugOption);
    std::string srcPath;
    srcPath.append(hapModuleInfo.moduleName).append("/").append(srcEntry);
    LoadJsFile(srcPath, hapModuleInfo, runtime);
    TAG_LOGD(AAFwkTag::PROCESSMGR, "HandleChildProcessBySelfFork end.");
    exit(0);
}

bool ChildProcessManager::LoadJsFile(const std::string &srcEntry, const AppExecFwk::HapModuleInfo &hapModuleInfo,
    std::unique_ptr<AbilityRuntime::Runtime> &runtime, std::shared_ptr<AppExecFwk::ChildProcessArgs> args)
{
    std::shared_ptr<ChildProcessStartInfo> processStartInfo = std::make_shared<ChildProcessStartInfo>();
    std::string filename = std::filesystem::path(srcEntry).stem();
    processStartInfo->name = filename;
    processStartInfo->moduleName = hapModuleInfo.moduleName;
    processStartInfo->hapPath = hapModuleInfo.hapPath;
    processStartInfo->srcEntry = srcEntry;
    processStartInfo->isEsModule = (hapModuleInfo.compileMode == AppExecFwk::CompileMode::ES_MODULE);

    auto process = ChildProcess::Create(runtime);
    if (process == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Failed to create ChildProcess.");
        return false;
    }
    bool ret = process->Init(processStartInfo);
    if (!ret) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "JsChildProcess init failed.");
        return false;
    }
    if (args) {
        process->OnStart(args);
    } else {
        process->OnStart();
    }
    TAG_LOGD(AAFwkTag::PROCESSMGR, "LoadJsFile end.");
    return true;
}

bool ChildProcessManager::LoadNativeLib(const std::string &moduleName,
    const std::string &libPath, const sptr<IRemoteObject> &mainProcessCb)
{
    auto childProcess = NativeChildIpcProcess::Create();
    if (childProcess == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Failed create NativeChildIpcProcess");
        return false;
    }
    
    std::shared_ptr<ChildProcessStartInfo> processStartInfo = std::make_shared<ChildProcessStartInfo>();
    processStartInfo->moduleName = moduleName;
    processStartInfo->name = std::filesystem::path(libPath).stem();
    processStartInfo->srcEntry = libPath;
    processStartInfo->ipcObj = mainProcessCb;
    if (!childProcess->Init(processStartInfo)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "NativeChildIpcProcess init failed.");
        return false;
    }
    
    childProcess->OnStart();
    TAG_LOGD(AAFwkTag::PROCESSMGR, "LoadNativeLib end.");
    return true;
}

std::unique_ptr<AbilityRuntime::Runtime> ChildProcessManager::CreateRuntime(const AppExecFwk::BundleInfo &bundleInfo,
    const AppExecFwk::HapModuleInfo &hapModuleInfo, const bool fromAppSpawn, const bool jitEnabled)
{
    AppExecFwk::ApplicationInfo applicationInfo = bundleInfo.applicationInfo;
    AbilityRuntime::Runtime::Options options;
    options.codePath = AbilityBase::Constants::LOCAL_CODE_PATH;
    options.bundleName = hapModuleInfo.bundleName;
    options.hapPath = hapModuleInfo.hapPath;
    options.moduleName = hapModuleInfo.moduleName;
    options.isBundle = (hapModuleInfo.compileMode != AppExecFwk::CompileMode::ES_MODULE);
    options.uid = applicationInfo.uid;
    options.isDebugVersion = applicationInfo.debug;
    options.arkNativeFilePath = applicationInfo.arkNativeFilePath;
    options.apiTargetVersion = applicationInfo.apiTargetVersion;
    options.loadAce = true;
    options.jitEnabled = jitEnabled;

    std::shared_ptr<AppExecFwk::EventRunner> eventRunner =
        fromAppSpawn ? AppExecFwk::EventRunner::GetMainEventRunner() : AppExecFwk::EventRunner::Create();
    options.eventRunner = eventRunner;

    return AbilityRuntime::Runtime::Create(options);
}

bool ChildProcessManager::GetBundleInfo(AppExecFwk::BundleInfo &bundleInfo)
{
    auto sysMrgClient = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMrgClient == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Failed to get SysMrgClient.");
        return false;
    }
    auto bundleObj = sysMrgClient->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleObj == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Failed to get bundle manager service.");
        return false;
    }
    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Bundle manager is nullptr.");
        return false;
    }
    return (bundleMgr->GetBundleInfoForSelf(
        (static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA)), bundleInfo) == ERR_OK);
}

bool ChildProcessManager::GetEntryHapModuleInfo(const AppExecFwk::BundleInfo &bundleInfo,
    AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    if (bundleInfo.hapModuleInfos.empty()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "hapModuleInfos empty!");
        return false;
    }
    TAG_LOGD(AAFwkTag::PROCESSMGR, "hapModueInfos size: %{public}zu", bundleInfo.hapModuleInfos.size());
    bool result = false;
    for (const auto &info : bundleInfo.hapModuleInfos) {
        if (info.moduleType == AppExecFwk::ModuleType::ENTRY) {
            result = true;
            hapModuleInfo = info;
            break;
        }
    }
    return result;
}

bool ChildProcessManager::GetHapModuleInfo(const AppExecFwk::BundleInfo &bundleInfo, const std::string &moduleName,
    AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    if (bundleInfo.hapModuleInfos.empty()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "hapModuleInfos empty!");
        return false;
    }
    TAG_LOGD(AAFwkTag::PROCESSMGR, "hapModueInfos size: %{public}zu", bundleInfo.hapModuleInfos.size());
    bool result = false;
    for (const auto &info : bundleInfo.hapModuleInfos) {
        if (info.name == moduleName) {
            result = true;
            hapModuleInfo = info;
            break;
        }
    }
    return result;
}

bool ChildProcessManager::HasChildProcessRecord()
{
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "GetAppMgr failed.");
        return false;
    }
    AppExecFwk::ChildProcessInfo info;
    return appMgr->GetChildProcessInfoForSelf(info) == ERR_OK;
}

sptr<AppExecFwk::IAppMgr> ChildProcessManager::GetAppMgr()
{
    auto sysMrgClient = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMrgClient == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Get SysMrgClient failed.");
        return nullptr;
    }
    auto object = sysMrgClient->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "GetAppMgr failed.");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IAppMgr>(object);
}

void ChildProcessManager::SetForkProcessJITEnabled(bool jitEnabled)
{
    g_jitEnabled = jitEnabled;
}

void ChildProcessManager::SetForkProcessDebugOption(const std::string bundleName, const bool isStartWithDebug,
    const bool isDebugApp, const bool isStartWithNative)
{
    g_debugOption.bundleName = bundleName;
    g_debugOption.isStartWithDebug = isStartWithDebug;
    g_debugOption.isDebugApp = isDebugApp;
    g_debugOption.isStartWithNative = isStartWithNative;
}

void ChildProcessManager::SetAppSpawnForkDebugOption(Runtime::DebugOption &debugOption,
    std::shared_ptr<AppExecFwk::ChildProcessInfo> processInfo)
{
    if (!processInfo) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "SetAppSpawnDebugOption failed, processInfo is nullptr.");
        return;
    }
    debugOption.processName = processInfo->processName;
    debugOption.isStartWithDebug = processInfo->isStartWithDebug;
    debugOption.isDebugApp = processInfo->isDebugApp;
    debugOption.isStartWithNative = processInfo->isStartWithNative;
}

void ChildProcessManager::MakeProcessName(const std::string &srcEntry)
{
    std::string processName = g_debugOption.bundleName;
    if (srcEntry.empty()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "srcEntry empty.");
    } else {
        TAG_LOGW(AAFwkTag::PROCESSMGR, "srcEntry is not empty.");
        std::string filename = std::filesystem::path(srcEntry).stem();
        if (!filename.empty()) {
            processName.append(":");
            processName.append(filename);
        }
    }
    processName.append(std::to_string(childProcessCount_));
    childProcessCount_++;
    TAG_LOGD(AAFwkTag::PROCESSMGR, "SetForkProcessDebugOption processName is %{public}s", processName.c_str());
    g_debugOption.processName = processName;
}

std::string ChildProcessManager::GetModuleNameFromSrcEntry(const std::string &srcEntry)
{
    std::string::size_type pos = srcEntry.find_first_of('/');
    if (pos == std::string::npos) {
        return "";
    }
    std::string moduleName = srcEntry.substr(0, pos);
    if (moduleName == ".") {
        return "";
    }
    return moduleName;
}
}  // namespace AbilityRuntime
}  // namespace OHOS