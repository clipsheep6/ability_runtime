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

#ifndef OHOS_ABILITY_RUNTIME_AMS_MGR_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_AMS_MGR_INTERFACE_H

#include "ability_debug_response_interface.h"
#include "ability_info.h"
#include "app_debug_listener_interface.h"
#include "app_record_id.h"
#include "application_info.h"
#include "configuration.h"
#include "iapp_state_callback.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "istart_specified_ability_response.h"
#include "running_process_info.h"

namespace OHOS {
namespace AppExecFwk {
class IAmsMgr : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.IAmsMgr");

    /**
     * LoadAbility, call LoadAbility() through proxy project, load the ability that needed to be started.
     *
     * @param token, the unique identification to start the ability.
     * @param preToken, the unique identification to call the ability.
     * @param abilityInfo, the ability information.
     * @param appInfo, the app information.
     * @return
     */
    virtual void LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
        const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo,
        const std::shared_ptr<AAFwk::Want> &want) = 0;

    /**
     * TerminateAbility, call TerminateAbility() through the proxy object, terminate the token ability.
     *
     * @param token, token, he unique identification to terminate the ability.
     * @param clearMissionFlag, indicates whether terminate the ability when clearMission.
     * @return
     */
    virtual void TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag) = 0;

    /**
     * UpdateAbilityState, call UpdateAbilityState() through the proxy object, update the ability status.
     *
     * @param token, the unique identification to update the ability.
     * @param state, ability status that needs to be updated.
     * @return
     */
    virtual void UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state) = 0;

    /**
     * UpdateExtensionState, call UpdateExtensionState() through the proxy object, update the extension status.
     *
     * @param token, the unique identification to update the extension.
     * @param state, extension status that needs to be updated.
     * @return
     */
    virtual void UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state) = 0;

    /**
     * RegisterAppStateCallback, call RegisterAppStateCallback() through the proxy object, register the callback.
     *
     * @param callback, Ams register the callback.
     * @return
     */
    virtual void RegisterAppStateCallback(const sptr<IAppStateCallback> &callback) = 0;

    /**
     * AbilityBehaviorAnalysis,call AbilityBehaviorAnalysis() through the proxy object,
     * ability behavior analysis assistant process optimization.
     *
     * @param token, the unique identification to start the ability.
     * @param preToken, the unique identification to call the ability.
     * @param visibility, the visibility information about windows info.
     * @param perceptibility, the Perceptibility information about windows info.
     * @param connectionState, the service ability connection state.
     * @return
     */
    virtual void AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
        const int32_t visibility, const int32_t perceptibility, const int32_t connectionState) = 0;

    /**
     * KillProcessByAbilityToken, call KillProcessByAbilityToken() through proxy object,
     * kill the process by ability token.
     *
     * @param token, the unique identification to the ability.
     * @return
     */
    virtual void KillProcessByAbilityToken(const sptr<IRemoteObject> &token) = 0;

    /**
     * KillProcessesByUserId, call KillProcessesByUserId() through proxy object,
     * kill the processes by userId.
     *
     * @param userId, the user id.
     * @return
     */
    virtual void KillProcessesByUserId(int32_t userId) = 0;

    /**
     * KillProcessWithAccount, call KillProcessWithAccount() through proxy object,
     * kill the process.
     *
     * @param bundleName, bundle name in Application record.
     * @param accountId, account ID.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int KillProcessWithAccount(const std::string &bundleName, const int accountId) = 0;

    /**
     * UpdateApplicationInfoInstalled, call UpdateApplicationInfoInstalled() through proxy object,
     * update the application info after new module installed.
     *
     * @param bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid) = 0;

    /**
     * KillApplication, call KillApplication() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int KillApplication(const std::string &bundleName) = 0;

    /**
     * KillApplicationByUid, call KillApplicationByUid() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  userId, userId.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int KillApplicationByUid(const std::string &bundleName, const int uid) = 0;

    /**
     * Kill the application self.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int KillApplicationSelf()
    {
        return ERR_OK;
    }

    virtual void AbilityAttachTimeOut(const sptr<IRemoteObject> &token) = 0;

    virtual void PrepareTerminate(const sptr<IRemoteObject> &token) = 0;

    virtual void GetRunningProcessInfoByToken(
        const sptr<IRemoteObject> &token, OHOS::AppExecFwk::RunningProcessInfo &info) = 0;

    virtual void GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) = 0;

    virtual void SetAbilityForegroundingFlagToAppRecord(const pid_t pid) = 0;

    virtual void StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo) = 0;

    virtual void RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response) = 0;

    virtual int GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug) = 0;

    /**
     * Set the current userId of appMgr.
     *
     * @param userId the user id.
     *
     * @return
     */
    virtual void SetCurrentUserId(const int32_t userId) = 0;

    /**
     * Get bundleName by pid.
     *
     * @param pid process id.
     * @param bundleName Output parameters, return bundleName.
     * @param uid Output parameters, return userId.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid) = 0;

    /**
     * @brief Register app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterAppDebugListener(const sptr<IAppDebugListener> &listener) = 0;

    /**
     * @brief Unregister app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener) = 0;

    /**
     * @brief Attach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t AttachAppDebug(const std::string &bundleName) = 0;

    /**
     * @brief Detach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t DetachAppDebug(const std::string &bundleName) = 0;

    /**
     * @brief Registering ability debug mode response.
     * @param response Response for ability debug object.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response) = 0;

    /**
     * @brief Determine whether it is an attachment debug application based on the bundle name.
     * @param bundleName The application bundle name.
     * @return Returns true if it is an attach debug application, otherwise it returns false.
     */
    virtual bool IsAttachDebug(const std::string &bundleName) = 0;

    enum class Message {
        LOAD_ABILITY = 0,
        TERMINATE_ABILITY,
        UPDATE_ABILITY_STATE,
        UPDATE_EXTENSION_STATE,
        REGISTER_APP_STATE_CALLBACK,
        ABILITY_BEHAVIOR_ANALYSIS,
        KILL_PEOCESS_BY_ABILITY_TOKEN,
        KILL_PROCESSES_BY_USERID,
        KILL_PROCESS_WITH_ACCOUNT,
        KILL_APPLICATION,
        ABILITY_ATTACH_TIMEOUT,
        PREPARE_TERMINATE_ABILITY,
        KILL_APPLICATION_BYUID,
        GET_RUNNING_PROCESS_INFO_BY_TOKEN,
        START_SPECIFIED_ABILITY,
        REGISTER_START_SPECIFIED_ABILITY_RESPONSE,
        UPDATE_CONFIGURATION,
        GET_CONFIGURATION,
        GET_APPLICATION_INFO_BY_PROCESS_ID,
        KILL_APPLICATION_SELF,
        GET_RUNNING_PROCESS_INFO_BY_PID,
        UPDATE_APPLICATION_INFO_INSTALLED,
        SET_CURRENT_USER_ID,
        Get_BUNDLE_NAME_BY_PID,
        SET_ABILITY_FOREGROUNDING_FLAG,
        REGISTER_APP_DEBUG_LISTENER,
        UNREGISTER_APP_DEBUG_LISTENER,
        ATTACH_APP_DEBUG,
        DETACH_APP_DEBUG,
        REGISTER_ABILITY_DEBUG_RESPONSE,
        IS_ATTACH_DEBUG,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_AMS_MGR_INTERFACE_H
