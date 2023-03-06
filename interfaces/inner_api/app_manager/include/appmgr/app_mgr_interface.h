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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_INTERFACE_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "want.h"

#include "ability_info.h"
#include "application_info.h"
#include "app_record_id.h"
#include "bundle_info.h"
#include "iapp_state_callback.h"
#include "ams_mgr_interface.h"
#include "running_process_info.h"
#include "system_memory_attr.h"
#include "iapplication_state_observer.h"
#include "iconfiguration_observer.h"
#include "iquick_fix_callback.h"
#include "running_process_info.h"

namespace OHOS {
namespace AppExecFwk {
class IAppMgr : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.AppMgr");

    /**
     * AttachApplication, call AttachApplication() through proxy object,
     * get all the information needed to start the Application (data related to the Application ).
     *
     * @param app, information needed to start the Application.
     * @return
     */
    virtual void AttachApplication(const sptr<IRemoteObject> &app) = 0;

    /**
     * ApplicationForegrounded, call ApplicationForegrounded() through proxy object,
     * set the application to Foreground State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationForegrounded(const int32_t recordId) = 0;

    /**
     * ApplicationBackgrounded, call ApplicationBackgrounded() through proxy object,
     * set the application to Backgrounded State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationBackgrounded(const int32_t recordId) = 0;

    /**
     * ApplicationTerminated, call ApplicationTerminated() through proxy object,
     * terminate the application.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationTerminated(const int32_t recordId) = 0;

    /**
     * CheckPermission, call CheckPermission() through proxy object, check the permission.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @param permission, check the permissions.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int CheckPermission(const int32_t recordId, const std::string &permission) = 0;

    /**
     * AbilityCleaned,call through AbilityCleaned() proxy project, clean Ability record.
     *
     * @param token, a unique record that identifies AbilityCleaned from others.
     * @return
     */
    virtual void AbilityCleaned(const sptr<IRemoteObject> &token) = 0;

    /**
     * GetAmsMgr, call GetAmsMgr() through proxy object, get AMS interface instance.
     *
     * @return sptr<IAmsMgr>, return to AMS interface instance.
     */
    virtual sptr<IAmsMgr> GetAmsMgr() = 0;

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @return
     */
    virtual int32_t ClearUpApplicationData(const std::string &bundleName) = 0;

    /**
     * GetAllRunningProcesses, call GetAllRunningProcesses() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int GetAllRunningProcesses(std::vector<RunningProcessInfo> &info) = 0;

    /**
     * GetProcessRunningInfosByUserId, call GetProcessRunningInfosByUserId() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @param userId, user Id in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId) = 0;

    /**
     * GetProcessRunningInformation, call GetProcessRunningInformation() through proxy project.
     * Obtains information about current application process which is running on the device.
     *
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetProcessRunningInformation(RunningProcessInfo &info) = 0;

    /**
     * NotifyMemoryLevel, call NotifyMemoryLevel() through proxy project.
     * Notify abilities background the current memory level.
     *
     * @param level, the current memory level
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int NotifyMemoryLevel(int32_t level) = 0;

    /**
     * Notify that the ability stage has been updated
     * @param recordId, the app record.
     */
    virtual void AddAbilityStageDone(const int32_t recordId) = 0;

    /**
     * Start a resident process
     */
    virtual void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos) = 0;

    /**
     * Register application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
        const std::vector<std::string> &bundleNameList = {}) = 0;

    /**
     * Unregister application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer) = 0;

    /**
     * Get foreground applications.
     * @param list, foreground apps.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetForegroundApplications(std::vector<AppStateData> &list) = 0;

    /**
     * Start user test process.
     * @param want, want object.
     * @param observer, test observer remote object.
     * @param bundleInfo, bundle info.
     * @param userId the user id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUserTestProcess(
        const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId) = 0;

    /**
     * @brief Finish user test.
     * @param msg user test message.
     * @param resultCode user test result Code.
     * @param bundleName user test bundleName.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName) = 0;

    virtual void ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag) = 0;

    /**
     *  Get the token of ability records by process ID.
     *
     * @param pid The process id.
     * @param tokens The token of ability records.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens) = 0;
    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     *  Block app service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAppService() = 0;
    #endif

    /**
     * Prestart nwebspawn process.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PreStartNWebSpawnProcess() = 0;

    /**
     * Start nweb render process, called by nweb host.
     *
     * @param renderParam, params passed to renderprocess.
     * @param ipcFd, ipc file descriptior for web browser and render process.
     * @param sharedFd, shared memory file descriptior.
     * @param renderPid, created render pid.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartRenderProcess(const std::string &renderParam, int32_t ipcFd,
        int32_t sharedFd, pid_t &renderPid) = 0;

    /**
     * Render process call this to attach app manager service.
     *
     * @param renderScheduler, scheduler of render process.
     */
    virtual void AttachRenderProcess(const sptr<IRemoteObject> &renderScheduler) = 0;

