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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_MISSION_LISTENER_STUB_MOCK_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_MISSION_LISTENER_STUB_MOCK_H
#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_stub.h>
#include "mission_listener_interface.h"

namespace OHOS {
namespace AAFwk {
class MissionListenerStubMock : public IRemoteStub<IMissionListener> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"MissionListenerStubMock");

    MissionListenerStubMock() : code_(0) {}
    virtual ~MissionListenerStubMock() {}

    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));

    int InvokeSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return NO_ERROR;
    }

    int code_ = 0;

    virtual void OnMissionCreated(int32_t missionId) {}
    virtual void OnMissionDestroyed(int32_t missionId) {}
    virtual void OnMissionSnapshotChanged(int32_t missionId) {}
    virtual void OnMissionMovedToFront(int32_t missionId) {}
#ifdef SUPPORT_GRAPHICS
    virtual void OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<OHOS::Media::PixelMap> &icon) {}
#endif
    virtual void OnMissionClosed(int32_t missionId) {}
    virtual void OnMissionLabelUpdated(int32_t missionId) {}
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // UNITTEST_OHOS_ABILITY_RUNTIME_MISSION_LISTENER_STUB_MOCK_H
