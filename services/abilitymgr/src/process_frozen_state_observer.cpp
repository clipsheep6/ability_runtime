/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "process_frozen_state_observer.h"

#include "ability_manager_service.h"
#include "hilog/log.h"
#include "suspend_manager_client.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t MAX_FAIL_COUNT = 10;
constexpr int32_t FAIL_RETRY_INTERVAL = 5000; // ms

class SuspendMgrDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    static void AddSuspendMgrDeathRecipient()
    {
        auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemManager) {
            HILOG_ERROR("Failed to get SystemAbilityManager.");
            return;
        }
        auto remoteObj = systemManager->GetSystemAbility(SUSPEND_MANAGER_SYSTEM_ABILITY_ID);
        if (!remoteObj) {
            HILOG_ERROR("Failed to get SuspendManager sa.");
            return;
        }
        if (!remoteObj->IsProxyObject()) {
            HILOG_ERROR("Not proxy object.");
            return;
        }
        if (!remoteObj->AddDeathRecipient(sptr(new SuspendMgrDeathRecipient))) {
            HILOG_ERROR("Add death recipient for SuspendManager failed.");
            return;
        }
    }

    void OnRemoteDied(const wptr<IRemoteObject>&)
    {
        auto taskHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
        if (!taskHandler) {
            HILOG_ERROR("taskHandler null.");
            return;
        }
        taskHandler->SubmitTask([taskHandler]() {
                ProcessFrozenStateObserver::RegisterSuspendObserver(taskHandler);
            }, FAIL_RETRY_INTERVAL);
    }
};
}
int ProcessFrozenStateObserver::g_registerCount = 0;
void ProcessFrozenStateObserver::RegisterSuspendObserver(std::shared_ptr<TaskHandlerWrap> taskHandler)
{
    if (!taskHandler) {
        HILOG_ERROR("taskHandler null");
        return;
    }
    auto ret = SuspendManager::SuspendManagerClient::GetInstance().RegisterSuspendObserver(
        sptr(new ProcessFrozenStateObserver()));
    if (ret != ERR_OK) {
        HILOG_ERROR("failed err: %{public}d", ret);
        g_registerCount++;
        if (g_registerCount < MAX_FAIL_COUNT) {
            taskHandler->SubmitTask([taskHandler]() {
                    RegisterSuspendObserver(taskHandler);
                }, FAIL_RETRY_INTERVAL);
        } else {
            HILOG_ERROR("failed 10 times");
            g_registerCount = 0;
        }
    } else {
        g_registerCount = 0;
        HILOG_INFO("RegisterSuspendObserver success");
        SuspendMgrDeathRecipient::AddSuspendMgrDeathRecipient();
    }
}

void ProcessFrozenStateObserver::OnActive(const std::vector<int32_t> &pidList, const int32_t uid)
{
    HILOG_DEBUG("OnActive: %{public}d", uid);
}

void ProcessFrozenStateObserver::OnDoze(const int32_t uid)
{
    HILOG_DEBUG("OnDoze: %{public}d", uid);
}

void ProcessFrozenStateObserver::OnFrozen(const std::vector<int32_t> &pidList, const int32_t uid)
{
    HILOG_INFO("OnFrozen: %{public}d", uid);
    if (pidList.empty()) {
        HILOG_WARN("OnFrozen pidList empty");
        return;
    }
    DelayedSingleton<AbilityManagerService>::GetInstance()->HandleProcessFrozen(pidList, uid);
}
} // namespace AAFwk
} // namespace OHOS