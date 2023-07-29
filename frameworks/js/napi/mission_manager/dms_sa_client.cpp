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
#include "dms_sa_client.h"

namespace OHOS {
namespace AAFwk {
DmsSaClient &DmsSaClient::GetInstance()
{
    HILOGI("%{public}s called.", __func__);
    static DmsSaClient instance;
    return instance;
}

bool DmsSaClient::SubscribeDmsSA()
{
    HILOGI("%{public}s called.", __func__);
    saMgrProxy_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgrProxy_ == nullptr) {
        HILOGE("fail to get saMgrProxy.");
        return false;
    }
    int32_t ret = saMgrProxy_->SubscribeSystemAbility(DISTRIBUTED_SCHED_SA_ID, this);
    HILOGI("MXD04.");
    if (ret != ERR_OK) {
        HILOGE("Failed to subscribe system ability DISTRIBUTED_SCHED_SA_ID ret:%{public}d", ret);
        return false;
    }
    return true;
}


int32_t DmsSaClient::AddListener(const std::string& type, const sptr<IRemoteOnListener>& listener)
{
    HILOGI("%{public}s called.", __func__);
    if (saMgrProxy_->CheckSystemAbility(DISTRIBUTED_SCHED_SA_ID)) {
        return AbilityManagerClient::GetInstance()->RegisterOnListener(type, listener);
    }
    listeners_[type] = listener;
    return NO_ERROR;
}

int32_t DmsSaClient::DelListener(const std::string& type, const sptr<IRemoteOnListener>& listener)
{
    HILOGI("%{public}s called.", __func__);
    if (saMgrProxy_->CheckSystemAbility(DISTRIBUTED_SCHED_SA_ID)) {
        return AbilityManagerClient::GetInstance()->RegisterOffListener(type, listener);
    }
    listeners_.erase(type);
    return NO_ERROR;
}

void DmsSaClient::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("%{public}s called.", __func__);
    if (systemAbilityId == DISTRIBUTED_SCHED_SA_ID) {
        HILOGI("%{public}s listeners_ size: %{public}lu .", __func__, listeners_.size());
        for (auto& listener : listeners_) {
            AbilityManagerClient::GetInstance()->RegisterOnListener(listener.first, listener.second);
        }
    } else {
        HILOGE("SystemAbilityId must be DISTRIBUTED_SCHED_SA_ID,but it is %{public}d", systemAbilityId);
    }
}

void DmsSaClient::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("%{public}s called.", __func__);
    if (systemAbilityId == DISTRIBUTED_SCHED_SA_ID) {
        for (auto& listener : listeners_) {
            AbilityManagerClient::GetInstance()->RegisterOffListener(listener.first, listener.second);
        }
    } else {
        HILOGE("SystemAbilityId must be DISTRIBUTED_SCHED_SA_ID,but it is %{public}d", systemAbilityId);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
