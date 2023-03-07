/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "mission_info_mgr.h"

#include "ability_manager_service.h"
#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"
#ifdef SUPPORT_GRAPHICS
#include "pixel_map.h"
#include "securec.h"
#endif

namespace OHOS {
namespace AAFwk {
MissionInfoMgr::MissionInfoMgr()
{
    HILOG_INFO("MissionInfoMgr instance is created");
}

MissionInfoMgr::~MissionInfoMgr()
{
    HILOG_INFO("MissionInfoMgr instance is destroyed");
}

bool MissionInfoMgr::GenerateMissionId(int32_t &missionId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (currentMissionId_ == MAX_MISSION_ID) {
        currentMissionId_ = MIN_MISSION_ID;
    }

    for (int32_t index = currentMissionId_; index < MAX_MISSION_ID; index++) {
        if (missionIdMap_.find(index) == missionIdMap_.end()) {
            missionId = index;
            missionIdMap_[missionId] = false;
            currentMissionId_ = missionId + 1;
            return true;
        }
    }

    HILOG_ERROR("cannot generate mission id");
    return false;
}

bool MissionInfoMgr::Init(int userId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!taskDataPersistenceMgr_) {
        taskDataPersistenceMgr_ = DelayedSingleton<TaskDataPersistenceMgr>::GetInstance();
        if (!taskDataPersistenceMgr_) {
            HILOG_ERROR("taskDataPersistenceMgr_ is nullptr");
            return false;
        }
    }

    if (!taskDataPersistenceMgr_->Init(userId)) {
        return false;
    }

    missionInfoList_.clear();
    missionIdMap_.clear();
    if (!LoadAllMissionInfo()) {
        return false;
    }

    return true;
}

bool MissionInfoMgr::AddMissionInfo(const InnerMissionInfo &missionInfo)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto id = missionInfo.missionInfo.id;
    if (missionIdMap_.find(id) != missionIdMap_.end() && missionIdMap_[id]) {
        HILOG_ERROR("add mission info failed, missionId %{public}d already exists", id);
        return false;
    }

    auto listIter = missionInfoList_.begin();
    for (; listIter != missionInfoList_.end(); listIter++) {
        if (listIter->missionInfo.time < missionInfo.missionInfo.time) {
            break;  // first listIter->time < missionInfo.time
        }
    }

    if (!taskDataPersistenceMgr_->SaveMissionInfo(missionInfo)) {
        HILOG_ERROR("save mission info failed");
        return false;
    }

    missionInfoList_.insert(listIter, missionInfo);
    missionIdMap_[id] = true;
    return true;
}

bool MissionInfoMgr::UpdateMissionInfo(const InnerMissionInfo &missionInfo)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto id = missionInfo.missionInfo.id;
    if (missionIdMap_.find(id) == missionIdMap_.end() || !missionIdMap_[id]) {
        HILOG_ERROR("update mission info failed, missionId %{public}d not exists", id);
        return false;
    }

    auto listIter = missionInfoList_.begin();
    for (; listIter != missionInfoList_.end(); listIter++) {
        if (listIter->missionInfo.id == id) {
            break;
        }
    }

    if (listIter == missionInfoList_.end()) {
        HILOG_ERROR("update mission info failed, missionId %{public}d not exists", id);
        return false;
    }

    if (missionInfo.missionInfo.time == listIter->missionInfo.time) {
        // time not changes, no need sort again
        *listIter = missionInfo;
        if (!taskDataPersistenceMgr_->SaveMissionInfo(missionInfo)) {
            HILOG_ERROR("save mission info failed.");
            return false;
        }
        return true;
    }

    missionInfoList_.erase(listIter);
    missionIdMap_.erase(id);
    return AddMissionInfo(missionInfo);
}

