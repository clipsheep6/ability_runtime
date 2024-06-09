/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef APP_NO_RESPONSE
#include "app_no_response_dialog.h"

#include <mutex>

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "scene_board_judgement.h"

namespace OHOS {
namespace AppExecFwk {
ModalSystemAppFreezeUIExtension &ModalSystemAppFreezeUIExtension::GetInstance()
{
    static ModalSystemAppFreezeUIExtension instance;
    return instance;
}

ModalSystemAppFreezeUIExtension::~ModalSystemAppFreezeUIExtension()
{
    dialogConnectionCallback_ = nullptr;
}

sptr<ModalSystemAppFreezeUIExtension::AppFreezeDialogConnection> ModalSystemAppFreezeUIExtension::GetConnection()
{
    if (dialogConnectionCallback_ == nullptr) {
        std::lock_guard lock(dialogConnectionMutex_);
        if (dialogConnectionCallback_ == nullptr) {
            dialogConnectionCallback_ = new (std::nothrow) AppFreezeDialogConnection();
        }
    }

    return dialogConnectionCallback_;
}

bool ModalSystemAppFreezeUIExtension::CreateModalUIExtension(const AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CreateModalUIExtension Called.");
    std::unique_lock<std::mutex> lockAssertResult(appFreezeResultMutex_);
    auto callback = GetConnection();
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CreateModalUIExtension Callback is nullptr.");
        return false;
    }
    callback->SetReqeustAppFreezeDialogWant(want);
    auto abilityManagerClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityManagerClient == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CreateModalUIExtension ConnectSystemUi AbilityManagerClient is nullptr");
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
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "CreateModalUIExtension ConnectSystemUi ConnectAbility dialog failed, result = %{public}d", result);
        return false;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR,
        "CreateModalUIExtension ConnectSystemUi ConnectAbility dialog success, result = %{public}d", result);
    return true;
}

void ModalSystemAppFreezeUIExtension::AppFreezeDialogConnection::SetReqeustAppFreezeDialogWant(const AAFwk::Want &want)
{
    want_ = want;
}

void ModalSystemAppFreezeUIExtension::AppFreezeDialogConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remote, int resultCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Input remote object is nullptr.");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInt32(MESSAGE_PARCEL_KEY_SIZE);
    data.WriteString16(u"bundleName");
    data.WriteString16(Str8ToStr16(want_.GetElement().GetBundleName()));
    data.WriteString16(u"abilityName");
    data.WriteString16(Str8ToStr16(want_.GetElement().GetAbilityName()));
    data.WriteString16(u"parameters");
    nlohmann::json param;
    param[UIEXTENSION_TYPE_KEY.c_str()] = want_.GetStringParam(UIEXTENSION_TYPE_KEY);
    param[APP_FREEZE_PID.c_str()] = want_.GetStringParam(APP_FREEZE_PID);
    param[START_BUNDLE_NAME.c_str()] = want_.GetStringParam(START_BUNDLE_NAME);
    std::string paramStr = param.dump();
    data.WriteString16(Str8ToStr16(paramStr));
    uint32_t code = !Rosen::SceneBoardJudgement::IsSceneBoardEnabled() ?
        COMMAND_START_DIALOG :
        AAFwk::IAbilityConnection::ON_ABILITY_CONNECT_DONE;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AppFreezeDialogConnection::OnAbilityConnectDone Show dialog");
    auto ret = remote->SendRequest(code, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Show dialog is failed");
        return;
    }
}

void ModalSystemAppFreezeUIExtension::AppFreezeDialogConnection::OnAbilityDisconnectDone(
    const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Called.");
}
} // namespace AppExecFwk
} // namespace OHOS
#endif // APP_NO_RESPONSE
