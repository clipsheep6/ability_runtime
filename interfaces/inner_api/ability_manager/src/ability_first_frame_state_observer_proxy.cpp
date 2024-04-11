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

#ifdef SUPPORT_GRAPHICS
#include "ability_first_frame_state_observer_proxy.h"

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
AbilityFistFrameStateObserverProxy::AbilityFistFrameStateObserverProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IAbilityFirstFrameStateObserver>(impl)
{}

bool AbilityFistFrameStateObserverProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AbilityFistFrameStateObserverProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return false;
    }
    return true;
}

void AbilityFistFrameStateObserverProxy::OnAbilityFirstFrameState(
    const AbilityFirstFrameStateData &abilityFirstFrameStateData)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write Token failed.");
        return;
    }
    if (!data.WriteParcelable(&abilityFirstFrameStateData)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to write abilityFirstFrameStateData.");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Remote is NULL.");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAbilityFirstFrameStateObserver::Message::ON_ABILITY_FIRST_FRAME_STATE),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest is failed, error code: %{public}d.", ret);
    }
}
} // namespace AppExecFwk
} // namespace OHOS
#endif