/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifdef WITH_DLP
#include "dlp_file_kits.h"
#endif // WITH_DLP
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "scene_board_judgement.h"
#include "session_info.h"
#include "session_manager_lite.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "ws_common.h"

namespace OHOS {
namespace AAFwk {
namespace {
static std::unordered_map<Rosen::WSError, int32_t> SCB_TO_MISSION_ERROR_CODE_MAP {
    { Rosen::WSError::WS_ERROR_INVALID_PERMISSION, CHECK_PERMISSION_FAILED },
    { Rosen::WSError::WS_ERROR_NOT_SYSTEM_APP, ERR_NOT_SYSTEM_APP },
    { Rosen::WSError::WS_ERROR_INVALID_PARAM, INVALID_PARAMETERS_ERR },
};
}

using OHOS::Rosen::SessionManagerLite;
std::shared_ptr<AbilityManagerClient> AbilityManagerClient::instance_ = nullptr;
std::once_flag AbilityManagerClient::singletonFlag_;
#ifdef WITH_DLP
const std::string DLP_PARAMS_SANDBOX = "ohos.dlp.params.sandbox";
#endif // WITH_DLP

#define CHECK_POINTER_RETURN(object)     \
    if (!object) {                       \
        HILOG_ERROR("proxy is nullptr"); \
        return;                          \
    }

#define CHECK_POINTER_RETURN_NOT_CONNECTED(object)   \
    if (!object) {                                   \
        HILOG_ERROR("proxy is nullptr.");            \
        return ABILITY_SERVICE_NOT_CONNECTED;        \
    }

#define CHECK_POINTER_RETURN_INVALID_VALUE(object)   \
    if (!object) {                                   \
        HILOG_ERROR("proxy is nullptr.");            \
        return ERR_INVALID_VALUE;                    \
    }

std::shared_ptr<AbilityManagerClient> AbilityManagerClient::GetInstance()
{
    std::call_once(singletonFlag_, [] () {
        instance_ = std::shared_ptr<AbilityManagerClient>(new AbilityManagerClient());
    });
    return instance_;
}

AbilityManagerClient::AbilityManagerClient()
{}

AbilityManagerClient::~AbilityManagerClient()
{
    HILOG_INFO("Remove DeathRecipient");
    RemoveDeathRecipient();
}

ErrCode AbilityManagerClient::AttachAbilityThread(
    sptr<IAbilityScheduler> scheduler, sptr<IRemoteObject> token)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->AttachAbilityThread(scheduler, token);
}

ErrCode AbilityManagerClient::AbilityTransitionDone(sptr<IRemoteObject> token, int state, const PacMap &saveData)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->AbilityTransitionDone(token, state, saveData);
}

ErrCode AbilityManagerClient::ScheduleConnectAbilityDone(
    sptr<IRemoteObject> token, sptr<IRemoteObject> remoteObject)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ScheduleConnectAbilityDone(token, remoteObject);
}

ErrCode AbilityManagerClient::ScheduleDisconnectAbilityDone(sptr<IRemoteObject> token)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ScheduleDisconnectAbilityDone(token);
}

ErrCode AbilityManagerClient::ScheduleCommandAbilityDone(sptr<IRemoteObject> token)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ScheduleCommandAbilityDone(token);
}

ErrCode AbilityManagerClient::ScheduleCommandAbilityWindowDone(
    sptr<IRemoteObject> token,
    sptr<SessionInfo> sessionInfo,
    WindowCommand winCmd,
    AbilityCommand abilityCmd)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ScheduleCommandAbilityWindowDone(token, sessionInfo, winCmd, abilityCmd);
}

ErrCode AbilityManagerClient::StartAbility(const Want &want, int requestCode, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HandleDlpApp(const_cast<Want &>(want));
    return abms->StartAbility(want, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbility(
    const Want &want, sptr<IRemoteObject> callerToken, int requestCode, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_DEBUG("ability:%{public}s, userId:%{public}d",
        want.GetElement().GetAbilityName().c_str(), userId);
    HandleDlpApp(const_cast<Want &>(want));
    return abms->StartAbility(want, callerToken, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbilityByInsightIntent(
    const Want &want, sptr<IRemoteObject> callerToken, uint64_t intentId, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_DEBUG("ability:%{public}s, bundle:%{public}s, intentId:%{public}" PRIu64,
        want.GetElement().GetAbilityName().c_str(), want.GetElement().GetBundleName().c_str(), intentId);
    HandleDlpApp(const_cast<Want &>(want));
    return abms->StartAbilityByInsightIntent(want, callerToken, intentId, userId);
}

ErrCode AbilityManagerClient::StartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
    sptr<IRemoteObject> callerToken, int requestCode, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HandleDlpApp(const_cast<Want &>(want));
    return abms->StartAbility(want, abilityStartSetting, callerToken, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbility(const Want &want, const StartOptions &startOptions,
    sptr<IRemoteObject> callerToken, int requestCode, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("abilityName:%{public}s, userId:%{public}d.", want.GetElement().GetAbilityName().c_str(), userId);
    HandleDlpApp(const_cast<Want &>(want));
    return abms->StartAbility(want, startOptions, callerToken, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbilityAsCaller(
    const Want &want, sptr<IRemoteObject> callerToken,
    sptr<IRemoteObject> asCallerSoureToken, int requestCode, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("ability:%{public}s, userId:%{public}d.",
               want.GetElement().GetAbilityName().c_str(), userId);
    HandleDlpApp(const_cast<Want &>(want));
    return abms->StartAbilityAsCaller(want, callerToken, asCallerSoureToken, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbilityAsCaller(const Want &want, const StartOptions &startOptions,
    sptr<IRemoteObject> callerToken, sptr<IRemoteObject> asCallerSourceToken,
    int requestCode, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("abilityName:%{public}s, userId:%{public}d", want.GetElement().GetAbilityName().c_str(), userId);
    HandleDlpApp(const_cast<Want &>(want));
    return abms->StartAbilityAsCaller(want, startOptions, callerToken, asCallerSourceToken, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbilityByUIContentSession(const Want &want, const StartOptions &startOptions,
    sptr<IRemoteObject> callerToken, sptr<AAFwk::SessionInfo> sessionInfo,
    int requestCode, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("abilityName:%{public}s, userId:%{public}d.", want.GetElement().GetAbilityName().c_str(), userId);
    HandleDlpApp(const_cast<Want &>(want));
    return abms->StartAbilityByUIContentSession(want, startOptions, callerToken, sessionInfo, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbilityByUIContentSession(const Want &want, sptr<IRemoteObject> callerToken,
    sptr<AAFwk::SessionInfo> sessionInfo, int requestCode, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("ability:%{public}s, userId:%{public}d",
        want.GetElement().GetAbilityName().c_str(), userId);
    HandleDlpApp(const_cast<Want &>(want));
    return abms->StartAbilityByUIContentSession(want, callerToken, sessionInfo, userId, requestCode);
}

ErrCode AbilityManagerClient::SendResultToAbility(int requestCode, int resultCode, Want& resultWant)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("call");
    return abms->SendResultToAbility(requestCode, resultCode, resultWant);
}

ErrCode AbilityManagerClient::StartExtensionAbility(const Want &want, sptr<IRemoteObject> callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_DEBUG("name:%{public}s %{public}s, userId=%{public}d.",
        want.GetElement().GetAbilityName().c_str(), want.GetElement().GetBundleName().c_str(), userId);
    return abms->StartExtensionAbility(want, callerToken, userId, extensionType);
}

ErrCode AbilityManagerClient::RequestModalUIExtension(const Want &want)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RequestModalUIExtension(want);
}

ErrCode AbilityManagerClient::ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ChangeAbilityVisibility(token, isShow);
}

ErrCode AbilityManagerClient::ChangeUIAbilityVisibilityBySCB(sptr<SessionInfo> sessionInfo, bool isShow)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("abilityName: %{public}s, isShow: %{public}d",
        sessionInfo->want.GetElement().GetAbilityName().c_str(), isShow);
    return abms->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow);
}

ErrCode AbilityManagerClient::StartUIExtensionAbility(sptr<SessionInfo> extensionSessionInfo, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    CHECK_POINTER_RETURN_INVALID_VALUE(extensionSessionInfo);
    HILOG_INFO("name: %{public}s %{public}s, persistentId: %{public}d, userId: %{public}d.",
        extensionSessionInfo->want.GetElement().GetAbilityName().c_str(),
        extensionSessionInfo->want.GetElement().GetBundleName().c_str(), extensionSessionInfo->persistentId, userId);
    return abms->StartUIExtensionAbility(extensionSessionInfo, userId);
}

ErrCode AbilityManagerClient::StartUIAbilityBySCB(sptr<SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("abilityName: %{public}s.", sessionInfo->want.GetElement().GetAbilityName().c_str());
    return abms->StartUIAbilityBySCB(sessionInfo);
}

ErrCode AbilityManagerClient::StopExtensionAbility(const Want &want, sptr<IRemoteObject> callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("name:%{public}s %{public}s, userId=%{public}d.",
        want.GetElement().GetAbilityName().c_str(), want.GetElement().GetBundleName().c_str(), userId);
    return abms->StopExtensionAbility(want, callerToken, userId, extensionType);
}

ErrCode AbilityManagerClient::TerminateAbility(sptr<IRemoteObject> token, int resultCode, const Want *resultWant)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_DEBUG("call");
    return abms->TerminateAbility(token, resultCode, resultWant);
}

ErrCode AbilityManagerClient::TerminateUIExtensionAbility(sptr<SessionInfo> extensionSessionInfo,
    int resultCode, const Want *resultWant)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    CHECK_POINTER_RETURN_INVALID_VALUE(extensionSessionInfo);
    HILOG_DEBUG("name: %{public}s %{public}s, persistentId: %{public}d.",
        extensionSessionInfo->want.GetElement().GetAbilityName().c_str(),
        extensionSessionInfo->want.GetElement().GetBundleName().c_str(), extensionSessionInfo->persistentId);
    return abms->TerminateUIExtensionAbility(extensionSessionInfo, resultCode, resultWant);
}

ErrCode AbilityManagerClient::MoveAbilityToBackground(sptr<IRemoteObject> token)
{
    HILOG_INFO("call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->MoveAbilityToBackground(token);
}

ErrCode AbilityManagerClient::MoveUIAbilityToBackground(const sptr<IRemoteObject> token)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->MoveUIAbilityToBackground(token);
}

ErrCode AbilityManagerClient::CloseAbility(sptr<IRemoteObject> token, int resultCode, const Want *resultWant)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_DEBUG("call");
        sptr<AAFwk::SessionInfo> info = new AAFwk::SessionInfo();
        info->want = *resultWant;
        info->resultCode = resultCode;
        info->sessionToken = token;
        auto err = sceneSessionManager->TerminateSessionNew(info, false);
        HILOG_INFO("CloseAbility Calling SceneBoard Interface ret=%{public}d", err);
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("call");
    return abms->CloseAbility(token, resultCode, resultWant);
}

ErrCode AbilityManagerClient::CloseUIAbilityBySCB(sptr<SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        HILOG_ERROR("failed, sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_DEBUG("call");
    return abms->CloseUIAbilityBySCB(sessionInfo);
}

ErrCode AbilityManagerClient::MinimizeAbility(sptr<IRemoteObject> token, bool fromUser)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("fromUser:%{public}d.", fromUser);
    return abms->MinimizeAbility(token, fromUser);
}

ErrCode AbilityManagerClient::MinimizeUIExtensionAbility(sptr<SessionInfo> extensionSessionInfo, bool fromUser)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    CHECK_POINTER_RETURN_INVALID_VALUE(extensionSessionInfo);
    HILOG_DEBUG("name: %{public}s %{public}s, persistentId: %{public}d, fromUser: %{public}d.",
        extensionSessionInfo->want.GetElement().GetAbilityName().c_str(),
        extensionSessionInfo->want.GetElement().GetBundleName().c_str(), extensionSessionInfo->persistentId, fromUser);
    return abms->MinimizeUIExtensionAbility(extensionSessionInfo, fromUser);
}

ErrCode AbilityManagerClient::MinimizeUIAbilityBySCB(sptr<SessionInfo> sessionInfo, bool fromUser)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        HILOG_ERROR("failed, sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("call");
    return abms->MinimizeUIAbilityBySCB(sessionInfo, fromUser);
}

ErrCode AbilityManagerClient::ConnectAbility(const Want &want, sptr<IAbilityConnection> connect, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_DEBUG("name:%{public}s %{public}s, userId:%{public}d.",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    return abms->ConnectAbilityCommon(want, connect, nullptr, AppExecFwk::ExtensionAbilityType::SERVICE, userId);
}

ErrCode AbilityManagerClient::ConnectAbility(
    const Want &want, sptr<IAbilityConnection> connect, sptr<IRemoteObject> callerToken, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("name:%{public}s %{public}s, userId:%{public}d.",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    return abms->ConnectAbilityCommon(want, connect, callerToken, AppExecFwk::ExtensionAbilityType::SERVICE, userId);
}

ErrCode AbilityManagerClient::ConnectDataShareExtensionAbility(const Want &want,
    sptr<IAbilityConnection> connect, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    if (abms == nullptr) {
        HILOG_ERROR("Connect failed, bundleName:%{public}s, abilityName:%{public}s, uri:%{public}s.",
            want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(),
            want.GetUriString().c_str());
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    HILOG_INFO("name:%{public}s %{public}s, uri:%{public}s.",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(),
        want.GetUriString().c_str());
    return abms->ConnectAbilityCommon(want, connect, nullptr, AppExecFwk::ExtensionAbilityType::DATASHARE, userId);
}

ErrCode AbilityManagerClient::ConnectExtensionAbility(const Want &want, sptr<IAbilityConnection> connect,
    int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    if (abms == nullptr) {
        HILOG_ERROR("Connect failed, bundleName:%{public}s, abilityName:%{public}s",
            want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str());
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    HILOG_INFO("name:%{public}s %{public}s, userId:%{public}d.",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    return abms->ConnectAbilityCommon(want, connect, nullptr, AppExecFwk::ExtensionAbilityType::UNSPECIFIED, userId);
}

ErrCode AbilityManagerClient::ConnectUIExtensionAbility(const Want &want, sptr<IAbilityConnection> connect,
    sptr<SessionInfo> sessionInfo, int32_t userId, sptr<UIExtensionAbilityConnectInfo> connectInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    if (abms == nullptr) {
        HILOG_ERROR("Connect failed, bundleName:%{public}s, abilityName:%{public}s, uri:%{public}s.",
            want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(),
            want.GetUriString().c_str());
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    HILOG_INFO("name:%{public}s %{public}s, uri:%{public}s.",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(),
        want.GetUriString().c_str());
    return abms->ConnectUIExtensionAbility(want, connect, sessionInfo, userId, connectInfo);
}

ErrCode AbilityManagerClient::DisconnectAbility(sptr<IAbilityConnection> connect)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_DEBUG("call");
    return abms->DisconnectAbility(connect);
}

sptr<IAbilityScheduler> AbilityManagerClient::AcquireDataAbility(
    const Uri &uri, bool tryBind, sptr<IRemoteObject> callerToken)
{
    auto abms = GetAbilityManager();
    if (!abms) {
        return nullptr;
    }
    return abms->AcquireDataAbility(uri, tryBind, callerToken);
}

ErrCode AbilityManagerClient::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, sptr<IRemoteObject> callerToken)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ReleaseDataAbility(dataAbilityScheduler, callerToken);
}

ErrCode AbilityManagerClient::DumpState(const std::string &args, std::vector<std::string> &state)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    abms->DumpState(args, state);
    return ERR_OK;
}

ErrCode AbilityManagerClient::DumpSysState(
    const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int UserID)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    abms->DumpSysState(args, state, isClient, isUserID, UserID);
    return ERR_OK;
}

ErrCode AbilityManagerClient::Connect()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (proxy_ != nullptr) {
        return ERR_OK;
    }
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        HILOG_ERROR("Fail to get registry.");
        return GET_ABILITY_SERVICE_FAILED;
    }
    sptr<IRemoteObject> remoteObj = systemManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObj == nullptr) {
        HILOG_ERROR("Fail to connect ability manager service.");
        return GET_ABILITY_SERVICE_FAILED;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new AbilityMgrDeathRecipient());
    if (deathRecipient_ == nullptr) {
        HILOG_ERROR("Failed to create AbilityMgrDeathRecipient!");
        return GET_ABILITY_SERVICE_FAILED;
    }
    if ((remoteObj->IsProxyObject()) && (!remoteObj->AddDeathRecipient(deathRecipient_))) {
        HILOG_ERROR("Add death recipient to AbilityManagerService failed.");
        return GET_ABILITY_SERVICE_FAILED;
    }

    proxy_ = iface_cast<IAbilityManager>(remoteObj);
    HILOG_DEBUG("Connect ability manager service success.");
    return ERR_OK;
}

void AbilityManagerClient::RemoveDeathRecipient()
{
    HILOG_INFO("RemoveDeathRecipient");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        HILOG_INFO("AbilityMgrProxy do not exist");
        return;
    }
    if (deathRecipient_ == nullptr) {
        HILOG_INFO("AbilityMgrDeathRecipient do not exist");
        return;
    }
    auto serviceRemote = proxy_->AsObject();
    if (serviceRemote != nullptr && serviceRemote->RemoveDeathRecipient(deathRecipient_)) {
        proxy_ = nullptr;
        deathRecipient_ = nullptr;
        HILOG_INFO("Remove DeathRecipient success");
    }
}

ErrCode AbilityManagerClient::StopServiceAbility(const Want &want, sptr<IRemoteObject> token)
{
    HILOG_INFO("call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StopServiceAbility(want, -1, token);
}

ErrCode AbilityManagerClient::KillProcess(const std::string &bundleName)
{
    HILOG_INFO("enter");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->KillProcess(bundleName);
}

#ifdef ABILITY_COMMAND_FOR_TEST
ErrCode AbilityManagerClient::ForceTimeoutForTest(const std::string &abilityName, const std::string &state)
{
    HILOG_DEBUG("enter");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ForceTimeoutForTest(abilityName, state);
}
#endif

ErrCode AbilityManagerClient::ClearUpApplicationData(const std::string &bundleName, const int32_t userId)
{
    HILOG_INFO("call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ClearUpApplicationData(bundleName, userId);
}

ErrCode AbilityManagerClient::ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
    int32_t missionId, sptr<IRemoteObject> callback, AAFwk::WantParams &wantParams)
{
    if (srcDeviceId.empty() || dstDeviceId.empty() || callback == nullptr) {
        HILOG_ERROR("srcDeviceId or dstDeviceId or callback is null!");
        return ERR_INVALID_VALUE;
    }

    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    int result = abms->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);
    return result;
}

ErrCode AbilityManagerClient::ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
    const std::string &bundleName, sptr<IRemoteObject> callback, AAFwk::WantParams &wantParams)
{
    if (srcDeviceId.empty() || dstDeviceId.empty() || callback == nullptr) {
        HILOG_ERROR("srcDeviceId or dstDeviceId or callback is null!");
        return ERR_INVALID_VALUE;
    }

    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    int result = abms->ContinueMission(srcDeviceId, dstDeviceId, bundleName, callback, wantParams);
    return result;
}

ErrCode AbilityManagerClient::StartContinuation(const Want &want, sptr<IRemoteObject> abilityToken,
    int32_t status)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    int result = abms->StartContinuation(want, abilityToken, status);
    return result;
}

void AbilityManagerClient::NotifyCompleteContinuation(const std::string &deviceId,
    int32_t sessionId, bool isSuccess)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->NotifyCompleteContinuation(deviceId, sessionId, isSuccess);
}

ErrCode AbilityManagerClient::ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ContinueAbility(deviceId, missionId, versionCode);
}

ErrCode AbilityManagerClient::NotifyContinuationResult(int32_t missionId, int32_t result)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->NotifyContinuationResult(missionId, result);
}

ErrCode AbilityManagerClient::LockMissionForCleanup(int32_t missionId)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_INFO("call");
        auto err = sceneSessionManager->LockSession(missionId);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->LockMissionForCleanup(missionId);
}

ErrCode AbilityManagerClient::UnlockMissionForCleanup(int32_t missionId)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_INFO("call");
        auto err = sceneSessionManager->UnlockSession(missionId);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->UnlockMissionForCleanup(missionId);
}

void AbilityManagerClient::SetLockedState(int32_t sessionId, bool lockedState)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->SetLockedState(sessionId, lockedState);
}

ErrCode AbilityManagerClient::RegisterMissionListener(sptr<IMissionListener> listener)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_INFO("call");
        auto err = sceneSessionManager->RegisterSessionListener(listener);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterMissionListener(listener);
}

