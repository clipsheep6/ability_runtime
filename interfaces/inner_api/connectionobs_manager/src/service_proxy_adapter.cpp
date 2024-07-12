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

#include "service_proxy_adapter.h"

#include "connection_observer_errors.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::u16string ABILITY_MGR_DESCRIPTOR = u"ohos.aafwk.AbilityManager";
constexpr uint32_t REGISTER_CONNECTION_OBSERVER = 2502;
constexpr uint32_t UNREGISTER_CONNECTION_OBSERVER = 2503;
#ifdef WITH_DLP
constexpr uint32_t GET_DLP_CONNECTION_INFOS = 2504;
#endif // WITH_DLP
constexpr uint32_t GET_CONNECTION_DATA = 2505;
constexpr int32_t CYCLE_LIMIT = 1000;
}
int32_t ServiceProxyAdapter::RegisterObserver(const sptr<IConnectionObserver> &observer)
{
    if (!observer) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid IConnectObserver");
        return ERR_INVALID_OBSERVER;
    }

    if (!remoteObj_) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no abilityms proxy");
        return ERR_NO_PROXY;
    }

    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(ABILITY_MGR_DESCRIPTOR)) {
        TAG_LOGE(AAFwkTag::CONNECTION, "write token failed");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteRemoteObject(observer->AsObject())) {
        TAG_LOGE(AAFwkTag::CONNECTION, "write remote obj failed");
        return ERR_INVALID_VALUE;
    }

    error = remoteObj_->SendRequest(REGISTER_CONNECTION_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::CONNECTION, "Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t ServiceProxyAdapter::UnregisterObserver(const sptr<IConnectionObserver> &observer)
{
    if (!observer) {
        TAG_LOGE(AAFwkTag::CONNECTION, "IConnectObserver invalid");
        return ERR_INVALID_OBSERVER;
    }

    if (!remoteObj_) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no abilityms proxy");
        return ERR_NO_PROXY;
    }

    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(ABILITY_MGR_DESCRIPTOR)) {
        TAG_LOGE(AAFwkTag::CONNECTION, "write token failed");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteRemoteObject(observer->AsObject())) {
        TAG_LOGE(AAFwkTag::CONNECTION, "write remote obj failed");
        return ERR_INVALID_VALUE;
    }

    error = remoteObj_->SendRequest(UNREGISTER_CONNECTION_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::CONNECTION, "Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

#ifdef WITH_DLP
int32_t ServiceProxyAdapter::GetDlpConnectionInfos(std::vector<DlpConnectionInfo> &infos)
{
    if (!remoteObj_) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no abilityms proxy");
        return ERR_NO_PROXY;
    }

    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(ABILITY_MGR_DESCRIPTOR)) {
        TAG_LOGE(AAFwkTag::CONNECTION, "write token failed");
        return ERR_INVALID_VALUE;
    }

    error = remoteObj_->SendRequest(GET_DLP_CONNECTION_INFOS, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::CONNECTION, "Send request error: %{public}d", error);
        return error;
    }

    auto result = reply.ReadInt32();
    if (result != 0) {
        TAG_LOGE(AAFwkTag::CONNECTION, "fail, result: %{public}d", result);
        return result;
    }

    int32_t infoSize = reply.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::CONNECTION, "infoSize too large");
        return ERR_INVALID_VALUE;
    }

    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<DlpConnectionInfo> info(reply.ReadParcelable<DlpConnectionInfo>());
        if (info == nullptr) {
            TAG_LOGE(AAFwkTag::CONNECTION, "Read infos failed");
            return ERR_READ_INFO_FAILED;
        }
        infos.emplace_back(*info);
    }

    return result;
}
#endif // WITH_DLP

int32_t ServiceProxyAdapter::GetConnectionData(std::vector<ConnectionData> &connectionData)
{
    if (!remoteObj_) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no abilityms proxy");
        return ERR_NO_PROXY;
    }

    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(ABILITY_MGR_DESCRIPTOR)) {
        TAG_LOGE(AAFwkTag::CONNECTION, "write token failed");
        return ERR_INVALID_VALUE;
    }

    error = remoteObj_->SendRequest(GET_CONNECTION_DATA, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::CONNECTION, "Send request error: %{public}d", error);
        return error;
    }

    auto result = reply.ReadInt32();
    if (result != 0) {
        TAG_LOGE(AAFwkTag::CONNECTION, "fail, result: %{public}d", result);
        return result;
    }

    int32_t infoSize = reply.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::CONNECTION, "infoSize too large");
        return ERR_INVALID_VALUE;
    }

    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<ConnectionData> item(reply.ReadParcelable<ConnectionData>());
        if (item == nullptr) {
            TAG_LOGE(AAFwkTag::CONNECTION, "Read infos failed");
            return ERR_READ_INFO_FAILED;
        }
        connectionData.emplace_back(*item);
    }

    return result;
}

sptr<IRemoteObject> ServiceProxyAdapter::GetProxyObject() const
{
    return remoteObj_;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
