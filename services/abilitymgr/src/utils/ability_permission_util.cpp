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

#include "utils/ability_permission_util.h"

#include "ability_connect_manager.h"
#include "ability_info.h"
#include "ability_util.h"
#include "accesstoken_kit.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "sub_managers_helper.h"

using OHOS::Security::AccessToken::AccessTokenKit;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t APP_ALIVE_TIME_MS = 1000;  // Allow background startup within 1 second after application startup
constexpr int32_t BASE_USER_RANGE = 200000;
constexpr const char* IS_DELEGATOR_CALL = "isDelegatorCall";
constexpr const char* BUNDLE_NAME_DIALOG = "com.ohos.amsdialog";
}

AbilityPermissionUtil &AbilityPermissionUtil::GetInstance()
{
    static AbilityPermissionUtil instance;
    return instance;
}

void AbilityPermissionUtil::Init(std::shared_ptr<SubManagersHelper> subManagersHelper)
{
    subManagersHelper_ = subManagersHelper;
}

bool AbilityPermissionUtil::IsCallerSceneBoard()
{
    int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    CHECK_POINTER_AND_RETURN(subManagersHelper_, false);
    auto connectManager = subManagersHelper_->GetConnectManagerByUserId(userId);
    CHECK_POINTER_AND_RETURN(connectManager, false);
    auto sceneBoardTokenId = connectManager->GetSceneBoardTokenId();
    if (sceneBoardTokenId != 0 && IPCSkeleton::GetCallingTokenID() == sceneBoardTokenId) {
        return true;
    }
    return false;
}

inline bool AbilityPermissionUtil::IsDelegatorCall(const AppExecFwk::RunningProcessInfo &processInfo,
    const AbilityRequest &abilityRequest) const
{
    /*  To make sure the AbilityDelegator is not counterfeited
     *   1. The caller-process must be test-process
     *   2. The callerToken must be nullptr
     */
    if (processInfo.isTestProcess &&
        !abilityRequest.callerToken && abilityRequest.want.GetBoolParam(IS_DELEGATOR_CALL, false)) {
        return true;
    }
    return false;
}

bool AbilityPermissionUtil::IsDominateScreen(const Want &want,
    const sptr<IRemoteObject> &callerToken, bool isForegroundToRestartApp)
{
    if (callerToken != nullptr) {
        return false;
    }
    if (!IsCallerSceneBoard() && !isForegroundToRestartApp && !PermissionVerification::GetInstance()->IsSACall() &&
        !PermissionVerification::GetInstance()->IsShellCall()) {
        auto callerPid = IPCSkeleton::GetCallingPid();
        AppExecFwk::RunningProcessInfo processInfo;
        DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(callerPid, processInfo);
        bool isDelegatorOrForegroundCall =
            (processInfo.isTestProcess && want.GetBoolParam(IS_DELEGATOR_CALL, false)) || processInfo.isFocused;
        if (!isDelegatorOrForegroundCall) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "caller is invalid.");
            return true;
        }
    }
    return false;
}

