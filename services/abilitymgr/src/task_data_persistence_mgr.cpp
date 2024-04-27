/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "task_data_persistence_mgr.h"
#include "ability_util.h"
#include "directory_ex.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AAFwk {
TaskDataPersistenceMgr::TaskDataPersistenceMgr()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "TaskDataPersistenceMgr instance is created");
}

TaskDataPersistenceMgr::~TaskDataPersistenceMgr()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "TaskDataPersistenceMgr instance is destroyed");
}

bool TaskDataPersistenceMgr::Init(int userId)
{
    if (!handler_) {
        handler_ = TaskHandlerWrap::GetFfrtHandler();
        CHECK_POINTER_RETURN_BOOL(handler_);
    }

    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (missionDataStorageMgr_.find(userId) == missionDataStorageMgr_.end()) {
        currentMissionDataStorage_ = std::make_shared<MissionDataStorage>(userId);
        missionDataStorageMgr_.insert(std::make_pair(userId, currentMissionDataStorage_));
    } else {
        currentMissionDataStorage_ = missionDataStorageMgr_[userId];
    }
    currentUserId_ = userId;

    CHECK_POINTER_RETURN_BOOL(currentMissionDataStorage_);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Init success.");
    return true;
}

bool TaskDataPersistenceMgr::LoadAllMissionInfo(std::list<InnerMissionInfo> &missionInfoList)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (!currentMissionDataStorage_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "currentMissionDataStorage_ is nullptr");
        return false;
    }

    return currentMissionDataStorage_->LoadAllMissionInfo(missionInfoList);
}

bool TaskDataPersistenceMgr::SaveMissionInfo(const InnerMissionInfo &missionInfo)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (!handler_ || !currentMissionDataStorage_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "handler_ or currentMissionDataStorage_ is nullptr");
        return false;
    }

    std::weak_ptr<MissionDataStorage> weakPtr(currentMissionDataStorage_);
    std::function<void()> SaveMissionInfoFunc = [weakPtr, missionInfo]() {
        auto missionDataStorage = weakPtr.lock();
        if (missionDataStorage) {
            missionDataStorage->SaveMissionInfo(missionInfo);
        }
    };
    handler_->SubmitTask(SaveMissionInfoFunc, SAVE_MISSION_INFO);
    return true;
}

bool TaskDataPersistenceMgr::DeleteMissionInfo(int missionId)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (!handler_ || !currentMissionDataStorage_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "handler_ or currentMissionDataStorage_ is nullptr");
        return false;
    }

    std::weak_ptr<MissionDataStorage> weakPtr(currentMissionDataStorage_);
    std::function<void()> DeleteMissionInfoFunc = [weakPtr, missionId]() {
        auto missionDataStorage = weakPtr.lock();
        if (missionDataStorage) {
            missionDataStorage->DeleteMissionInfo(missionId);
        }
    };
    handler_->SubmitTask(DeleteMissionInfoFunc, DELETE_MISSION_INFO);
    return true;
}

bool TaskDataPersistenceMgr::RemoveUserDir(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (currentUserId_ == userId) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "can not removed current user dir");
        return false;
    }
    std::string userDir = std::string(TASK_DATA_FILE_BASE_PATH) + "/" + std::to_string(userId);
    bool ret = OHOS::ForceRemoveDirectory(userDir);
    if (!ret) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remove user dir %{public}s failed.", userDir.c_str());
        return false;
    }
    return true;
}

bool TaskDataPersistenceMgr::SaveMissionSnapshot(int missionId, const MissionSnapshot& snapshot)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (!handler_ || !currentMissionDataStorage_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "snapshot: handler_ or currentMissionDataStorage_ is nullptr");
        return false;
    }

    std::weak_ptr<MissionDataStorage> weakPtr(currentMissionDataStorage_);
    std::function<void()> SaveMissionSnapshotFunc = [weakPtr, missionId, snapshot]() {
        auto missionDataStorage = weakPtr.lock();
        if (missionDataStorage) {
            missionDataStorage->SaveMissionSnapshot(missionId, snapshot);
        }
    };
    handler_->SubmitTask(SaveMissionSnapshotFunc, SAVE_MISSION_SNAPSHOT);
    return true;
}

#ifdef SUPPORT_GRAPHICS
std::shared_ptr<Media::PixelMap> TaskDataPersistenceMgr::GetSnapshot(int missionId) const
{
    if (!currentMissionDataStorage_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "snapshot: currentMissionDataStorage_ is nullptr");
        return nullptr;
    }
    return currentMissionDataStorage_->GetSnapshot(missionId);
}
#endif

bool TaskDataPersistenceMgr::GetMissionSnapshot(int missionId, MissionSnapshot& snapshot, bool isLowResolution)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (!currentMissionDataStorage_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "snapshot: currentMissionDataStorage_ is nullptr");
        return false;
    }
    return currentMissionDataStorage_->GetMissionSnapshot(missionId, snapshot, isLowResolution);
}
}  // namespace AAFwk
}  // namespace OHOS
