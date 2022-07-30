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

#include "want_agent_client.h"

#include "ability_manager_errors.h"
#include "ability_manager_interface.h"
#include "ability_util.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
WantAgentClient &WantAgentClient::GetInstance()
{
    static WantAgentClient client;
    return client;
}

WantAgentClient::WantAgentClient() {}

WantAgentClient::~WantAgentClient() {}

sptr<IWantSender> WantAgentClient::GetWantSender(
    const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }
    if (!data.WriteParcelable(&wantSenderInfo)) {
        HILOG_ERROR("wantSenderInfo write failed.");
        return nullptr;
    }
    if (callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            HILOG_ERROR("flag and callerToken write failed.");
            return nullptr;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return nullptr;
        }
    }

    auto error = abms->SendRequest(IAbilityManager::GET_PENDING_WANT_SENDER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return nullptr;
    }
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(reply.ReadRemoteObject());
    if (!wantSender) {
        return nullptr;
    }
    return wantSender;
}

ErrCode WantAgentClient::SendWantSender(const sptr<IWantSender> &target, const SenderInfo &senderInfo)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ABILITY_SERVICE_NOT_CONNECTED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (target == nullptr || !data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&senderInfo)) {
        HILOG_ERROR("senderInfo write failed.");
        return INNER_ERR;
    }

    auto error = abms->SendRequest(IAbilityManager::SEND_PENDING_WANT_SENDER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

void WantAgentClient::CancelWantSender(const sptr<IWantSender> &sender)
{
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (sender == nullptr || !data.WriteRemoteObject(sender->AsObject())) {
        HILOG_ERROR("sender write failed.");
        return;
    }
    auto error = abms->SendRequest(IAbilityManager::CANCEL_PENDING_WANT_SENDER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return;
    }
}

ErrCode WantAgentClient::GetPendingWantUid(const sptr<IWantSender> &target, int32_t &uid)
{
    CHECK_POINTER_AND_RETURN(target, INVALID_PARAMETERS_ERR);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ABILITY_SERVICE_NOT_CONNECTED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return ERR_INVALID_VALUE;
    }
    auto error = abms->SendRequest(IAbilityManager::GET_PENDING_WANT_UID, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return INNER_ERR;
    }
    uid = reply.ReadInt32();
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantUserId(const sptr<IWantSender> &target, int32_t &userId)
{
    CHECK_POINTER_AND_RETURN(target, INVALID_PARAMETERS_ERR);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ABILITY_SERVICE_NOT_CONNECTED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return ERR_INVALID_VALUE;
    }
    auto error = abms->SendRequest(IAbilityManager::GET_PENDING_WANT_USERID, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return INNER_ERR;
    }
    userId = reply.ReadInt32();
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantBundleName(const sptr<IWantSender> &target, std::string &bundleName)
{
    CHECK_POINTER_AND_RETURN(target, INVALID_PARAMETERS_ERR);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ABILITY_SERVICE_NOT_CONNECTED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return INNER_ERR;
    }
    auto error = abms->SendRequest(IAbilityManager::GET_PENDING_WANT_BUNDLENAME, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return INNER_ERR;
    }
    bundleName = Str16ToStr8(reply.ReadString16());
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantCode(const sptr<IWantSender> &target, int32_t &code)
{
    CHECK_POINTER_AND_RETURN(target, INVALID_PARAMETERS_ERR);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ABILITY_SERVICE_NOT_CONNECTED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return ERR_INVALID_VALUE;
    }
    auto error = abms->SendRequest(IAbilityManager::GET_PENDING_WANT_CODE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return INNER_ERR;
    }
    code = reply.ReadInt32();
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantType(const sptr<IWantSender> &target, int32_t &type)
{
    CHECK_POINTER_AND_RETURN(target, INVALID_PARAMETERS_ERR);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ABILITY_SERVICE_NOT_CONNECTED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return ERR_INVALID_VALUE;
    }
    auto error = abms->SendRequest(IAbilityManager::GET_PENDING_WANT_TYPE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return INNER_ERR;
    }
    type = reply.ReadInt32();
    type < 0 ? type = 0 : type;
    return ERR_OK;
}

