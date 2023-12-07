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
namespace {
const std::string SYS_PARAM_PRODUCT_DEVICE_TYPE = "const.product.devicetype";
}

bool ChildProcessManager::signalRegistered_ = false;

ChildProcessManager::ChildProcessManager()
{
    HILOG_DEBUG("ChildProcessManager constructor called");
    std::string deviceType = OHOS::system::GetParameter(SYS_PARAM_PRODUCT_DEVICE_TYPE, "");
    multiProcessModelEnabled_ = deviceType == "2in1" || deviceType == "tablet";
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

ChildProcessManagerErrorCode ChildProcessManager::StartChildProcessBySelfFork(const std::string &srcEntry, pid_t &pid)
{
    HILOG_DEBUG("StartChildProcessBySelfFork called");
    ChildProcessManagerErrorCode errorCode = PreCheck();
    if (errorCode != ChildProcessManagerErrorCode::ERR_OK) {
        return errorCode;
    }
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        HILOG_ERROR("Get applicationContext failed.");
        return ChildProcessManagerErrorCode::ERR_GET_APPLICATION_CONTEXT_FAILED;
    }
    std::string bundleName = applicationContext->GetBundleName();
    AppExecFwk::HapModuleInfo hapModuleInfo;
    if (!GetHapModuleInfo(bundleName, hapModuleInfo)) {
        HILOG_ERROR("GetHapModuleInfo failed");
        return ChildProcessManagerErrorCode::ERR_GET_HAP_INFO_FAILED;
    }

    pid = fork();
    if (pid < 0) {
        HILOG_ERROR("Fork process failed");
        return ChildProcessManagerErrorCode::ERR_FORK_FAILED;
    }
    if (pid == 0) {
        HILOG_DEBUG("Child process start");
        isChildProcess_ = true;
        HandleChildProcess(srcEntry, hapModuleInfo);
        HILOG_DEBUG("Child process end");
        exit(0);
    }
    return ChildProcessManagerErrorCode::ERR_OK;
}

ChildProcessManagerErrorCode ChildProcessManager::PreCheck()
{
    if (!MultiProcessModelEnabled()) {
        HILOG_ERROR("Multi process model is not enabled");
        return ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED;
    }
    if (IsChildProcess()) {
        HILOG_ERROR("Already in child process");
        return ChildProcessManagerErrorCode::ERR_ALREADY_IN_CHILD_PROCESS;
    }
    return ChildProcessManagerErrorCode::ERR_OK;
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
    if (eventRunner == nullptr) {
        HILOG_ERROR("Get main eventRunner failed.");
        return;
    }
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
    if (process == nullptr) {
        HILOG_ERROR("Failed to create ChildProcess.");
        return;
    }
    bool ret = process->Init(processStartInfo);
    if (!ret) {
        HILOG_ERROR("JsChildProcess init failed.");
        return;
    }
    process->OnStart();
}

bool ChildProcessManager::GetHapModuleInfo(const std::string &bundleName, AppExecFwk::HapModuleInfo &hapModuleInfo)
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
        HILOG_ERROR("hapModuleInfos empty!");
        return false;
    }
    HILOG_DEBUG("hapModueInfos size: %{public}zu", bundleInfo.hapModuleInfos.size());
    bool result = false;
    for (auto info : bundleInfo.hapModuleInfos) {
        if (info.moduleType == AppExecFwk::ModuleType::ENTRY) {
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
    if (applicationContext == nullptr) {
        HILOG_ERROR("Get applicationContext failed.");
        return nullptr;
    }
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