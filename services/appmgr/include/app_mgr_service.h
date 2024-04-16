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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_SERVICE_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_SERVICE_H

#include <list>
#include <string>
#include <vector>

#include "ability_info.h"
#include "ability_running_record.h"
#include "ams_mgr_scheduler.h"
#include "app_malloc_info.h"
#include "app_mgr_constants.h"
#include "app_mgr_service_event_handler.h"
#include "app_mgr_service_inner.h"
#include "app_mgr_stub.h"
#include "app_record_id.h"
#include "app_running_record.h"
#include "app_running_status_listener_interface.h"
#include "app_scheduler_proxy.h"
#include "appexecfwk_errors.h"
#include "application_info.h"
#include "if_system_ability_manager.h"
#include "nocopyable.h"
#include "system_ability.h"
#include "task_handler_wrap.h"
#include "app_jsheap_mem_info.h"

namespace OHOS {
namespace AppExecFwk {
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

struct AppMgrServiceState {
    ServiceRunningState serviceRunningState = ServiceRunningState::STATE_NOT_START;
    SpawnConnectionState connectionState = SpawnConnectionState::STATE_NOT_CONNECT;
};

class AMSEventHandler;

class AppMgrService : public SystemAbility, public AppMgrStub {
public:
    DECLEAR_SYSTEM_ABILITY(AppMgrService);

    AppMgrService();
    explicit AppMgrService(const int32_t serviceId, bool runOnCreate = false);
    virtual ~AppMgrService() override;

    // the function about application
    // attach the application to ams, then ams can control it.
    /**
     * AttachApplication, call AttachApplication() through proxy object,
     * get all the information needed to start the Application (data related to the Application ).
     *
     * @param app, information needed to start the Application.
     * @return
     */
    virtual void AttachApplication(const sptr<IRemoteObject> &app) override;

    // notify the ams update the state of an app, when it entered foreground.

    /**
     * ApplicationForegrounded, call ApplicationForegrounded() through proxy object,
     * set the application to Foreground State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationForegrounded(const int32_t recordId) override;

    /**
     * ApplicationBackgrounded, call ApplicationBackgrounded() through proxy object,
     * set the application to Backgrounded State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationBackgrounded(const int32_t recordId) override;

    /**
     * ApplicationTerminated, call ApplicationTerminated() through proxy object,
     * terminate the application.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationTerminated(const int32_t recordId) override;

    /**
     * AbilityCleaned,call through AbilityCleaned() proxy project, clean Ability record.
     *
     * @param token, a unique record that identifies AbilityCleaned from others.
     * @return
     */
    virtual void AbilityCleaned(const sptr<IRemoteObject> &token) override;

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ClearUpApplicationData(const std::string &bundleName,
        const int32_t userId = -1) override;

    /**
     * ClearUpApplicationDataBySelf, call ClearUpApplicationDataBySelf() through proxy project,
     * clear the application data by self.
     *
     * @param userId, user ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ClearUpApplicationDataBySelf(int32_t userId = -1) override;

    /**
     * GetAllRunningProcesses, call GetAllRunningProcesses() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetAllRunningProcesses(std::vector<RunningProcessInfo> &info) override;

    /**
     * GetAllRenderProcesses, call GetAllRenderProcesses() through proxy project.
     * Obtains information about render processes that are running on the device.
     *
     * @param info, render process info.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t GetAllRenderProcesses(std::vector<RenderProcessInfo> &info) override;

    /**
     * JudgeSandboxByPid, call JudgeSandboxByPid() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param pid, the pid of current app running record.
     * @param isSandbox, current app is or not a sandbox.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t JudgeSandboxByPid(pid_t pid, bool &isSandbox) override;

    /**
     * GetProcessRunningInfosByUserId, call GetProcessRunningInfosByUserId() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @param userId, userId.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId) override;

    /**
     * GetProcessRunningInformation, call GetProcessRunningInformation() through proxy project.
     * Obtains information about current application process which is running on the device.
     *
     * @param info, app name in Application record.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetProcessRunningInformation(RunningProcessInfo &info) override;

    /**
     * NotifyMemoryLevel, call NotifyMemoryLevel() through proxy project.
     * Notify applications background the current memory level.
     *
     * @param level, current memory level.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyMemoryLevel(int32_t level) override;

    /**
     * NotifyProcMemoryLevel, call NotifyMemoryLevel() through proxy project.
     * Notify applications the current memory level.
     *
     * @param  procLevelMap , <pid_t, MemoryLevel> map.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap) override;

    /**
     * DumpHeapMemory, call DumpHeapMemory() through proxy project.
     * Get the application's memory allocation info.
     *
     * @param pid, pid input.
     * @param mallocInfo, dynamic storage information output.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo) override;

    /**
     * Authenticate dump permissions
     *
     * @return Returns true on permission, others on false
     */
    bool HasDumpPermission() const;
    /**
     * DumpJsHeapMemory, call DumpJsHeapMemory() through proxy project.
     * triggerGC and dump the application's jsheap memory info.
     *
     * @param info, pid, tid, needGc, needSnapshot
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info) override;

    // the function about service running info
    /**
     * QueryServiceState, Query application service status.
     *
     * @return the application service status.
     */
    AppMgrServiceState QueryServiceState();

