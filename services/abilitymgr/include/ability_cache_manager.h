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

#ifndef OHOS_ABILITY_RUNTIME__ABILITY_CACHE_MANAGER_H
#define OHOS_ABILITY_RUNTIME__ABILITY_CACHE_MANAGER_H

#include <map>
#include <list>
#include <string>
#include <mutex>

#include "ability_record.h"
namespace OHOS {
namespace AAFwk {
/**
 * @class AbilityCacheManager
 * AbilityCacheManager provides a lru cache for managing ability record.
 */
class AbilityCacheManager {
public:
    using AbilityInfo = OHOS::AppExecFwk::AbilityInfo;
    /**
     * Get ability cache manager.
     * @return AbilityCacheManager
     */
    static AbilityCacheManager &GetInstance(void);

    /**
     * Init the ability cache manager with capacity for the device (devCapacity)
     * and the capacity for a single process(procCapacity)
     */
    void Init(uint32_t devCapacity, uint32_t procCapacity);

    /**
     * Put a single ability record into ability cache manager.
     * @param abilityRecord the ability record to be putted into cache manager.
     * @return AbilityRecord if one is eliminated, otherwise nullptr.
     */
    std::shared_ptr<AbilityRecord> Put(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * Remove a single ability record from ability cache manager.
     * @param abilityRecord, the ability record to be removed into cache manager.
     */
    void Remove(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * Get a single ability by abilityRequest record from ability cache manager,
     * this will remove the AbilityRecord by default
     * @param abilityRequest the ability request to be searched in cache manager.
     * @return AbilityRecord if one is matched, otherwise nullptr.
     */
    std::shared_ptr<AbilityRecord> Get(const AbilityRequest &abilityRequest);

     /**
     * Get a single ability by token from ability cache manager.
     * @param token the ability token to be searched in cache manager.
     * @return AbilityRecord if one is matched, otherwise nullptr.
     */
    std::shared_ptr<AbilityRecord> FindRecordByToken(const sptr<IRemoteObject> &token);
    private:
        AbilityCacheManager();
        ~AbilityCacheManager();
        struct ProcRecordsInfo {
            std::list<std::shared_ptr<AbilityRecord>> recList;
            uint32_t cnt;
        };
        uint32_t devLruCapacity_ = 0;
        uint32_t procLruCapacity_ = 0;
        uint32_t devLruCnt_ = 0;
        std::mutex mutex_;
        std::map<uint32_t, ProcRecordsInfo> procLruMap_;
        std::list<std::shared_ptr<AbilityRecord>> devRecLru_;
        std::shared_ptr<AbilityRecord> AddToProcLru(std::shared_ptr<AbilityRecord> abilityRecord);
        std::shared_ptr<AbilityRecord> AddToDevLru(std::shared_ptr<AbilityRecord> abilityRecord,
            std::shared_ptr<AbilityRecord> rec);
        void RemoveAbilityRecInDevList(std::shared_ptr<AbilityRecord> abilityRecord);
        void RemoveAbilityRecInProcList(std::shared_ptr<AbilityRecord> abilityRecord);
        std::shared_ptr<AbilityRecord> GetAbilityRecInProcList(const AbilityRequest &abilityRequest);
        bool IsRecInfoSame(const AbilityRequest& abilityRequest, std::shared_ptr<AbilityRecord> abilityRecord);
        DISALLOW_COPY_AND_MOVE(AbilityCacheManager);
};
} // namespace AAFwk
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME__ABILITY_CACHE_MANAGER_H