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

#include "app_lifecycle_deal.h"

#include "freeze_util.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "time_util.h"

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AppExecFwk {
AppLifeCycleDeal::AppLifeCycleDeal()
{}

AppLifeCycleDeal::~AppLifeCycleDeal()
{
    HILOG_INFO("AppLifeCycleDeal deconstructor");
}

void AppLifeCycleDeal::LaunchApplication(const AppLaunchData &launchData, const Configuration &config)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("LoadLifecycle: Launch application");
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
        auto abilityInfo = ability->GetAbilityInfo();
        if (abilityInfo == nullptr) {
            HILOG_WARN("LoadLifecycle: abilityInfo null.");
            return;
        }
        if (abilityInfo->type == AbilityType::PAGE) {
            FreezeUtil::LifecycleFlow flow = {ability->GetToken(), FreezeUtil::TimeoutState::LOAD};
            auto entry = std::to_string(AbilityRuntime::TimeUtil::SystemTimeMillisecond()) +
                "; AppLifeCycleDeal::LaunchAbility; the LoadAbility lifecycle.";
            FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
        }
        HILOG_INFO("LoadLifecycle: Launch ability.");
        appThread_->ScheduleLaunchAbility(*abilityInfo, ability->GetToken(),
            ability->GetWant());
    } else {
        HILOG_WARN("LoadLifecycle.");
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

void AppLifeCycleDeal::ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleHeapMemory(pid, mallocInfo);
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

void AppLifeCycleDeal::ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName)
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleNewProcessRequest(want, moduleName);
}

int32_t AppLifeCycleDeal::UpdateConfiguration(const Configuration &config)
{
    HILOG_DEBUG("call");
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
    HILOG_DEBUG("call");
    if (appThread_ == nullptr) {
        HILOG_ERROR("appThread_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return appThread_->ScheduleNotifyLoadRepairPatch(bundleName, callback, recordId);
}

int32_t AppLifeCycleDeal::NotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("call");
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

int32_t AppLifeCycleDeal::NotifyAppFault(const FaultData &faultData)
{
    HILOG_DEBUG("called.");
    if (appThread_ == nullptr) {
        HILOG_ERROR("appThread_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return appThread_->ScheduleNotifyAppFault(faultData);
}

int32_t AppLifeCycleDeal::ChangeAppGcState(int32_t state)
{
    HILOG_DEBUG("called.");
    if (appThread_ == nullptr) {
        HILOG_ERROR("appThread_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return appThread_->ScheduleChangeAppGcState(state);
}

int32_t AppLifeCycleDeal::AttachAppDebug()
{
    HILOG_DEBUG("Called.");
    if (appThread_ == nullptr) {
        HILOG_ERROR("appThread_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    appThread_->AttachAppDebug();
    return ERR_OK;
}

int32_t AppLifeCycleDeal::DetachAppDebug()
{
    HILOG_DEBUG("Called.");
    if (appThread_ == nullptr) {
        HILOG_ERROR("appThread_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    appThread_->DetachAppDebug();
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
