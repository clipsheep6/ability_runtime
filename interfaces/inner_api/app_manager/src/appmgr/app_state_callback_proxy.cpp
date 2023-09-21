/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "app_state_callback_proxy.h"

#include "ipc_types.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AppStateCallbackProxy::AppStateCallbackProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAppStateCallback>(impl)
{}

bool AppStateCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AppStateCallbackProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

void AppStateCallbackProxy::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const AbilityState state)
{
    HILOG_DEBUG("begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (token) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(token.GetRefPtr())) {
            HILOG_ERROR("Failed to write flag and token");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("Failed to write flag");
            return;
        }
    }

    int32_t abilityState = static_cast<int32_t>(state);
    data.WriteInt32(abilityState);
    if (!SendRequest(IAppStateCallback::Message::TRANSACT_ON_ABILITY_REQUEST_DONE, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
    HILOG_DEBUG("end");
}

void AppStateCallbackProxy::OnAppStateChanged(const AppProcessData &appProcessData)
{
    HILOG_DEBUG("begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&appProcessData);
    if (!SendRequest(IAppStateCallback::Message::TRANSACT_ON_APP_STATE_CHANGED, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
    HILOG_DEBUG("end");
}

bool AppStateCallbackProxy::SendRequest(IAppStateCallback::Message code, MessageParcel &data,
                                        MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("remote object is nullptr.");
        return false;
    }

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest failed. code is %{public}d, ret is %{public}d.", code, ret);
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
