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

#include "connection_state_manager.h"

#include <fstream>

#include "app_mgr_interface.h"
#include "connection_observer_errors.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
namespace {
static const int MAX_PROCESS_LEN = 256;
OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetAppMgr()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        return nullptr;
    }
    OHOS::sptr<OHOS::IRemoteObject> object = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(object);
}
}
using namespace OHOS::AbilityRuntime;

ConnectionStateManager::ConnectionStateManager() {}

ConnectionStateManager::~ConnectionStateManager() {}

std::string ConnectionStateManager::GetProcessNameByPid(int32_t pid)
{
    char path[MAX_PROCESS_LEN] = { 0 };
    if (snprintf_s(path, MAX_PROCESS_LEN, MAX_PROCESS_LEN - 1, "/proc/%d/cmdline", pid) <= 0) {
        return "";
    }
    std::ifstream file(path);
    std::string name = "";
    getline(file, name);
    return name;
}

void ConnectionStateManager::Init()
{
    if (!observerController_) {
        observerController_ = std::make_shared<ConnectionObserverController>();
    }
    InitAppStateObserver();
}

int ConnectionStateManager::RegisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    if (!observerController_) {
        return ERR_SERVICE_NOT_INIT;
    }

    return observerController_->AddObserver(observer);
}

int ConnectionStateManager::UnregisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    if (!observerController_) {
        return ERR_SERVICE_NOT_INIT;
    }
    observerController_->RemoveObserver(observer);

    return 0;
}

void ConnectionStateManager::AddConnection(const std::shared_ptr<ConnectionRecord> &connectionRecord)
{
    std::shared_ptr<ConnectionObserverController> controller = observerController_;
    if (!controller) {
        return;
    }

    if (!connectionRecord) {
        HILOG_ERROR("connection record is invalid");
        return;
    }

    ConnectionData connectionData;
    if (!AddConnectionInner(connectionRecord, connectionData)) {
        HILOG_WARN("add connection, no need to notify observers");
        return;
    }
    controller->NotifyExtensionConnected(connectionData);
}

void ConnectionStateManager::RemoveConnection(const std::shared_ptr<ConnectionRecord> &connectionRecord,
    bool isCallerDied)
{
    std::shared_ptr<ConnectionObserverController> controller = observerController_;
    if (!controller) {
        return;
    }

    if (!connectionRecord) {
        HILOG_ERROR("connection record is invalid when remove connection");
        return;
    }

    // if caller died, notify at once.
    if (isCallerDied) {
        HandleCallerDied(connectionRecord->GetCallerPid());
        return;
    }

    ConnectionData connectionData;
    if (!RemoveConnectionInner(connectionRecord, connectionData)) {
        HILOG_WARN("remove connection, no need to notify observers");
        return;
    }
    controller->NotifyExtensionDisconnected(connectionData);
}

void ConnectionStateManager::AddDataAbilityConnection(const DataAbilityCaller &caller,
    const std::shared_ptr<DataAbilityRecord> &record)
{
    if (!CheckDataAbilityConnectionParams(caller, record)) {
        return;
    }

    ConnectionData connectionData;
    if (!AddDataAbilityConnectionInner(caller, record, connectionData)) {
        HILOG_WARN("add data ability onnection, no need to notify observers");
        return;
    }
    observerController_->NotifyExtensionConnected(connectionData);
}

void ConnectionStateManager::RemoveDataAbilityConnection(const DataAbilityCaller &caller,
    const std::shared_ptr<DataAbilityRecord> &record)
{
    if (!CheckDataAbilityConnectionParams(caller, record)) {
        return;
    }

    ConnectionData connectionData;
    if (!RemoveDataAbilityConnectionInner(caller, record, connectionData)) {
        HILOG_WARN("remove data ability, no need to notify observers");
        return;
    }
    observerController_->NotifyExtensionDisconnected(connectionData);
}

