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

#include "ability_info_callback_stub.h"
#include "appexecfwk_errors.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
AbilityInfoCallbackStub::AbilityInfoCallbackStub()
{
    memberFuncMap_[static_cast<uint32_t>(
        IAbilityInfoCallback::Notify_ABILITY_TOKEN)] = &AbilityInfoCallbackStub::HandleNotifyAbilityToken;
}

AbilityInfoCallbackStub::~AbilityInfoCallbackStub()
{
    memberFuncMap_.clear();
}

int AbilityInfoCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_INFO("AbilityInfoCallbackStub::OnReceived, code = %{public}u, flags= %{public}d.", code, option.GetFlags());
    std::u16string descriptor = AbilityInfoCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("local descriptor is not equal to remote");
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

void AbilityInfoCallbackStub::NotifyAbilityToken(const sptr<IRemoteObject> token, const Want &want)
{
}

int32_t AbilityInfoCallbackStub::HandleNotifyAbilityToken(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("AbilityInfoCallbackStub");
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    Want *want = data.ReadParcelable<Want>();

    if (!want) {
        HILOG_ERROR("ReadParcelable<Want> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    NotifyAbilityToken(token, *want);
    delete want;
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
