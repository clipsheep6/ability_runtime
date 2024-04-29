/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ability_connect_callback_stub.h"

#include "ability_connect_callback_proxy.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
bool AbilityConnectionProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AbilityConnectionProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return false;
    }
    return true;
}

void AbilityConnectionProxy::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OnAbilityConnectDone resultCode: %{public}d", resultCode);
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&element)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Connect done element error.");
        return;
    }

    if (!data.WriteRemoteObject(remoteObject)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Connect done remote object error.");
        return;
    }

    if (!data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Connect done result code error.");
        return;
    }

    error = SendTransactCmd(IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Connect done fail, error: %{public}d", error);
        return;
    }
}

void AbilityConnectionProxy::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OnAbilityDisconnectDone resultCode: %{public}d", resultCode);
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return;
    }
    if (!data.WriteParcelable(&element) || !data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Disconnect done data write error.");
        return;
    }

    error = SendTransactCmd(IAbilityConnection::ON_ABILITY_DISCONNECT_DONE, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Disconnect done fail, error: %d", error);
        return;
    }
}

int32_t AbilityConnectionProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remote object is nullptr.");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest failed. code is %{public}d, ret is %{public}d.", code, ret);
        return ret;
    }
    return NO_ERROR;
}

AbilityConnectionStub::AbilityConnectionStub()
{}

AbilityConnectionStub::~AbilityConnectionStub()
{}

int AbilityConnectionStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "code: %{public}u", code);
    std::u16string descriptor = AbilityConnectionStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    std::unique_ptr<AppExecFwk::ElementName> element(data.ReadParcelable<AppExecFwk::ElementName>());
    if (element == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callback stub receive element is nullptr");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call callback");
    switch (code) {
        case IAbilityConnection::ON_ABILITY_CONNECT_DONE: {
            auto remoteObject = data.ReadRemoteObject();
            if (remoteObject == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "callback stub receive remoteObject is nullptr");
                return ERR_INVALID_VALUE;
            }
            auto resultCode = data.ReadInt32();
            TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityConnectionStub ON_ABILITY_CONNECT_DONE");
            OnAbilityConnectDone(*element, remoteObject, resultCode);
            TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityConnectionStub ON_ABILITY_CONNECT_DONE end");
            return NO_ERROR;
        }
        case IAbilityConnection::ON_ABILITY_DISCONNECT_DONE: {
            auto resultCode = data.ReadInt32();
            OnAbilityDisconnectDone(*element, resultCode);
            TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityConnectionStub ON_ABILITY_DISCONNECT_DONE");
            return NO_ERROR;
        }
        case IAbilityConnection::ON_REMOTE_STATE_CHANGED: {
            int32_t abilityState = data.ReadInt32();
            OnRemoteStateChanged(*element, abilityState);
            return NO_ERROR;
        }
        default: {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "AbilityConnectionStub default");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AbilityConnectCallbackRecipient::OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (handler_) {
        handler_(remote);
    }
}

AbilityConnectCallbackRecipient::AbilityConnectCallbackRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

AbilityConnectCallbackRecipient::~AbilityConnectCallbackRecipient()
{}
}  // namespace AAFwk
}  // namespace OHOS
