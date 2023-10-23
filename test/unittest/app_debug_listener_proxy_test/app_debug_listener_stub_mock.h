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

#ifndef OHOS_ABILITY_RUNTIME_APP_DEBUG_LISTENER_STUB_MOCK_H
#define OHOS_ABILITY_RUNTIME_APP_DEBUG_LISTENER_STUB_MOCK_H

#include <gmock/gmock.h>
#include <iremote_stub.h>
#define private public
#include "app_debug_listener_interface.h"
#undef private

namespace OHOS {
namespace AppExecFwk {

class AppDebugListenerStubMock : public IRemoteStub<IAppDebugListener> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"AppDebugListenerStubMock");
    AppDebugListenerStubMock() : code_(0) {}
    virtual ~ AppDebugListenerStubMock() {}

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

    uint32_t code_ = 0;
    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));
    MOCK_METHOD1(OnAppDebugStarted, void(const std::vector<AppDebugInfo> &));
    MOCK_METHOD1(OnAppDebugStoped, void(const std::vector<AppDebugInfo> &));
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_APP_DEBUG_LISTENER_STUB_MOCK_H
