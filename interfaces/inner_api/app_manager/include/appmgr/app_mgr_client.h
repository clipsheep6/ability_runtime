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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_CLIENT_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_CLIENT_H

#include "ability_debug_response_interface.h"
#include "ability_info.h"
#include "app_debug_listener_interface.h"
#include "app_malloc_info.h"
#include "app_mem_info.h"
#include "app_mgr_constants.h"
#include "app_mgr_interface.h"
#include "app_running_status_listener_interface.h"
#include "application_info.h"
#include "bundle_info.h"
#include "fault_data.h"
#include "iapplication_state_observer.h"
#include "iapp_state_callback.h"
#include "iconfiguration_observer.h"
#include "iremote_object.h"
#include "irender_scheduler.h"
#include "irender_state_observer.h"
#include "istart_specified_ability_response.h"
#include "refbase.h"
#include "render_process_info.h"
#include "running_process_info.h"
#include "system_memory_attr.h"
#include "want.h"
#include "app_jsheap_mem_info.h"

namespace OHOS {
namespace AppExecFwk {
class AppServiceManager;
class Configuration;
class AppMgrRemoteHolder;
class AppMgrClient {
public:
    AppMgrClient();
    virtual ~AppMgrClient();

    /**
     * Load ability.
     *
     * @param token Ability identify.
     * @param preToken Ability identify of caller.
     * @param abilityInfo Ability information.
     * @param appInfo Application information.
     * @param want Want.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode LoadAbility(sptr<IRemoteObject> token, sptr<IRemoteObject> preToken,
        const AbilityInfo &abilityInfo, const ApplicationInfo &appInfo, const AAFwk::Want &want,
        int32_t abilityRecordId);

    /**
     * Terminate ability.
     *
     * @param token Ability identify.
     * @param clearMissionFlag, indicates whether terminate the ability when clearMission.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag);

    /**
     * Update ability state.
     *
     * @param token Ability identify.
     * @param state Ability running state.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state);

    /**
     * UpdateExtensionState, call UpdateExtensionState() through the proxy object, update the extension status.
     *
     * @param token, the unique identification to update the extension.
     * @param state, extension status that needs to be updated.
     * @return
     */
    virtual AppMgrResultCode UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state);

    /**
     * Register Application state callback.
     *
     * @param callback IAppStateCallback
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode RegisterAppStateCallback(const sptr<IAppStateCallback> &callback);

    /**
     * Connect service.
     *
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode ConnectAppMgrService();

    /**
     * AbilityBehaviorAnalysis, ability behavior analysis assistant process optimization.
     *
     * @param token, the unique identification to start the ability.
     * @param preToken, the unique identification to call the ability.
     * @param visibility, the visibility information about windows info.
     * @param perceptibility, the Perceptibility information about windows info.
     * @param connectionState, the service ability connection state.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token,
        const sptr<IRemoteObject> &preToken, const int32_t visibility, const int32_t perceptibility,
        const int32_t connectionState);

    /**
     * KillProcessByAbilityToken, call KillProcessByAbilityToken() through proxy object,
     * kill the process by ability token.
     *
     * @param token, the unique identification to the ability.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode KillProcessByAbilityToken(const sptr<IRemoteObject> &token);

    /**
     * KillProcessesByUserId, call KillProcessesByUserId() through proxy object,
     * kill the processes by user id.
     *
     * @param userId, the user id.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode KillProcessesByUserId(int32_t userId);

    /**
     * UpdateApplicationInfoInstalled, call UpdateApplicationInfoInstalled() through proxy object,
     * update the application info after new module installed.
     *
     * @param bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid);

    /**
     * KillApplication, call KillApplication() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @return ERR_OK, return back success, others fail.
     */
    virtual AppMgrResultCode KillApplication(const std::string &bundleName);

    /**
     * KillApplication, call KillApplication() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @return ERR_OK, return back success, others fail.
     */
    virtual AppMgrResultCode KillApplicationByUid(const std::string &bundleName, const int uid);

    /**
     * Kill the application self.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual AppMgrResultCode KillApplicationSelf();

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @param userId, the user id.
     * @return
     */
    virtual AppMgrResultCode ClearUpApplicationData(const std::string &bundleName,
        const int32_t userId = -1);

    /**
     * ClearUpApplicationDataBySelf, call ClearUpApplicationDataBySelf() through proxy project,
     * clear the application data.
     *
     * @param userId, the user id.
     * @return
     */
    virtual AppMgrResultCode ClearUpApplicationDataBySelf(int32_t userId = -1);