ErrCode AbilityManagerClient::UnRegisterMissionListener(sptr<IMissionListener> listener)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_INFO("call");
        auto err = sceneSessionManager->UnRegisterSessionListener(listener);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->UnRegisterMissionListener(listener);
}

ErrCode AbilityManagerClient::RegisterMissionListener(const std::string &deviceId,
    sptr<IRemoteMissionListener> listener)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterMissionListener(deviceId, listener);
}

ErrCode AbilityManagerClient::RegisterOnListener(const std::string &type,
    sptr<IRemoteOnListener> listener)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterOnListener(type, listener);
}

ErrCode AbilityManagerClient::RegisterOffListener(const std::string &type,
    sptr<IRemoteOnListener> listener)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterOffListener(type, listener);
}

ErrCode AbilityManagerClient::UnRegisterMissionListener(const std::string &deviceId,
    sptr<IRemoteMissionListener> listener)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->UnRegisterMissionListener(deviceId, listener);
}

ErrCode AbilityManagerClient::GetMissionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_INFO("call");
        auto err = sceneSessionManager->GetSessionInfos(deviceId, numMax, missionInfos);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetMissionInfos(deviceId, numMax, missionInfos);
}

ErrCode AbilityManagerClient::GetMissionInfo(const std::string& deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_DEBUG("call");
        auto err = sceneSessionManager->GetSessionInfo(deviceId, missionId, missionInfo);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetMissionInfo(deviceId, missionId, missionInfo);
}