bool MissionInfoMgr::DeleteMissionInfo(int missionId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (missionIdMap_.find(missionId) == missionIdMap_.end()) {
        HILOG_WARN("missionId %{public}d not exists, no need delete", missionId);
        return true;
    }

    if (!missionIdMap_[missionId]) {
        HILOG_WARN("missionId %{public}d distributed but not saved, no need delete", missionId);
        missionIdMap_.erase(missionId);
        return true;
    }

    if (!taskDataPersistenceMgr_) {
        HILOG_ERROR("taskDataPersistenceMgr_ is nullptr");
        return false;
    }

    if (!taskDataPersistenceMgr_->DeleteMissionInfo(missionId)) {
        HILOG_ERROR("delete mission info failed");
        return false;
    }

    for (auto listIter = missionInfoList_.begin(); listIter != missionInfoList_.end(); listIter++) {
        if (listIter->missionInfo.id == missionId) {
            missionInfoList_.erase(listIter);
            break;
        }
    }

    missionIdMap_.erase(missionId);
    return true;
}

bool MissionInfoMgr::DeleteAllMissionInfos(const std::shared_ptr<MissionListenerController> &listenerController)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!taskDataPersistenceMgr_) {
        HILOG_ERROR("taskDataPersistenceMgr_ is nullptr");
        return false;
    }

    auto abilityMs_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();

    for (auto listIter = missionInfoList_.begin(); listIter != missionInfoList_.end();) {
        if (!((listIter->missionInfo.lockedState) || (abilityMs_->IsBackgroundTaskUid(listIter->uid)))) {
            missionIdMap_.erase(listIter->missionInfo.id);
            taskDataPersistenceMgr_->DeleteMissionInfo(listIter->missionInfo.id);
            if (listenerController) {
                listenerController->NotifyMissionDestroyed(listIter->missionInfo.id);
            }
            missionInfoList_.erase(listIter++);
        } else {
            ++listIter;
        }
    }
    return true;
}

static bool DoesNotShowInTheMissionList(const InnerMissionInfo &mission)
{
    bool isStartByCall = false;
    switch (static_cast<StartMethod>(mission.startMethod)) {
        case StartMethod::START_CALL:
            isStartByCall = true;
            break;
        default:
            isStartByCall = false;
    }
    return (isStartByCall && !mission.missionInfo.want.GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false));
}

int MissionInfoMgr::GetMissionInfos(int32_t numMax, std::vector<MissionInfo> &missionInfos)
{
    HILOG_INFO("GetMissionInfos, numMax:%{public}d", numMax);
    if (numMax < 0) {
        return -1;
    }

    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto &mission : missionInfoList_) {
        if (static_cast<int>(missionInfos.size()) >= numMax) {
            break;
        }

        if (DoesNotShowInTheMissionList(mission)) {
            HILOG_INFO("MissionId[%{public}d] don't show in mission list", mission.missionInfo.id);
            continue;
        }
        MissionInfo info = mission.missionInfo;
        missionInfos.emplace_back(info);
    }

    return 0;
}

int MissionInfoMgr::GetMissionInfoById(int32_t missionId, MissionInfo &missionInfo)
{
    HILOG_INFO("GetMissionInfoById, missionId:%{public}d", missionId);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (missionIdMap_.find(missionId) == missionIdMap_.end()) {
        HILOG_ERROR("missionId %{public}d not exists, get mission info failed", missionId);
        return -1;
    }

    auto it = std::find_if(missionInfoList_.begin(), missionInfoList_.end(),
        [&missionId](const InnerMissionInfo item) {
            return item.missionInfo.id == missionId;
        }
    );
    if (it == missionInfoList_.end()) {
        HILOG_ERROR("no such mission:%{public}d", missionId);
        return -1;
    }

    if (DoesNotShowInTheMissionList(*it)) {
        HILOG_INFO("MissionId[%{public}d] don't show in mission list", (*it).missionInfo.id);
        return -1;
    }

    HILOG_INFO("GetMissionInfoById, find missionId missionId:%{public}d", missionId);
    missionInfo = (*it).missionInfo;
    return 0;
}

