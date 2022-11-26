/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_MISSION_LISTENER_STUB_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_MISSION_LISTENER_STUB_H

#include <gmock/gmock.h>

#define private public
#include "mission_listener_stub.h"
#undef private

namespace OHOS {
namespace AAFwk {
class MockMissionListenerStub : public MissionListenerStub {
public:
    MockMissionListenerStub() = default;
    virtual ~MockMissionListenerStub() = default;

    MOCK_METHOD1(OnMissionCreated, void(int32_t missionId));
    MOCK_METHOD1(OnMissionDestroyed, void(int32_t missionId));
    MOCK_METHOD1(OnMissionSnapshotChanged, void(int32_t missionId));
    MOCK_METHOD1(OnMissionMovedToFront, void(int32_t missionId));
    MOCK_METHOD2(OnMissionIconUpdated, void(int32_t missionId, const std::shared_ptr<OHOS::Media::PixelMap> &icon));
    MOCK_METHOD1(OnMissionClosed, void(int32_t missionId));
    MOCK_METHOD1(OnMissionLabelUpdated, void(int32_t missionId));
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_MISSION_LISTENER_STUB_H
