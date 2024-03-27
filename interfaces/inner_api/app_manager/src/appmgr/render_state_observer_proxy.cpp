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

#include "render_state_observer_proxy.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t ERR_INVALID_STUB = 32;
}
RenderStateObserverProxy::RenderStateObserverProxy(
    const sptr<IRemoteObject> &impl) : IRemoteProxy<IRenderStateObserver>(impl)
{}

bool RenderStateObserverProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(RenderStateObserverProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write interface token failed.");
        return false;
    }
    return true;
}

void RenderStateObserverProxy::OnRenderStateChanged(const RenderStateData &renderStateData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteParcelable(&renderStateData)) {
        HILOG_ERROR("Failed to write renderStateData");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote() is NULL");
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IRenderStateObserver::ON_RENDER_STATE_CHANGED),
        data, reply, option);
    if (ret != NO_ERROR || ret != ERR_INVALID_STUB) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
}

int32_t RenderStateObserverProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    return remote->SendRequest(code, data, reply, option);
}
} // namespace AppExecFwk
} // namespace OHOS