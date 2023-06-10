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

#ifndef OHOS_ABILITY_RUNTIME_MY_MISSION_LISTENER_H
#define OHOS_ABILITY_RUNTIME_MY_MISSION_LISTENER_H

#include "mission_listener_stub.h"

namespace OHOS {
namespace AAFwk {
class MyMissionListener : public AAFwk::MissionListenerStub {
public:
    MyMissionListener();
    virtual ~MyMissionListener() override;

    virtual void OnMissionCreated(int32_t missionId) override;
    virtual void OnMissionDestroyed(int32_t missionId) override;
    virtual void OnMissionSnapshotChanged(int32_t missionId) override;
    virtual void OnMissionMovedToFront(int32_t missionId) override;
    virtual void OnMissionFocused(int32_t missionId) override;
    virtual void OnMissionUnfocused(int32_t missionId) override;
    virtual void OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<OHOS::Media::PixelMap> &icon) override;
    virtual void OnMissionClosed(int32_t missionId) override;
    virtual void OnMissionLabelUpdated(int32_t missionId) override;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif /* OHOS_ABILITY_RUNTIME_MY_MISSION_LISTENER_H */
