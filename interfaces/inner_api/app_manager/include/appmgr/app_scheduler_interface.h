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

#ifndef OHOS_ABILITY_RUNTIME_APP_SCHEDULER_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_APP_SCHEDULER_INTERFACE_H

#include "iremote_broker.h"
#include "ability_info.h"
#include "app_launch_data.h"
#include "configuration.h"
#include "hap_module_info.h"
#include "iquick_fix_callback.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class IAppScheduler : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.AppScheduler");

    /**
     * ScheduleForegroundApplication, call ScheduleForegroundApplication() through proxy project,
     * Notify application to switch to foreground.
     *
     * @return
     */
    virtual void ScheduleForegroundApplication() = 0;

    /**
     * ScheduleBackgroundApplication, call ScheduleBackgroundApplication() through proxy project,
     * Notify application to switch to background.
     *
     * @return
     */
    virtual void ScheduleBackgroundApplication() = 0;

    /**
     * ScheduleTerminateApplication, call ScheduleTerminateApplication() through proxy project,
     * Notify application to terminate.
     *
     * @return
     */
    virtual void ScheduleTerminateApplication() = 0;

    /**
     * ScheduleShrinkMemory, call ScheduleShrinkMemory() through proxy project,
     * Notifies the application of the memory seen.
     *
     * @param The memory value.
     *
     * @return
     */
    virtual void ScheduleShrinkMemory(const int) = 0;

    /**
     * ScheduleLowMemory, call ScheduleLowMemory() through proxy project,
     * Notify application to low memory.
     *
     * @return
     */
    virtual void ScheduleLowMemory() = 0;

    /**
     * ScheduleMemoryLevel, call ScheduleMemoryLevel() through proxy project,
     * Notify applications background the current memory level.
     *
     * @return
     */
    virtual void ScheduleMemoryLevel(int32_t level) = 0;

    /**
     * ScheduleLaunchApplication, call ScheduleLaunchApplication() through proxy project,
     * Notify application to launch application.
     *
     * @param The app data value.
     *
     * @return
     */
    virtual void ScheduleLaunchApplication(const AppLaunchData &, const Configuration &) = 0;

    /**
     * ScheduleUpdateApplicationInfoInstalled, call ScheduleUpdateApplicationInfoInstalled() through proxy object,
     * update the application info after new module installed.
     *
     * @param appInfo The latest application info obtained from bms for update abilityRuntimeContext.
     *
     * @return
     */
    virtual void ScheduleUpdateApplicationInfoInstalled(const ApplicationInfo &) = 0;

    /**
     * ScheduleAbilityStageInfo, call ScheduleAbilityStageInfo() through proxy project,
     * Notify application to launch application.
     *
     * @param The app data value.
     *
     * @return
     */
    virtual void ScheduleAbilityStage(const HapModuleInfo &) = 0;

    virtual void ScheduleLaunchAbility(const AbilityInfo &, const sptr<IRemoteObject> &,
        const std::shared_ptr<AAFwk::Want> &want) = 0;

    /**
     * ScheduleCleanAbility, call ScheduleCleanAbility() through proxy project,
     * Notify application to clean ability.
     *
     * @param The ability token.
     * @return
     */
    virtual void ScheduleCleanAbility(const sptr<IRemoteObject> &) = 0;

    /**
     * ScheduleProfileChanged, call ScheduleProfileChanged() through proxy project,
     * Notify application to profile update.
     *
     * @param The profile data.
     * @return
     */
    virtual void ScheduleProfileChanged(const Profile &) = 0;

    /**
     * ScheduleConfigurationUpdated, call ScheduleConfigurationUpdated() through proxy project,
     * Notify application to configuration update.
     *
     * @param The configuration data.
     * @return
     */
    virtual void ScheduleConfigurationUpdated(const Configuration &config) = 0;

    /**
     * ScheduleProcessSecurityExit, call ScheduleProcessSecurityExit() through proxy project,
     * Notify application process exit safely.
     *
     * @return
     */
    virtual void ScheduleProcessSecurityExit() = 0;

    virtual void ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName) = 0;

    /**
     * @brief Notify application load patch.
     *
     * @param bundleName Bundle name
     * @param callback called when LoadPatch finished.
     * @return Returns 0 on success, error code on failure.
     */
    virtual int32_t ScheduleNotifyLoadRepairPatch(const std::string &bundleName,
        const sptr<IQuickFixCallback> &callback, const int32_t recordId) = 0;

    /**
     * @brief Notify application reload page.
     *
     * @param callback called when HotReload finished.
     * @return Returns 0 on success, error code on failure.
     */
    virtual int32_t ScheduleNotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId) = 0;

    /**
     * @brief Notify application unload patch.
     *
     * @param bundleName Bundle name
     * @param callback called when UnloadPatch finished.
     * @return Returns 0 on success, error code on failure.
     */
    virtual int32_t ScheduleNotifyUnLoadRepairPatch(const std::string &bundleName,
        const sptr<IQuickFixCallback> &callback, const int32_t recordId) = 0;

    enum class Message {
        SCHEDULE_FOREGROUND_APPLICATION_TRANSACTION = 0,
        SCHEDULE_BACKGROUND_APPLICATION_TRANSACTION,
        SCHEDULE_TERMINATE_APPLICATION_TRANSACTION,
        SCHEDULE_LOWMEMORY_APPLICATION_TRANSACTION,
        SCHEDULE_SHRINK_MEMORY_APPLICATION_TRANSACTION,
        SCHEDULE_LAUNCH_ABILITY_TRANSACTION,
        SCHEDULE_CLEAN_ABILITY_TRANSACTION,
        SCHEDULE_LAUNCH_APPLICATION_TRANSACTION,
        SCHEDULE_PROFILE_CHANGED_TRANSACTION,
        SCHEDULE_CONFIGURATION_UPDATED,
        SCHEDULE_PROCESS_SECURITY_EXIT_TRANSACTION,
        SCHEDULE_ABILITY_STAGE_INFO,
        SCHEDULE_ACCEPT_WANT,
        SCHEDULE_MEMORYLEVEL_APPLICATION_TRANSACTION,
        SCHEDULE_NOTIFY_LOAD_REPAIR_PATCH,
        SCHEDULE_NOTIFY_HOT_RELOAD_PAGE,
        SCHEDULE_NOTIFY_UNLOAD_REPAIR_PATCH,
        SCHEDULE_UPDATE_APPLICATION_INFO_INSTALLED,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_SCHEDULER_INTERFACE_H