    /**
     * GetAllRunningProcesses, call GetAllRunningProcesses() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetAllRunningProcesses(std::vector<RunningProcessInfo> &info);

     /**
     * GetProcessRunningInfosByUserId, call GetProcessRunningInfosByUserId() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @param userId, user Id in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId);

     /**
     * GetProcessRunningInformation, call GetProcessRunningInformation() through proxy project.
     * Obtains information about current application processes which is running on the device.
     *
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetProcessRunningInformation(RunningProcessInfo &info);

    /**
     * GetAllRenderProcesses, call GetAllRenderProcesses() through proxy project.
     * Obtains information about render processes that are running on the device.
     *
     * @param info, render process info.
     * @return ERR_OK, return back success, others fail.
     */
    virtual AppMgrResultCode GetAllRenderProcesses(std::vector<RenderProcessInfo> &info);

    /**
     * NotifyMemoryLevel, call NotifyMemoryLevel() through proxy project.
     * Notify abilities background the current memory level.
     *
     * @param level, the current memory level
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode NotifyMemoryLevel(MemoryLevel level);

    /**
     * DumpHeapMemory, call DumpHeapMemory() through proxy project.
     * Get the application's memory allocation info.
     *
     * @param pid, pid input.
     * @param mallocInfo, dynamic storage information output.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo);

    /**
     * DumpJsHeapMemory, call DumpJsHeapMemory() through proxy project.
     * triggerGC and dump the application's jsheap memory info.
     *
     * @param info, pid tid needGc needSnapshot
     * @return ERR_OK ,return back success, others fail.
     */
    virtual AppMgrResultCode DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info);

    /**
     * GetConfiguration
     *
     * @param info, configuration.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetConfiguration(Configuration& config);

    /**
     * Ability attach timeout. If start ability encounter failure, attach timeout to terminate.
     *
     * @param token Ability identify.
     */
    virtual void AbilityAttachTimeOut(const sptr<IRemoteObject> &token);

    /**
     * Prepare terminate.
     *
     * @param token Ability identify.
     */
    virtual void PrepareTerminate(const sptr<IRemoteObject> &token);

