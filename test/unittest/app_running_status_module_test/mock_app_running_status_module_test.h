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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_APP_RUNNING_STATUS_MODULE_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_APP_RUNNING_STATUS_MODULE_H

#include <app_running_status_listener_interface.h>
#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_stub.h>

namespace OHOS {
namespace AbilityRuntime {
class MockAppRunningStatusListenerInterface : public IRemoteStub<AppRunningStatusListenerInterface> {
public:
    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel&, MessageParcel&, MessageOption&));
    MOCK_METHOD3(NotifyAppRunningStatus, void(const std::string &bundle, int32_t uid, RunningStatus runningStatus));
    int InvokeSendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        code_ = code;
        return 0;
    }
    sptr<IRemoteObject> AsObject() override
    {
        if (!asObject_) {
            return nullptr;
        }
        return this;
    };

private:
    bool asObject_ = true;
    int code_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_CONNECT_CALLBACK_STUB_H
