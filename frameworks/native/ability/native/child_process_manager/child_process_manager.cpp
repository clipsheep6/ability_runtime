/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "child_process_manager_error_utils.h"
#include "child_process_start_info.h"
#include "constants.h"
#include "event_runner.h"
#include "errors.h"
#include "hap_module_info.h"
#include "hilog_wrapper.h"
#include "parameters.h"
#include "runtime.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
bool ChildProcessManager::signalRegistered_ = false;

ChildProcessManager::ChildProcessManager()
{
    HILOG_DEBUG("ChildProcessManager constructor called");
}

ChildProcessManager::~ChildProcessManager()
{
    HILOG_DEBUG("ChildProcessManager deconstructor called");
}

ChildProcessManagerErrorCode ChildProcessManager::StartChildProcessBySelfFork(const std::string &srcEntry, pid_t &pid)
{
    HILOG_INFO("called.");
    ChildProcessManagerErrorCode errorCode = PreCheck();
    if (errorCode != ChildProcessManagerErrorCode::ERR_OK) {
        return errorCode;
    }

    AppExecFwk::BundleInfo bundleInfo;
    if (!GetBundleInfo(bundleInfo)) {
        HILOG_ERROR("GetBundleInfo failed.");
        return ChildProcessManagerErrorCode::ERR_GET_BUNDLE_INFO_FAILED;
    }
    
    RegisterSignal();
    pid = fork();
    if (pid < 0) {
        HILOG_ERROR("Fork process failed");
        return ChildProcessManagerErrorCode::ERR_FORK_FAILED;
    }
    if (pid == 0) {
        HandleChildProcessBySelfFork(srcEntry, bundleInfo);
    }
    return ChildProcessManagerErrorCode::ERR_OK;
}

ChildProcessManagerErrorCode ChildProcessManager::StartChildProcessByAppSpawnFork(
    const std::string &srcEntry, pid_t &pid)
{
    HILOG_INFO("called.");
    ChildProcessManagerErrorCode errorCode = PreCheck();
    if (errorCode != ChildProcessManagerErrorCode::ERR_OK) {
        return errorCode;
    }
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        HILOG_ERROR("GetAppMgr failed.");
        return ChildProcessManagerErrorCode::ERR_GET_APP_MGR_FAILED;
    }
    auto ret = appMgr->StartChildProcess(srcEntry, pid);
    HILOG_DEBUG("AppMgr StartChildProcess ret:%{public}d", ret);
    if (ret != ERR_OK) {
        HILOG_ERROR("AppMgr StartChildProcess failed, ret:%{public}d", ret);
        return ChildProcessManagerErrorCode::ERR_GET_APP_MGR_START_PROCESS_FAILED;
    }
    return ChildProcessManagerErrorCode::ERR_OK;
}

void ChildProcessManager::RegisterSignal()
{
    if (!signalRegistered_) {
        signalRegistered_ = true;
        HILOG_DEBUG("Register signal");
        signal(SIGCHLD, ChildProcessManager::HandleSigChild);
    }
}

void ChildProcessManager::HandleSigChild(int32_t signo)
{
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        continue;
    }
}

ChildProcessManagerErrorCode ChildProcessManager::PreCheck()
{
    if (!AAFwk::AppUtils::GetInstance().isMultiProcessModel()) {
        HILOG_ERROR("Multi process model is not enabled");
        return ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED;
    }
    if (IsChildProcess()) {
        HILOG_ERROR("Already in child process");
        return ChildProcessManagerErrorCode::ERR_ALREADY_IN_CHILD_PROCESS;
    }
    return ChildProcessManagerErrorCode::ERR_OK;
}

bool ChildProcessManager::IsChildProcess()
{
    return isChildProcessBySelfFork_ || hasChildProcessRecord();
}

