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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_REMOTE_MISSION_LISTENER_STUB_MOCK_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_REMOTE_MISSION_LISTENER_STUB_MOCK_H
#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_stub.h>
#include "remote_mission_listener_interface.h"

namespace OHOS {
namespace AAFwk {
class RemoteMissionListenerStubMock : public IRemoteStub<IRemoteMissionListener> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteMissionListenerStubMock");

    RemoteMissionListenerStubMock() : code_(0) {}
    virtual ~RemoteMissionListenerStubMock() {}

    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));

    int InvokeSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return NO_ERROR;
    }

    int InvokeErrorSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return 1;
    }

    int code_ = 0;

    virtual void NotifyMissionsChanged(const std::string& deviceId) {};
    virtual void NotifySnapshot(const std::string& deviceId, int32_t missionId) {};
    virtual void NotifyNetDisconnect(const std::string& deviceId, int32_t state) {};
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // UNITTEST_OHOS_ABILITY_RUNTIME_REMOTE_MISSION_LISTENER_STUB_MOCK_H