void WantAgentClient::RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    if (!CheckSenderAndRecevier(sender, receiver)) {
        return;
    }
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(sender->AsObject())) {
        HILOG_ERROR("sender write failed.");
        return;
    }
    if (!data.WriteRemoteObject(receiver->AsObject())) {
        HILOG_ERROR("receiver write failed.");
        return;
    }
    auto error = abms->SendRequest(IAbilityManager::REGISTER_CANCEL_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return;
    }
}

void WantAgentClient::UnregisterCancelListener(
    const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    if (!CheckSenderAndRecevier(sender, receiver)) {
        return;
    }
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(sender->AsObject())) {
        HILOG_ERROR("sender write failed.");
        return;
    }
    if (!data.WriteRemoteObject(receiver->AsObject())) {
        HILOG_ERROR("receiver write failed.");
        return;
    }
    auto error = abms->SendRequest(IAbilityManager::UNREGISTER_CANCEL_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return;
    }
}

ErrCode WantAgentClient::GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    CHECK_POINTER_AND_RETURN(target, INVALID_PARAMETERS_ERR);
    CHECK_POINTER_AND_RETURN(want, INVALID_PARAMETERS_ERR);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ABILITY_SERVICE_NOT_CONNECTED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(want.get())) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }
    auto error = abms->SendRequest(IAbilityManager::GET_PENDING_REQUEST_WANT, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    std::unique_ptr<Want> wantInfo(reply.ReadParcelable<Want>());
    if (!wantInfo) {
        HILOG_ERROR("readParcelableInfo failed");
        return INNER_ERR;
    }
    want = std::move(wantInfo);

    return NO_ERROR;
}

ErrCode WantAgentClient::GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info)
{
    CHECK_POINTER_AND_RETURN(target, INVALID_PARAMETERS_ERR);
    CHECK_POINTER_AND_RETURN(info, INVALID_PARAMETERS_ERR);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ABILITY_SERVICE_NOT_CONNECTED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(info.get())) {
        HILOG_ERROR("info write failed.");
        return INNER_ERR;
    }
    auto error = abms->SendRequest(IAbilityManager::GET_PENDING_WANT_SENDER_INFO, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    std::unique_ptr<WantSenderInfo> wantSenderInfo(reply.ReadParcelable<WantSenderInfo>());
    if (!wantSenderInfo) {
        HILOG_ERROR("readParcelable Info failed");
        return INNER_ERR;
    }
    info = std::move(wantSenderInfo);

    return NO_ERROR;
}

sptr<IRemoteObject> WantAgentClient::GetAbilityManager()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemManager == nullptr) {
            HILOG_ERROR("Fail to get registry.");
            return nullptr;
        }
        auto remoteObj = systemManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
        if (remoteObj == nullptr) {
            HILOG_ERROR("Fail to connect ability manager service.");
            return nullptr;
        }

        deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new (std::nothrow) WantAgentDeathRecipient());
        if (deathRecipient_ == nullptr) {
            HILOG_ERROR("%{public}s :Failed to create WantAgentDeathRecipient!", __func__);
            return nullptr;
        }
        if (!remoteObj->AddDeathRecipient(deathRecipient_)) {
            HILOG_INFO("%{public}s :Add death recipient to failed, maybe already add.", __func__);
        }
        proxy_ = remoteObj;
    }

    return proxy_;
}

void WantAgentClient::WantAgentDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOG_INFO("WantAgentDeathRecipient handle remote died.");
    WantAgentClient::GetInstance().ResetProxy(remote);
}

void WantAgentClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!proxy_) {
        return;
    }
    if (proxy_ == remote.promote()) {
        proxy_->RemoveDeathRecipient(deathRecipient_);
    }
    proxy_ = nullptr;
}

bool WantAgentClient::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(IAbilityManager::GetDescriptor())) {
        HILOG_ERROR("write interface token failed.");
        return false;
    }
    return true;
}

bool WantAgentClient::CheckSenderAndRecevier(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    if (sender == nullptr) {
        HILOG_ERROR("sender is nullptr.");
        return false;
    }
    if (receiver == nullptr) {
        HILOG_ERROR("receiver is nullptr.");
        return false;
    }

    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
