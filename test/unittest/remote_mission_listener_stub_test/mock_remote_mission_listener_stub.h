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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_REMOTE_MISSION_LISTENER_STUB_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_REMOTE_MISSION_LISTENER_STUB_H

#include <gmock/gmock.h>

#define private public
#include "remote_mission_listener_stub.h"
#undef private

namespace OHOS {
namespace AAFwk {
class MockRemoteMissionListenerStub : public RemoteMissionListenerStub {
public:
    MockRemoteMissionListenerStub() = default;
    virtual ~MockRemoteMissionListenerStub() = default;

    MOCK_METHOD1(NotifyMissionsChanged, void(const std::string& deviceId));
    MOCK_METHOD2(NotifySnapshot, void(const std::string& deviceId, int32_t missionId));
    MOCK_METHOD2(NotifyNetDisconnect, void(const std::string& deviceId, int32_t state));
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_REMOTE_MISSION_LISTENER_STUB_H
