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

#include "ability_info_callback_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
AbilityInfoCallbackProxy::AbilityInfoCallbackProxy(
    const sptr<IRemoteObject> &impl) : IRemoteProxy<IAbilityInfoCallback>(impl)
{}

bool AbilityInfoCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AbilityInfoCallbackProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

void AbilityInfoCallbackProxy::NotifyAbilityToken(const sptr<IRemoteObject> token, const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    data.WriteRemoteObject(token);
    data.WriteParcelable(&want);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(IAbilityInfoCallback::Notify_ABILITY_TOKEN, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
        return;
    }
}

void AbilityInfoCallbackProxy::NotifyStartSpecifiedAbility(const sptr<IRemoteObject> &callerToken,
    const Want &want, int requestCode, sptr<Want> &extraParam)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    data.WriteRemoteObject(callerToken);
    data.WriteParcelable(&want);
    data.WriteInt32(requestCode);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(IAbilityInfoCallback::Notify_START_SPECIFIED_ABILITY, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
        return;
    }
    sptr<Want> tempWant = reply.ReadParcelable<Want>();
    if (tempWant != nullptr) {
        SetExtraParam(tempWant, extraParam);
    }
}

void AbilityInfoCallbackProxy::SetExtraParam(const sptr<Want> &want, sptr<Want> &extraParam)
{
    if (!want || !extraParam) {
        HILOG_ERROR("want or extraParam is invalid.");
        return;
    }

    sptr<IRemoteObject> tempCallBack = want->GetRemoteObject(Want::PARAM_RESV_ABILITY_INFO_CALLBACK);
    if (tempCallBack == nullptr) {
        return;
    }
    extraParam->SetParam(Want::PARAM_RESV_REQUEST_PROC_CODE,
        want->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0));
    extraParam->SetParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE,
        want->GetIntParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE, 0));
    extraParam->SetParam(Want::PARAM_RESV_ABILITY_INFO_CALLBACK, tempCallBack);
}
}  // namespace AppExecFwk
}  // namespace OHOS
