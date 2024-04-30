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
#include "ability_first_frame_state_observer_stub.h"

#include "hilog_tag_wrapper.h"
#include "appexecfwk_errors.h"
#include "ipc_types.h"
#include "iremote_object.h"

#include "ability_first_frame_state_data.h"

namespace OHOS {
namespace AppExecFwk {
AbilityFirstFrameStateObserverStub::AbilityFirstFrameStateObserverStub()
{
    memberFuncMap_[static_cast<uint32_t>(IAbilityFirstFrameStateObserver::Message::ON_ABILITY_FIRST_FRAME_STATE)] =
        &AbilityFirstFrameStateObserverStub::HandleOnAbilityFirstFrameStateChanged;
}

AbilityFirstFrameStateObserverStub::~AbilityFirstFrameStateObserverStub()
{
    memberFuncMap_.clear();
}

int32_t AbilityFirstFrameStateObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    std::u16string descriptor = AbilityFirstFrameStateObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Local descriptor is not equal to remote.");
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

int32_t AbilityFirstFrameStateObserverStub::HandleOnAbilityFirstFrameStateChanged(MessageParcel &data,
    MessageParcel &reply)
{
    std::unique_ptr<AbilityFirstFrameStateData> stateData(data.ReadParcelable<AbilityFirstFrameStateData>());
    if (stateData == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "stateData is null.");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    OnAbilityFirstFrameState(*stateData);
    return NO_ERROR;
}

AbilityFirstFrameStateObserverRecipient::AbilityFirstFrameStateObserverRecipient(RemoteDiedHandler handler)
    : handler_(handler)
{}

void AbilityFirstFrameStateObserverRecipient::OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote)
{
    TAG_LOGE(AAFwkTag::ABILITYMGR, "Remote died.");
    if (handler_) {
        handler_(remote);
    }
}
} // namespace AppExecFwk
} // namespace OHOS
#endif