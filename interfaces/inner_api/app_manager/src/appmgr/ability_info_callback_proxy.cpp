/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    data.WriteRemoteObject(token);
    data.WriteParcelable(&want);
    if (!SendRequest(IAbilityInfoCallback::Notify_ABILITY_TOKEN, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void AbilityInfoCallbackProxy::NotifyRestartSpecifiedAbility(const sptr<IRemoteObject> &token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    data.WriteRemoteObject(token);
    if (!SendRequest(IAbilityInfoCallback::Notify_RESTART_SPECIFIED_ABILITY, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
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
    if (!SendRequest(IAbilityInfoCallback::Notify_START_SPECIFIED_ABILITY, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
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

void AbilityInfoCallbackProxy::NotifyStartAbilityResult(const Want &want, int result)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    data.WriteParcelable(&want);
    data.WriteInt32(result);
    if (!SendRequest(IAbilityInfoCallback::Notify_START_ABILITY_RESULT, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

bool AbilityInfoCallbackProxy::SendRequest(uint32_t code, MessageParcel &data,
                                           MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("remote object is nullptr.");
        return false;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest failed. code is %{public}d, ret is %{public}d.", code, ret);
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