void ChildProcessManager::HandleChildProcessBySelfFork(const std::string &srcEntry,
    const AppExecFwk::BundleInfo &bundleInfo)
{
    HILOG_DEBUG("HandleChildProcessBySelfFork start.");
    isChildProcessBySelfFork_ = true;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (eventRunner == nullptr) {
        HILOG_ERROR("Get main eventRunner failed.");
        return;
    }
    eventRunner->Stop();
    
    AppExecFwk::HapModuleInfo hapModuleInfo;
    if (!GetHapModuleInfo(bundleInfo, hapModuleInfo)) {
        HILOG_ERROR("GetHapModuleInfo failed.");
        return;
    }

    auto runtime = CreateRuntime(bundleInfo, hapModuleInfo, false);
    if (!runtime) {
        HILOG_ERROR("Failed to create child process runtime");
        return;
    }
    LoadJsFile(srcEntry, hapModuleInfo, runtime);
    HILOG_DEBUG("HandleChildProcessBySelfFork end.");
    exit(0);
}

bool ChildProcessManager::LoadJsFile(const std::string &srcEntry, const AppExecFwk::HapModuleInfo &hapModuleInfo,
    std::unique_ptr<AbilityRuntime::Runtime> &runtime)
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
        HILOG_ERROR("Failed to create ChildProcess.");
        return false;
    }
    bool ret = process->Init(processStartInfo);
    if (!ret) {
        HILOG_ERROR("JsChildProcess init failed.");
        return false;
    }
    process->OnStart();
    HILOG_DEBUG("LoadJsFile end.");
    return true;
}

std::unique_ptr<AbilityRuntime::Runtime> ChildProcessManager::CreateRuntime(const AppExecFwk::BundleInfo &bundleInfo,
    const AppExecFwk::HapModuleInfo &hapModuleInfo, const bool fromAppSpawn)
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

    std::shared_ptr<AppExecFwk::EventRunner> eventRunner =
        fromAppSpawn ? AppExecFwk::EventRunner::GetMainEventRunner() : AppExecFwk::EventRunner::Create();
    options.eventRunner = eventRunner;

    return AbilityRuntime::Runtime::Create(options);
}

bool ChildProcessManager::GetBundleInfo(AppExecFwk::BundleInfo &bundleInfo)
{
    auto sysMrgClient = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMrgClient == nullptr) {
        HILOG_ERROR("Failed to get SysMrgClient.");
        return false;
    }
    auto bundleObj = sysMrgClient->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleObj == nullptr) {
        HILOG_ERROR("Failed to get bundle manager service.");
        return false;
    }
    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
    if (bundleMgr == nullptr) {
        HILOG_ERROR("Bundle manager is nullptr.");
        return false;
    }
    return (bundleMgr->GetBundleInfoForSelf(
        (static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA)), bundleInfo) == ERR_OK);
}

bool ChildProcessManager::GetHapModuleInfo(const AppExecFwk::BundleInfo &bundleInfo,
    AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    if (bundleInfo.hapModuleInfos.empty()) {
        HILOG_ERROR("hapModuleInfos empty!");
        return false;
    }
    HILOG_DEBUG("hapModueInfos size: %{public}zu", bundleInfo.hapModuleInfos.size());
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

bool ChildProcessManager::hasChildProcessRecord()
{
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        HILOG_ERROR("GetAppMgr failed.");
        return false;
    }
    AppExecFwk::ChildProcessInfo info;
    return appMgr->GetChildProcessInfoForSelf(info) == ERR_OK;
}

sptr<AppExecFwk::IAppMgr> ChildProcessManager::GetAppMgr()
{
    auto sysMrgClient = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMrgClient == nullptr) {
        HILOG_ERROR("Get SysMrgClient failed.");
        return nullptr;
    }
    auto object = sysMrgClient->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (object == nullptr) {
        HILOG_ERROR("GetAppMgr failed.");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IAppMgr>(object);
}

}  // namespace AbilityRuntime
}  // namespace OHOS