bool ConnectionStateManager::CheckDataAbilityConnectionParams(const DataAbilityCaller &caller,
    const std::shared_ptr<DataAbilityRecord> &record) const
{
    if (!observerController_) {
        return false;
    }

    if (!record) {
        HILOG_ERROR("data ability record is invalid");
        return false;
    }

    if (caller.callerPid == 0) {
        HILOG_ERROR("data ability, invalid caller pid");
        return false;
    }

    return true;
}

void ConnectionStateManager::HandleDataAbilityDied(const std::shared_ptr<DataAbilityRecord> &record)
{
    if (!record) {
        HILOG_ERROR("invalid data ability.");
        return;
    }

    auto token = record->GetToken();
    if (!token) {
        HILOG_ERROR("invalid data ability token.");
        return;
    }

    std::vector<AbilityRuntime::ConnectionData> allData;
    HandleDataAbilityDiedInner(token, allData);
    if (allData.empty()) {
        HILOG_WARN("allConnectionData is empty.");
        return;
    }

    std::shared_ptr<ConnectionObserverController> controller = observerController_;
    if (!controller) {
        return;
    }

    for (auto& item : allData) {
        controller->NotifyExtensionDisconnected(item);
    }
}

void ConnectionStateManager::HandleDataAbilityCallerDied(int32_t callerPid)
{
    if (callerPid <= 0) {
        HILOG_WARN("invalid data ability caller pid.");
        return;
    }

    HandleCallerDied(callerPid);
}

void ConnectionStateManager::AddDlpManager(const std::shared_ptr<AbilityRecord> &dlpManger)
{
    if (!dlpManger) {
        return;
    }

    auto userId = dlpManger->GetOwnerMissionUserId();
    std::lock_guard<std::recursive_mutex> guard(dlpLock_);
    auto it = dlpItems_.find(userId);
    if (it == dlpItems_.end()) {
        dlpItems_[userId] = std::make_shared<DlpStateItem>(dlpManger->GetUid(), dlpManger->GetPid());
    }
}

void ConnectionStateManager::RemoveDlpManager(const std::shared_ptr<AbilityRecord> &dlpManger)
{
    if (!dlpManger) {
        return;
    }

    std::lock_guard<std::recursive_mutex> guard(dlpLock_);
    dlpItems_.erase(dlpManger->GetOwnerMissionUserId());
}

void ConnectionStateManager::AddDlpAbility(const std::shared_ptr<AbilityRecord> &dlpAbility)
{
    std::shared_ptr<ConnectionObserverController> controller = observerController_;
    if (!controller) {
        return;
    }

    DlpStateData dlpData;
    if (!HandleDlpAbilityInner(dlpAbility, true, dlpData)) {
        HILOG_DEBUG("no need to report dlp opened connection state.");
        return;
    }
    controller->NotifyDlpAbilityOpened(dlpData);
}

void ConnectionStateManager::RemoveDlpAbility(const std::shared_ptr<AbilityRecord> &dlpAbility)
{
    std::shared_ptr<ConnectionObserverController> controller = observerController_;
    if (!controller) {
        return;
    }

    DlpStateData dlpData;
    if (!HandleDlpAbilityInner(dlpAbility, false, dlpData)) {
        HILOG_DEBUG("no need to report dlp closed connection state.");
        return;
    }
    controller->NotifyDlpAbilityClosed(dlpData);
}

void ConnectionStateManager::HandleAppDied(int32_t pid)
{
    HandleCallerDied(pid);
}

void ConnectionStateManager::GetDlpConnectionInfos(std::vector<AbilityRuntime::DlpConnectionInfo> &infos)
{
    std::lock_guard<std::recursive_mutex> guard(dlpLock_);
    for (auto it = dlpItems_.begin(); it != dlpItems_.end(); it++) {
        auto item = it->second;
        if (!item) {
            continue;
        }

        AbilityRuntime::DlpConnectionInfo info;
        info.dlpUid = item->GetDlpUid();
        info.openedAbilityCount = item->GetOpenedAbilitySize();
        infos.emplace_back(info);
    }
}

