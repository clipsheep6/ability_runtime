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

#include <vector>
#include <sstream>
#include "hitrace_meter.h"
#include "parameters.h"
#include "hilog_tag_wrapper.h"
#include "app_state_observer_manager.h"
#include "app_mgr_service_inner.h"
#include "cache_process_manager.h"

namespace {
const std::string MAX_PROC_CACHE_NUM = "persist.sys.abilityms.maxProcessCacheNum";
const std::string PROCESS_CACHE_API_CHECK_CONFIG = "persist.sys.abilityms.processCacheApiCheck";
constexpr int32_t API12 = 12;
constexpr int32_t API_VERSION_MOD = 100;
}

namespace OHOS {
namespace AppExecFwk {

CacheProcessManager::CacheProcessManager()
{
    maxProcCacheNum_ = OHOS::system::GetIntParameter<int>(MAX_PROC_CACHE_NUM, 0);
    shouldCheckApi = OHOS::system::GetBoolParameter(PROCESS_CACHE_API_CHECK_CONFIG, true);
    TAG_LOGW(AAFwkTag::APPMGR, "maxProcCacheNum is =%{public}d", maxProcCacheNum_);
}

CacheProcessManager::~CacheProcessManager()
{
}

void CacheProcessManager::SetAppMgr(const std::weak_ptr<AppMgrServiceInner> &appMgr)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    appMgr_ = appMgr;
}

void CacheProcessManager::RefreshCacheNum()
{
    maxProcCacheNum_ = OHOS::system::GetIntParameter<int>(MAX_PROC_CACHE_NUM, 0);
    TAG_LOGW(AAFwkTag::APPMGR, "maxProcCacheNum is =%{public}d", maxProcCacheNum_);
}

bool CacheProcessManager::QueryEnableProcessCache()
{
    return maxProcCacheNum_ > 0;
}

bool CacheProcessManager::PenddingCacheProcess(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    if (!QueryEnableProcessCache()) {
        return false;
    }
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "nullptr precheck failed");
        return false;
    }
    if (appRecord->IsKeepAliveApp()) {
        TAG_LOGW(AAFwkTag::APPMGR, "Not cache keepalive process");
        return false;
    }
    {
        std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
        cachedAppRecordQueue_.push_back(appRecord);
        AddToApplicationSet(appRecord);
    }
    ShrinkAndKillCache();
    TAG_LOGI(AAFwkTag::APPMGR, "Pending %{public}s success, %{public}s", appRecord->GetName().c_str(),
        PrintCacheQueue().c_str());
    return true;
}

bool CacheProcessManager::CheckAndCacheProcess(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    if (!QueryEnableProcessCache()) {
        return false;
    }
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord nullptr precheck failed");
        return false;
    }
    if (!IsCachedProcess(appRecord)) {
        return false;
    }
    if (!IsAppAbilitiesEmpty(appRecord)) {
        TAG_LOGD(AAFwkTag::APPMGR, "%{public}s not cache for abilities not empty",
            appRecord->GetName().c_str());
        return true;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "%{public}s is cached, %{public}s", appRecord->GetName().c_str(),
        PrintCacheQueue().c_str());
    CheckAndNotifyCachedState(appRecord);
    return true;
}

bool CacheProcessManager::CheckAndNotifyCachedState(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord nullptr precheck failed");
        return false;
    }
    auto appMgrSptr = appMgr_.lock();
    if (appMgrSptr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr is nullptr");
        return false;
    }
    auto &bundleName = appRecord->GetBundleName();
    auto uid = appRecord->GetUid();
    std::shared_ptr<AppRunningRecord> notifyRecord = nullptr;
    {
        std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
        if (sameAppSet.find(bundleName) == sameAppSet.end() ||
            sameAppSet[bundleName].find(uid) == sameAppSet[bundleName].end()) {
            TAG_LOGD(AAFwkTag::APPMGR, "app set not found.");
            return false;
        }
        if (sameAppSet[bundleName][uid].size() == 0) {
            return false;
        }
        if (!appMgrSptr->IsAppProcessesAllCached(bundleName, uid, sameAppSet[bundleName][uid])) {
            TAG_LOGI(AAFwkTag::APPMGR, "Not all processes of one app is cached, abort notify");
            return false;
        }
        notifyRecord = *(sameAppSet[bundleName][uid].begin());
    }
    appMgrSptr->OnAppCacheStateChanged(notifyRecord, ApplicationState::APP_STATE_CACHED);
    TAG_LOGI(AAFwkTag::APPMGR, "app cached state is notified: %{public}s, uid:%{public}d", bundleName.c_str(), uid);
    return true;
}