    /**
     * Get running process information by ability token.
     *
     * @param token Ability identify.
     * @param info Running process info.
     */
    virtual void GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info);

    /**
     * Get running process information by pid.
     *
     * @param token Process id.
     * @param info Running process info.
     */
    virtual void GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) const;

    /**
     * Notify that the ability stage has been updated
     * @param recordId, the app record.
     */
    virtual void AddAbilityStageDone(const int32_t recordId);

    /**
     * Start a resident process
     */
    virtual void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos);

     /**
     *  ANotify application update system environment changes.
     *
     * @param config System environment change parameters.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual AppMgrResultCode UpdateConfiguration(const Configuration &config);

    /**
     * Register configuration observer.
     *
     * @param observer Configuration observer. When configuration changed, observer will be called.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer);

    /**
     * Unregister configuration observer.
     *
     * @param observer Configuration observer.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer);

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Block app service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAppService();
    #endif

    /**
     * Start a user test
     */
    virtual int StartUserTestProcess(
        const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId);

    /**
     * @brief Finish user test.
     * @param msg user test message.
     * @param resultCode user test result Code.
     * @param bundleName user test bundleName.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName);

    /**
     * Start specified ability.
     *
     * @param want Want contains information wish to start.
     * @param abilityInfo Ability information.
     */
    virtual void StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo);

    /**
     * Register response of start specified ability.
     *
     * @param response Response of start specified ability.
     */
    virtual void RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response);

    /**
     * Start specified process.
     *
     * @param want Want contains information wish to start.
     * @param abilityInfo Ability information.
     */
    virtual void StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo);

    /**
     * Schedule new process request done.
     *
     * @param recordId Application record.
     * @param want Want.
     * @param flag flag get from OnNewProcessRequest.
     */
    virtual void ScheduleNewProcessRequest(const int32_t recordId, const AAFwk::Want &want, const std::string &flag);

    /**
     * Schedule accept want done.
     *
     * @param recordId Application record.
     * @param want Want.
     * @param flag flag get from OnAcceptWant.
     */
    virtual void ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag);

    /**
     *  Get the token of ability records by process ID.
     *
     * @param pid The process id.
     * @param tokens The token of ability records.
     * @return Returns true on success, others on failure.
     */
    virtual int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens);

    /**
     * Prestart nwebspawn process.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PreStartNWebSpawnProcess();

    /**
     * Start nweb render process, called by nweb host.
     *
     * @param renderParam, params passed to renderprocess.
     * @param ipcFd, ipc file descriptior for web browser and render process.
     * @param sharedFd, shared memory file descriptior.
     * @param crashFd, crash signal file descriptior.
     * @param renderPid, created render pid.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartRenderProcess(const std::string &renderParam,
                                   int32_t ipcFd, int32_t sharedFd,
                                   int32_t crashFd, pid_t &renderPid);

    /**
     * Render process call this to attach app manager service.
     *
     * @param renderScheduler, scheduler of render process.
     */
    virtual void AttachRenderProcess(const sptr<IRenderScheduler> &renderScheduler);

    /**
     * Get render process termination status, called by nweb host.
     *
     * @param renderPid, target render pid.
     * @param status, termination status of the render process.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetRenderProcessTerminationStatus(pid_t renderPid, int &status);

    /**
     * Get application info by process id.
     *
     * @param pid Process id.
     * @param application Application information got.
     * @param debug Whether IsDebugApp.
     * @return Returns ERR_OK on success, others on failure.
     */
    int GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug);

    /**
     * Get application info by process id.
     *
     * @param pid Process id.
     * @param application Application information got.
     * @param debug Whether IsDebugApp.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t StartNativeProcessForDebugger(const AAFwk::Want &want) const;

    /**
     * Set the current userId of appMgr.
     *
     * @param userId the user id.
     *
     * @return
     */
    void SetCurrentUserId(const int32_t userId);

    /**
     * Get bundleName by pid.
     *
     * @param pid process id.
     * @param bundleName Output parameters, return bundleName.
     * @param uid Output parameters, return userId.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid);

    /**
     * Notify Fault Data
     *
     * @param faultData the fault data.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyAppFault(const FaultData &faultData);

    /**
     * Notify App Fault Data By SA
     *
     * @param faultData the fault data notified by SA.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyAppFaultBySA(const AppFaultDataBySA &faultData);

    /**
     * Set AbilityForegroundingFlag of an app-record to true.
     *
     * @param pid, pid.
     *
     */
    void SetAbilityForegroundingFlagToAppRecord(const pid_t pid) const;

    /**
     * @brief Notify NativeEngine GC of status change.
     *
     * @param state GC state
     * @param pid pid
     * @return Is the status change completed..
     */
    int32_t ChangeAppGcState(pid_t pid, int32_t state);

     /**
     * @brief Register app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAppDebugListener(const sptr<IAppDebugListener> &listener);

    /**
     * @brief Unregister app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener);

    /**
     * @brief Attach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t AttachAppDebug(const std::string &bundleName);

    /**
     * @brief Detach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t DetachAppDebug(const std::string &bundleName);

    /**
     * @brief Registering ability Debug Mode response.
     * @param abilityResponse Response of ability debug object.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response);

    /**
     * @brief Determine whether it is an attachment debug application based on the bundle name.
     * @param bundleName The application bundle name.
     * @return Returns true if it is an attach debug application, otherwise it returns false.
     */
    bool IsAttachDebug(const std::string &bundleName);

    /**
     * Set application assertion pause state.
     *
     * @param pid App process pid.
     * @param flag assertion pause state.
     */
    void SetAppAssertionPauseState(int32_t pid, bool flag);

    /**
     * Register application or process state observer.
     * @param observer, ability token.
     * @param bundleNameList, the list of bundle names.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
        const std::vector<std::string> &bundleNameList = {});

    /**
     * Unregister application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer);

    /**
     * @brief Notify AbilityManagerService the page show.
     * @param token Ability identify.
     * @param pageStateData The data of ability's page state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyPageShow(const sptr<IRemoteObject> &token, const PageStateData &pageStateData);

    /**
     * @brief Notify AbilityManagerService the page hide.
     * @param token Ability identify.
     * @param pageStateData The data of ability's page state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyPageHide(const sptr<IRemoteObject> &token, const PageStateData &pageStateData);

    /**
     * Register appRunning status listener.
     *
     * @param listener Running status listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener);

    /**
     * Unregister appRunning status listener.
     *
     * @param listener Running status listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener);

    /**
     * Whether the current application process is the last surviving process.
     *
     * @return Returns true is final application process, others return false.
     */
    bool IsFinalAppProcess();

    /**
     * To clear the process by ability token.
     *
     * @param token the unique identification to the ability.
     */
    void ClearProcessByToken(sptr<IRemoteObject> token) const;

    int32_t RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer);

    int32_t UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer);

    int32_t UpdateRenderState(pid_t renderPid, int32_t state);

    int32_t GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId);
private:
    void SetServiceManager(std::unique_ptr<AppServiceManager> serviceMgr);
    /**
     * This function is implemented for the Unittests only.
     *
     * @return Returns private variable remote_ of the inner class aka "AppMgrRemoteHolder".
     */
    sptr<IRemoteObject> GetRemoteObject();

    bool IsAmsServiceReady();

private:
    std::shared_ptr<AppMgrRemoteHolder> mgrHolder_;
    sptr<IAmsMgr> amsService_ {};
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_MGR_CLIENT_H
