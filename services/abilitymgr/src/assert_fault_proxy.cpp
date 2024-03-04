/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "ability_manager_client.h"
#include "assert_fault_proxy.h"
#include "hilog_wrapper.h"
#include "scene_board_judgement.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char ASSERT_FAULT_DETAIL[] = "assertFaultDialogDetail";
constexpr char UIEXTENSION_TYPE_KEY[] = "ability.want.params.uiExtensionType";
constexpr char SYS_DIALOG_Z_ORDER[] = "sysDialogZOrder";
constexpr char SYS_DIALOG_Z_ORDER_VALUE[] = "2";
constexpr int32_t DEFAULT_VAL = 0;
constexpr int32_t INVALID_USERID = -1;
constexpr int32_t MESSAGE_PARCEL_KEY_SIZE = 3;
constexpr uint32_t COMMAND_START_DIALOG = 1;
}
AssertFaultProxy::AssertFaultProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IAssertFaultInterface>(impl)
{}

void AssertFaultProxy::NotifyDebugAssertResult(AAFwk::UserStatus status)
{
    HILOG_DEBUG("Notify user action result to assert fault application.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(AssertFaultProxy::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteInt32(static_cast<int32_t>(status))) {
        HILOG_ERROR("Write status failed.");
        return;
    }

    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Get remote failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (remote->SendRequest(MessageCode::NOTIFY_USER_ACTION_RESULT, data, reply, option)
        != NO_ERROR) {
        HILOG_ERROR("Remote send request failed.");
    }
}

AssertFaultRemoteDeathRecipient::AssertFaultRemoteDeathRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

void AssertFaultRemoteDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("Callback is nullptr.");
        return;
    }
    handler_(remote);
}

ModalSystemAssertUIExtension::~ModalSystemAssertUIExtension()
{
    dialogConnectionCallback_ = nullptr;
}

sptr<ModalSystemAssertUIExtension::AssertDialogConnection> ModalSystemAssertUIExtension::GetConnection()
{
    if (dialogConnectionCallback_ == nullptr) {
        std::lock_guard lock(dialogConnectionMutex_);
        if (dialogConnectionCallback_ == nullptr) {
            dialogConnectionCallback_ = new (std::nothrow) AssertDialogConnection();
        }
    }

    return dialogConnectionCallback_;
}

bool ModalSystemAssertUIExtension::CreateModalUIExtension(const AAFwk::Want& want)
{
    HILOG_ERROR("Called.");
    auto callback = GetConnection();
    if (callback == nullptr) {
        HILOG_ERROR("Callback is nullptr.");
        return false;
    }
    if (callback->RequestShowDialog(want)) {
        HILOG_DEBUG("Start consumption want.");
        return true;
    }

    auto abilityManagerClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityManagerClient == nullptr) {
        HILOG_ERROR("ConnectSystemUi AbilityManagerClient is nullptr");
        return false;
    }

    AAFwk::Want systemUIWant;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        systemUIWant.SetElementName("com.ohos.sceneboard", "com.ohos.sceneboard.systemdialog");
    } else {
        systemUIWant.SetElementName("com.ohos.systemui", "com.ohos.systemui.dialog");
    }
    auto result = abilityManagerClient->ConnectAbility(systemUIWant, callback, INVALID_USERID);
    if (result != ERR_OK) {
        HILOG_ERROR("ConnectSystemUi ConnectAbility dialog failed, result = %{public}d", result);
        return false;
    }
    return true;
}

ModalSystemAssertUIExtension::AssertDialogConnection::~AssertDialogConnection()
{
    HILOG_DEBUG("Called.");
    CleanUp();
}

bool ModalSystemAssertUIExtension::AssertDialogConnection::RequestShowDialog(const AAFwk::Want& want)
{
    HILOG_DEBUG("Called.");
    {
        std::lock_guard lock(mutex_);
        consumptionList_.push(want);
    }
    if (!isDialogShow_) {
        HILOG_DEBUG("Connection not ready.");
        return false;
    }

    AppExecFwk::ElementName element;
    OnAbilityConnectDone(element, remoteObject_, DEFAULT_VAL);
    return true;
}

void ModalSystemAssertUIExtension::AssertDialogConnection::CleanUp()
{
    HILOG_DEBUG("Called.");
    std::lock_guard lock(mutex_);
    if (!consumptionList_.empty()) {
        std::queue<AAFwk::Want> temp;
        consumptionList_.swap(temp);
    }
    if (remoteObject_ != nullptr) {
        remoteObject_->RemoveDeathRecipient(deathRecipient_);
        remoteObject_ = nullptr;
    }
    deathRecipient_ = nullptr;
}

void ModalSystemAssertUIExtension::AssertDialogConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remote, int resultCode)
{
    HILOG_DEBUG("Called.");
    if (remote == nullptr) {
        HILOG_ERROR("Input remote object is nullptr.");
        return;
    }
    std::lock_guard lock(mutex_);
    if (remoteObject_ == nullptr) {
        remoteObject_ = remote;
        wptr<AssertDialogConnection> weakThis = iface_cast<AssertDialogConnection>(this->AsObject());
        deathRecipient_ = 
            new (std::nothrow) AssertFaultRemoteDeathRecipient([weakThis] (const wptr<IRemoteObject> &remote) {
                auto remoteObj = weakThis.promote();
                if (remoteObj == nullptr) {
                    HILOG_ERROR("Invalid remote object.");
                    return;
                }
                remoteObj->CleanUp();
            });
        remoteObject_->AddDeathRecipient(deathRecipient_);
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto &want = consumptionList_.front();
    data.WriteInt32(MESSAGE_PARCEL_KEY_SIZE);
    data.WriteString16(u"bundleName");
    data.WriteString16(Str8ToStr16(want.GetElement().GetBundleName()));
    data.WriteString16(u"abilityName");
    data.WriteString16(Str8ToStr16(want.GetElement().GetAbilityName()));
    data.WriteString16(u"parameters");
    nlohmann::json param;
    param[UIEXTENSION_TYPE_KEY] = want.GetStringParam(UIEXTENSION_TYPE_KEY);
    param[ASSERT_FAULT_DETAIL] = want.GetStringParam(ASSERT_FAULT_DETAIL);
    param[AAFwk::Want::PARAM_ASSERT_FAULT_SESSION_ID] =
        want.GetStringParam(AAFwk::Want::PARAM_ASSERT_FAULT_SESSION_ID);
    param[SYS_DIALOG_Z_ORDER] = SYS_DIALOG_Z_ORDER_VALUE;
    std::string paramStr = param.dump();
    data.WriteString16(Str8ToStr16(paramStr));
    consumptionList_.pop();
    uint32_t code = !Rosen::SceneBoardJudgement::IsSceneBoardEnabled() ? COMMAND_START_DIALOG :
        AAFwk::IAbilityConnection::ON_ABILITY_CONNECT_DONE;
    auto ret = remote->SendRequest(code, data, reply, option);
    if (ret != ERR_OK) {
        HILOG_ERROR("Show dialog is failed");
        return;
    }
    isDialogShow_ = true;
}

void ModalSystemAssertUIExtension::AssertDialogConnection::OnAbilityDisconnectDone(
    const AppExecFwk::ElementName& element, int resultCode)
{
    HILOG_DEBUG("Called.");
    CleanUp();
}
} // namespace AbilityRuntime
} // namespace OHOS