    /**
     * GetAmsMgr, call GetAmsMgr() through proxy object, get AMS interface instance.
     *
     * @return sptr<IAmsMgr>, return to AMS interface instance.
     */
    virtual sptr<IAmsMgr> GetAmsMgr() override;

    /**
     * Notify that the ability stage has been updated
     * @param recordId, the app record.
     */
    virtual void AddAbilityStageDone(const int32_t recordId) override;

    /**
     * Start all resident process
     */
    virtual void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos) override;

    /**
     * Start user test process.
     * @param want, want object.
     * @param observer, test observer remote object.
     * @param bundleInfo, bundle info.
     * @param userId the user id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUserTestProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
        const AppExecFwk::BundleInfo &bundleInfo, int32_t userId) override;

    /**
     * @brief Finish user test.
     * @param msg user test message.
     * @param resultCode user test result Code.
     * @param bundleName user test bundleName.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FinishUserTest(
        const std::string &msg, const int64_t &resultCode, const std::string &bundleName) override;

    /**
     * @brief Application hidumper.
     * @param fd Indicates the fd.
     * @param args Indicates the params.
     * @return Returns the dump result.
     */
    int Dump(int fd, const std::vector<std::u16string>& args) override;

    virtual void ScheduleAcceptWantDone(
        const int32_t recordId, const AAFwk::Want &want, const std::string &flag) override;

    virtual void ScheduleNewProcessRequestDone(
        const int32_t recordId, const AAFwk::Want &want, const std::string &flag) override;

    virtual int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens) override;

    virtual int PreStartNWebSpawnProcess() override;

    virtual int StartRenderProcess(const std::string &renderParam,
                                   int32_t ipcFd, int32_t sharedFd,
                                   int32_t crashFd, pid_t &renderPid) override;

    virtual void AttachRenderProcess(const sptr<IRemoteObject> &shceduler) override;

    virtual int GetRenderProcessTerminationStatus(pid_t renderPid, int &status) override;

    virtual int32_t GetConfiguration(Configuration& config) override;

    virtual int32_t UpdateConfiguration(const Configuration &config) override;

    virtual int32_t UpdateConfigurationByBundleName(const Configuration &config, const std::string &name) override;

    virtual int32_t RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer) override;

    virtual int32_t UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer) override;

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Block app service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAppService() override;
    #endif

    bool GetAppRunningStateByBundleName(const std::string &bundleName) override;

    int32_t NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) override;

    int32_t NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) override;

    int32_t NotifyUnLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) override;

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    int32_t SetContinuousTaskProcess(int32_t pid, bool isContinuousTask) override;
#endif

    /**
     * @brief Check whether the shared bundle is running.
     *
     * @param bundleName Shared bundle name.
     * @param versionCode Shared bundle version code.
     * @return Returns the shared bundle running result. The result is true if running, false otherwise.
     */
    virtual bool IsSharedBundleRunning(const std::string &bundleName, uint32_t versionCode) override;

    virtual int32_t StartNativeProcessForDebugger(const AAFwk::Want &want) override;

    /**
     * Get bundleName by pid.
     *
     * @param pid process id.
     * @param bundleName Output parameters, return bundleName.
     * @param uid Output parameters, return userId.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetBundleNameByPid(const int32_t pid, std::string &bundleName, int32_t &uid) override;

    /**
     * Notify Fault Data
     *
     * @param faultData the fault data.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyAppFault(const FaultData &faultData) override;

    /**
     * Notify Fault Data By SA
     *
     * @param faultData the fault data notified by SA.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyAppFaultBySA(const AppFaultDataBySA &faultData) override;

    /**
     * get memorySize by pid.
     *
     * @param pid process id.
     * @param memorySize Output parameters, return memorySize in KB.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetProcessMemoryByPid(const int32_t pid, int32_t &memorySize) override;

    /**
     * get application processes information list by bundleName.
     *
     * @param bundleName Bundle name.
     * @param userId user Id in Application record.
     * @param info Output parameters, return running process info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetRunningProcessInformation(
        const std::string &bundleName, int32_t userId, std::vector<RunningProcessInfo> &info) override;

    /**
     * on add systemAbility.
     *
     * @return
     */
    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    /**
     * on remove systemAbility.
     *
     * @return
     */
    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    /**
     * @brief Notify NativeEngine GC of status change.
     *
     * @param state GC state
     * @param pid pid
     *
     * @return Is the status change completed.
     */
    virtual int32_t ChangeAppGcState(pid_t pid, int32_t state) override;

    /**
     * Register appRunning status listener.
     *
     * @param listener Running status listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener) override;

    /**
     * Unregister appRunning status listener.
     *
     * @param listener Running status listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener) override;

    /**
     * Register application foreground state observer.
     * @param observer Is App Foreground Statue Observer
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer) override;

    /**
     * Unregister application foreground state observer.
     * @param observer Is App Foreground Statue Observer
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer) override;

    /**
     * Start child process, called by ChildProcessManager.
     *
     * @param srcEntry Child process source file entrance path to be started.
     * @param childPid Created child process pid.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t StartChildProcess(const std::string &srcEntry, pid_t &childPid) override;

    /**
     * Get child process record for self.
     *
     * @return child process info.
     */
    int32_t GetChildProcessInfoForSelf(ChildProcessInfo &info) override;

    /**
     * Attach child process scheduler to app manager service.
     *
     * @param childScheduler scheduler of child process.
     */
    void AttachChildProcess(const sptr<IRemoteObject> &childScheduler) override;

    /**
     * Exit child process, called by itself.
     */
    void ExitChildProcessSafely() override;

    int32_t RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer) override;

    int32_t UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer) override;

    int32_t UpdateRenderState(pid_t renderPid, int32_t state) override;

    int32_t SignRestartAppFlag(const std::string &bundleName) override;

    int32_t GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId) override;

    int32_t GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids) override;

    int32_t GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids) override;

    int32_t NotifyMemonySizeStateChanged(bool isMemorySizeSufficent) override;

