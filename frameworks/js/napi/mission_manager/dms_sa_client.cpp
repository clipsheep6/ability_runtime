/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
DmsSaClient &DmsSaClient::GetInstance()
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    static DmsSaClient instance;
    return instance;
}

bool DmsSaClient::SubscribeDmsSA()
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    sptr<DmsSystemAbilityStatusChange> callback(new DmsSystemAbilityStatusChange());
    int32_t ret = saMgrProxy_->SubscribeSystemAbility(DISTRIBUTED_SCHED_SA_ID, callback);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::MISSION, "SubscribeSystemAbility ret:%{public}d", ret);
        return false;
    }
    return true;
}

int32_t DmsSaClient::AddListener(const std::string& type, const sptr<IRemoteOnListener>& listener)
{
    TAG_LOGI(AAFwkTag::MISSION, "called, type: %{public}s", type.c_str());
    saMgrProxy_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgrProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "get saMgrProxy failed");
        return INNER_ERR;
    }
    if (saMgrProxy_->CheckSystemAbility(DISTRIBUTED_SCHED_SA_ID)) {
        AbilityManagerClient::GetInstance()->RegisterOnListener(type, listener);
    }
    std::lock_guard<std::mutex> lock(eventMutex_);
    listeners_[type] = listener;
    if (!hasSubscribeDmsSA_) {
        if (SubscribeDmsSA()) {
            hasSubscribeDmsSA_ = true;
        } else {
            return INNER_ERR;
        }
    }
    return NO_ERROR;
}

int32_t DmsSaClient::DelListener(const std::string& type, const sptr<IRemoteOnListener>& listener)
{
    TAG_LOGI(AAFwkTag::MISSION, "called, type: %{public}s", type.c_str());
    if (saMgrProxy_->CheckSystemAbility(DISTRIBUTED_SCHED_SA_ID)) {
        AbilityManagerClient::GetInstance()->RegisterOffListener(type, listener);
    }
    std::lock_guard<std::mutex> lock(eventMutex_);
    listeners_.erase(type);
    return NO_ERROR;
}

void DmsSaClient::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    TAG_LOGI(AAFwkTag::MISSION, "called, systemAbilityId: %{public}d", systemAbilityId);
    std::lock_guard<std::mutex> lock(eventMutex_);
    if (systemAbilityId == DISTRIBUTED_SCHED_SA_ID) {
        TAG_LOGI(AAFwkTag::MISSION, "listeners size: %{public}zu", listeners_.size());
        for (auto& listener : listeners_) {
            AbilityManagerClient::GetInstance()->RegisterOnListener(listener.first, listener.second);
        }
    } else {
        TAG_LOGE(AAFwkTag::MISSION, "SystemAbilityId: %{public}d, need DISTRIBUTED_SCHED_SA_ID",
            systemAbilityId);
    }
}

void DmsSaClient::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    TAG_LOGI(AAFwkTag::MISSION, "called, systemAbilityId: %{public}d", systemAbilityId);
    std::lock_guard<std::mutex> lock(eventMutex_);
    if (systemAbilityId == DISTRIBUTED_SCHED_SA_ID) {
        for (auto& listener : listeners_) {
            AbilityManagerClient::GetInstance()->RegisterOffListener(listener.first, listener.second);
        }
    } else {
        TAG_LOGE(AAFwkTag::MISSION, "SystemAbilityId: %{public}d, need DISTRIBUTED_SCHED_SA_ID",
            systemAbilityId);
    }
}

DmsSystemAbilityStatusChange::DmsSystemAbilityStatusChange()
{
    TAG_LOGI(AAFwkTag::MISSION, "create");
}

DmsSystemAbilityStatusChange::~DmsSystemAbilityStatusChange()
{
    TAG_LOGI(AAFwkTag::MISSION, "delete");
}

void DmsSystemAbilityStatusChange::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    TAG_LOGI(AAFwkTag::MISSION, "called, systemAbilityId: %d", systemAbilityId);
    if (systemAbilityId != DISTRIBUTED_SCHED_SA_ID) {
        TAG_LOGE(AAFwkTag::MISSION, "SystemAbilityId: %d, need DISTRIBUTED_SCHED_SA_ID", systemAbilityId);
        return;
    }

    DmsSaClient::GetInstance().OnAddSystemAbility(systemAbilityId, deviceId);
    TAG_LOGI(AAFwkTag::MISSION, "called end, systemAbilityId: %d", systemAbilityId);
}

void DmsSystemAbilityStatusChange::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    TAG_LOGI(AAFwkTag::MISSION, "called, systemAbilityId: %d", systemAbilityId);
    if (systemAbilityId != DISTRIBUTED_SCHED_SA_ID) {
        TAG_LOGE(AAFwkTag::MISSION, "SystemAbilityId: %d, need DISTRIBUTED_SCHED_SA_ID", systemAbilityId);
        return;
    }
}
}  // namespace AAFwk
}  // namespace OHOS
