/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "quick_fix_error_utils.h"
#include "quick_fix_util.h"

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

int32_t QuickFixManagerService::ApplyQuickFix(const std::vector<std::string> &quickFixFiles)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");

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
    applyTask->Run(quickFixFiles);

    HILOG_DEBUG("function finished.");
    return QUICK_FIX_OK;
}

int32_t QuickFixManagerService::GetApplyedQuickFixInfo(const std::string &bundleName,
    ApplicationQuickFixInfo &quickFixInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");

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

    quickFixInfo.bundleName = bundleName;
    quickFixInfo.bundleVersionCode = bundleInfo.versionCode;
    quickFixInfo.bundleVersionName = bundleInfo.versionName;
    quickFixInfo.appqfInfo = bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo;

    HILOG_DEBUG("function finished.");
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
}  // namespace AAFwk
}  // namespace OHOS