ErrCode AbilityManagerClient::CleanMission(int32_t missionId)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_INFO("call");
        auto err = sceneSessionManager->ClearSession(missionId);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->CleanMission(missionId);
}

ErrCode AbilityManagerClient::CleanAllMissions()
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_INFO("call");
        auto err = sceneSessionManager->ClearAllSessions();
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->CleanAllMissions();
}

ErrCode AbilityManagerClient::MoveMissionToFront(int32_t missionId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->MoveMissionToFront(missionId);
}

ErrCode AbilityManagerClient::MoveMissionToFront(int32_t missionId, const StartOptions &startOptions)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->MoveMissionToFront(missionId, startOptions);
}

ErrCode AbilityManagerClient::MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId)
{
    HILOG_INFO("MoveMissionsToForeground begin, topMissionId:%{public}d", topMissionId);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_INFO("call");
        auto err = sceneSessionManager->MoveSessionsToForeground(missionIds, topMissionId);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        auto abms = GetAbilityManager();
        CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
        if (missionIds.empty()) {
            return ERR_INVALID_VALUE;
        }
        int32_t missionId = topMissionId;
        if (topMissionId > 0) {
            missionId = topMissionId;
        } else {
            missionId = missionIds[0];
        }
        auto errAMS = abms->MoveMissionToFront(missionId);
        return static_cast<int>(errAMS);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->MoveMissionsToForeground(missionIds, topMissionId);
}

ErrCode AbilityManagerClient::MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
    std::vector<int32_t>& result)
{
    HILOG_INFO("MoveMissionsToBackground begin.");
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_INFO("call");
        auto err = sceneSessionManager->MoveSessionsToBackground(missionIds, result);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->MoveMissionsToBackground(missionIds, result);
}

ErrCode AbilityManagerClient::GetMissionIdByToken(sptr<IRemoteObject> token, int32_t &missionId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    missionId = abms->GetMissionIdByToken(token);
    if (missionId <= 0) {
        HILOG_ERROR("get missionid by token failed!");
        return MISSION_NOT_FOUND;
    }
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartAbilityByCall(const Want &want, sptr<IAbilityConnection> connect)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_DEBUG("AbilityManagerClient::StartAbilityByCall called.");
    return abms->StartAbilityByCall(want, connect, nullptr, DEFAULT_INVAL_VALUE);
}

ErrCode AbilityManagerClient::StartAbilityByCall(const Want &want, sptr<IAbilityConnection> connect,
    sptr<IRemoteObject> callToken, int32_t accountId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_DEBUG("AbilityManagerClient::StartAbilityByCall called.");
    return abms->StartAbilityByCall(want, connect, callToken, accountId);
}

void AbilityManagerClient::CallRequestDone(sptr<IRemoteObject> token, sptr<IRemoteObject> callStub)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->CallRequestDone(token, callStub);
}

void AbilityManagerClient::GetAbilityTokenByCalleeObj(sptr<IRemoteObject> callStub, sptr<IRemoteObject> &token)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->GetAbilityTokenByCalleeObj(callStub, token);
}

ErrCode AbilityManagerClient::ReleaseCall(
    sptr<IAbilityConnection> connect, const AppExecFwk::ElementName &element)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ReleaseCall(connect, element);
}

ErrCode AbilityManagerClient::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetAbilityRunningInfos(info);
}

ErrCode AbilityManagerClient::GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetExtensionRunningInfos(upperLimit, info);
}

ErrCode AbilityManagerClient::GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetProcessRunningInfos(info);
}

ErrCode AbilityManagerClient::RequestDialogService(
    const Want &want, sptr<IRemoteObject> callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("request is:%{public}s.", want.GetElement().GetURI().c_str());
    HandleDlpApp(const_cast<Want &>(want));
    return abms->RequestDialogService(want, callerToken);
}

ErrCode AbilityManagerClient::ReportDrawnCompleted(sptr<IRemoteObject> callerToken)
{
    HILOG_DEBUG("called.");
    auto abilityMgr = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abilityMgr);
    return abilityMgr->ReportDrawnCompleted(callerToken);
}

