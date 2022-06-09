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

#include "background_task_observer.h"

namespace OHOS {
namespace AAFwk {
BackgroundTaskObserver::BackgroundTaskObserver()
{}

BackgroundTaskObserver::~BackgroundTaskObserver()
{}

void BackgroundTaskObserver::OnContinuousTaskStart(const std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>
    &continuousTaskCallbackInfo)
{
    std::lock_guard<std::mutex> lock(bgTaskMutex_);
    bgTaskUids_.push_front(continuousTaskCallbackInfo->GetCreatorUid());
}

void BackgroundTaskObserver::OnContinuousTaskStop(const std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>
    &continuousTaskCallbackInfo)
{
    std::lock_guard<std::mutex> lock(bgTaskMutex_);
    bgTaskUids_.remove(continuousTaskCallbackInfo->GetCreatorUid());
}

std::list<int> BackgroundTaskObserver::GetBgTaskUids()
{
    return bgTaskUids_;
}
}  // namespace AAFwk
}  // namespace OHOS