bool CacheProcessManager::IsCachedProcess(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "appRecord nullptr precheck failed");
        return false;
    }
    std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
    for (auto& tmpAppRecord : cachedAppRecordQueue_) {
        if (tmpAppRecord == appRecord) {
            return true;
        }
    }
    return false;
}

void CacheProcessManager::OnProcessKilled(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!QueryEnableProcessCache()) {
        return;
    }
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord nullptr precheck failed");
        return;
    }
    CheckAndNotifyCachedState(appRecord);
    {
        std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
        srvExtRecords.erase(appRecord);
    }
    if (!IsCachedProcess(appRecord)) {
        return;
    }
    RemoveCacheRecord(appRecord);
    TAG_LOGI(AAFwkTag::APPMGR, "%{public}s is killed, %{public}s", appRecord->GetName().c_str(),
        PrintCacheQueue().c_str());
}

void CacheProcessManager::ReuseCachedProcess(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!QueryEnableProcessCache()) {
        return;
    }
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord nullptr precheck failed");
        return;
    }
    if (!IsCachedProcess(appRecord)) {
        return;
    }
    RemoveCacheRecord(appRecord);
    auto appMgrSptr = appMgr_.lock();
    if (appMgrSptr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr is nullptr");
        return;
    }
    appMgrSptr->OnAppCacheStateChanged(appRecord, ApplicationState::APP_STATE_READY);
    TAG_LOGI(AAFwkTag::APPMGR, "app none cached state is notified: %{public}s, uid: %{public}d, %{public}s",
        appRecord->GetBundleName().c_str(), appRecord->GetUid(), PrintCacheQueue().c_str());
}

bool CacheProcessManager::IsAppSupportProcessCache(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "appRecord nullptr precheck failed");
        return false;
    }
    auto appInfo = appRecord->GetApplicationInfo();
    if (appInfo == nullptr) {
        TAG_LOGD(AAFwkTag::APPMGR, "appinfo nullptr");
        return false;
    }
    auto actualVer = appInfo->apiTargetVersion % API_VERSION_MOD;
    if (shouldCheckApi && actualVer < API12) {
        TAG_LOGD(AAFwkTag::APPMGR, "App %{public}s 's apiTargetVersion has %{public}d, smaller than 12",
            appRecord->GetName().c_str(), actualVer);
        return false;
    }
    if (srvExtRecords.find(appRecord) != srvExtRecords.end()) {
        TAG_LOGD(AAFwkTag::APPMGR, "%{public}s of %{public}s is service, not support cache",
            appRecord->GetProcessName().c_str(), appRecord->GetBundleName().c_str());
        return false;
    }
    if (appRecord->IsKeepAliveApp()) {
        TAG_LOGD(AAFwkTag::APPMGR, "Keepalive app.");
        return false;
    }
    auto supportState = appRecord->GetSupportProcessCacheState();
    switch (supportState) {
        case SupportProcessCacheState::UNSPECIFIED:
            return true;
        case SupportProcessCacheState::SUPPORT:
            return true;
        case SupportProcessCacheState::NOT_SUPPORT:
            return false;
        default:
            return true;
    }
}

bool CacheProcessManager::IsAppShouldCache(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        return false;
    }
    if (!QueryEnableProcessCache()) {
        return false;
    }
    if (IsCachedProcess(appRecord)) {
        return true;
    }
    if (!IsAppSupportProcessCache(appRecord)) {
        return false;
    }
    return true;
}

bool CacheProcessManager::IsAppAbilitiesEmpty(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "appRecord nullptr precheck failed");
        return false;
    }
    auto allModuleRecord = appRecord->GetAllModuleRecord();
    for (auto moduleRecord : allModuleRecord) {
        if (moduleRecord != nullptr && !moduleRecord->GetAbilities().empty()) {
            return false;
        }
    }
    TAG_LOGD(AAFwkTag::APPMGR, "abilities all empty: %{public}s",
        appRecord->GetName().c_str());
    return true;
}

int CacheProcessManager::GetCurrentCachedProcNum()
{
    std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
    return static_cast<int>(cachedAppRecordQueue_.size());
}

void CacheProcessManager::RemoveCacheRecord(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
    for (auto it = cachedAppRecordQueue_.begin(); it != cachedAppRecordQueue_.end();) {
        if (appRecord == *it) {
            RemoveFromApplicationSet(*it);
            it = cachedAppRecordQueue_.erase(it);
        } else {
            it++;
        }
    }
}

