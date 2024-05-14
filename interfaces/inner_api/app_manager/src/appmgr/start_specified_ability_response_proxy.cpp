/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "start_specified_ability_response_proxy.h"
#include "ipc_types.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
StartSpecifiedAbilityResponseProxy::StartSpecifiedAbilityResponseProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStartSpecifiedAbilityResponse>(impl)
{}

bool StartSpecifiedAbilityResponseProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(StartSpecifiedAbilityResponseProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write interface token failed");
        return false;
    }
    return true;
}

void StartSpecifiedAbilityResponseProxy::OnAcceptWantResponse(
    const AAFwk::Want &want, const std::string &flag, int32_t requestId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "On accept want by proxy.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want) || !data.WriteString(flag) ||
        !data.WriteInt32(requestId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write data failed.");
        return;
    }

    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IStartSpecifiedAbilityResponse::Message::ON_ACCEPT_WANT_RESPONSE), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void StartSpecifiedAbilityResponseProxy::OnTimeoutResponse(const AAFwk::Want &want, int32_t requestId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "On timeout response by proxy.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want) || !data.WriteInt32(requestId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write data failed.");
        return;
    }

    int32_t ret = SendTransactCmd(static_cast<uint32_t>(
        IStartSpecifiedAbilityResponse::Message::ON_TIMEOUT_RESPONSE), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

int32_t StartSpecifiedAbilityResponseProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    return remote->SendRequest(code, data, reply, option);
}

void StartSpecifiedAbilityResponseProxy::OnNewProcessRequestResponse(const AAFwk::Want &want, const std::string &flag,
    int32_t requestId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "On satrt specified process response by proxy.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want) || !data.WriteString(flag) ||
        !data.WriteInt32(requestId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write data failed.");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IStartSpecifiedAbilityResponse::Message::ON_NEW_PROCESS_REQUEST_RESPONSE),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void StartSpecifiedAbilityResponseProxy::OnNewProcessRequestTimeoutResponse(const AAFwk::Want &want,
    int32_t requestId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "On start specified process timeout response by proxy.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want) || data.WriteInt32(requestId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write data failed.");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(
        IStartSpecifiedAbilityResponse::Message::ON_NEW_PROCESS_REQUEST_TIMEOUT_RESPONSE),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
