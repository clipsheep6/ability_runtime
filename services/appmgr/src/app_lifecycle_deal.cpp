/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "time_util.h"
#include "app_mgr_service_const.h"
#include "app_mgr_service_dump_error_code.h"

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AppExecFwk {
AppLifeCycleDeal::AppLifeCycleDeal()
{}

AppLifeCycleDeal::~AppLifeCycleDeal()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

void AppLifeCycleDeal::LaunchApplication(const AppLaunchData &launchData, const Configuration &config)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appThread = GetApplicationClient();
    if (appThread) {
        appThread->ScheduleLaunchApplication(launchData, config);
    }
}

void AppLifeCycleDeal::UpdateApplicationInfoInstalled(const ApplicationInfo &appInfo)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return;
    }

    appThread->ScheduleUpdateApplicationInfoInstalled(appInfo);
}

void AppLifeCycleDeal::AddAbilityStage(const HapModuleInfo &abilityStage)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return;
    }

    appThread->ScheduleAbilityStage(abilityStage);
}

void AppLifeCycleDeal::LaunchAbility(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    auto appThread = GetApplicationClient();
    if (appThread && ability) {
        auto abilityInfo = ability->GetAbilityInfo();
        if (abilityInfo == nullptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "abilityInfo null.");
            return;
        }
        if (abilityInfo->type == AbilityType::PAGE) {
            FreezeUtil::LifecycleFlow flow = {ability->GetToken(), FreezeUtil::TimeoutState::LOAD};
            auto entry = std::to_string(AbilityRuntime::TimeUtil::SystemTimeMillisecond()) +
                "; AppLifeCycleDeal::LaunchAbility; the LoadAbility lifecycle.";
            FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
        }
        TAG_LOGD(AAFwkTag::APPMGR, "Launch ability.");
        appThread->ScheduleLaunchAbility(*abilityInfo, ability->GetToken(),
            ability->GetWant(), ability->GetAbilityRecordId());
    } else {
        TAG_LOGW(AAFwkTag::APPMGR, "LoadLifecycle.");
    }
}

void AppLifeCycleDeal::ScheduleTerminate(bool isLastProcess)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return;
    }

    appThread->ScheduleTerminateApplication(isLastProcess);
}

void AppLifeCycleDeal::ScheduleForegroundRunning()
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return;
    }

    appThread->ScheduleForegroundApplication();
}

void AppLifeCycleDeal::ScheduleBackgroundRunning()
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return;
    }

    appThread->ScheduleBackgroundApplication();
}

void AppLifeCycleDeal::ScheduleTrimMemory(int32_t timeLevel)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return;
    }

    appThread->ScheduleShrinkMemory(timeLevel);
}

void AppLifeCycleDeal::ScheduleMemoryLevel(int32_t Level)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return;
    }

    appThread->ScheduleMemoryLevel(Level);
}

void AppLifeCycleDeal::ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return;
    }

    appThread->ScheduleHeapMemory(pid, mallocInfo);
}

void AppLifeCycleDeal::ScheduleJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return;
    }

    appThread->ScheduleJsHeapMemory(info);
}

void AppLifeCycleDeal::LowMemoryWarning()
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return;
    }

    appThread->ScheduleLowMemory();
}

void AppLifeCycleDeal::ScheduleCleanAbility(const sptr<IRemoteObject> &token)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return;
    }
    appThread->ScheduleCleanAbility(token);
}

void AppLifeCycleDeal::ScheduleProcessSecurityExit()
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return;
    }

    appThread->ScheduleProcessSecurityExit();
}

void AppLifeCycleDeal::SetApplicationClient(const sptr<IAppScheduler> &thread)
{
    std::lock_guard guard(schedulerMutex_);
    appThread_ = thread;
}

sptr<IAppScheduler> AppLifeCycleDeal::GetApplicationClient() const
{
    std::lock_guard guard(schedulerMutex_);
    return appThread_;
}

void AppLifeCycleDeal::ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return;
    }

    appThread->ScheduleAcceptWant(want, moduleName);
}

void AppLifeCycleDeal::ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return;
    }

    appThread->ScheduleNewProcessRequest(want, moduleName);
}

int32_t AppLifeCycleDeal::UpdateConfiguration(const Configuration &config)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr");
        return ERR_INVALID_VALUE;
    }
    appThread->ScheduleConfigurationUpdated(config);
    return ERR_OK;
}

int32_t AppLifeCycleDeal::NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback,
    const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return appThread->ScheduleNotifyLoadRepairPatch(bundleName, callback, recordId);
}

int32_t AppLifeCycleDeal::NotifyLoadPatch(const std::string &bundleName, const std::string &moduleName,
                                          const sptr<IQuickFixCallback> &callback,
                                          const int32_t recordId, const int &patchVersion)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return appThread->ScheduleNotifyLoadPatch(bundleName, moduleName, callback, recordId, patchVersion);
}

int32_t AppLifeCycleDeal::NotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return appThread->ScheduleNotifyHotReloadPage(callback, recordId);
}

int32_t AppLifeCycleDeal::NotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "function called.");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return appThread->ScheduleNotifyUnLoadRepairPatch(bundleName, callback, recordId);
}

int32_t AppLifeCycleDeal::NotifyAppFault(const FaultData &faultData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called.");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return appThread->ScheduleNotifyAppFault(faultData);
}

int32_t AppLifeCycleDeal::ChangeAppGcState(int32_t state)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called.");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return appThread->ScheduleChangeAppGcState(state);
}

int32_t AppLifeCycleDeal::AttachAppDebug()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr.");
        return ERR_INVALID_VALUE;
    }
    appThread->AttachAppDebug();
    return ERR_OK;
}

int32_t AppLifeCycleDeal::DetachAppDebug()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr.");
        return ERR_INVALID_VALUE;
    }
    appThread->DetachAppDebug();
    return ERR_OK;
}

int AppLifeCycleDeal::DumpIpcStart(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        result.append(MSG_DUMP_IPC_START_STAT)
            .append(MSG_DUMP_IPC_FAIL)
            .append(MSG_DUMP_IPC_FAIL_REASON_INTERNAL);
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr.");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appThread->ScheduleDumpIpcStart(result);
}

int AppLifeCycleDeal::DumpIpcStop(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        result.append(MSG_DUMP_IPC_STOP_STAT)
            .append(MSG_DUMP_IPC_FAIL)
            .append(MSG_DUMP_IPC_FAIL_REASON_INTERNAL);
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr.");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appThread->ScheduleDumpIpcStop(result);
}

int AppLifeCycleDeal::DumpIpcStat(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        result.append(MSG_DUMP_IPC_STAT)
            .append(MSG_DUMP_IPC_FAIL)
            .append(MSG_DUMP_IPC_FAIL_REASON_INTERNAL);
        TAG_LOGE(AAFwkTag::APPMGR, "appThread is nullptr.");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appThread->ScheduleDumpIpcStat(result);
}
}  // namespace AppExecFwk
}  // namespace OHOS
