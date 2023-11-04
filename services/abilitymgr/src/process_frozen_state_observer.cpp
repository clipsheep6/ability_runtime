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
constexpr int32_t FAIL_RETRY_SPAN = 5000; // ms
}
int ProcessFrozenStateObserver::g_regieterCount = 0;
void ProcessFrozenStateObserver::ResiterSuspendObserver(std::shared_ptr<TaskHandlerWrap> taskHandler)
{
    if (!taskHandler) {
        HILOG_ERROR("taskhandler null");
        return;
    }
    HILOG_INFO("ResiterSuspendObserver begin");
    auto ret = SuspendManager::SuspendManagerClient::GetInstance().RegisterSuspendObserver(
        sptr(new ProcessFrozenStateObserver()));
    if (ret != ERR_OK) {
        HILOG_ERROR("failed err: %{public}d", ret);
        g_regieterCount++;
        if (g_regieterCount < MAX_FAIL_COUNT) {
            taskHandler->SubmitTask([taskHandler]() {
                    ResiterSuspendObserver(taskHandler);
                }, FAIL_RETRY_SPAN);
        }
    } else {
        g_regieterCount = 0;
        HILOG_INFO("RegisterSuspendObserver success");
    }
}

void ProcessFrozenStateObserver::OnActive(const std::vector<int32_t> &pidList, const int32_t uid)
{
    HILOG_INFO("OnActive: %{public}d", uid);
}

void ProcessFrozenStateObserver::OnDoze(const int32_t uid)
{
    HILOG_INFO("OnDoze: %{public}d", uid);
}

void ProcessFrozenStateObserver::OnFrozen(const std::vector<int32_t> &pidList, const int32_t uid)
{
    HILOG_INFO("OnFrozen: %{public}d", uid);
    if (pidList.empty()) {
        HILOG_WARN("OnFrozen pidlist empty");
        return;
    }
    DelayedSingleton<AbilityManagerService>::GetInstance()->HandleProcessFrozen(pidList, uid);
}
} // namespace AAFwk
} // namespace OHOS