bool ConnectionStateManager::AddConnectionInner(const std::shared_ptr<ConnectionRecord> &connectionRecord,
    AbilityRuntime::ConnectionData &data)
{
    std::shared_ptr<ConnectionStateItem> targetItem = nullptr;
    auto callerPid = connectionRecord->GetCallerPid();
    std::lock_guard<std::recursive_mutex> guard(stateLock_);
    auto it = connectionStates_.find(callerPid);
    if (it == connectionStates_.end()) {
        targetItem = ConnectionStateItem::CreateConnectionStateItem(connectionRecord);
        if (targetItem) {
            connectionStates_[callerPid] = targetItem;
        }
    } else {
        targetItem = it->second;
    }

    if (!targetItem) {
        HILOG_ERROR("failed to find target connection state item.");
        return false;
    }

    return targetItem->AddConnection(connectionRecord, data);
}

bool ConnectionStateManager::RemoveConnectionInner(const std::shared_ptr<ConnectionRecord> &connectionRecord,
    AbilityRuntime::ConnectionData &data)
{
    auto callerPid = connectionRecord->GetCallerPid();
    std::lock_guard<std::recursive_mutex> guard(stateLock_);
    auto it = connectionStates_.find(callerPid);
    if (it == connectionStates_.end()) {
        HILOG_WARN("can not find target item, connection caller pid:%{public}d.", callerPid);
        return false;
    }

    auto targetItem = it->second;
    if (!targetItem) {
        HILOG_ERROR("failed to find target connection state item.");
        return false;
    }

    bool result = targetItem->RemoveConnection(connectionRecord, data);
    if (result && targetItem->IsEmpty()) {
        connectionStates_.erase(it);
    }
    return result;
}

void ConnectionStateManager::HandleCallerDied(int32_t callerPid)
{
    auto connectionStateItem = RemoveDiedCaller(callerPid);
    if (!connectionStateItem) {
        HILOG_WARN("no connectionStateItem, may already handled.");
        return;
    }

    std::vector<AbilityRuntime::ConnectionData> allConnectionData;
    connectionStateItem->GenerateAllConnectionData(allConnectionData);
    if (allConnectionData.empty()) {
        HILOG_WARN("allConnectionData is empty.");
        return;
    }

    std::shared_ptr<ConnectionObserverController> controller = observerController_;
    if (!controller) {
        return;
    }

    for (auto& connectionData : allConnectionData) {
        controller->NotifyExtensionDisconnected(connectionData);
    }
}

std::shared_ptr<ConnectionStateItem> ConnectionStateManager::RemoveDiedCaller(int32_t callerPid)
{
    std::lock_guard<std::recursive_mutex> guard(stateLock_);
    auto it = connectionStates_.find(callerPid);
    if (it == connectionStates_.end()) {
        HILOG_WARN("can not find target item, connection caller pid:%{public}d, may already handled.", callerPid);
        return nullptr;
    }
    auto stateItem = it->second;
    (void)connectionStates_.erase(it);

    return stateItem;
}

bool ConnectionStateManager::AddDataAbilityConnectionInner(const DataAbilityCaller &caller,
    const std::shared_ptr<DataAbilityRecord> &record, ConnectionData &data)
{
    std::shared_ptr<ConnectionStateItem> targetItem = nullptr;
    std::lock_guard<std::recursive_mutex> guard(stateLock_);
    auto it = connectionStates_.find(caller.callerPid);
    if (it == connectionStates_.end()) {
        targetItem = ConnectionStateItem::CreateConnectionStateItem(caller);
        if (targetItem) {
            connectionStates_[caller.callerPid] = targetItem;
        }
    } else {
        targetItem = it->second;
    }

    if (!targetItem) {
        HILOG_ERROR("failed to find target connection state item.");
        return false;
    }

    return targetItem->AddDataAbilityConnection(caller, record, data);
}