private:
    /**
     * Init, Initialize application services.
     *
     * @return ERR_OK, return back success, others fail.
     */
    ErrCode Init();

    // the function that overrode from SystemAbility
    /**
     * OnStart, Start application service.
     *
     * @return
     */
    virtual void OnStart() override;

    /**
     * OnStop, Stop application service.
     *
     * @return
     */
    virtual void OnStop() override;

    /**
     * @brief Judge whether the application service is ready.
     *
     * @return Returns true means service is ready, otherwise service is not ready.
     */
    bool IsReady() const;

    /**
     * SetInnerService, Setting application service Inner instance.
     *
     * @return
     */
    void SetInnerService(const std::shared_ptr<AppMgrServiceInner> &innerService);

    /**
     * Register application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
        const std::vector<std::string> &bundleNameList = {}) override;

    /**
     * Unregister application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer) override;

    /**
     * Register application or process state observer.
     * @param observer, Is ability foreground state observer
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer) override;

    /**
     * Unregister application or process state observer.
     * @param observer, Is ability foreground state observer
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer) override;

    /**
     * Get foreground applications.
     * @param list, foreground apps.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetForegroundApplications(std::vector<AppStateData> &list) override;

    int Dump(const std::vector<std::u16string>& args, std::string& result);
    void ShowHelp(std::string& result) const;
    int DumpIpc(const std::vector<std::u16string>& args, std::string& result);
    int DumpIpcAllStart(std::string& result);
    int DumpIpcAllStop(std::string& result);
    int DumpIpcAllStat(std::string& result);
    int DumpIpcStart(const int32_t pid, std::string& result);
    int DumpIpcStop(const int32_t pid, std::string& result);
    int DumpIpcStat(const int32_t pid, std::string& result);

    bool JudgeAppSelfCalled(int32_t recordId);

    /**
     * @brief Notify AbilityManagerService the page show.
     * @param token Ability identify.
     * @param pageStateData The data of ability's page state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyPageShow(const sptr<IRemoteObject> &token, const PageStateData &pageStateData) override;

    /**
     * @brief Notify AbilityManagerService the page hide.
     * @param token Ability identify.
     * @param pageStateData The data of ability's page state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyPageHide(const sptr<IRemoteObject> &token, const PageStateData &pageStateData) override;

    /**
     * Check whether the bundle is running.
     *
     * @param bundleName Indicates the bundle name of the bundle.
     * @param isRunning Obtain the running status of the application, the result is true if running, false otherwise.
     * @return Return ERR_OK if success, others fail.
     */
    int32_t IsApplicationRunning(const std::string &bundleName, bool &isRunning) override;

    /**
     * Whether the current application process is the last surviving process.
     *
     * @return Returns true is final application process, others return false.
     */
    bool IsFinalAppProcess() override;

    enum DumpIpcKey {
        KEY_DUMP_IPC_START = 0,
        KEY_DUMP_IPC_STOP,
        KEY_DUMP_IPC_STAT,
    };

private:
    void DumpIpcAllFuncInit();
    void DumpIpcFuncInit();
    int DumpIpcAllInner(const AppMgrService::DumpIpcKey key, std::string& result);
    int DumpIpcWithPidInner(const AppMgrService::DumpIpcKey key,
        const std::string& optionPid, std::string& result);

private:
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner_;
    AppMgrServiceState appMgrServiceState_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AMSEventHandler> eventHandler_;
    sptr<ISystemAbilityManager> systemAbilityMgr_;
    sptr<IAmsMgr> amsMgrScheduler_;

    const static std::map<std::string, AppMgrService::DumpIpcKey> dumpIpcMap;

    using DumpIpcAllFuncType = int (AppMgrService::*)(std::string& result);
    std::map<uint32_t, DumpIpcAllFuncType> dumpIpcAllFuncMap_;

    using DumpIpcFuncType = int (AppMgrService::*)(const int32_t pid, std::string& result);
    std::map<uint32_t, DumpIpcFuncType> dumpIpcFuncMap_;

    DISALLOW_COPY_AND_MOVE(AppMgrService);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_MGR_SERVICE_H
