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

#ifndef OHOS_ABILITY_RUNTIME_CACHE_PROCESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CACHE_PROCESS_MANAGER_H

#include <memory>
#include <deque>
#include <mutex>
#include <set>
#include "singleton.h"
#include "app_running_record.h"
#include "cpp/mutex.h"

namespace OHOS {
namespace AppExecFwk {

class CacheProcessManager {
    DECLARE_DELAYED_SINGLETON(CacheProcessManager);
public:
    bool QueryEnableProcessCache();
    void SetAppMgr(const std::weak_ptr<AppMgrServiceInner> &appMgr);
    bool PenddingCacheProcess(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool CheckAndCacheProcess(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool IsCachedProcess(const std::shared_ptr<AppRunningRecord> &appRecord);
    void OnProcessKilled(const std::shared_ptr<AppRunningRecord> &appRecord);
    void ReuseCachedProcess(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool IsAppSupportProcessCache(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool IsAppShouldCache(const std::shared_ptr<AppRunningRecord> &appRecord);
    void RefreshCacheNum();
    std::string PrintCacheQueue();
    void UpdateTypeByToken(const sptr<IRemoteObject> &token, const std::shared_ptr<AppRunningRecord> &appRecord);
    void UpdateTypeByAbility(const std::shared_ptr<AbilityRunningRecord> &abilityRecord,
        const std::shared_ptr<AppRunningRecord> &appRecord);
    void PrepareActivateCache(const std::shared_ptr<AppRunningRecord> &appRecord);
private:
    bool IsAppAbilitiesEmpty(const std::shared_ptr<AppRunningRecord> &appRecord);
    int GetCurrentCachedProcNum();
    void RemoveCacheRecord(const std::shared_ptr<AppRunningRecord> &appRecord);
    void ShrinkAndKillCache();
    bool KillProcessByRecord(const std::shared_ptr<AppRunningRecord> &appRecord);
    void AddToApplicationSet(const std::shared_ptr<AppRunningRecord> &appRecord);
    void RemoveFromApplicationSet(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool CheckAndNotifyCachedState(const std::shared_ptr<AppRunningRecord> &appRecord);
    int32_t maxProcCacheNum_ = 0;
    std::deque<std::shared_ptr<AppRunningRecord>> cachedAppRecordQueue_;
    ffrt::recursive_mutex cacheQueueMtx;
    std::weak_ptr<AppMgrServiceInner> appMgr_;
    bool shouldCheckApi = true;
    // bundleName->uid->record
    std::map<std::string, std::map<int32_t, std::set<std::shared_ptr<AppRunningRecord>>>> sameAppSet;
    // stores records that are servcie extension
    std::set<std::shared_ptr<AppRunningRecord>> srvExtRecords;
};
} // namespace OHOS
} // namespace AppExecFwk

#endif