bool ConnectionStateManager::RemoveDataAbilityConnectionInner(const DataAbilityCaller &caller,
    const std::shared_ptr<DataAbilityRecord> &record, AbilityRuntime::ConnectionData &data)
{
    std::lock_guard<std::recursive_mutex> guard(stateLock_);
    auto it = connectionStates_.find(caller.callerPid);
    if (it == connectionStates_.end()) {
        HILOG_WARN("can not find target item, connection caller pid:%{public}d.", caller.callerPid);
        return false;
    }

    auto targetItem = it->second;
    if (!targetItem) {
        HILOG_ERROR("failed to find target data ability state item.");
        return false;
    }

    bool result = targetItem->RemoveDataAbilityConnection(caller, record, data);
    if (result && targetItem->IsEmpty()) {
        connectionStates_.erase(it);
    }
    return result;
}

void ConnectionStateManager::HandleDataAbilityDiedInner(const sptr<IRemoteObject> &abilityToken,
    std::vector<AbilityRuntime::ConnectionData> &allData)
{
    std::lock_guard<std::recursive_mutex> guard(stateLock_);
    for (auto it = connectionStates_.begin(); it != connectionStates_.end();) {
        auto item = it->second;
        if (!item) {
            connectionStates_.erase(it++);
            continue;
        }

        AbilityRuntime::ConnectionData data;
        if (item->HandleDataAbilityDied(abilityToken, data)) {
            allData.emplace_back(data);
        }

        if (item->IsEmpty()) {
            connectionStates_.erase(it++);
        } else {
            it++;
        }
    }
}

bool ConnectionStateManager::HandleDlpAbilityInner(const std::shared_ptr<AbilityRecord> &dlpAbility,
    bool isAdd, AbilityRuntime::DlpStateData &dlpData)
{
    if (!dlpAbility) {
        HILOG_DEBUG("invalid dlp ability.");
        return false;
    }

    if (dlpAbility->GetAppIndex() == 0) {
        HILOG_DEBUG("this is not dlp ability, do not report connection stat.");
        return false;
    }

    std::lock_guard<std::recursive_mutex> guard(dlpLock_);
    auto it = dlpItems_.find(dlpAbility->GetOwnerMissionUserId());
    if (it == dlpItems_.end()) {
        HILOG_WARN("no dlp manager, invalid state.");
        return false;
    }

    auto dlpItem = it->second;
    if (!dlpItem) {
        HILOG_WARN("invalid dlpItem.");
        return false;
    }

    if (isAdd) {
        return dlpItem->AddDlpConnectionState(dlpAbility, dlpData);
    }

    return dlpItem->RemoveDlpConnectionState(dlpAbility, dlpData);
}

void ConnectionStateManager::InitAppStateObserver()
{
    if (appStateObserver_) {
        return;
    }

    sptr<OHOS::AppExecFwk::IAppMgr> appManager = GetAppMgr();
    if (!appManager) {
        HILOG_WARN("%{public}s app manager nullptr!", __func__);
        return;
    }

    appStateObserver_ = new (std::nothrow)InnerAppStateObserver([](int32_t pid) {
        DelayedSingleton<ConnectionStateManager>::GetInstance()->HandleAppDied(pid);
    });
    if (appStateObserver_ == nullptr) {
        HILOG_ERROR("appStateObserver_ == nullptr");
        return;
    }
    int32_t err = appManager->RegisterApplicationStateObserver(appStateObserver_);
    if (err != 0) {
        HILOG_ERROR("%{public}s register to appmanager failed. err:%{public}d", __func__, err);
        appStateObserver_ = nullptr;
        return;
    }
}
} // namespace AAFwk
} // namespace OHOS
