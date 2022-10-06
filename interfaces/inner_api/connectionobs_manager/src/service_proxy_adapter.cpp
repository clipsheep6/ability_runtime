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

#include "service_proxy_adapter.h"

#include "connection_observer_errors.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::u16string ABILITY_MGR_DESCRIPTOR = u"ohos.aafwk.AbilityManager";
constexpr uint32_t REGISTER_CONNECTION_OBSERVER = 2502;
constexpr uint32_t UNREGISTER_CONNECTION_OBSERVER = 2503;
constexpr uint32_t GET_DLP_CONNECTION_INFOS = 2504;
}
int32_t ServiceProxyAdapter::RegisterObserver(const sptr<IConnectionObserver> &observer)
{
    if (!observer) {
        HILOG_ERROR("IConnectObserver is invalid.");
        return ERR_INVALID_OBSERVER;
    }

    if (!remoteObj_) {
        HILOG_ERROR("no abilityms proxy.");
        return ERR_NO_PROXY;
    }

    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(ABILITY_MGR_DESCRIPTOR)) {
        HILOG_ERROR("register observer write interface token failed.");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOG_ERROR("register observer write observer remote obj failed.");
        return ERR_INVALID_VALUE;
    }

    error = remoteObj_->SendRequest(REGISTER_CONNECTION_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("register observer Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t ServiceProxyAdapter::UnregisterObserver(const sptr<IConnectionObserver> &observer)
{
    if (!observer) {
        HILOG_ERROR("unregister observer, IConnectObserver is invalid.");
        return ERR_INVALID_OBSERVER;
    }

    if (!remoteObj_) {
        HILOG_ERROR("unregister observer, no abilityms proxy.");
        return ERR_NO_PROXY;
    }

    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(ABILITY_MGR_DESCRIPTOR)) {
        HILOG_ERROR("unregister observer, write interface token failed.");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOG_ERROR("unregister observer, write observer remote obj failed.");
        return ERR_INVALID_VALUE;
    }

    error = remoteObj_->SendRequest(UNREGISTER_CONNECTION_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("unregister observer, Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t ServiceProxyAdapter::GetDlpConnectionInfos(std::vector<DlpConnectionInfo> &infos)
{
    if (!remoteObj_) {
        HILOG_ERROR("GetDlpConnectionInfos, no abilityms proxy.");
        return ERR_NO_PROXY;
    }

    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(ABILITY_MGR_DESCRIPTOR)) {
        HILOG_ERROR("GetDlpConnectionInfos, write interface token failed.");
        return ERR_INVALID_VALUE;
    }

    error = remoteObj_->SendRequest(GET_DLP_CONNECTION_INFOS, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("GetDlpConnectionInfos, Send request error: %{public}d", error);
        return error;
    }

    auto result = reply.ReadInt32();
    if (result != 0) {
        HILOG_ERROR("GetDlpConnectionInfos fail, result: %{public}d", result);
        return result;
    }

    int32_t infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<DlpConnectionInfo> info(reply.ReadParcelable<DlpConnectionInfo>());
        if (info == nullptr) {
            HILOG_ERROR("Read GetDlpConnectionInfo infos failed");
            return ERR_READ_INFO_FAILED;
        }
        infos.emplace_back(*info);
    }

    return result;
}

sptr<IRemoteObject> ServiceProxyAdapter::GetProxyObject() const
{
    return remoteObj_;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
