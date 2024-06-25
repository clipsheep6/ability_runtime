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

#include "ability_cache_manager.h"

#include <algorithm>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

AbilityCacheManager::AbilityCacheManager() {}

AbilityCacheManager::~AbilityCacheManager() {}

AbilityCacheManager &AbilityCacheManager::GetInstance()
{
    static AbilityCacheManager abilityRecMgr;
    return abilityRecMgr;
}

void AbilityCacheManager::Init(uint32_t devCapacity, uint32_t procCapacity)
{
    devLruCapacity_ = devCapacity;
    procLruCapacity_ = procCapacity;
}

void AbilityCacheManager::RemoveAbilityRecInDevList(std::shared_ptr<AbilityRecord> abilityRecord)
{
    AbilityInfo abilityInfo = abilityRecord->GetAbilityInfo();
    auto it = devRecLru_.begin();
    uint32_t accessTokenId = abilityRecord->GetApplicationInfo().accessTokenId;
    while (it != devRecLru_.end()) {
        if ((*it)->GetAbilityInfo().bundleName == abilityInfo.bundleName &&
            (*it)->GetAbilityInfo().moduleName == abilityInfo.moduleName &&
            (*it)->GetApplicationInfo().accessTokenId == accessTokenId) {
            devRecLru_.erase(it);
            devLruCnt_--;
            return;
        } else {
            it++;
        }
    }
}

void AbilityCacheManager::RemoveAbilityRecInProcList(std::shared_ptr<AbilityRecord> abilityRecord)
{
    AbilityInfo abilityInfo = abilityRecord->GetAbilityInfo();
    const Want want = abilityRecord->GetWant();
    uint32_t accessTokenId = abilityRecord->GetApplicationInfo().accessTokenId;
    auto findProcInfo = procLruMap_.find(accessTokenId);
    if (findProcInfo == procLruMap_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Can't found the abilityRecord in process list for remove.");
        return;
    }
    auto it = findProcInfo->second.recList.begin();

    while (it != findProcInfo->second.recList.end()) {
        if ((*it)->GetAbilityInfo().moduleName == abilityInfo.moduleName &&
            (*it)->GetWant().GetElement().GetAbilityName() == want.GetElement().GetAbilityName()) {
            findProcInfo->second.recList.erase(it);
            findProcInfo->second.cnt--;
            if (findProcInfo->second.cnt == 0) {
                procLruMap_.erase(findProcInfo);
            }
            return;
        } else {
            it++;
        }
    }
}

std::shared_ptr<AbilityRecord> AbilityCacheManager::AddToProcLru(std::shared_ptr<AbilityRecord> abilityRecord)
{
    auto findProcInfo = procLruMap_.find(abilityRecord->GetApplicationInfo().accessTokenId);
    if (findProcInfo == procLruMap_.end()) {
        std::list<std::shared_ptr<AbilityRecord>> recList;
        ProcRecordsInfo procRecInfo = {recList, 1};
        procRecInfo.recList.push_back(abilityRecord);
        procLruMap_[abilityRecord->GetApplicationInfo().accessTokenId] = procRecInfo;
        return nullptr;
    }
    if (findProcInfo->second.cnt == procLruCapacity_) {
        RemoveAbilityRecInDevList(findProcInfo->second.recList.front());
        std::shared_ptr<AbilityRecord> rec = findProcInfo->second.recList.front();
        findProcInfo->second.recList.pop_front();
        findProcInfo->second.recList.push_back(abilityRecord);
        return rec;
    }
    findProcInfo->second.cnt++;
    findProcInfo->second.recList.push_back(abilityRecord);
    return nullptr;
}

std::shared_ptr<AbilityRecord> AbilityCacheManager::AddToDevLru(std::shared_ptr<AbilityRecord> abilityRecord,
    std::shared_ptr<AbilityRecord> rec)
{
    if (rec != nullptr) {
        devRecLru_.push_back(abilityRecord);
        devLruCnt_++;
        return rec;
    }
    if (devLruCnt_ == devLruCapacity_) {
        rec = devRecLru_.front();
        RemoveAbilityRecInProcList(rec);
        devRecLru_.pop_front();
        devLruCnt_--;
    }
    devRecLru_.push_back(abilityRecord);
    devLruCnt_++;
    return rec;
}

std::shared_ptr<AbilityRecord> AbilityCacheManager::Put(std::shared_ptr<AbilityRecord> abilityRecord)
{
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The param abilityRecord is nullptr for Put operation.");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<AbilityRecord> rec = AddToProcLru(abilityRecord);
    return AddToDevLru(abilityRecord, rec);
}

void AbilityCacheManager::Remove(std::shared_ptr<AbilityRecord> abilityRecord)
{
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The param abilityRecord is nullptr for Remove operation.");
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    RemoveAbilityRecInProcList(abilityRecord);
    RemoveAbilityRecInDevList(abilityRecord);
}

bool AbilityCacheManager::IsRecInfoSame(const AbilityRequest& abilityRequest,
    std::shared_ptr<AbilityRecord> abilityRecord)
{
    return abilityRequest.abilityInfo.moduleName == abilityRecord->GetAbilityInfo().moduleName &&
        abilityRequest.want.GetElement().GetAbilityName() == abilityRecord->GetWant().GetElement().GetAbilityName();
}

std::shared_ptr<AbilityRecord> AbilityCacheManager::GetAbilityRecInProcList(const AbilityRequest &abilityRequest)
{
    auto findProcInfo = procLruMap_.find(abilityRequest.appInfo.accessTokenId);
    if (findProcInfo == procLruMap_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Can't found the bundleName in process list for get.");
        return nullptr;
    }
    ProcRecordsInfo &procRecordsInfo = findProcInfo->second;
    auto recIter = procRecordsInfo.recList.begin();
    while (recIter != procRecordsInfo.recList.end()) {
        if (IsRecInfoSame(abilityRequest, *recIter)) {
            std::shared_ptr<AbilityRecord> &abilityRecord = *recIter;
            procRecordsInfo.recList.erase(recIter);
            procRecordsInfo.cnt--;
            return abilityRecord;
        }
        recIter++;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Can't found the abilityRecord in process list for get.");
    return nullptr;
}

std::shared_ptr<AbilityRecord> AbilityCacheManager::Get(const AbilityRequest& abilityRequest)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecInProcList(abilityRequest);
    if (abilityRecord == nullptr) {
        return nullptr;
    }
    RemoveAbilityRecInDevList(abilityRecord);
    return abilityRecord;
}

std::shared_ptr<AbilityRecord> AbilityCacheManager::FindRecordByToken(const sptr<IRemoteObject> &token)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The param token is nullptr for FindRecordByToken operation.");
        return nullptr;
    }
    auto it = devRecLru_.begin();
    while (it != devRecLru_.end()) {
        sptr<IRemoteObject> srcToken = (*it)->GetToken();
        if (srcToken == token) {
            std::shared_ptr<AbilityRecord> &abilityRecord = *it;
            return abilityRecord;
        } else {
            it++;
        }
    }
    return nullptr;
}
}  // namespace AAFwk
} // namespace OHOS