/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "sub_managers_helper.h"

#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "mission_info_mgr.h"
#include "scene_board_judgement.h"
#include "os_account_manager_wrapper.h"

namespace OHOS {
namespace AAFwk {
constexpr int32_t U0_USER_ID = 0;
constexpr int32_t INVALID_USER_ID = -1;

SubManagersHelper::SubManagersHelper(
    std::shared_ptr<TaskHandlerWrap> taskHandler, std::shared_ptr<AbilityEventHandler> eventHandler)
    : taskHandler_(taskHandler), eventHandler_(eventHandler) {}

void SubManagersHelper::InitSubManagers(int userId, bool switchUser)
{
    InitConnectManager(userId, switchUser);
    InitDataAbilityManager(userId, switchUser);
    InitPendWantManager(userId, switchUser);
    if (userId != U0_USER_ID) {
        if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
            InitUIAbilityManager(userId, switchUser);
        } else {
            InitMissionListManager(userId, switchUser);
        }
    }
}

void SubManagersHelper::InitConnectManager(int32_t userId, bool switchUser)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = connectManagers_.find(userId);
    if (it != connectManagers_.end()) {
        if (switchUser) {
            currentConnectManager_ = it->second;
        }
        return;
    }
    auto manager = std::make_shared<AbilityConnectManager>(userId);
    manager->SetTaskHandler(taskHandler_);
    manager->SetEventHandler(eventHandler_);
    connectManagers_.emplace(userId, manager);
    if (switchUser) {
        currentConnectManager_ = manager;
    }
}

void SubManagersHelper::InitDataAbilityManager(int32_t userId, bool switchUser)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = dataAbilityManagers_.find(userId);
    if (it != dataAbilityManagers_.end()) {
        if (switchUser) {
            currentDataAbilityManager_ = it->second;
        }
        return;
    }
    auto manager = std::make_shared<DataAbilityManager>();
    dataAbilityManagers_.emplace(userId, manager);
    if (switchUser) {
        currentDataAbilityManager_ = manager;
    }
}

void SubManagersHelper::InitPendWantManager(int32_t userId, bool switchUser)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = pendingWantManagers_.find(userId);
    if (it != pendingWantManagers_.end()) {
        if (switchUser) {
            currentPendingWantManager_ = it->second;
        }
        return;
    }
    auto manager = std::make_shared<PendingWantManager>();
    pendingWantManagers_.emplace(userId, manager);
    if (switchUser) {
        currentPendingWantManager_ = manager;
    }
}

void SubManagersHelper::InitMissionListManager(int userId, bool switchUser)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = missionListManagers_.find(userId);
    if (it != missionListManagers_.end()) {
        if (switchUser) {
            DelayedSingleton<MissionInfoMgr>::GetInstance()->Init(userId);
            currentMissionListManager_ = it->second;
        }
        return;
    }
    auto manager = std::make_shared<MissionListManager>(userId);
    manager->Init();
    missionListManagers_.emplace(userId, manager);
    if (switchUser) {
        currentMissionListManager_ = manager;
    }
}

void SubManagersHelper::InitUIAbilityManager(int userId, bool switchUser)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = uiAbilityManagers_.find(userId);
    if (it != uiAbilityManagers_.end()) {
        if (switchUser) {
            currentUIAbilityManager_ = it->second;
        }
        return;
    }
    auto manager = std::make_shared<UIAbilityLifecycleManager>(userId);
    uiAbilityManagers_.emplace(userId, manager);
    if (switchUser) {
        currentUIAbilityManager_ = manager;
    }
}

void SubManagersHelper::ClearSubManagers(int userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityManagers_.erase(userId);
    } else {
        missionListManagers_.erase(userId);
    }
    connectManagers_.erase(userId);
    dataAbilityManagers_.erase(userId);
    pendingWantManagers_.erase(userId);
}

std::shared_ptr<DataAbilityManager> SubManagersHelper::GetCurrentDataAbilityManager()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return currentDataAbilityManager_;
}

std::shared_ptr<DataAbilityManager> SubManagersHelper::GetDataAbilityManager(const sptr<IAbilityScheduler> &scheduler)
{
    if (scheduler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "the param ability scheduler is nullptr");
        return nullptr;
    }

    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto& item: dataAbilityManagers_) {
        if (item.second && item.second->ContainsDataAbility(scheduler)) {
            return item.second;
        }
    }

    return nullptr;
}
std::shared_ptr<DataAbilityManager> SubManagersHelper::GetDataAbilityManagerByUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = dataAbilityManagers_.find(userId);
    if (it != dataAbilityManagers_.end()) {
        return it->second;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get Manager. UserId = %{public}d", userId);
    return nullptr;
}

std::shared_ptr<DataAbilityManager> SubManagersHelper::GetDataAbilityManagerByToken(const sptr<IRemoteObject> &token)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto& item: dataAbilityManagers_) {
        if (item.second && item.second->GetAbilityRecordByToken(token)) {
            return item.second;
        }
    }

    return nullptr;
}

std::unordered_map<int, std::shared_ptr<AbilityConnectManager>> SubManagersHelper::GetConnectManagers()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return connectManagers_;
}

std::shared_ptr<AbilityConnectManager> SubManagersHelper::GetCurrentConnectManager()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return currentConnectManager_;
}

std::shared_ptr<AbilityConnectManager> SubManagersHelper::GetConnectManagerByUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = connectManagers_.find(userId);
    if (it != connectManagers_.end()) {
        return it->second;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get Manager. UserId = %{public}d", userId);
    return nullptr;
}