int MissionInfoMgr::GetInnerMissionInfoById(int32_t missionId, InnerMissionInfo &innerMissionInfo)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (missionIdMap_.find(missionId) == missionIdMap_.end()) {
        HILOG_ERROR("missionId %{public}d not exists, get inner mission info failed", missionId);
        return MISSION_NOT_FOUND;
    }

    auto it = std::find_if(missionInfoList_.begin(), missionInfoList_.end(),
        [&missionId](const InnerMissionInfo item) {
            return item.missionInfo.id == missionId;
        }
    );
    if (it == missionInfoList_.end()) {
        HILOG_ERROR("no such mission:%{public}d", missionId);
        return MISSION_NOT_FOUND;
    }
    innerMissionInfo = *it;
    return 0;
}

bool MissionInfoMgr::FindReusedMissionInfo(const std::string &missionName,
    const std::string &flag, bool isFindRecentStandard, InnerMissionInfo &info)
{
    if (missionName.empty()) {
        return false;
    }

    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto it = std::find_if(missionInfoList_.begin(), missionInfoList_.end(),
        [&missionName, &flag, &isFindRecentStandard](const InnerMissionInfo item) {
            if (missionName != item.missionName) {
                return false;
            }

            // already sorted, return head of list
            if (isFindRecentStandard && item.launchMode == static_cast<int32_t>(AppExecFwk::LaunchMode::STANDARD)) {
                return true;
            }

            if (item.launchMode == static_cast<int32_t>(AppExecFwk::LaunchMode::SINGLETON)) {
                return true;
            }

            if (item.launchMode == static_cast<int32_t>(AppExecFwk::LaunchMode::SPECIFIED)) {
                return flag == item.specifiedFlag;
            }
            return false;
        }
    );
    if (it == missionInfoList_.end()) {
        HILOG_WARN("can not find target singleton mission:%{public}s", missionName.c_str());
        return false;
    }
    info = *it;
    return true;
}

int MissionInfoMgr::UpdateMissionLabel(int32_t missionId, const std::string& label)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!taskDataPersistenceMgr_) {
        HILOG_ERROR("task data persist not init.");
        return -1;
    }
    auto it = find_if(missionInfoList_.begin(), missionInfoList_.end(), [missionId](const InnerMissionInfo &info) {
        return missionId == info.missionInfo.id;
    });
    if (it == missionInfoList_.end()) {
        HILOG_ERROR("UpdateMissionLabel failed, missionId %{public}d not exists", missionId);
        return -1;
    }

    it->missionInfo.label = label;
    if (!taskDataPersistenceMgr_->SaveMissionInfo(*it)) {
        HILOG_ERROR("save mission info failed.");
        return -1;
    }
    return 0;
}

bool MissionInfoMgr::LoadAllMissionInfo()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!taskDataPersistenceMgr_) {
        HILOG_ERROR("taskDataPersistenceMgr_ is nullptr");
        return false;
    }

    if (!taskDataPersistenceMgr_->LoadAllMissionInfo(missionInfoList_)) {
        HILOG_ERROR("load mission info failed");
        return false;
    }

    // sort by time
    auto cmpFunc = [] (const InnerMissionInfo &infoBase, const InnerMissionInfo &infoCmp) {
        return infoBase.missionInfo.time > infoCmp.missionInfo.time;
    };
    missionInfoList_.sort(cmpFunc);

    for (const auto &info : missionInfoList_) {
        missionIdMap_[info.missionInfo.id] = true;
    }
    return true;
}

void MissionInfoMgr::HandleUnInstallApp(const std::string &bundleName, int32_t uid, std::list<int32_t> &missions)
{
    HILOG_INFO("HandleUnInstallApp, bundleName:%{public}s, uid:%{public}d", bundleName.c_str(), uid);
    GetMatchedMission(bundleName, uid, missions);
    if (missions.empty()) {
        return;
    }

    for (auto missionId : missions) {
        DeleteMissionInfo(missionId);
    }
}

