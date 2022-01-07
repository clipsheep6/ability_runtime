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

#ifndef OHOS_MISSION_CALLBACK_INTERFACE_H
#define OHOS_MISSION_CALLBACK_INTERFACE_H

#include "element_name.h"
#include "iremote_broker.h"

namespace OHOS {
namespace DistributedSchedule {
class IMissionCallback : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.DistributedSchedule.IMissionCallback");

    virtual void NotifyMissionsChanged(const std::u16string deviceId) = 0;
    virtual void NotifySnapshot(const std::u16string deviceId, int32_t taskId) = 0;
    virtual void NotifyNetDisconnect(const std::u16string deviceId, int32_t state) = 0;
    virtual void OnContinueDone(int32_t result) = 0;

    enum RequestCode {
        NOTIFY_MISSIONS_CHANGED = 1,

        NOTIFY_SNAP_SHOT = 2,

        NOTIFY_NET_DISCONNECT = 3,

        NOTIFY_CONTINUATION_RESULT = 4,
    };
};
}  // namespace DistributedSchedule
}  // namespace OHOS
#endif  // OHOS_MISSION_CALLBACK_INTERFACE_H