std::shared_ptr<AbilityConnectManager> SubManagersHelper::GetConnectManagerByToken(const sptr<IRemoteObject> &token)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto& item: connectManagers_) {
        if (item.second && item.second->GetExtensionByTokenFromServiceMap(token)) {
            return item.second;
        }
        if (item.second && item.second->GetExtensionByTokenFromTerminatingMap(token)) {
            return item.second;
        }
    }

    return nullptr;
}

std::shared_ptr<PendingWantManager> SubManagersHelper::GetCurrentPendingWantManager()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return currentPendingWantManager_;
}

std::shared_ptr<PendingWantManager> SubManagersHelper::GetPendingWantManagerByUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = pendingWantManagers_.find(userId);
    if (it != pendingWantManagers_.end()) {
        return it->second;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get Manager. UserId = %{public}d", userId);
    return nullptr;
}

std::unordered_map<int, std::shared_ptr<MissionListManager>> SubManagersHelper::GetMissionListManagers()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return missionListManagers_;
}

std::shared_ptr<MissionListManager> SubManagersHelper::GetCurrentMissionListManager()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return currentMissionListManager_;
}

std::shared_ptr<MissionListManager> SubManagersHelper::GetMissionListManagerByUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = missionListManagers_.find(userId);
    if (it != missionListManagers_.end()) {
        return it->second;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get Manager. UserId = %{public}d", userId);
    return nullptr;
}

std::unordered_map<int, std::shared_ptr<UIAbilityLifecycleManager>> SubManagersHelper::GetUIAbilityManagers()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return uiAbilityManagers_;
}

std::shared_ptr<UIAbilityLifecycleManager> SubManagersHelper::GetCurrentUIAbilityManager()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return currentUIAbilityManager_;
}

std::shared_ptr<UIAbilityLifecycleManager> SubManagersHelper::GetUIAbilityManagerByUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = uiAbilityManagers_.find(userId);
    if (it != uiAbilityManagers_.end()) {
        return it->second;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get Manager. UserId = %{public}d", userId);
    return nullptr;
}

std::shared_ptr<UIAbilityLifecycleManager> SubManagersHelper::GetUIAbilityManagerByUid(int32_t uid)
{
    int32_t userId = INVALID_USER_ID;
    if (DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->GetOsAccountLocalIdFromUid(
        uid, userId) != 0) {
        return nullptr;
    }
    if (userId == U0_USER_ID) {
        std::lock_guard<ffrt::mutex> lock(managersMutex_);
        return currentUIAbilityManager_;
    }
    return GetUIAbilityManagerByUserId(userId);
}

void SubManagersHelper::UninstallApp(const std::string &bundleName, int32_t uid)
{
    int32_t userId = INVALID_USER_ID;
    if (DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->GetOsAccountLocalIdFromUid(
        uid, userId) != 0) {
        return;
    }
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        UninstallAppInUIAbilityManagers(userId, bundleName, uid);
    } else {
        UninstallAppInMissionListManagers(userId, bundleName, uid);
    }

    auto currentPendingWantManager = GetCurrentPendingWantManager();
    if (currentPendingWantManager) {
        currentPendingWantManager->ClearPendingWantRecord(bundleName, uid);
    }
}

void SubManagersHelper::UninstallAppInUIAbilityManagers(int32_t userId, const std::string &bundleName, int32_t uid)
{
    if (userId == U0_USER_ID) {
        auto uiAbilityManagers = GetUIAbilityManagers();
        for (auto& item : uiAbilityManagers) {
            if (item.second) {
                item.second->UninstallApp(bundleName, uid);
            }
        }
    } else {
        auto manager = GetUIAbilityManagerByUserId(userId);
        if (manager) {
            manager->UninstallApp(bundleName, uid);
        }
    }
}

void SubManagersHelper::UninstallAppInMissionListManagers(int32_t userId, const std::string &bundleName, int32_t uid)
{
    if (userId == U0_USER_ID) {
        auto missionListManagers = GetMissionListManagers();
        for (auto& item : missionListManagers) {
            if (item.second) {
                item.second->UninstallApp(bundleName, uid);
            }
        }
    } else {
        auto listManager = GetMissionListManagerByUserId(userId);
        if (listManager) {
            listManager->UninstallApp(bundleName, uid);
        }
    }
}

bool SubManagersHelper::VerificationAllToken(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "VerificationAllToken.");
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        for (auto& item: uiAbilityManagers_) {
            if (item.second && item.second->IsContainsAbility(token)) {
                return true;
            }
        }
    } else {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "VerificationAllToken::SearchMissionListManagers");
        for (auto& item: missionListManagers_) {
            if (item.second && item.second->GetAbilityRecordByToken(token)) {
                return true;
            }
            if (item.second && item.second->GetAbilityFromTerminateList(token)) {
                return true;
            }
        }
    }
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "VerificationAllToken::SearchDataAbilityManagers_");
        for (auto& item: dataAbilityManagers_) {
            if (item.second && item.second->GetAbilityRecordByToken(token)) {
                return true;
            }
        }
    }
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "VerificationAllToken::SearchConnectManagers_");
        for (auto& item: connectManagers_) {
            if (item.second && item.second->GetExtensionByTokenFromServiceMap(token)) {
                return true;
            }
            if (item.second && item.second->GetExtensionByTokenFromTerminatingMap(token)) {
                return true;
            }
        }
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to verify all token.");
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS