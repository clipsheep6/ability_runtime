/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "quick_fix_manager_service.h"

#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "permission_verification.h"
#include "quick_fix_error_utils.h"
#include "quick_fix_utils.h"

namespace OHOS {
namespace AAFwk {
std::mutex QuickFixManagerService::mutex_;
sptr<QuickFixManagerService> QuickFixManagerService::instance_;

sptr<QuickFixManagerService> QuickFixManagerService::GetInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (instance_ == nullptr) {
        instance_ = new QuickFixManagerService();
    }
    return instance_;
}

bool QuickFixManagerService::Init()
{
    std::lock_guard<std::mutex> lock(mutex_);
    eventRunner_ = AppExecFwk::EventRunner::Create("QuickFixMgrSvrMain");
    if (eventRunner_ == nullptr) {
        HILOG_ERROR("Create event runner failed.");
        return false;
    }

    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(eventRunner_);
    if (eventHandler_ == nullptr) {
        HILOG_ERROR("Create event handler failed.");
        return false;
    }

    return true;
}

int32_t QuickFixManagerService::ApplyQuickFix(const std::vector<std::string> &quickFixFiles, bool isDebug)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        HILOG_ERROR("The caller is not system-app, can not use system-api");
        return QUICK_FIX_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyInstallBundlePermission()) {
        return QUICK_FIX_VERIFY_PERMISSION_FAILED;
    }

    auto bundleQfMgr = QuickFixUtil::GetBundleQuickFixMgrProxy();
    if (bundleQfMgr == nullptr) {
        HILOG_ERROR("Bundle quick fix manager is nullptr.");
        return QUICK_FIX_CONNECT_FAILED;
    }

    auto appMgr = QuickFixUtil::GetAppManagerProxy();
    if (appMgr == nullptr) {
        HILOG_ERROR("App manager is nullptr.");
        return QUICK_FIX_CONNECT_FAILED;
    }
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr, appMgr, eventHandler_, this);
    AddApplyTask(applyTask);
    applyTask->Run(quickFixFiles, isDebug);

    HILOG_DEBUG("function finished.");
    return QUICK_FIX_OK;
}

int32_t QuickFixManagerService::GetApplyedQuickFixInfo(const std::string &bundleName,
    ApplicationQuickFixInfo &quickFixInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        HILOG_ERROR("The caller is not system-app, can not use system-api");
        return QUICK_FIX_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyGetBundleInfoPrivilegedPermission()) {
        return QUICK_FIX_VERIFY_PERMISSION_FAILED;
    }

    auto bundleMgr = QuickFixUtil::GetBundleManagerProxy();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("Failed to get bundle manager!");
        return QUICK_FIX_CONNECT_FAILED;
    }

    AppExecFwk::BundleInfo bundleInfo;
    if (!bundleMgr->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo,
        AppExecFwk::Constants::ANY_USERID)) {
        HILOG_ERROR("Get bundle info failed!");
        return QUICK_FIX_GET_BUNDLE_INFO_FAILED;
    }

    quickFixInfo.bundleName = bundleName;
    quickFixInfo.bundleVersionCode = bundleInfo.versionCode;
    quickFixInfo.bundleVersionName = bundleInfo.versionName;
    quickFixInfo.appqfInfo = bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo;

    HILOG_DEBUG("function finished.");
    return QUICK_FIX_OK;
}

int32_t QuickFixManagerService::RevokeQuickFix(const std::string &bundleName)
{
    HILOG_DEBUG("Function called.");
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        HILOG_ERROR("The caller is not system-app, can not use system-api");
        return QUICK_FIX_NOT_SYSTEM_APP;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->VerifyGetBundleInfoPrivilegedPermission() ||
        !AAFwk::PermissionVerification::GetInstance()->VerifyInstallBundlePermission()) {
        HILOG_ERROR("Permission verification failed");
        return QUICK_FIX_VERIFY_PERMISSION_FAILED;
    }

    if (CheckTaskRunningState(bundleName)) {
        HILOG_ERROR("Has a apply quick fix task");
        return QUICK_FIX_DEPLOYING_TASK;
    }

    auto bundleMgr = QuickFixUtil::GetBundleManagerProxy();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("Failed to get bundle manager.");
        return QUICK_FIX_CONNECT_FAILED;
    }

    AppExecFwk::BundleInfo bundleInfo;
    if (!bundleMgr->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo,
        AppExecFwk::Constants::ANY_USERID)) {
        HILOG_ERROR("Get bundle info failed.");
        return QUICK_FIX_GET_BUNDLE_INFO_FAILED;
    }

    auto isSoContained = !bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.nativeLibraryPath.empty();
    auto patchExists = true;
    for (auto &item : bundleInfo.hapModuleInfos) {
        if (!item.hqfInfo.moduleName.empty()) {
            patchExists = false;
            break;
        }
    }

    if (patchExists) {
        HILOG_ERROR("Patch does not exist.");
        return QUICK_FIX_GET_BUNDLE_INFO_FAILED;
    }

    auto appMgr = QuickFixUtil::GetAppManagerProxy();
    if (appMgr == nullptr) {
        HILOG_ERROR("App manager is nullptr.");
        return QUICK_FIX_CONNECT_FAILED;
    }

    auto bundleQfMgr = QuickFixUtil::GetBundleQuickFixMgrProxy();
    if (bundleQfMgr == nullptr) {
        HILOG_ERROR("Bundle quick fix manager is nullptr.");
        return QUICK_FIX_CONNECT_FAILED;
    }

    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr, appMgr, eventHandler_, this);
    if (applyTask == nullptr) {
        HILOG_ERROR("Task connect failed.");
        return QUICK_FIX_CONNECT_FAILED;
    }

    applyTask->InitRevokeTask(bundleName, isSoContained);
    AddApplyTask(applyTask);
    applyTask->RunRevoke();
    HILOG_DEBUG("Function finished.");
    return QUICK_FIX_OK;
}

void QuickFixManagerService::AddApplyTask(std::shared_ptr<QuickFixManagerApplyTask> applyTask)
{
    std::lock_guard<std::mutex> lock(mutex_);
    applyTasks_.emplace_back(applyTask);
}

void QuickFixManagerService::RemoveApplyTask(std::shared_ptr<QuickFixManagerApplyTask> applyTask)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = applyTasks_.begin(); it != applyTasks_.end();) {
        if (*it == applyTask) {
            it = applyTasks_.erase(it);
        } else {
            it++;
        }
    }
}

bool QuickFixManagerService::CheckTaskRunningState(const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : applyTasks_) {
        if (item != nullptr && item->GetBundleName() == bundleName) {
            return true;
        }
    }

    HILOG_DEBUG("bundleName %{public}s not found in tasks.", bundleName.c_str());
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS
