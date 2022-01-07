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

#include "mission_callback_stub.h"

#include "dtbschedmgr_log.h"
#include "ipc_types.h"
#include "message_parcel.h"
#include "want.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
    const std::string TAG = "MissionCallbackStub";
    const std::u16string MISSION_CALLBACK_STUB_TOKEN = u"ohos.DistributedSchedule.IMissionCallback";
}

MissionCallbackStub::MissionCallbackStub()
{}

MissionCallbackStub::~MissionCallbackStub()
{}

int32_t MissionCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != MISSION_CALLBACK_STUB_TOKEN) {
        HILOGE("Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case IMissionCallback::NOTIFY_MISSIONS_CHANGED: {
            auto deviceId = data.ReadString16();
            HILOGI("NOTIFY_MISSIONS_CHANGED deviceId is %{public}s", Str16ToStr8(deviceId).c_str());
            if (deviceId.empty()) {
                HILOGE("callback stub receive deviceId is empty");
                return ERR_INVALID_VALUE;
            }
            NotifyMissionsChanged(deviceId);
            return NO_ERROR;
        }
        case IMissionCallback::NOTIFY_SNAP_SHOT: {
            auto deviceId = data.ReadString16();
            if (deviceId.empty()) {
                HILOGE("callback stub receive deviceId is empty");
                return ERR_INVALID_VALUE;
            }
            int32_t missionId = data.ReadInt32();
            HILOGI("NOTIFY_SNAP_SHOT deviceId: %{public}s, missionId: %{public}d",
                Str16ToStr8(deviceId).c_str(), missionId);
            NotifySnapshot(deviceId, missionId);
            return NO_ERROR;
        }
        case IMissionCallback::NOTIFY_NET_DISCONNECT: {
            auto deviceId = data.ReadString16();
            if (deviceId.empty()) {
                HILOGE("callback stub receive deviceId is empty");
                return ERR_INVALID_VALUE;
            }
            int32_t state = data.ReadInt32();
            HILOGI("NOTIFY_NET_DISCONNECT deviceId: %{public}s, state: %{public}d",
                Str16ToStr8(deviceId).c_str(), state);
            NotifyNetDisconnect(deviceId, state);
            return NO_ERROR;
        }
        case IMissionCallback::NOTIFY_CONTINUATION_RESULT: {
            int32_t result = data.ReadInt32();
            HILOGI("NOTIFY_CONTINUATION_RESULT result: %{public}d", result);
            OnContinueDone(result);
            return NO_ERROR;
        }
        default: {
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}
}  // namespace DistributedSchedule
}  // namespace OHOS