void CacheProcessManager::ShrinkAndKillCache()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    if (maxProcCacheNum_ <= 0) {
        TAG_LOGI(AAFwkTag::APPMGR, "Cache disabled.");
        return;
    }
    std::vector<std::shared_ptr<AppRunningRecord>> cleanList;
    {
        std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
        while (GetCurrentCachedProcNum() > maxProcCacheNum_) {
            const auto& tmpAppRecord = cachedAppRecordQueue_.front();
            cachedAppRecordQueue_.pop_front();
            RemoveFromApplicationSet(tmpAppRecord);
            if (tmpAppRecord == nullptr) {
                continue;
            }
            cleanList.push_back(tmpAppRecord);
            TAG_LOGI(AAFwkTag::APPMGR, "need clean record %{public}s, current =%{public}d",
                tmpAppRecord->GetName().c_str(), GetCurrentCachedProcNum());
        }
    }
    for (auto& tmpAppRecord : cleanList) {
        KillProcessByRecord(tmpAppRecord);
    }
}

bool CacheProcessManager::KillProcessByRecord(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "appRecord nullptr precheck failed");
        return false;
    }
    auto priorityObject = appRecord->GetPriorityObject();
    if (priorityObject == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "priorityObject is nullptr.");
        return false;
    }
    auto pid = priorityObject->GetPid();
    if (pid < 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "Pid error");
        return false;
    }
    auto appMgrSptr = appMgr_.lock();
    if (appMgrSptr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr is nullptr");
        return false;
    }
    auto result = appMgrSptr->KillProcessByPid(pid);
    if (result < 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "Kill application directly failed, pid: %{public}d", pid);
        return false;
    }
    return true;
}

std::string CacheProcessManager::PrintCacheQueue()
{
    std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
    std::stringstream ss;
    ss << "queue size: " << cachedAppRecordQueue_.size() << ", record in queue: ";
    for (auto& record : cachedAppRecordQueue_) {
        if (record == nullptr) {
            ss << "null, ";
        } else {
            ss << record->GetName() << ", ";
        }
    }
    ss << ".";
    return ss.str();
}

void CacheProcessManager::AddToApplicationSet(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        return;
    }
    auto &bundleName = appRecord->GetBundleName();
    std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
    if (sameAppSet.find(bundleName) == sameAppSet.end()) {
        std::map<int32_t, std::set<std::shared_ptr<AppRunningRecord>>> uidMap;
        std::set<std::shared_ptr<AppRunningRecord>> recordSet;
        recordSet.insert(appRecord);
        uidMap.insert(std::make_pair(appRecord->GetUid(), recordSet));
        sameAppSet.insert(std::make_pair(bundleName, uidMap));
    }
    auto uid = appRecord->GetUid();
    if (sameAppSet[bundleName].find(uid) == sameAppSet[bundleName].end()) {
        std::set<std::shared_ptr<AppRunningRecord>> recordSet;
        recordSet.insert(appRecord);
        sameAppSet[bundleName].insert(std::make_pair(uid, recordSet));
        return;
    }
    sameAppSet[bundleName][uid].insert(appRecord);
}

void CacheProcessManager::RemoveFromApplicationSet(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        return;
    }
    auto &bundleName = appRecord->GetBundleName();
    std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
    if (sameAppSet.find(bundleName) == sameAppSet.end()) {
        return;
    }
    auto uid = appRecord->GetUid();
    if (sameAppSet[bundleName].find(uid) == sameAppSet[bundleName].end()) {
        return;
    }
    sameAppSet[bundleName][uid].erase(appRecord);
    if (sameAppSet[bundleName][uid].size() == 0) {
        sameAppSet[bundleName].erase(uid);
    }
    if (sameAppSet[bundleName].size() == 0) {
        sameAppSet.erase(bundleName);
    }
}

void CacheProcessManager::UpdateTypeByToken(const sptr<IRemoteObject> &token,
    const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!QueryEnableProcessCache()) {
        return;
    }
    if (token == nullptr || appRecord == nullptr) {
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (abilityRecord == nullptr) {
        return;
    }
    UpdateTypeByAbility(abilityRecord, appRecord);
}

void CacheProcessManager::UpdateTypeByAbility(const std::shared_ptr<AbilityRunningRecord> &abilityRecord,
    const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!QueryEnableProcessCache()) {
        return;
    }
    if (abilityRecord == nullptr || appRecord == nullptr) {
        return;
    }
    auto abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        return;
    }
    auto type = abilityInfo->type;
    if (type == AppExecFwk::AbilityType::EXTENSION &&
        abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE) {
        std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
        srvExtRecords.insert(appRecord);
        // incase service record is in cache queue due to delay
        RemoveCacheRecord(appRecord);
        TAG_LOGD(AAFwkTag::APPMGR, "%{public}s is service, will not cache, service records size: %{public}zu.",
            appRecord->GetBundleName().c_str(), srvExtRecords.size());
    }
}
} // namespace OHOS
} // namespace AppExecFwk