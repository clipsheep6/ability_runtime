/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "component_interception_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"


namespace OHOS {
namespace AppExecFwk {
ComponentInterceptionProxy::ComponentInterceptionProxy(
    const sptr<IRemoteObject> &impl) : IRemoteProxy<IComponentInterception>(impl)
{}

bool ComponentInterceptionProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(ComponentInterceptionProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

bool ComponentInterceptionProxy::AllowComponentStart(const Want &want, const sptr<IRemoteObject> &callerToken,
    int requestCode, int componentStatus, sptr<Want> &extraParam)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return true;
    }
    data.WriteParcelable(&want);

    if (callerToken == nullptr) {
        data.WriteBool(false);
    } else {
        data.WriteBool(true);
        data.WriteRemoteObject(callerToken);
    }

    data.WriteInt32(requestCode);
    data.WriteInt32(componentStatus);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return true;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IComponentInterception::Message::TRANSACT_ON_ALLOW_COMPONENT_START),
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
        return true;
    }

    bool hasExtraParam = reply.ReadBool();
    if (hasExtraParam) {
        sptr<Want> tempWant = reply.ReadParcelable<Want>();
        if (tempWant != nullptr) {
            SetExtraParam(tempWant, extraParam);
        }
    }
    return reply.ReadBool();
}

void ComponentInterceptionProxy::SetExtraParam(const sptr<Want> &want, sptr<Want> &extraParam)
{
    if (extraParam == nullptr) {
        return;
    }
    int requestResult = want->GetIntParam("requestResult", ERR_OK);
    extraParam->SetParam("requestResult", requestResult == ERR_OK ? ERR_OK : ERR_WOULD_BLOCK);

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

void ComponentInterceptionProxy::NotifyHandleAbilityStateChange(const sptr<IRemoteObject> &abilityToken, int opCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (abilityToken == nullptr) {
        data.WriteBool(false);
    } else {
        data.WriteBool(true);
        data.WriteRemoteObject(abilityToken);
    }
    data.WriteInt32(opCode);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IComponentInterception::Message::TRANSACT_ON_HANDLE_MOVE_ABILITY),
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
