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

#include "ability_manager_client.h"
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
std::mutex g_mockMutex;
sptr<IRemoteObject> g_remoteObject = nullptr;
std::shared_ptr<AbilityManagerClient> g_mockInstance = nullptr;

std::shared_ptr<AbilityManagerClient> AbilityManagerClient::GetInstance()
{
    if (g_mockInstance == nullptr) {
        std::lock_guard<std::mutex> lock_l(g_mockMutex);
        if (g_mockInstance == nullptr) {
            g_mockInstance = std::make_shared<AbilityManagerClient>();
        }
    }
    return g_mockInstance;
}

AbilityManagerClient::AbilityManagerClient()
{}

AbilityManagerClient::~AbilityManagerClient()
{}

ErrCode AbilityManagerClient::AttachAbilityThread(
    const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    HILOG_INFO("AbilityManagerClient::AttachAbilityThread start");
    ErrCode err = Connect();
    if (err != ERR_OK) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->AttachAbilityThread(scheduler, token);
}

ErrCode AbilityManagerClient::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->AbilityTransitionDone(token, state, saveData);
}

ErrCode AbilityManagerClient::ScheduleConnectAbilityDone(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->ScheduleConnectAbilityDone(token, remoteObject);
}

ErrCode AbilityManagerClient::ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->ScheduleDisconnectAbilityDone(token);
}

ErrCode AbilityManagerClient::ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token)
{
    if (g_remoteObject == nullptr) {
        HILOG_ERROR("%{private}s:ability service not command", __func__);
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->ScheduleCommandAbilityDone(token);
}

ErrCode AbilityManagerClient::StartAbility(const Want &want, int32_t userId, int requestCode)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StartAbility(want, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbility(
    const Want &want, const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StartAbility(want, callerToken, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbilityByCall(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("AbilityManagerClient::StartAbilityByCall start");
    if (g_remoteObject == nullptr) {
        HILOG_INFO("AbilityManagerClient::StartAbilityByCall fail because remoteObject is null");
        g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    HILOG_INFO("AbilityManagerClient::StartAbilityByCall end");
    return abms->StartAbilityByCall(want, connect, callerToken);
}

ErrCode AbilityManagerClient::ReleaseCall(
    const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
{
    HILOG_INFO("AbilityManagerClient::ReleaseCall start");
    if (g_remoteObject == nullptr) {
        HILOG_INFO("AbilityManagerClient::ReleaseCall fail because remoteObject is null");
                g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    HILOG_INFO("AbilityManagerClient::ReleaseCall end");
    return abms->ReleaseCall(connect, element);
}

ErrCode AbilityManagerClient::TerminateAbility(
    const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    HILOG_INFO("AbilityManagerClient::TerminateAbility start");
    if (g_remoteObject == nullptr) {
        g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    HILOG_INFO("AbilityManagerClient::TerminateAbility end");
    return abms->TerminateAbility(token, resultCode, resultWant);
}

ErrCode AbilityManagerClient::TerminateAbility(const sptr<IRemoteObject> &callerToken, int requestCode)
{
    if (g_remoteObject == nullptr) {
        HILOG_ERROR("%{private}s:ability service not connect", __func__);
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->TerminateAbility(callerToken, requestCode);
}

ErrCode AbilityManagerClient::ConnectAbility(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    if (g_remoteObject == nullptr) {
        g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->ConnectAbility(want, connect, callerToken, userId);
}

ErrCode AbilityManagerClient::DisconnectAbility(const sptr<IAbilityConnection> &connect)
{
    if (g_remoteObject == nullptr) {
        g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->DisconnectAbility(connect);
}

ErrCode AbilityManagerClient::DumpState(const std::string &args, std::vector<std::string> &state)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    abms->DumpState(args, state);
    return ERR_OK;
}

ErrCode AbilityManagerClient::Connect()
{
    std::lock_guard<std::mutex> lock(g_mockMutex);
    g_remoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (g_remoteObject == nullptr) {
        HILOG_ERROR("AbilityManagerClient::Connect g_remoteObject == nullptr");
        return ERR_NO_MEMORY;
    }

    return ERR_OK;
}

ErrCode AbilityManagerClient::StopServiceAbility(const Want &want)
{
    if (g_remoteObject == nullptr) {
        g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StopServiceAbility(want);
}

sptr<IAbilityScheduler> AbilityManagerClient::AcquireDataAbility(
    const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken)
{
    g_remoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (g_remoteObject == nullptr) {
        return nullptr;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->AcquireDataAbility(uri, tryBind, callerToken);
}

ErrCode AbilityManagerClient::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
{
    g_remoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (g_remoteObject == nullptr) {
        return -1;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->ReleaseDataAbility(dataAbilityScheduler, callerToken);
}

ErrCode AbilityManagerClient::ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
    int32_t missionId, const sptr<IRemoteObject> &callback, AAFwk::WantParams &wantParams)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);
}

ErrCode AbilityManagerClient::StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken,
    int32_t status)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StartContinuation(want, abilityToken, status);
}

void AbilityManagerClient::NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess)
{
    if (g_remoteObject == nullptr) {
        return;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    abms->NotifyCompleteContinuation(deviceId, sessionId, isSuccess);
}

ErrCode AbilityManagerClient::StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StartSyncRemoteMissions(devId, fixConflict, tag);
}

ErrCode AbilityManagerClient::StopSyncRemoteMissions(const std::string& devId)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StopSyncRemoteMissions(devId);
}

ErrCode AbilityManagerClient::StartUser(int accountId)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StartUser(accountId);
}

ErrCode AbilityManagerClient::StopUser(int accountId, const sptr<IStopUserCallback> &callback)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StopUser(accountId, callback);
}
} // namespace AAFwk
} // namespace OHOS
