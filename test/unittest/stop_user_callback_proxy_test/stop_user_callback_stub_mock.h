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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_STOP_USER_CALLBACK_MOCK_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_STOP_USER_CALLBACK_MOCK_H
#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_stub.h>
#include "stop_user_callback.h"

namespace OHOS {
namespace AAFwk {
class StopUserCallbackStubMock : public IRemoteStub<IStopUserCallback> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"StopUserCallbackStubMock");

    StopUserCallbackStubMock() : code_(0) {}
    virtual ~StopUserCallbackStubMock() {}

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

    virtual void OnStopUserDone(int userId, int errcode) {};
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // UNITTEST_OHOS_ABILITY_RUNTIME_STOP_USER_CALLBACK_MOCK_H
