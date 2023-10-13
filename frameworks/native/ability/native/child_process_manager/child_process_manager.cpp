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

#include "application_info.h"
#include "application_context.h"
#include "bundle_info.h"
#include "bundle_mgr_interface.h"
#include "child_process.h"
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
namespace {
constexpr pid_t INVALID_PID = -1;
const std::string SYS_PARAM_MULTI_PROCESS_MODEL = "persist.sys.multi_process_model";
}

bool ChildProcessManager::signalRegistered_ = false;

ChildProcessManager::ChildProcessManager()
{
    HILOG_DEBUG("ChildProcessManager constructor called");
    multiProcessModelEnabled_ = OHOS::system::GetBoolParameter(SYS_PARAM_MULTI_PROCESS_MODEL, false);
    if (!signalRegistered_) {
        signalRegistered_ = true;
        HILOG_DEBUG("Register signal");
        signal(SIGCHLD, ChildProcessManager::HandleSigChild);
    }
}

ChildProcessManager::~ChildProcessManager()
{
    HILOG_DEBUG("ChildProcessManager deconstructor called");
}

void ChildProcessManager::HandleSigChild(int32_t signo)
{
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        continue;
    }
}

pid_t ChildProcessManager::StartChildProcessBySelfFork(const std::string &srcEntry)
{
    HILOG_DEBUG("StartChildProcessBySelfFork called");
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    std::string bundleName = applicationContext->GetBundleName();
    std::string moduleName = GetModuleNameFromSrcEntry(srcEntry);
    AppExecFwk::HapModuleInfo hapModuleInfo;
    if (!GetHapModuleInfo(bundleName, moduleName, hapModuleInfo)) {
        HILOG_ERROR("GetHapModuleInfo failed");
        return INVALID_PID;
    }

    pid_t pid = fork();
    if (pid < 0) {
        HILOG_ERROR("Fork process failed");
        return pid;
    }
    if (pid == 0) {
        HILOG_DEBUG("Child process start");
        isChildProcess_ = true;
        HandleChildProcess(srcEntry, hapModuleInfo);
        HILOG_DEBUG("Child process end");
        exit(0);
    }
    return pid;
}

bool ChildProcessManager::MultiProcessModelEnabled()
{
    return multiProcessModelEnabled_;
}

bool ChildProcessManager::IsChildProcess()
{
    return isChildProcess_;
}

void ChildProcessManager::HandleChildProcess(const std::string &srcEntry, AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    eventRunner->Stop();

    auto runtime = CreateRuntime(hapModuleInfo);
    if (!runtime) {
        HILOG_ERROR("Failed to create child process runtime");
        return;
    }

    std::shared_ptr<ChildProcessStartInfo> processStartInfo = std::make_shared<ChildProcessStartInfo>();
    std::string filename = std::filesystem::path(srcEntry).stem();
    processStartInfo->name = filename;
    processStartInfo->moduleName = hapModuleInfo.moduleName;
    processStartInfo->hapPath = hapModuleInfo.hapPath;
    processStartInfo->srcEntry = srcEntry;
    processStartInfo->isEsModule = (hapModuleInfo.compileMode == AppExecFwk::CompileMode::ES_MODULE);

    auto process = ChildProcess::Create(runtime);
    process->Init(processStartInfo);
    process->OnStart();
}

std::string ChildProcessManager::GetModuleNameFromSrcEntry(const std::string &srcEntry)
{
    std::string::size_type iPos = srcEntry.find_first_of('/');
    if (iPos == std::string::npos) {
        return "";
    }
    std::string moduleName = srcEntry.substr(0, iPos);
    if (moduleName == ".") {
        return "";
    }
    return moduleName;
}

bool ChildProcessManager::GetHapModuleInfo(const std::string &bundleName,
                                           const std::string &moduleName, AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    auto bundleObj =
        DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleObj == nullptr) {
        HILOG_ERROR("Failed to get bundle manager service.");
        return false;
    }

    sptr<AppExecFwk::IBundleMgr> bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
    if (bundleMgr == nullptr) {
        HILOG_ERROR("Bundle manager is nullptr.");
        return false;
    }

    AppExecFwk::BundleInfo bundleInfo;
    bool queryResult = (bundleMgr->GetBundleInfoForSelf(
        (static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_DISABLE) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA)), bundleInfo) == ERR_OK);
    if (!queryResult) {
        HILOG_ERROR("GetBundleInfo failed!");
        return false;
    }
    if (bundleInfo.hapModuleInfos.empty()) {
        return false;
    }
    HILOG_DEBUG("hapModueInfos size: %{public}zu", bundleInfo.hapModuleInfos.size());
    bool result = false;
    const bool moduleNameExist = moduleName.length() > 0;
    for (auto info : bundleInfo.hapModuleInfos) {
        if (moduleNameExist) {
            if (info.moduleName == moduleName) {
                result = true;
                hapModuleInfo = info;
                break;
            }
        } else if (info.moduleType == AppExecFwk::ModuleType::ENTRY) {
            result = true;
            hapModuleInfo = info;
            break;
        }
    }
    return result;
}

std::unique_ptr<AbilityRuntime::Runtime> ChildProcessManager::CreateRuntime(AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = applicationContext->GetApplicationInfo();
    if (applicationInfo == nullptr) {
        HILOG_ERROR("applicationInfo is nullptr");
        return nullptr;
    }

    AbilityRuntime::Runtime::Options options;
    options.codePath = AbilityBase::Constants::LOCAL_CODE_PATH;
    options.bundleName = hapModuleInfo.bundleName;
    options.hapPath = hapModuleInfo.hapPath;
    options.moduleName = hapModuleInfo.moduleName;
    options.isBundle = (hapModuleInfo.compileMode != AppExecFwk::CompileMode::ES_MODULE);
    options.uid = applicationInfo->uid;
    options.isDebugVersion = applicationInfo->debug;
    options.arkNativeFilePath = applicationInfo->arkNativeFilePath;
    options.apiTargetVersion = applicationInfo->apiTargetVersion;
    options.loadAce = true;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create();
    options.eventRunner = eventRunner;

    return AbilityRuntime::Runtime::Create(options);
}
}  // namespace AbilityRuntime
}  // namespace OHOS