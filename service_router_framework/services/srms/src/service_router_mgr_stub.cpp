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

#include "service_router_mgr_stub.h"

#include <vector>

#include "accesstoken_kit.h"
#include "appexecfwk_errors.h"
#include "bundle_constants.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "service_info.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
ServiceRouterMgrStub::ServiceRouterMgrStub()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ServiceRouterMgrStub instance is created");
}

ServiceRouterMgrStub::~ServiceRouterMgrStub()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ServiceRouterMgrStub instance is destroyed");
}

int ServiceRouterMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    std::u16string descriptor = ServiceRouterMgrStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case static_cast<uint32_t>(IServiceRouterManager::Message::QUERY_BUSINESS_ABILITY_INFOS):
            return HandleQueryBusinessAbilityInfos(data, reply);
        case static_cast<uint32_t>(IServiceRouterManager::Message::QUERY_PURPOSE_INFOS):
            return HandleQueryPurposeInfos(data, reply);
        default:
            TAG_LOGW(AAFwkTag::SER_ROUTER, "ServiceRouterMgrStub receives unknown code, code = %{public}d", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int ServiceRouterMgrStub::HandleQueryBusinessAbilityInfos(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ServiceRouterMgrStub handle query service infos");
    if (!VerifySystemApp()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "verify system app failed");
        return ERR_BUNDLE_MANAGER_SYSTEM_API_DENIED;
    }
    if (!VerifyCallingPermission(Constants::PERMISSION_GET_BUNDLE_INFO_PRIVILEGED)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "verify GET_BUNDLE_INFO_PRIVILEGED failed");
        return ERR_BUNDLE_MANAGER_PERMISSION_DENIED;
    }

    std::unique_ptr<BusinessAbilityFilter> filter(data.ReadParcelable<BusinessAbilityFilter>());
    if (filter == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ReadParcelable<filter> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    std::vector<BusinessAbilityInfo> infos;
    int ret = QueryBusinessAbilityInfos(*filter, infos);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write ret failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (ret == ERR_OK) {
        if (!WriteParcelableVector<BusinessAbilityInfo>(infos, reply)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "QueryBusinessAbilityInfos write failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    return ERR_OK;
}

int ServiceRouterMgrStub::HandleQueryPurposeInfos(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ServiceRouterMgrStub handle query purpose infos");
    if (!VerifyCallingPermission(Constants::PERMISSION_GET_BUNDLE_INFO_PRIVILEGED)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "verify GET_BUNDLE_INFO_PRIVILEGED failed");
        return ERR_BUNDLE_MANAGER_PERMISSION_DENIED;
    }
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ReadParcelable<want> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    std::string purposeName = data.ReadString();
    std::vector<PurposeInfo> infos;
    int ret = QueryPurposeInfos(*want, purposeName, infos);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write ret failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (ret == ERR_OK) {
        if (!WriteParcelableVector<PurposeInfo>(infos, reply)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "QueryPurposeInfos write failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    return ERR_OK;
}

int ServiceRouterMgrStub::HandleStartUIExtensionAbility(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ServiceRouterMgrStub handle start ui extension ability");
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    int32_t userId = data.ReadInt32();
    int32_t result = StartUIExtensionAbility(sessionInfo, userId);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write result failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return ERR_OK;
}

int ServiceRouterMgrStub::HandleConnectUIExtensionAbility(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ServiceRouterMgrStub handle connect ui extension ability");
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ReadParcelable<want> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    sptr<IAbilityConnection> callback = nullptr;
    if (data.ReadBool()) {
        callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    }
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    int32_t userId = data.ReadInt32();
    int32_t result = ConnectUIExtensionAbility(*want, callback, sessionInfo, userId);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write result failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return ERR_OK;
}

bool ServiceRouterMgrStub::VerifyCallingPermission(const std::string &permissionName)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "VerifyCallingPermission permission %{public}s", permissionName.c_str());
    OHOS::Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    OHOS::Security::AccessToken::ATokenTypeEnum tokenType =
        OHOS::Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == OHOS::Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        return true;
    }
    int32_t ret = OHOS::Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (ret == OHOS::Security::AccessToken::PermissionState::PERMISSION_DENIED) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "PERMISSION_DENIED: %{public}s", permissionName.c_str());
        return false;
    }
    return true;
}

bool ServiceRouterMgrStub::VerifySystemApp()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "verifying systemApp");
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::ATokenTypeEnum tokenType =
        Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE
        || IPCSkeleton::GetCallingUid() == Constants::ROOT_UID) {
        return true;
    }
    uint64_t accessTokenIdEx = IPCSkeleton::GetCallingFullTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(accessTokenIdEx)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "non-system app calling system api");
        return false;
    }
    return true;
}

template <typename T>
bool ServiceRouterMgrStub::WriteParcelableVector(std::vector<T> &parcelableVector, Parcel &reply)
{
    if (!reply.WriteInt32(parcelableVector.size())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write ParcelableVector size failed");
        return false;
    }

    for (auto &parcelable : parcelableVector) {
        if (!reply.WriteParcelable(&parcelable)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "write ParcelableVector failed");
            return false;
        }
    }
    return true;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