bool AbilityPermissionUtil::IsCallFromBackground(
    const AbilityRequest &abilityRequest, bool &isBackgroundCall, bool backgroundJudgeFlag, bool isData)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!isData && AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        isBackgroundCall = true;
        return true;
    }

    if (!isData && (AAFwk::PermissionVerification::GetInstance()->IsSACall() ||
        AbilityUtil::IsStartFreeInstall(abilityRequest.want))) {
        isBackgroundCall = false;
        return true;
    }

    AppExecFwk::RunningProcessInfo processInfo;
    std::shared_ptr<AbilityRecord> callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerAbility && callerAbility->GetAbilityInfo().bundleName == BUNDLE_NAME_DIALOG) {
        callerAbility = callerAbility->GetCallerRecord();
    }
    if (callerAbility) {
        if (callerAbility->IsForeground() || callerAbility->GetAbilityForegroundingFlag()) {
            isBackgroundCall = false;
            return true;
        }
        // CallerAbility is not foreground, so check process state
        DelayedSingleton<AppScheduler>::GetInstance()->
            GetRunningProcessInfoByToken(callerAbility->GetToken(), processInfo);
        if (AbilityPermissionUtil::GetInstance().IsDelegatorCall(processInfo, abilityRequest)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "The call is from AbilityDelegator, allow background-call.");
            isBackgroundCall = false;
            return true;
        }
        auto abilityState = callerAbility->GetAbilityState();
        if (abilityState == AbilityState::BACKGROUND || abilityState == AbilityState::BACKGROUNDING ||
            // If uiability or uiextensionability ability state is foreground when terminate,
            // it will move to background firstly. So if startAbility in onBackground() lifecycle,
            // the actual ability state may be had changed to terminating from background or backgrounding.
            abilityState == AbilityState::TERMINATING) {
            return true;
        }
    } else {
        auto callerPid = IPCSkeleton::GetCallingPid();
        DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(callerPid, processInfo);
        if (processInfo.processName_.empty()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Can not find caller application by callerPid: %{private}d.", callerPid);
            if (AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
                PermissionConstants::PERMISSION_START_ABILITIES_FROM_BACKGROUND)) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "Caller has PERMISSION_START_ABILITIES_FROM_BACKGROUND, PASS.");
                isBackgroundCall = false;
                return true;
            }
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Caller does not have PERMISSION_START_ABILITIES_FROM_BACKGROUND, REJECT.");
            return false;
        }
    }
    return SetBackgroundCall(processInfo, abilityRequest, isBackgroundCall, backgroundJudgeFlag);
}

bool AbilityPermissionUtil::SetBackgroundCall(const AppExecFwk::RunningProcessInfo &processInfo,
    const AbilityRequest &abilityRequest, bool &isBackgroundCall, bool backgroundJudgeFlag) const
{
    if (IsDelegatorCall(processInfo, abilityRequest)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "The call is from AbilityDelegator, allow background-call.");
        isBackgroundCall = false;
        return true;
    }

    if (backgroundJudgeFlag) {
        isBackgroundCall = processInfo.state_ != AppExecFwk::AppProcessState::APP_STATE_FOREGROUND &&
            !processInfo.isFocused && !processInfo.isAbilityForegrounding;
    } else {
        isBackgroundCall = !processInfo.isFocused;
        if (!processInfo.isFocused && processInfo.state_ == AppExecFwk::AppProcessState::APP_STATE_FOREGROUND) {
            // Allow background startup within 1 second after application startup if state is FOREGROUND
            int64_t aliveTime = AbilityUtil::SystemTimeMillis() - processInfo.startTimeMillis_;
            isBackgroundCall = aliveTime > APP_ALIVE_TIME_MS;
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Process %{public}s is alive %{public}s ms.",
                processInfo.processName_.c_str(), std::to_string(aliveTime).c_str());
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "backgroundJudgeFlag: %{public}d, isBackgroundCall: %{public}d, callerAppState: %{public}d.",
        static_cast<int32_t>(backgroundJudgeFlag),
        static_cast<int32_t>(isBackgroundCall),
        static_cast<int32_t>(processInfo.state_));

    return true;
}

int AbilityPermissionUtil::CheckStaticCfgPermissionForAbility(const AppExecFwk::AbilityInfo &abilityInfo,
    uint32_t tokenId)
{
    if (abilityInfo.permissions.empty() || AccessTokenKit::VerifyAccessToken(tokenId,
        PermissionConstants::PERMISSION_START_INVISIBLE_ABILITY, false) == ERR_OK) {
        return AppExecFwk::Constants::PERMISSION_GRANTED;
    }

    for (auto permission : abilityInfo.permissions) {
        if (AccessTokenKit::VerifyAccessToken(tokenId, permission, false) !=
            AppExecFwk::Constants::PERMISSION_GRANTED) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "verify access token fail, Ability permission: %{public}s",
                permission.c_str());
            return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
        }
    }

    return AppExecFwk::Constants::PERMISSION_GRANTED;
}

