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

#include "want_agent_client.h"

#include "ability_runtime_error_util.h"
#include "ability_manager_errors.h"
#include "ability_manager_interface.h"
#include "ability_util.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace AAFwk {
WantAgentClient &WantAgentClient::GetInstance()
{
    static WantAgentClient client;
    return client;
}

WantAgentClient::WantAgentClient() {}

WantAgentClient::~WantAgentClient() {}

ErrCode WantAgentClient::GetWantSender(
    const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken, sptr<IWantSender> &wantSender)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    if (!data.WriteParcelable(&wantSenderInfo)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "wantSenderInfo write failed.");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    if (callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "flag and callerToken write failed.");
            return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "flag write failed.");
            return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        }
    }

    auto error = abms->SendRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER),
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Send request error: %{public}d", error);
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT;
    }
    wantSender = iface_cast<IWantSender>(reply.ReadRemoteObject());
    return ERR_OK;
}

ErrCode WantAgentClient::SendWantSender(sptr<IWantSender> target, const SenderInfo &senderInfo)
{
    CHECK_POINTER_AND_RETURN(target, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    if (target == nullptr || !data.WriteRemoteObject(target->AsObject())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "SendWantSender, target write failed.");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT;
    }
    if (!data.WriteParcelable(&senderInfo)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "SendWantSender, senderInfo write failed.");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    auto error = abms->SendRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_PENDING_WANT_SENDER),
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "SendWantSender, Send request error: %{public}d", error);
        return ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_TIMEOUT;
    }
    return reply.ReadInt32();
}

ErrCode WantAgentClient::CancelWantSender(const sptr<IWantSender> &sender)
{
    CHECK_POINTER_AND_RETURN(sender, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    ErrCode error;
    MessageParcel reply;
    if (!SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::CANCEL_PENDING_WANT_SENDER),
        abms, sender->AsObject(), reply, error)) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT;
    }
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantUid(const sptr<IWantSender> &target, int32_t &uid)
{
    CHECK_POINTER_AND_RETURN(target, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    ErrCode error;
    MessageParcel reply;
    if (!SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_UID),
        abms, target->AsObject(), reply, error)) {
        return error;
    }
    uid = reply.ReadInt32();
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantUserId(const sptr<IWantSender> &target, int32_t &userId)
{
    CHECK_POINTER_AND_RETURN(target, INVALID_PARAMETERS_ERR);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ABILITY_SERVICE_NOT_CONNECTED);
    ErrCode error;
    MessageParcel reply;
    if (!SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_USERID),
        abms, target->AsObject(), reply, error)) {
        return error;
    }
    userId = reply.ReadInt32();
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantBundleName(const sptr<IWantSender> &target, std::string &bundleName)
{
    CHECK_POINTER_AND_RETURN(target, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    ErrCode error;
    MessageParcel reply;
    if (!SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_BUNDLENAME),
        abms, target->AsObject(), reply, error)) {
        return error;
    }
    bundleName = Str16ToStr8(reply.ReadString16());
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantCode(const sptr<IWantSender> &target, int32_t &code)
{
    CHECK_POINTER_AND_RETURN(target, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    ErrCode error;
    MessageParcel reply;
    if (!SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_CODE),
        abms, target->AsObject(), reply, error)) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_TIMEOUT;
    }
    code = reply.ReadInt32();
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantType(sptr<IWantSender> target, int32_t &type)
{
    CHECK_POINTER_AND_RETURN(target, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    ErrCode error;
    MessageParcel reply;
    if (!SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_TYPE),
        abms, target->AsObject(), reply, error)) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_TIMEOUT;
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
        TAG_LOGE(AAFwkTag::WANTAGENT, "RegisterCancelListener, ability proxy is nullptr.");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(sender->AsObject())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "RegisterCancelListener, sender write failed.");
        return;
    }
    if (!data.WriteRemoteObject(receiver->AsObject())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "RegisterCancelListener, receiver write failed.");
        return;
    }
    auto error = abms->SendRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_CANCEL_LISTENER),
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "RegisterCancelListener, Send request error: %{public}d", error);
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
        TAG_LOGE(AAFwkTag::WANTAGENT, "UnregisterCancelListener, ability proxy is nullptr.");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(sender->AsObject())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "UnregisterCancelListener, sender write failed.");
        return;
    }
    if (!data.WriteRemoteObject(receiver->AsObject())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "UnregisterCancelListener, receiver write failed.");
        return;
    }
    auto error = abms->SendRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_CANCEL_LISTENER),
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "UnregisterCancelListener, Send request error: %{public}d", error);
        return;
    }
}