/**
 * Start synchronizing remote device mission
 * @param devId, deviceId.
 * @param fixConflict, resolve synchronizing conflicts flag.
 * @param tag, call tag.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode AbilityManagerClient::StartSyncRemoteMissions(const std::string &devId, bool fixConflict, int64_t tag)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StartSyncRemoteMissions(devId, fixConflict, tag);
}
/**
 * Stop synchronizing remote device mission
 * @param devId, deviceId.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode AbilityManagerClient::StopSyncRemoteMissions(const std::string &devId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StopSyncRemoteMissions(devId);
}

ErrCode AbilityManagerClient::StartUser(int accountId, sptr<IUserCallback> callback)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StartUser(accountId, callback);
}
ErrCode AbilityManagerClient::StopUser(int accountId, sptr<IUserCallback> callback)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StopUser(accountId, callback);
}

ErrCode AbilityManagerClient::LogoutUser(int32_t accountId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->LogoutUser(accountId);
}

ErrCode AbilityManagerClient::RegisterSnapshotHandler(sptr<ISnapshotHandler> handler)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterSnapshotHandler(handler);
}

ErrCode AbilityManagerClient::GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& snapshot, bool isLowResolution)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_INFO("call");
        auto err = sceneSessionManager->GetSessionSnapshot(deviceId, missionId, snapshot, isLowResolution);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetMissionSnapshot(deviceId, missionId, snapshot, isLowResolution);
}

ErrCode AbilityManagerClient::StartUserTest(const Want &want, sptr<IRemoteObject> observer)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StartUserTest(want, observer);
}

ErrCode AbilityManagerClient::FinishUserTest(
    const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->FinishUserTest(msg, resultCode, bundleName);
}

ErrCode AbilityManagerClient::GetTopAbility(sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_DEBUG("call");
        auto err = sceneSessionManager->GetFocusSessionToken(token);
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetTopAbility(token);
}

AppExecFwk::ElementName AbilityManagerClient::GetElementNameByToken(sptr<IRemoteObject> token,
    bool isNeedLocalDeviceId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    if (abms == nullptr) {
        HILOG_ERROR("abms == nullptr");
        return {};
    }
    return abms->GetElementNameByToken(token, isNeedLocalDeviceId);
}

ErrCode AbilityManagerClient::CheckUIExtensionIsFocused(uint32_t uiExtensionTokenId, bool& isFocused)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->CheckUIExtensionIsFocused(uiExtensionTokenId, isFocused);
}

ErrCode AbilityManagerClient::DelegatorDoAbilityForeground(sptr<IRemoteObject> token)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_DEBUG("call");
        sceneSessionManager->PendingSessionToForeground(token);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->DelegatorDoAbilityForeground(token);
}

ErrCode AbilityManagerClient::DelegatorDoAbilityBackground(sptr<IRemoteObject> token)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_DEBUG("call");
        sceneSessionManager->PendingSessionToBackgroundForDelegator(token);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->DelegatorDoAbilityBackground(token);
}

ErrCode AbilityManagerClient::SetMissionContinueState(sptr<IRemoteObject> token,
    const AAFwk::ContinueState &state)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_DEBUG("call");
        uint32_t value = static_cast<uint32_t>(state);
        Rosen::ContinueState continueState = static_cast<Rosen::ContinueState>(value);
        auto err = sceneSessionManager->SetSessionContinueState(token, continueState);
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SetMissionContinueState(token, state);
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityManagerClient::SetMissionLabel(sptr<IRemoteObject> token, const std::string& label)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_INFO("call");
        auto err = sceneSessionManager->SetSessionLabel(token, label);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SetMissionLabel(token, label);
}

ErrCode AbilityManagerClient::SetMissionIcon(
    sptr<IRemoteObject> abilityToken, std::shared_ptr<OHOS::Media::PixelMap> icon)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        HILOG_INFO("call");
        auto err = sceneSessionManager->SetSessionIcon(abilityToken, icon);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SetMissionIcon(abilityToken, icon);
}

ErrCode AbilityManagerClient::RegisterWindowManagerServiceHandler(sptr<IWindowManagerServiceHandler> handler)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterWindowManagerServiceHandler(handler);
}

void AbilityManagerClient::CompleteFirstFrameDrawing(sptr<IRemoteObject> abilityToken)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->CompleteFirstFrameDrawing(abilityToken);
}

ErrCode AbilityManagerClient::PrepareTerminateAbility(sptr<IRemoteObject> token,
    sptr<IPrepareTerminateCallback> callback)
{
    if (callback == nullptr) {
        HILOG_ERROR("callback is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto abms = GetAbilityManager();
    if (abms == nullptr) {
        HILOG_ERROR("abms is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return abms->PrepareTerminateAbility(token, callback);
}

ErrCode AbilityManagerClient::GetDialogSessionInfo(const std::string dialogSessionId, sptr<DialogSessionInfo> &info)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetDialogSessionInfo(dialogSessionId, info);
}

ErrCode AbilityManagerClient::SendDialogResult(const Want &want, const std::string dialogSessionId, const bool isAllow)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SendDialogResult(want, dialogSessionId, isAllow);
}
#endif

ErrCode AbilityManagerClient::DoAbilityForeground(sptr<IRemoteObject> token, uint32_t flag)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->DoAbilityForeground(token, flag);
}

ErrCode AbilityManagerClient::DoAbilityBackground(sptr<IRemoteObject> token, uint32_t flag)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->DoAbilityBackground(token, flag);
}

ErrCode AbilityManagerClient::SetAbilityController(sptr<AppExecFwk::IAbilityController> abilityController,
    bool imAStabilityTest)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SetAbilityController(abilityController, imAStabilityTest);
}

ErrCode AbilityManagerClient::SendANRProcessID(int pid)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SendANRProcessID(pid);
}

void AbilityManagerClient::UpdateMissionSnapShot(sptr<IRemoteObject> token,
    std::shared_ptr<Media::PixelMap> pixelMap)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    return abms->UpdateMissionSnapShot(token, pixelMap);
}

void AbilityManagerClient::EnableRecoverAbility(sptr<IRemoteObject> token)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    return abms->EnableRecoverAbility(token);
}

void AbilityManagerClient::ScheduleRecoverAbility(sptr<IRemoteObject> token, int32_t reason, const Want *want)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    return abms->ScheduleRecoverAbility(token, reason, want);
}

#ifdef ABILITY_COMMAND_FOR_TEST
ErrCode AbilityManagerClient::BlockAmsService()
{
    HILOG_DEBUG("enter");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->BlockAmsService();
}

ErrCode AbilityManagerClient::BlockAbility(int32_t abilityRecordId)
{
    HILOG_DEBUG("enter");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->BlockAbility(abilityRecordId);
}

ErrCode AbilityManagerClient::BlockAppService()
{
    HILOG_DEBUG("enter");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->BlockAppService();
}
#endif

sptr<IAbilityManager> AbilityManagerClient::GetAbilityManager()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!proxy_) {
        (void)Connect();
    }

    return proxy_;
}

void AbilityManagerClient::ResetProxy(wptr<IRemoteObject> remote)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!proxy_) {
        return;
    }

    auto serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        HILOG_DEBUG("To remove death recipient.");
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
    }
}

void AbilityManagerClient::AbilityMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOG_DEBUG("AbilityMgrDeathRecipient handle remote died.");
    AbilityManagerClient::GetInstance()->ResetProxy(remote);
}

ErrCode AbilityManagerClient::FreeInstallAbilityFromRemote(const Want &want, sptr<IRemoteObject> callback,
    int32_t userId, int requestCode)
{
    HILOG_INFO("enter");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->FreeInstallAbilityFromRemote(want, callback, userId, requestCode);
}

AppExecFwk::ElementName AbilityManagerClient::GetTopAbility(bool isNeedLocalDeviceId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    {
        std::lock_guard<std::recursive_mutex> lock_l(mutex_);
        if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
            AppExecFwk::ElementName elementName = {};
            sptr<IRemoteObject> token = nullptr;
            auto ret = GetTopAbility(token);
            if (ret != ERR_OK) {
                HILOG_ERROR("get top ability token failed");
                return elementName;
            }
            if (token == nullptr) {
                HILOG_ERROR("token is nullptr");
                return elementName;
            }
            return GetElementNameByToken(token, isNeedLocalDeviceId);
        }
    }
    HILOG_DEBUG("enter.");
    auto abms = GetAbilityManager();
    if (abms == nullptr) {
        HILOG_ERROR("abms == nullptr");
        return {};
    }

    return abms->GetTopAbility(isNeedLocalDeviceId);
}

ErrCode AbilityManagerClient::DumpAbilityInfoDone(std::vector<std::string> &infos,
    sptr<IRemoteObject> callerToken)
{
    HILOG_INFO("call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->DumpAbilityInfoDone(infos, callerToken);
}

void AbilityManagerClient::HandleDlpApp(Want &want)
{
#ifdef WITH_DLP
    if (!want.GetParams().HasParam(DLP_PARAMS_SANDBOX)) {
        bool sandboxFlag = Security::DlpPermission::DlpFileKits::GetSandboxFlag(want);
        want.SetParam(DLP_PARAMS_SANDBOX, sandboxFlag);
    }
#endif // WITH_DLP
}

ErrCode AbilityManagerClient::AddFreeInstallObserver(sptr<AbilityRuntime::IFreeInstallObserver> observer)
{
    HILOG_INFO("call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->AddFreeInstallObserver(observer);
}

int32_t AbilityManagerClient::IsValidMissionIds(
    const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results)
{
    HILOG_INFO("call");
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        std::vector<bool> isValidList;
        auto err = sceneSessionManager->IsValidSessionIds(missionIds, isValidList);
        HILOG_DEBUG("IsValidSessionIds %{public}d size %{public}d",
            static_cast<int>(err), static_cast<int32_t>(isValidList.size()));
        for (auto i = 0; i < static_cast<int32_t>(isValidList.size()); ++i) {
            MissionValidResult missionResult = {};
            missionResult.missionId = missionIds.at(i);
            missionResult.isValid = isValidList.at(i);
            results.push_back(missionResult);
        }
        return static_cast<int>(err);
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->IsValidMissionIds(missionIds, results);
}

ErrCode AbilityManagerClient::VerifyPermission(const std::string &permission, int pid, int uid)
{
    HILOG_INFO("call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->VerifyPermission(permission, pid, uid);
}

ErrCode AbilityManagerClient::AcquireShareData(
    int32_t missionId, sptr<IAcquireShareDataCallback> shareData)
{
    HILOG_INFO("call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->AcquireShareData(missionId, shareData);
}

ErrCode AbilityManagerClient::ShareDataDone(
    sptr<IRemoteObject> token, int32_t resultCode, int32_t uniqueId, WantParams &wantParam)
{
    HILOG_INFO("call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ShareDataDone(token, resultCode, uniqueId, wantParam);
}

ErrCode AbilityManagerClient::ForceExitApp(const int32_t pid, const ExitReason &exitReason)
{
    HILOG_DEBUG("begin.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ForceExitApp(pid, exitReason);
}

ErrCode AbilityManagerClient::RecordAppExitReason(const ExitReason &exitReason)
{
    HILOG_DEBUG("RecordAppExitReason reason:%{public}d, exitMsg: %{public}s", exitReason.reason,
        exitReason.exitMsg.c_str());
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RecordAppExitReason(exitReason);
}

ErrCode AbilityManagerClient::RecordProcessExitReason(const int32_t pid, const ExitReason &exitReason)
{
    HILOG_DEBUG("RecordProcessExitReason pid:%{public}d, reason:%{public}d, exitMsg: %{public}s",
        pid, exitReason.reason, exitReason.exitMsg.c_str());
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RecordProcessExitReason(pid, exitReason);
}

void AbilityManagerClient::SetRootSceneSession(sptr<IRemoteObject> rootSceneSession)
{
    HILOG_INFO("call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->SetRootSceneSession(rootSceneSession);
}

void AbilityManagerClient::CallUIAbilityBySCB(sptr<SessionInfo> sessionInfo)
{
    HILOG_INFO("call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->CallUIAbilityBySCB(sessionInfo);
}

void AbilityManagerClient::StartSpecifiedAbilityBySCB(const Want &want)
{
    HILOG_INFO("call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->StartSpecifiedAbilityBySCB(want);
}

ErrCode AbilityManagerClient::NotifySaveAsResult(const Want &want, int resultCode, int requestCode)
{
    HILOG_DEBUG("call.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->NotifySaveAsResult(want, resultCode, requestCode);
}

ErrCode AbilityManagerClient::SetSessionManagerService(sptr<IRemoteObject> sessionManagerService)
{
    HILOG_INFO("AbilityManagerClient::SetSessionManagerService call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SetSessionManagerService(sessionManagerService);
}

ErrCode AbilityManagerClient::RegisterIAbilityManagerCollaborator(
    int32_t type, sptr<IAbilityManagerCollaborator> impl)
{
    HILOG_INFO("call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterIAbilityManagerCollaborator(type, impl);
}

ErrCode AbilityManagerClient::UnregisterIAbilityManagerCollaborator(int32_t type)
{
    HILOG_INFO("call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->UnregisterIAbilityManagerCollaborator(type);
}

ErrCode AbilityManagerClient::RegisterAutoStartupSystemCallback(sptr<IRemoteObject> callback)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterAutoStartupSystemCallback(callback);
}

ErrCode AbilityManagerClient::UnregisterAutoStartupSystemCallback(sptr<IRemoteObject> callback)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->UnregisterAutoStartupSystemCallback(callback);
}

ErrCode AbilityManagerClient::SetApplicationAutoStartup(const AutoStartupInfo &info)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SetApplicationAutoStartup(info);
}

ErrCode AbilityManagerClient::CancelApplicationAutoStartup(const AutoStartupInfo &info)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->CancelApplicationAutoStartup(info);
}

ErrCode AbilityManagerClient::QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->QueryAllAutoStartupApplications(infoList);
}

ErrCode AbilityManagerClient::PrepareTerminateAbilityBySCB(sptr<SessionInfo> sessionInfo,
    bool &isPrepareTerminate)
{
    HILOG_INFO("call.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->PrepareTerminateAbilityBySCB(sessionInfo, isPrepareTerminate);
}

ErrCode AbilityManagerClient::RegisterSessionHandler(sptr<IRemoteObject> object)
{
    HILOG_DEBUG("call");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterSessionHandler(object);
}

ErrCode AbilityManagerClient::RegisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterAppDebugListener(listener);
}

ErrCode AbilityManagerClient::UnregisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->UnregisterAppDebugListener(listener);
}

ErrCode AbilityManagerClient::AttachAppDebug(const std::string &bundleName)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->AttachAppDebug(bundleName);
}

ErrCode AbilityManagerClient::DetachAppDebug(const std::string &bundleName)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->DetachAppDebug(bundleName);
}

ErrCode AbilityManagerClient::ExecuteIntent(uint64_t key, sptr<IRemoteObject> callerToken,
    const InsightIntentExecuteParam &param)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ExecuteIntent(key, callerToken, param);
}

bool AbilityManagerClient::IsAbilityControllerStart(const Want &want)
{
    HILOG_DEBUG("call");
    auto abms = GetAbilityManager();
    if (abms == nullptr) {
        HILOG_ERROR("abms is nullptr.");
        return true;
    }
    return abms->IsAbilityControllerStart(want);
}

ErrCode AbilityManagerClient::ExecuteInsightIntentDone(sptr<IRemoteObject> token, uint64_t intentId,
    const InsightIntentExecuteResult &result)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ExecuteInsightIntentDone(token, intentId, result);
}

int32_t AbilityManagerClient::GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_INVALID_VALUE(abms);
    return abms->GetForegroundUIAbilities(list);
}

int32_t AbilityManagerClient::OpenFile(const Uri& uri, uint32_t flag)
{
    HILOG_DEBUG("call OpenFile");
    auto abms = GetAbilityManager();
    if (abms == nullptr) {
        HILOG_ERROR("abms is nullptr.");
        return true;
    }
    return abms->OpenFile(uri, flag);
}

void AbilityManagerClient::UpdateSessionInfoBySCB(const std::vector<SessionInfo> &sessionInfos, int32_t userId)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->UpdateSessionInfoBySCB(sessionInfos, userId);
}

ErrCode AbilityManagerClient::GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token,
    UIExtensionHostInfo &hostInfo, int32_t userId)
{
    HILOG_DEBUG("Get ui extension host info.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetUIExtensionRootHostInfo(token, hostInfo, userId);
}

int32_t AbilityManagerClient::RestartApp(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_INVALID_VALUE(abms);
    return abms->RestartApp(want);
}

AppExecFwk::ElementName AbilityManagerClient::GetElementNameByAppId(const std::string &appId)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    if (abms == nullptr) {
        HILOG_ERROR("abms is nullptr.");
        return {};
    }
    return abms->GetElementNameByAppId(appId);
}

int32_t AbilityManagerClient::OpenAtomicService(Want& want, sptr<IRemoteObject> callerToken, int32_t requestCode,
    int32_t userId)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_INVALID_VALUE(abms);
    return abms->OpenAtomicService(want, callerToken, requestCode, userId);
}

bool AbilityManagerClient::IsEmbeddedOpenAllowed(sptr<IRemoteObject> callerToken, const std::string &appId)
{
    HILOG_DEBUG("Get ui extension host info.");
    auto abms = GetAbilityManager();
    if (abms == nullptr) {
        HILOG_ERROR("abms is nullptr.");
        return false;
    }
    return abms->IsEmbeddedOpenAllowed(callerToken, appId);
}
} // namespace AAFwk
} // namespace OHOS
