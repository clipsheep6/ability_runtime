/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "service_router_mgr_proxy.h"

#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "parcel_macro.h"
#include "service_router_mgr_interface.h"

namespace OHOS {
namespace AbilityRuntime {
ServiceRouterMgrProxy::ServiceRouterMgrProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IServiceRouterManager>(object)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "created");
}

ServiceRouterMgrProxy::~ServiceRouterMgrProxy()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "destroyed");
}

int32_t ServiceRouterMgrProxy::QueryBusinessAbilityInfos(const BusinessAbilityFilter &filter,
    std::vector<BusinessAbilityInfo> &abilityInfos)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Called");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Write interfaceToken failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(&filter)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Write filter failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    int32_t res = GetParcelableInfos<BusinessAbilityInfo>(ServiceRouterMgrProxy::Message::QUERY_BUSINESS_ABILITY_INFOS,
        data, abilityInfos);
    if (res != OHOS::NO_ERROR) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "QueryBusinessAbilityInfos error: %{public}d", res);
    }
    return res;
}

int32_t ServiceRouterMgrProxy::QueryPurposeInfos(const Want &want, const std::string purposeName,
    std::vector<PurposeInfo> &purposeInfos)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Called");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Write interfaceToken failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Write want failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteString(purposeName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Write purposeName failed");
        return false;
    }
    int32_t res = GetParcelableInfos<PurposeInfo>(ServiceRouterMgrProxy::Message::QUERY_PURPOSE_INFOS, data,
        purposeInfos);
    if (res != OHOS::NO_ERROR) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "QueryPurposeInfos error: %{public}d", res);
    }
    return res;
}

int32_t ServiceRouterMgrProxy::StartUIExtensionAbility(const sptr<SessionInfo> &sessionInfo, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Write interfaceToken failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (sessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Flag or sessionInfo write failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Flag write failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }

    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "UserId write failed.");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int32_t error = SendRequest(ServiceRouterMgrProxy::Message::START_UI_EXTENSION, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t ServiceRouterMgrProxy::ConnectUIExtensionAbility(const Want &want, const sptr<IAbilityConnection> &connect,
    const sptr<SessionInfo> &sessionInfo, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor()) || !data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Write interfaceToken or want failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!connect) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null connect");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (connect->AsObject()) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(connect->AsObject())) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Flag or connect write failed.");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Flag write failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    if (sessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Flag or sessionInfo write failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Flag write failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "UserId write failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    int32_t error = SendRequest(ServiceRouterMgrProxy::Message::CONNECT_UI_EXTENSION, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t ServiceRouterMgrProxy::SendRequest(ServiceRouterMgrProxy::Message code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null remote");
        return ERR_APPEXECFWK_FAILED_GET_REMOTE_PROXY;
    }
    int32_t result = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != NO_ERROR) {
        TAG_LOGE(
            AAFwkTag::SER_ROUTER, "Send %{public}d cmd to service failed, transact error:%{public}d", code, result);
    }
    return result;
}

template <typename T>
int32_t ServiceRouterMgrProxy::GetParcelableInfos(
    ServiceRouterMgrProxy::Message code, MessageParcel &data, std::vector<T> &parcelableInfos)
{
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    int32_t result = SendRequest(code, data, reply, option);
    if (result != OHOS::NO_ERROR) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "SendRequest result failed");
        return result;
    }

    int32_t res = reply.ReadInt32();
    if (res != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "reply error: %{public}d", res);
        return res;
    }

    int32_t infosSize = reply.ReadInt32();
    for (int32_t j = 0; j < infosSize; j++) {
        std::unique_ptr<T> info(reply.ReadParcelable<T>());
        if (!info) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Read parcelableInfos failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
        parcelableInfos.emplace_back(*info);
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "Get parcelableInfos success");
    return OHOS::NO_ERROR;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
