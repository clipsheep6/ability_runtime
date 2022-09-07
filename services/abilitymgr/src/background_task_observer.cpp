/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
#include "background_task_observer.h"
#include "hilog_wrapper.h"
#include <unistd.h>

namespace OHOS {
namespace AAFwk {
BackgroundTaskObserver::BackgroundTaskObserver()
{}

BackgroundTaskObserver::~BackgroundTaskObserver()
{}

void BackgroundTaskObserver::OnContinuousTaskStart(const std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>
    &continuousTaskCallbackInfo)
{
    HILOG_DEBUG("OnContinuousTaskStart, uid:%{public}d", continuousTaskCallbackInfo->GetCreatorUid());
    std::lock_guard<std::mutex> lock(bgTaskMutex_);
    bgTaskUids_.push_front(continuousTaskCallbackInfo->GetCreatorUid());
}

void BackgroundTaskObserver::OnContinuousTaskStop(const std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>
    &continuousTaskCallbackInfo)
{
    HILOG_DEBUG("OnContinuousTaskStop, uid:%{public}d", continuousTaskCallbackInfo->GetCreatorUid());
    std::lock_guard<std::mutex> lock(bgTaskMutex_);
    bgTaskUids_.remove(continuousTaskCallbackInfo->GetCreatorUid());
}

void BackgroundTaskObserver::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    HILOG_DEBUG("OnRemoteDied");
    int attemptNums = 0;
    while ((BackgroundTaskMgr::BackgroundTaskMgrHelper::SubscribeBackgroundTask(
        *(shared_from_this()))) != ERR_OK) {
        ++attemptNums;
        if (attemptNums > SUBSCRIBE_BACKGROUND_TASK_TRY) {
            HILOG_ERROR("subscribeBackgroundTask fail, attemptNums:%{public}d", attemptNums);
            return;
        }
        usleep(REPOLL_TIME_MICRO_SECONDS);
    }

    std::lock_guard<std::mutex> lock(bgTaskMutex_);
    bgTaskUids_.clear();
    std::vector<std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>> continuousTasks;
    BackgroundTaskMgr::BackgroundTaskMgrHelper::GetContinuousTaskApps(continuousTasks);
    for (size_t index = 0; index < continuousTasks.size(); index++) {
        bgTaskUids_.push_front(continuousTasks[index]->GetCreatorUid());
    }
}

bool BackgroundTaskObserver::IsBackgroundTaskUid(const int uid)
{
    std::lock_guard<std::mutex> lock(bgTaskMutex_);
    auto iter = find(bgTaskUids_.begin(), bgTaskUids_.end(), uid);
    if (iter != bgTaskUids_.end()) {
        return true;
    }
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS
#endif
