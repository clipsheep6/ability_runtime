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

#ifndef OHOS_AAFWK_CONNECTION_STATE_MANAGER_H
#define OHOS_AAFWK_CONNECTION_STATE_MANAGER_H

#include <mutex>
#include <unordered_map>

#include "application_state_observer_stub.h"
#include "connection_state_item.h"
#include "connection_observer_controller.h"
#include "dlp_connection_info.h"
#include "dlp_state_item.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class ConnectionStateManager
 * ConnectionStateManager manage connection states.
 */
class ConnectionStateManager : public std::enable_shared_from_this<ConnectionStateManager> {
    DECLARE_DELAYED_SINGLETON(ConnectionStateManager)
public:
    /**
     * Get process name of a pid.
     *
     * @param pid target pid.
     * @return process name of target pid.
     */
    static std::string GetProcessNameByPid(int32_t pid);

    /**
     * init manager.
     *
     */
    void Init();

    /**
     * register connection state observer.
     *
     * @param observer callback of client.
     * @return Returns ERR_OK if success.
     */
    int RegisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer);

    /**
     * unregister connection state observer.
     *
     * @param observer callback of client.
     * @return Returns ERR_OK if success.
     */
    int UnregisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer);

    /**
     * add an connection to manager.
     *
     * @param connectionRecord connection record info.
     */
    void AddConnection(const std::shared_ptr<ConnectionRecord> &connectionRecord);

    /**
     * remove an connection.
     *
     * @param connectionRecord connection record info.
     * @param isCallerDied whether caller was died.
     */
    void RemoveConnection(const std::shared_ptr<ConnectionRecord> &connectionRecord, bool isCallerDied);

    /**
     * add a data ability acquired information to manager.
     *
     * @param caller caller of data ability.
     * @param record target data ability.
     */
    void AddDataAbilityConnection(const DataAbilityCaller &caller,
        const std::shared_ptr<DataAbilityRecord> &record);

    /**
     * remove a data ability acquired information from manager.
     *
     * @param caller caller of data ability.
     * @param record target data ability.
     */
    void RemoveDataAbilityConnection(const DataAbilityCaller &caller,
        const std::shared_ptr<DataAbilityRecord> &record);

    /**
     * handle when data ability was died.
     *
     * @param record target data ability.
     */
    void HandleDataAbilityDied(const std::shared_ptr<DataAbilityRecord> &record);

    /**
     * handle when data ability caller was died.
     *
     * @param callerPid caller pid of data ability.
     */
    void HandleDataAbilityCallerDied(int32_t callerPid);

    /**
     * add dlp manager to manager.
     *
     * @param dlpManger dlp manager record.
     */
    void AddDlpManager(const std::shared_ptr<AbilityRecord> &dlpManger);

    /**
     * remove dlp manager from manager.
     *
     * @param dlpManger dlp manager record.
     */
    void RemoveDlpManager(const std::shared_ptr<AbilityRecord> &dlpManger);

    /**
     * a dlp ability was started.
     *
     * @param dlpAbility dlp manager record.
     */
    void AddDlpAbility(const std::shared_ptr<AbilityRecord> &dlpAbility);

    /**
     * a dlp ability was terminated.
     *
     * @param dlpAbility dlp manager record.
     */
    void RemoveDlpAbility(const std::shared_ptr<AbilityRecord> &dlpAbility);

    /**
     * handle app process died.
     *
     * @param pid app process pid.
     */
    void HandleAppDied(int32_t pid);

    /**
     * get exist dlp connection infos.
     *
     * @param infos output dlp connection result.
     */
    void GetDlpConnectionInfos(std::vector<AbilityRuntime::DlpConnectionInfo> &infos);

private:
    bool CheckDataAbilityConnectionParams(const DataAbilityCaller &caller,
        const std::shared_ptr<DataAbilityRecord> &record) const;

private:
    class InnerAppStateObserver : public AppExecFwk::ApplicationStateObserverStub {
    public:
        using ProcessDiedHandler = std::function<void(int32_t)>;
        explicit InnerAppStateObserver(const ProcessDiedHandler handler) : handler_(handler) {}
        ~InnerAppStateObserver() = default;
        void OnForegroundApplicationChanged(const AppExecFwk::AppStateData &appStateData) {}
        void OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) {}
        void OnExtensionStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) {}
        void OnProcessCreated(const AppExecFwk::ProcessData &processData) {}
        void OnProcessStateChanged(const AppExecFwk::ProcessData &processData) {}
        void OnApplicationStateChanged(const AppExecFwk::AppStateData &appStateData) {}
        void OnProcessDied(const AppExecFwk::ProcessData &processData)
        {
            if (handler_) {
                handler_(processData.pid);
            }
        }

    private:
        ProcessDiedHandler handler_;
    };

    bool AddConnectionInner(const std::shared_ptr<ConnectionRecord> &connectionRecord,
        AbilityRuntime::ConnectionData &data);
    bool RemoveConnectionInner(const std::shared_ptr<ConnectionRecord> &connectionRecord,
        AbilityRuntime::ConnectionData &data);
    bool AddDataAbilityConnectionInner(const DataAbilityCaller &caller,
        const std::shared_ptr<DataAbilityRecord> &record, AbilityRuntime::ConnectionData &data);
    bool RemoveDataAbilityConnectionInner(const DataAbilityCaller &caller,
        const std::shared_ptr<DataAbilityRecord> &record, AbilityRuntime::ConnectionData &data);
    void HandleCallerDied(int32_t callerPid);
    std::shared_ptr<ConnectionStateItem> RemoveDiedCaller(int32_t callerPid);
    void HandleDataAbilityDiedInner(const sptr<IRemoteObject> &abilityToken,
        std::vector<AbilityRuntime::ConnectionData> &allData);
    bool HandleDlpAbilityInner(const std::shared_ptr<AbilityRecord> &dlpAbility,
        bool isAdd, AbilityRuntime::DlpStateData &dlpData);
    void InitAppStateObserver();

private:
    std::shared_ptr<ConnectionObserverController> observerController_;

    std::recursive_mutex stateLock_;
    std::unordered_map<int32_t, std::shared_ptr<ConnectionStateItem>> connectionStates_;

    std::recursive_mutex dlpLock_;
    std::unordered_map<int32_t, std::shared_ptr<DlpStateItem>> dlpItems_;

    sptr<InnerAppStateObserver> appStateObserver_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_CONNECTION_STATE_MANAGER_H