void MissionInfoMgr::GetMatchedMission(const std::string &bundleName, int32_t uid, std::list<int32_t> &missions)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (const auto& innerMissionInfo : missionInfoList_) {
        if (innerMissionInfo.bundleName == bundleName && innerMissionInfo.uid == uid) {
            missions.push_back(innerMissionInfo.missionInfo.id);
        }
    }
}

void MissionInfoMgr::Dump(std::vector<std::string> &info)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (const auto& innerMissionInfo : missionInfoList_) {
        innerMissionInfo.Dump(info);
    }
}

void MissionInfoMgr::RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    snapshotHandler_ = handler;
}

bool MissionInfoMgr::UpdateMissionSnapshot(int32_t missionId, const sptr<IRemoteObject>& abilityToken,
    MissionSnapshot& missionSnapshot, bool isLowResolution)
{
    HILOG_INFO("Update mission snapshot, missionId:%{public}d.", missionId);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto it = find_if(missionInfoList_.begin(), missionInfoList_.end(), [missionId](const InnerMissionInfo &info) {
        return missionId == info.missionInfo.id;
    });
    if (it == missionInfoList_.end()) {
        HILOG_ERROR("snapshot: get mission failed, missionId %{public}d not exists", missionId);
        return false;
    }
    if (!snapshotHandler_) {
        HILOG_ERROR("snapshot: snapshotHandler_ is nullptr");
        return false;
    }
    Snapshot snapshot;
    int32_t result = snapshotHandler_->GetSnapshot(abilityToken, snapshot);
    if (result != 0) {
        HILOG_ERROR("snapshot: get WMS snapshot failed, result = %{public}d", result);
        return false;
    }
    if (!taskDataPersistenceMgr_) {
        HILOG_ERROR("snapshot: taskDataPersistenceMgr_ is nullptr");
        return false;
    }

#ifdef SUPPORT_GRAPHICS
    if (missionSnapshot.isPrivate) {
        CreateWhitePixelMap(snapshot);
    }
    missionSnapshot.snapshot = isLowResolution ?
        MissionDataStorage::GetReducedPixelMap(snapshot.GetPixelMap()) : snapshot.GetPixelMap();
#endif
    missionSnapshot.topAbility = it->missionInfo.want.GetElement();

    MissionSnapshot savedSnapshot = missionSnapshot;
#ifdef SUPPORT_GRAPHICS
    savedSnapshot.snapshot = snapshot.GetPixelMap();
#endif
    {
        std::lock_guard<std::mutex> lock(savingSnapshotLock_);
        auto search = savingSnapshot_.find(missionId);
        if (search == savingSnapshot_.end()) {
            savingSnapshot_[missionId] = 1;
        } else {
            auto savingCount = search->second + 1;
            savingSnapshot_.insert_or_assign(missionId, savingCount);
        }
    }
    if (!taskDataPersistenceMgr_->SaveMissionSnapshot(missionId, savedSnapshot)) {
        HILOG_ERROR("snapshot: save mission snapshot failed");
        CompleteSaveSnapshot(missionId);
        return false;
    }
    HILOG_INFO("snapshot: update mission snapshot success");
    return true;
}

void MissionInfoMgr::CompleteSaveSnapshot(int32_t missionId)
{
    std::unique_lock<std::mutex> lock(savingSnapshotLock_);
    auto search = savingSnapshot_.find(missionId);
    if (search != savingSnapshot_.end()) {
        auto savingCount = search->second - 1;
        if (savingCount == 0) {
            savingSnapshot_.erase(search);
            waitSavingCondition_.notify_one();
        } else {
            savingSnapshot_.insert_or_assign(missionId, savingCount);
        }
    }
}