bool AbilityPermissionUtil::CheckOneSkillPermission(const AppExecFwk::Skill &skill, uint32_t tokenId)
{
    for (auto permission : skill.permissions) {
        if (AccessTokenKit::VerifyAccessToken(tokenId, permission, false) !=
            AppExecFwk::Constants::PERMISSION_GRANTED) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "verify access token fail, Skill permission: %{public}s",
                permission.c_str());
            return false;
        }
    }

    return true;
}

int AbilityPermissionUtil::CheckStaticCfgPermissionForSkill(const AbilityRequest &abilityRequest,
    uint32_t tokenId)
{
    auto abilityInfo = abilityRequest.abilityInfo;
    auto resultAbilityPermission = CheckStaticCfgPermissionForAbility(abilityInfo, tokenId);
    if (resultAbilityPermission != AppExecFwk::Constants::PERMISSION_GRANTED) {
        return resultAbilityPermission;
    }

    if (abilityInfo.skills.empty()) {
        return AppExecFwk::Constants::PERMISSION_GRANTED;
    }
    int32_t result = AppExecFwk::Constants::PERMISSION_GRANTED;
    for (auto skill : abilityInfo.skills) {
        if (skill.Match(abilityRequest.want)) {
            if (CheckOneSkillPermission(skill, tokenId)) {
                return AppExecFwk::Constants::PERMISSION_GRANTED;
            } else {
                result = AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
            }
        }
    }
    return result;
}

int AbilityPermissionUtil::CheckStaticCfgPermission(const AbilityRequest &abilityRequest,
    bool isStartAsCaller, uint32_t callerTokenId, bool isData, bool isSaCall, bool isImplicit)
{
    auto abilityInfo = abilityRequest.abilityInfo;
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!isData) {
        isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    }
    if (isSaCall) {
        // do not need check static config permission when start ability by SA
        return AppExecFwk::Constants::PERMISSION_GRANTED;
    }

    uint32_t tokenId;
    if (isStartAsCaller) {
        tokenId = callerTokenId;
    } else {
        tokenId = IPCSkeleton::GetCallingTokenID();
    }

    if (abilityInfo.applicationInfo.accessTokenId == tokenId) {
        return ERR_OK;
    }

    if ((abilityInfo.type == AppExecFwk::AbilityType::EXTENSION &&
        abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::DATASHARE) ||
        (abilityInfo.type == AppExecFwk::AbilityType::DATA)) {
        // just need check the read permission and write permission of extension ability or data ability
        if (!abilityInfo.readPermission.empty()) {
            int checkReadPermission = AccessTokenKit::VerifyAccessToken(tokenId, abilityInfo.readPermission, false);
            if (checkReadPermission == ERR_OK) {
                return AppExecFwk::Constants::PERMISSION_GRANTED;
            }
            TAG_LOGW(AAFwkTag::ABILITYMGR,
                "verify access token fail, read permission: %{public}s", abilityInfo.readPermission.c_str());
        }
        if (!abilityInfo.writePermission.empty()) {
            int checkWritePermission = AccessTokenKit::VerifyAccessToken(tokenId, abilityInfo.writePermission, false);
            if (checkWritePermission == ERR_OK) {
                return AppExecFwk::Constants::PERMISSION_GRANTED;
            }
            TAG_LOGW(AAFwkTag::ABILITYMGR,
                "verify access token fail, write permission: %{public}s", abilityInfo.writePermission.c_str());
        }

        if (!abilityInfo.readPermission.empty() || !abilityInfo.writePermission.empty()) {
            // 'readPermission' and 'writePermission' take precedence over 'permission'
            // when 'readPermission' or 'writePermission' is not empty, no need check 'permission'
            return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
        }
    }

    if (!isImplicit) {
        return CheckStaticCfgPermissionForAbility(abilityInfo, tokenId);
    }
    return CheckStaticCfgPermissionForSkill(abilityRequest, tokenId);
}
} // AAFwk
} // OHOS