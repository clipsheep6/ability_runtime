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

#ifndef OHOS_ABILITY_RUNTIME_SUB_MANAGERS_HELPER_H
#define OHOS_ABILITY_RUNTIME_SUB_MANAGERS_HELPER_H

#include <mutex>
#include <string>
#include <unordered_map>

#include "ability_connect_manager.h"
#include "ability_event_handler.h"
#include "cpp/mutex.h"
#include "data_ability_manager.h"
#include "mission_list_manager.h"
#include "nocopyable.h"
#include "pending_want_manager.h"
#include "scene_board/ui_ability_lifecycle_manager.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AAFwk {
class SubManagersHelper {
public:
    SubManagersHelper(std::shared_ptr<TaskHandlerWrap> taskHandler, std::shared_ptr<AbilityEventHandler> eventHandler);
    virtual ~SubManagersHelper() = default;

    void InitSubManagers(int userId, bool switchUser);
    void InitMissionListManager(int userId, bool switchUser);
    void InitUIAbilityManager(int userId, bool switchUser);
    void InitConnectManager(int32_t userId, bool switchUser);
    void InitDataAbilityManager(int32_t userId, bool switchUser);
    void InitPendWantManager(int32_t userId, bool switchUser);

    void ClearSubManagers(int userId);

    std::shared_ptr<DataAbilityManager> GetCurrentDataAbilityManager();
    std::shared_ptr<DataAbilityManager> GetDataAbilityManager(const sptr<IAbilityScheduler> &scheduler);
    std::shared_ptr<DataAbilityManager> GetDataAbilityManagerByUserId(int32_t userId);
    std::shared_ptr<DataAbilityManager> GetDataAbilityManagerByToken(const sptr<IRemoteObject> &token);

    std::unordered_map<int, std::shared_ptr<AbilityConnectManager>> GetConnectManagers();
    std::shared_ptr<AbilityConnectManager> GetCurrentConnectManager();
    std::shared_ptr<AbilityConnectManager> GetConnectManagerByUserId(int32_t userId);
    std::shared_ptr<AbilityConnectManager> GetConnectManagerByToken(const sptr<IRemoteObject> &token);

    std::shared_ptr<PendingWantManager> GetCurrentPendingWantManager();
    std::shared_ptr<PendingWantManager> GetPendingWantManagerByUserId(int32_t userId);

    std::unordered_map<int, std::shared_ptr<MissionListManager>> GetMissionListManagers();
    std::shared_ptr<MissionListManager> GetCurrentMissionListManager();
    std::shared_ptr<MissionListManager> GetMissionListManagerByUserId(int32_t userId);

    std::unordered_map<int, std::shared_ptr<UIAbilityLifecycleManager>> GetUIAbilityManagers();
    std::shared_ptr<UIAbilityLifecycleManager> GetCurrentUIAbilityManager();
    std::shared_ptr<UIAbilityLifecycleManager> GetUIAbilityManagerByUserId(int32_t userId);
    std::shared_ptr<UIAbilityLifecycleManager> GetUIAbilityManagerByUid(int32_t uid);

    void UninstallApp(const std::string &bundleName, int32_t uid);
    void UninstallAppInUIAbilityManagers(int32_t userId, const std::string &bundleName, int32_t uid);
    void UninstallAppInMissionListManagers(int32_t userId, const std::string &bundleName, int32_t uid);
    bool VerificationAllToken(const sptr<IRemoteObject> &token);

private:
    DISALLOW_COPY_AND_MOVE(SubManagersHelper);

    std::shared_ptr<TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AbilityEventHandler> eventHandler_;

    ffrt::mutex managersMutex_;
    std::unordered_map<int, std::shared_ptr<AbilityConnectManager>> connectManagers_;
    std::shared_ptr<AbilityConnectManager> currentConnectManager_;
    std::unordered_map<int, std::shared_ptr<DataAbilityManager>> dataAbilityManagers_;
    std::shared_ptr<DataAbilityManager> currentDataAbilityManager_;
    std::unordered_map<int, std::shared_ptr<PendingWantManager>> pendingWantManagers_;
    std::shared_ptr<PendingWantManager> currentPendingWantManager_;
    std::unordered_map<int, std::shared_ptr<MissionListManager>> missionListManagers_;
    std::shared_ptr<MissionListManager> currentMissionListManager_;
    std::unordered_map<int, std::shared_ptr<UIAbilityLifecycleManager>> uiAbilityManagers_;
    std::shared_ptr<UIAbilityLifecycleManager> currentUIAbilityManager_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_SUB_MANAGERS_HELPER_H
