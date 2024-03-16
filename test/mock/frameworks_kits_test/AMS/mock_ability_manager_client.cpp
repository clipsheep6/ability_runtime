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

#define private public
#define protected public
#include "ability_manager_client.h"
#undef private
#undef protected
#include "ability_manager_interface.h"
#include "string_ex.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"

namespace OHOS {
namespace AAFwk {
std::shared_ptr<AbilityManagerClient> mockMTInstance_ = nullptr;
std::mutex mockMTMutex_;
sptr<IRemoteObject> remoteObject_;

std::shared_ptr<AbilityManagerClient> AbilityManagerClient::GetInstance()
{
    if (mockMTInstance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(mockMTMutex_);
        if (mockMTInstance_ == nullptr) {
            mockMTInstance_ = std::make_shared<AbilityManagerClient>();
        }
    }
    return mockMTInstance_;
}

AbilityManagerClient::AbilityManagerClient()
{}

AbilityManagerClient::~AbilityManagerClient()
{}

ErrCode AbilityManagerClient::AttachAbilityThread(
    sptr<IAbilityScheduler> scheduler, sptr<IRemoteObject> token)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::AttachAbilityThread start");
    ErrCode err = Connect();
    if (err != ERR_OK) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->AttachAbilityThread(scheduler, token);
}

ErrCode AbilityManagerClient::AbilityTransitionDone(sptr<IRemoteObject> token, int state, const PacMap& saveData)
{
    if (remoteObject_ == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->AbilityTransitionDone(token, state, saveData);
}

ErrCode AbilityManagerClient::ScheduleConnectAbilityDone(
    sptr<IRemoteObject> token, sptr<IRemoteObject> remoteObject)
{
    if (remoteObject_ == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ScheduleConnectAbilityDone(token, remoteObject);
}

ErrCode AbilityManagerClient::ScheduleDisconnectAbilityDone(sptr<IRemoteObject> token)
{
    if (remoteObject_ == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ScheduleDisconnectAbilityDone(token);
}

ErrCode AbilityManagerClient::ScheduleCommandAbilityDone(sptr<IRemoteObject> token)
{
    if (remoteObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "%{private}s:ability service not command", __func__);
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ScheduleCommandAbilityDone(token);
}

ErrCode AbilityManagerClient::ScheduleCommandAbilityWindowDone(
    sptr<IRemoteObject> token,
    sptr<SessionInfo> sessionInfo,
    WindowCommand winCmd,
    AbilityCommand abilityCmd)
{
    if (remoteObject_ == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ScheduleCommandAbilityWindowDone(token, sessionInfo, winCmd, abilityCmd);
}

ErrCode AbilityManagerClient::StartAbility(const Want& want, int requestCode, int32_t userId)
{
    if (remoteObject_ == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->StartAbility(want, userId, requestCode);
}

ErrCode AbilityManagerClient::TerminateAbility(sptr<IRemoteObject> token, int resultCode, const Want* resultWant)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::TerminateAbility start");
    if (remoteObject_ == nullptr) {
        remoteObject_ =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::TerminateAbility end");
    return abms->TerminateAbility(token, resultCode, resultWant);
}

ErrCode AbilityManagerClient::ConnectAbility(
    const Want& want, sptr<IAbilityConnection> connect, sptr<IRemoteObject> callerToken, int32_t userId)
{
    if (remoteObject_ == nullptr) {
        remoteObject_ =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ConnectAbility(want, connect, callerToken);
}

ErrCode AbilityManagerClient::DisconnectAbility(sptr<IAbilityConnection> connect)
{
    if (remoteObject_ == nullptr) {
        remoteObject_ =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->DisconnectAbility(connect);
}

ErrCode AbilityManagerClient::DumpState(const std::string& args, std::vector<std::string>& state)
{
    if (remoteObject_ == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    abms->DumpState(args, state);
    return ERR_OK;
}

ErrCode AbilityManagerClient::Connect()
{
    std::lock_guard<std::mutex> lock(mockMTMutex_);
    remoteObject_ =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "AbilityManagerClient::Connect remoteObject_ == nullptr");
        return ERR_NO_MEMORY;
    }

    return ERR_OK;
}

ErrCode AbilityManagerClient::StopServiceAbility(const Want& want, sptr<IRemoteObject> token)
{
    if (remoteObject_ == nullptr) {
        remoteObject_ =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->StopServiceAbility(want);
}

sptr<IAbilityScheduler> AbilityManagerClient::AcquireDataAbility(
    const Uri& uri, bool tryBind, sptr<IRemoteObject> callerToken)
{
    remoteObject_ =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        return nullptr;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->AcquireDataAbility(uri, tryBind, callerToken);
}

ErrCode AbilityManagerClient::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, sptr<IRemoteObject> callerToken)
{
    remoteObject_ =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        return -1;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ReleaseDataAbility(dataAbilityScheduler, callerToken);
}
}  // namespace AAFwk
}  // namespace OHOS