#ifdef SUPPORT_GRAPHICS
std::shared_ptr<Media::PixelMap> MissionInfoMgr::GetSnapshot(int32_t missionId) const
{
    HILOG_INFO("mission_list_info GetSnapshot, missionId:%{public}d", missionId);
    auto it = find_if(missionInfoList_.begin(), missionInfoList_.end(), [missionId](const InnerMissionInfo &info) {
        return missionId == info.missionInfo.id;
    });
    if (it == missionInfoList_.end()) {
        HILOG_ERROR("snapshot: get mission failed, missionId %{public}d not exists", missionId);
        return nullptr;
    }
    if (!taskDataPersistenceMgr_) {
        HILOG_ERROR("snapshot: taskDataPersistenceMgr_ is nullptr");
        return nullptr;
    }

    return taskDataPersistenceMgr_->GetSnapshot(missionId);
}
#endif

bool MissionInfoMgr::GetMissionSnapshot(int32_t missionId, const sptr<IRemoteObject>& abilityToken,
    MissionSnapshot& missionSnapshot, bool isLowResolution, bool force)
{
    HILOG_INFO("mission_list_info GetMissionSnapshot, missionId:%{public}d, force:%{public}d", missionId, force);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto it = find_if(missionInfoList_.begin(), missionInfoList_.end(), [missionId](const InnerMissionInfo &info) {
        return missionId == info.missionInfo.id;
    });
    if (it == missionInfoList_.end()) {
        HILOG_ERROR("snapshot: get mission failed, missionId %{public}d not exists", missionId);
        return false;
    }
    if (!taskDataPersistenceMgr_) {
        HILOG_ERROR("snapshot: taskDataPersistenceMgr_ is nullptr");
        return false;
    }

    if (force) {
        HILOG_INFO("force to get snapshot");
        return UpdateMissionSnapshot(missionId, abilityToken, missionSnapshot, isLowResolution);
    }

    {
        std::unique_lock<std::mutex> lock(savingSnapshotLock_);
        auto search = savingSnapshot_.find(missionId);
        if (search != savingSnapshot_.end()) {
            auto savingSnapshotTimeout = 100; // ms
            std::chrono::milliseconds timeout { savingSnapshotTimeout };
            auto waitingCount = 5;
            auto waitingNum = 0;
            while (waitSavingCondition_.wait_for(lock, timeout) == std::cv_status::no_timeout) {
                ++waitingNum;
                auto iter = savingSnapshot_.find(missionId);
                if (iter == savingSnapshot_.end() || waitingNum == waitingCount) {
                    HILOG_INFO("Saved successfully or waiting failed.");
                    break;
                }
            }
        }
    }

    if (taskDataPersistenceMgr_->GetMissionSnapshot(missionId, missionSnapshot, isLowResolution)) {
        missionSnapshot.topAbility = it->missionInfo.want.GetElement();
        HILOG_ERROR("mission_list_info GetMissionSnapshot, find snapshot OK, missionId:%{public}d", missionId);
        return true;
    }
    HILOG_INFO("snapshot: storage mission snapshot not exists, create new snapshot");
    return UpdateMissionSnapshot(missionId, abilityToken, missionSnapshot, isLowResolution);
}

#ifdef SUPPORT_GRAPHICS
void MissionInfoMgr::CreateWhitePixelMap(Snapshot &snapshot) const
{
    if (snapshot.GetPixelMap() == nullptr) {
        HILOG_ERROR("CreateWhitePixelMap error.");
        return;
    }
    int32_t dataLength = snapshot.GetPixelMap()->GetByteCount();
    const uint8_t *pixelData = snapshot.GetPixelMap()->GetPixels();
    uint8_t *data = const_cast<uint8_t *>(pixelData);
    if (memset_s(data, dataLength, 0xff, dataLength) != EOK) {
        HILOG_ERROR("CreateWhitePixelMap memset_s error.");
    }
}
#endif
}  // namespace AAFwk
}  // namespace OHOS
