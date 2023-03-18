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

#include "app_lifecycle_deal.h"

#include "hilog_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AppExecFwk {
AppLifeCycleDeal::AppLifeCycleDeal()
{}

AppLifeCycleDeal::~AppLifeCycleDeal()
{}

void AppLifeCycleDeal::LaunchApplication(const AppLaunchData &launchData, const Configuration &config)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("AppLifeCycleDeal ScheduleLaunchApplication");
    if (appThread_) {
        appThread_->ScheduleLaunchApplication(launchData, config);
    }
}

void AppLifeCycleDeal::UpdateApplicationInfoInstalled(const ApplicationInfo &appInfo)
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleUpdateApplicationInfoInstalled(appInfo);
}

void AppLifeCycleDeal::AddAbilityStage(const HapModuleInfo &abilityStage)
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleAbilityStage(abilityStage);
}

void AppLifeCycleDeal::LaunchAbility(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    if (appThread_ && ability) {
        appThread_->ScheduleLaunchAbility(*(ability->GetAbilityInfo()), ability->GetToken(),
            ability->GetWant());
    }
}

void AppLifeCycleDeal::ScheduleTerminate()
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleTerminateApplication();
}

void AppLifeCycleDeal::ScheduleForegroundRunning()
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleForegroundApplication();
}

void AppLifeCycleDeal::ScheduleBackgroundRunning()
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleBackgroundApplication();
}

void AppLifeCycleDeal::ScheduleTrimMemory(int32_t timeLevel)
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleShrinkMemory(timeLevel);
}

void AppLifeCycleDeal::ScheduleMemoryLevel(int32_t Level)
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleMemoryLevel(Level);
}

void AppLifeCycleDeal::ScheduleHeapMemory(std::vector<int32_t> &mallinfo)
{
    HILOG_ERROR("AppLifeCycleDeal ScheduleHeapMemory is called.");
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleHeapMemory(mallinfo);
}

void AppLifeCycleDeal::LowMemoryWarning()
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleLowMemory();
}

void AppLifeCycleDeal::ScheduleCleanAbility(const sptr<IRemoteObject> &token)
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }
    appThread_->ScheduleCleanAbility(token);
}

void AppLifeCycleDeal::ScheduleProcessSecurityExit()
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleProcessSecurityExit();
}

void AppLifeCycleDeal::SetApplicationClient(const sptr<IAppScheduler> &thread)
{
    appThread_ = thread;
}

sptr<IAppScheduler> AppLifeCycleDeal::GetApplicationClient() const
{
    return appThread_;
}

void AppLifeCycleDeal::ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName)
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleAcceptWant(want, moduleName);
}

int32_t AppLifeCycleDeal::UpdateConfiguration(const Configuration &config)
{
    HILOG_INFO("call %{public}s", __func__);
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    appThread_->ScheduleConfigurationUpdated(config);
    return ERR_OK;
}

int32_t AppLifeCycleDeal::NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback,
    const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("call %{public}s", __func__);
    if (appThread_ == nullptr) {
        HILOG_ERROR("appThread_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return appThread_->ScheduleNotifyLoadRepairPatch(bundleName, callback, recordId);
}

int32_t AppLifeCycleDeal::NotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("call %{public}s", __func__);
    if (appThread_ == nullptr) {
        HILOG_ERROR("appThread_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return appThread_->ScheduleNotifyHotReloadPage(callback, recordId);
}

int32_t AppLifeCycleDeal::NotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    if (appThread_ == nullptr) {
        HILOG_ERROR("appThread_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return appThread_->ScheduleNotifyUnLoadRepairPatch(bundleName, callback, recordId);
}
}  // namespace AppExecFwk
}  // namespace OHOS
