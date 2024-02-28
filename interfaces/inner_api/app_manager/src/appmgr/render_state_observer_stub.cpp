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

#include "render_state_observer_stub.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
RenderStateObserverStub::RenderStateObserverStub()
{
    memberFuncMap_[IRenderStateObserver::ON_RENDER_STATE_CHANGED] =
        &RenderStateObserverStub::OnRenderStateChangedInner;
}

RenderStateObserverStub::~RenderStateObserverStub()
{
    memberFuncMap_.clear();
}

int32_t RenderStateObserverStub::OnRenderStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    pid_t renderPid = data.ReadInt32();
    int32_t state = data.ReadInt32();

    OnRenderStateChanged(renderPid, state);
    return NO_ERROR;
}

int RenderStateObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = RenderStateObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
} // namespace AppExecFwk
} // namespace OHOS