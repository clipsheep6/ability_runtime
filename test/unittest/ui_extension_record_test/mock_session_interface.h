/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_ISESSION_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_ISESSION_H

#include <iremote_stub.h>

#include "session/host/include/zidl/session_interface.h"

namespace OHOS {
namespace AbilityRuntime {
class MockISession : public IRemoteStub<Rosen::ISession> {
public:
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }
    Rosen::WSError Connect(const sptr<Rosen::ISessionStage> &sessionStage,
        const sptr<Rosen::IWindowEventChannel> &eventChannel, const std::shared_ptr<Rosen::RSSurfaceNode> &surfaceNode,
        Rosen::SystemSessionConfig &systemConfig, sptr<Rosen::WindowSessionProperty> property = nullptr,
        sptr<IRemoteObject> token = nullptr, int32_t pid = -1, int32_t uid = -1) override
    {
        return Rosen::WSError::WS_OK;
    }
    Rosen::WSError Foreground(sptr<Rosen::WindowSessionProperty> property) override
    {
        return Rosen::WSError::WS_OK;
    }
    Rosen::WSError Background() override
    {
        return Rosen::WSError::WS_OK;
    }
    Rosen::WSError Disconnect(bool isFromClient = false) override
    {
        return Rosen::WSError::WS_OK;
    }
    Rosen::WSError Show(sptr<Rosen::WindowSessionProperty> property) override
    {
        return Rosen::WSError::WS_OK;
    }
    Rosen::WSError Hide() override
    {
        return Rosen::WSError::WS_OK;
    }

    void NotifyExtensionTimeout(int32_t errorCode) override
    {
        timeoutCode_ = errorCode;
    }

    int32_t timeoutCode_ = 0;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_ISESSION_H
