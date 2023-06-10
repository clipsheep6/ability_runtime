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

#include "my_mission_listener.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
MyMissionListener::MyMissionListener()
{}

MyMissionListener::~MyMissionListener()
{}

void MyMissionListener::OnMissionFocused(int32_t missionId)
{
    HILOG_INFO("OnMissionFocused:%{public}d", missionId);
}

void MyMissionListener::OnMissionUnfocused(int32_t missionId)
{
    HILOG_INFO("OnMissionUnfocused:%{public}d", missionId);
}

void MyMissionListener::OnMissionClosed(int32_t missionId)
{
    HILOG_INFO("OnMissionClosed:%{public}d", missionId);
}

void MyMissionListener::OnMissionCreated(int32_t missionId) {}

void MyMissionListener::OnMissionDestroyed(int32_t missionId) {}

void MyMissionListener::OnMissionSnapshotChanged(int32_t missionId) {}

void MyMissionListener::OnMissionMovedToFront(int32_t missionId) {}

void MyMissionListener::OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<OHOS::Media::PixelMap> &icon) {}

void MyMissionListener::OnMissionLabelUpdated(int32_t missionId) {}
}  // namespace AAFwk
}  // namespace OHOS