ErrCode WantAgentClient::GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    CHECK_POINTER_AND_RETURN(target, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    CHECK_POINTER_AND_RETURN(want, INVALID_PARAMETERS_ERR);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    if (!data.WriteRemoteObject(target->AsObject())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "GetPendingRequestWant, target write failed.");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    if (!data.WriteParcelable(want.get())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "GetPendingRequestWant, want write failed.");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    auto error = abms->SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::GET_PENDING_REQUEST_WANT),
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "GetPendingRequestWant, Send request error: %{public}d", error);
        return ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_TIMEOUT;
    }
    std::unique_ptr<Want> wantInfo(reply.ReadParcelable<Want>());
    if (!wantInfo) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "GetPendingRequestWant, readParcelableInfo failed");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
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
        TAG_LOGE(AAFwkTag::WANTAGENT, "GetWantSenderInfo, target write failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(info.get())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "GetWantSenderInfo, info write failed.");
        return INNER_ERR;
    }
    auto error = abms->SendRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER_INFO),
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "GetWantSenderInfo, Send request error: %{public}d", error);
        return error;
    }
    std::unique_ptr<WantSenderInfo> wantSenderInfo(reply.ReadParcelable<WantSenderInfo>());
    if (!wantSenderInfo) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "GetWantSenderInfo, readParcelable Info failed");
        return INNER_ERR;
    }
    info = std::move(wantSenderInfo);

    return NO_ERROR;
}

sptr<IRemoteObject> WantAgentClient::GetAbilityManager()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemManager == nullptr) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "Fail to get registry.");
            return nullptr;
        }
        auto remoteObj = systemManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
        if (remoteObj == nullptr) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "Fail to connect ability manager service.");
            return nullptr;
        }

        deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new (std::nothrow) WantAgentDeathRecipient());
        if (deathRecipient_ == nullptr) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "%{public}s :Failed to create WantAgentDeathRecipient!", __func__);
            return nullptr;
        }
        if (!remoteObj->AddDeathRecipient(deathRecipient_)) {
            TAG_LOGI(AAFwkTag::WANTAGENT, "%{public}s :Add death recipient to failed, maybe already add.", __func__);
        }
        proxy_ = remoteObj;
    }

    return proxy_;
}

void WantAgentClient::WantAgentDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "WantAgentDeathRecipient handle remote died.");
    WantAgentClient::GetInstance().ResetProxy(remote);
}

void WantAgentClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
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
        TAG_LOGE(AAFwkTag::WANTAGENT, "write interface token failed.");
        return false;
    }
    return true;
}

bool WantAgentClient::CheckSenderAndRecevier(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    if (sender == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "sender is nullptr.");
        return false;
    }
    if (receiver == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "receiver is nullptr.");
        return false;
    }

    return true;
}

bool WantAgentClient::SendRequest(int32_t operation, const sptr<IRemoteObject> &abms,
    const sptr<IRemoteObject> &remoteObject, MessageParcel &reply, ErrCode &error)
{
    MessageParcel data;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        error = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return false;
    }
    if (!data.WriteRemoteObject(remoteObject)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "write failed.");
        error = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return false;
    }
    error = abms->SendRequest(operation, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Send request error: %{public}d", error);
        error = ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY;
        return false;
    }

    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