    /**
     * Get render process termination status, called by nweb host.
     *
     * @param renderPid, target render pid.
     * @param status, termination status of the render process.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetRenderProcessTerminationStatus(pid_t renderPid, int &status) = 0;

    virtual int32_t GetConfiguration(Configuration& config) = 0;

    virtual int32_t UpdateConfiguration(const Configuration &config) = 0;

    virtual int32_t RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer) = 0;

    virtual int32_t UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer) = 0;

    /**
     * @brief Get the running state of application by bundle name.
     *
     * @param bundleName Bundle name
     * @return Returns true if process is running, false if process isn't running.
     */
    virtual bool GetAppRunningStateByBundleName(const std::string &bundleName) = 0;

    /**
     * @brief Notify application load patch.
     *
     * @param bundleName Bundle name
     * @param callback called when LoadPatch finished.
     * @return Returns 0 on success, error code on failure.
     */
    virtual int32_t NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) = 0;

    /**
     * @brief Notify application reload page.
     *
     * @param bundleName Bundle name
     * @param callback called when HotReload finished.
     * @return Returns 0 on success, error code on failure.
     */
    virtual int32_t NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) = 0;

    /**
     * @brief Notify application unload patch.
     *
     * @param bundleName Bundle name
     * @param callback called when UnloadPatch finished.
     * @return Returns 0 on success, error code on failure.
     */
    virtual int32_t NotifyUnLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) = 0;

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    /**
     * @brief Set whether the process is continuousTask.
     *
     * @param pid Process id.
     * @param isContinuousTask whether the process is continuousTask.
     * @return Returns 0 on success, error code on failure.
     */
    virtual int32_t SetContinuousTaskProcess(int32_t pid, bool isContinuousTask)
    {
        return 0;
    };
#endif

    /**
     * @brief Check whether the shared bundle is running.
     *
     * @param bundleName Shared bundle name.
     * @param versionCode Shared bundle version code.
     * @return Returns the shared bundle running result. The result is true if running, false otherwise.
     */
    virtual bool IsSharedBundleRunning(const std::string &bundleName, uint32_t versionCode) = 0;

    // please add new message item to the bottom in order to prevent some unexpected BUG
    enum class Message {
        APP_ATTACH_APPLICATION = 0,
        APP_APPLICATION_FOREGROUNDED,
        APP_APPLICATION_BACKGROUNDED,
        APP_APPLICATION_TERMINATED,
        APP_CHECK_PERMISSION,
        APP_ABILITY_CLEANED,
        APP_GET_MGR_INSTANCE,
        APP_CLEAR_UP_APPLICATION_DATA,
        APP_GET_ALL_RUNNING_PROCESSES,
        APP_GET_RUNNING_PROCESSES_BY_USER_ID,
        APP_ADD_ABILITY_STAGE_INFO_DONE,
        STARTUP_RESIDENT_PROCESS,
        REGISTER_APPLICATION_STATE_OBSERVER,
        UNREGISTER_APPLICATION_STATE_OBSERVER,
        GET_FOREGROUND_APPLICATIONS,
        START_USER_TEST_PROCESS,
        FINISH_USER_TEST,
        SCHEDULE_ACCEPT_WANT_DONE,
        BLOCK_APP_SERVICE,
        APP_GET_ABILITY_RECORDS_BY_PROCESS_ID,
        START_RENDER_PROCESS,
        ATTACH_RENDER_PROCESS,
        GET_RENDER_PROCESS_TERMINATION_STATUS,
        GET_CONFIGURATION,
        UPDATE_CONFIGURATION,
        REGISTER_CONFIGURATION_OBSERVER,
        UNREGISTER_CONFIGURATION_OBSERVER,
        APP_NOTIFY_MEMORY_LEVEL,
        GET_APP_RUNNING_STATE,
        NOTIFY_LOAD_REPAIR_PATCH,
        NOTIFY_HOT_RELOAD_PAGE,
        SET_CONTINUOUSTASK_PROCESS,
        NOTIFY_UNLOAD_REPAIR_PATCH,
        PRE_START_NWEBSPAWN_PROCESS,
        APP_GET_PROCESS_RUNNING_INFORMATION,
        IS_SHARED_BUNDLE_RUNNING,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_APP_MGR_INTERFACE_H
