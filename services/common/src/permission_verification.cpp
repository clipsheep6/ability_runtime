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

#include "permission_verification.h"

#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "hilog_wrapper.h"
#include "permission_constants.h"
#include "support_system_ability_permission.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AAFwk {
const std::string DLP_PARAMS_INDEX = "ohos.dlp.params.index";
const std::string DLP_PARAMS_SECURITY_FLAG = "ohos.dlp.params.securityFlag";
namespace {
const int32_t BROKER_UID = 5557;
const std::set<std::string> OBSERVER_NATIVE_CALLER = {
    "memmgrservice",
    "resource_schedule_service",
};
}
bool PermissionVerification::VerifyPermissionByTokenId(const int &tokenId, const std::string &permissionName) const
{
    HILOG_DEBUG("VerifyPermissionByTokenId permission %{public}s", permissionName.c_str());
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, permissionName);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        HILOG_ERROR("permission %{public}s: PERMISSION_DENIED", permissionName.c_str());
        return false;
    }
    HILOG_DEBUG("verify AccessToken success");
    return true;
}

bool PermissionVerification::VerifyCallingPermission(const std::string &permissionName) const
{
    HILOG_DEBUG("VerifyCallingPermission permission %{public}s", permissionName.c_str());
    auto callerToken = GetCallingTokenID();
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        HILOG_ERROR("permission %{public}s: PERMISSION_DENIED", permissionName.c_str());
        return false;
    }
    HILOG_DEBUG("verify AccessToken success");
    return true;
}

bool PermissionVerification::IsSACall() const
{
    HILOG_DEBUG("%{public}s: is called.", __func__);
    auto callerToken = GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        HILOG_DEBUG("caller tokenType is native, verify success");
        return true;
    }
    HILOG_DEBUG("Not SA called.");
    return false;
}

bool PermissionVerification::IsShellCall() const
{
    HILOG_DEBUG("%{public}s: is called.", __func__);
    auto callerToken = GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL) {
        HILOG_DEBUG("caller tokenType is shell, verify success");
        return true;
    }
    HILOG_DEBUG("Not shell called.");
    return false;
}

bool PermissionVerification::CheckSpecificSystemAbilityAccessPermission(const std::string &processName) const
{
    HILOG_DEBUG("PermissionVerification::CheckSpecifidSystemAbilityAccessToken is called.");
    if (!IsSACall()) {
        HILOG_ERROR("caller tokenType is not native, verify failed.");
        return false;
    }
    auto callerToken = GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerToken, nativeTokenInfo);
    if (result != ERR_OK || nativeTokenInfo.processName != processName) {
        HILOG_ERROR("Check process name failed.");
        return false;
    }
    return true;
}

bool PermissionVerification::CheckObserverCallerPermission() const
{
    HILOG_DEBUG("called");
    if (!IsSACall()) {
        HILOG_ERROR("caller tokenType is not native");
        return false;
    }
    auto callerToken = GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerToken, nativeTokenInfo);
    if (result != ERR_OK ||
        OBSERVER_NATIVE_CALLER.find(nativeTokenInfo.processName) == OBSERVER_NATIVE_CALLER.end()) {
        HILOG_ERROR("Check native token failed.");
        return false;
    }
    return true;
}

bool PermissionVerification::VerifyRunningInfoPerm() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_GET_RUNNING_INFO)) {
        HILOG_DEBUG("%{public}s: Permission verification succeeded.", __func__);
        return true;
    }
    HILOG_ERROR("%{public}s: Permission verification failed.", __func__);
    return false;
}

bool PermissionVerification::VerifyControllerPerm() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_SET_ABILITY_CONTROLLER)) {
        HILOG_DEBUG("%{public}s: Permission verification succeeded.", __func__);
        return true;
    }
    HILOG_ERROR("%{public}s: Permission verification failed.", __func__);
    return false;
}

bool PermissionVerification::VerifyDlpPermission(Want &want) const
{
    if (want.GetIntParam(DLP_PARAMS_INDEX, 0) == 0) {
        want.RemoveParam(DLP_PARAMS_SECURITY_FLAG);
        return true;
    }

    if (VerifyCallingPermission(PermissionConstants::PERMISSION_ACCESS_DLP)) {
        return true;
    }
    HILOG_ERROR("%{public}s: Permission verification failed", __func__);
    return false;
}

int PermissionVerification::VerifyAccountPermission() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_INTERACT_ACROSS_LOCAL_ACCOUNTS)) {
        return ERR_OK;
    }
    HILOG_ERROR("%{public}s: Permission verification failed", __func__);
    return CHECK_PERMISSION_FAILED;
}

bool PermissionVerification::VerifyMissionPermission() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        HILOG_DEBUG("%{public}s: Permission verification succeeded.", __func__);
        return true;
    }
    HILOG_ERROR("%{public}s: Permission verification failed", __func__);
    return false;
}

int PermissionVerification::VerifyAppStateObserverPermission() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_RUNNING_STATE_OBSERVER)) {
        HILOG_DEBUG("Permission verification succeeded.");
        return ERR_OK;
    }
    HILOG_ERROR("Permission verification failed.");
    return ERR_PERMISSION_DENIED;
}

int32_t PermissionVerification::VerifyUpdateConfigurationPerm() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_UPDATE_CONFIGURATION)) {
        HILOG_INFO("Verify permission %{public}s succeed.", PermissionConstants::PERMISSION_UPDATE_CONFIGURATION);
        return ERR_OK;
    }
    HILOG_ERROR("Verify permission %{public}s failed.", PermissionConstants::PERMISSION_UPDATE_CONFIGURATION);
    return ERR_PERMISSION_DENIED;
}

bool PermissionVerification::VerifyInstallBundlePermission() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_INSTALL_BUNDLE)) {
        HILOG_INFO("Verify permission %{public}s succeed.", PermissionConstants::PERMISSION_INSTALL_BUNDLE);
        return true;
    }

    HILOG_ERROR("Verify permission %{public}s failed.", PermissionConstants::PERMISSION_INSTALL_BUNDLE);
    return false;
}

bool PermissionVerification::VerifyGetBundleInfoPrivilegedPermission() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_GET_BUNDLE_INFO_PRIVILEGED)) {
        HILOG_INFO("Verify permission %{public}s succeed.", PermissionConstants::PERMISSION_GET_BUNDLE_INFO_PRIVILEGED);
        return true;
    }

    HILOG_ERROR("Verify permission %{public}s failed.", PermissionConstants::PERMISSION_GET_BUNDLE_INFO_PRIVILEGED);
    return false;
}

bool PermissionVerification::VerifyStartRecentAbilityPermission() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_START_RECENT_ABILITY)) {
        HILOG_INFO("Verify permission %{public}s succeed.", PermissionConstants::PERMISSION_START_RECENT_ABILITY);
        return true;
    }
    return VerifyMissionPermission();
}

int PermissionVerification::CheckCallDataAbilityPermission(const VerificationInfo &verificationInfo, bool isShell) const
{
    if ((verificationInfo.apiTargetVersion > API8 || isShell) &&
        !JudgeStartAbilityFromBackground(verificationInfo.isBackgroundCall, verificationInfo.withContinuousTask)) {
        HILOG_ERROR("Application can not start DataAbility from background after API8.");
        return CHECK_PERMISSION_FAILED;
    }
    if (!JudgeStartInvisibleAbility(verificationInfo.accessTokenId, verificationInfo.visible)) {
        HILOG_ERROR("Target DataAbility is not visible, and caller does not have INVISIBLE permission.");
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }
    if (!JudgeAssociatedWakeUp(verificationInfo.accessTokenId, verificationInfo.associatedWakeUp)) {
        HILOG_ERROR("Target DataAbility's associatedWakeUp is false, reject start it from other application.");
        return CHECK_PERMISSION_FAILED;
    }

    return ERR_OK;
}

int PermissionVerification::CheckCallServiceAbilityPermission(const VerificationInfo &verificationInfo) const
{
    if ((verificationInfo.apiTargetVersion > API8 || IsShellCall()) &&
        !JudgeStartAbilityFromBackground(verificationInfo.isBackgroundCall, verificationInfo.withContinuousTask)) {
        HILOG_ERROR("Application can not start ServiceAbility from background after API8.");
        return CHECK_PERMISSION_FAILED;
    }
    if (!JudgeStartInvisibleAbility(verificationInfo.accessTokenId, verificationInfo.visible)) {
        HILOG_ERROR("Target ServiceAbility is not visible, and caller does not have INVISIBLE permission.");
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }
    if (!JudgeAssociatedWakeUp(verificationInfo.accessTokenId, verificationInfo.associatedWakeUp)) {
        HILOG_ERROR("Target ServiceAbility's associatedWakeUp is false, reject start it from other application.");
        return CHECK_PERMISSION_FAILED;
    }

    return ERR_OK;
}

int PermissionVerification::CheckCallAbilityPermission(const VerificationInfo &verificationInfo) const
{
    return JudgeInvisibleAndBackground(verificationInfo);
}

int PermissionVerification::CheckCallServiceExtensionPermission(const VerificationInfo &verificationInfo) const
{
    return JudgeInvisibleAndBackground(verificationInfo);
}

int PermissionVerification::CheckStartByCallPermission(const VerificationInfo &verificationInfo) const
{
    if (IsCallFromSameAccessToken(verificationInfo.accessTokenId)) {
        HILOG_ERROR("Not remote call, Caller is from same APP, StartAbilityByCall reject");
        return CHECK_PERMISSION_FAILED;
    }
    // Different APP call, check permissions
    if (!VerifyCallingPermission(PermissionConstants::PERMISSION_ABILITY_BACKGROUND_COMMUNICATION)) {
        HILOG_ERROR("PERMISSION_ABILITY_BACKGROUND_COMMUNICATION verification failed, StartAbilityByCall reject");
        return CHECK_PERMISSION_FAILED;
    }
    if (!JudgeStartInvisibleAbility(verificationInfo.accessTokenId, verificationInfo.visible)) {
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }
    if (!JudgeStartAbilityFromBackground(verificationInfo.isBackgroundCall, verificationInfo.withContinuousTask)) {
        return CHECK_PERMISSION_FAILED;
    }

    return ERR_OK;
}

unsigned int PermissionVerification::GetCallingTokenID() const
{
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    HILOG_DEBUG("callerToken : %{private}u", callerToken);
    return callerToken;
}

bool PermissionVerification::JudgeStartInvisibleAbility(const uint32_t accessTokenId, const bool visible) const
{
    if (visible) {
        HILOG_DEBUG("TargetAbility visible is true, PASS.");
        return true;
    }
    if (IsCallFromSameAccessToken(accessTokenId)) {
        HILOG_DEBUG("TargetAbility is in same APP, PASS.");
        return true;
    }
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_START_INVISIBLE_ABILITY)) {
        HILOG_DEBUG("Caller has PERMISSION_START_INVISIBLE_ABILITY, PASS.");
        return true;
    }
    HILOG_ERROR("PERMISSION_START_INVISIBLE_ABILITY verification failed.");
    return false;
}

bool PermissionVerification::JudgeStartAbilityFromBackground(
    const bool isBackgroundCall, bool withContinuousTask) const
{
    if (!isBackgroundCall) {
        HILOG_DEBUG("Caller is not background, PASS.");
        return true;
    }

    if (withContinuousTask) {
        HILOG_DEBUG("Caller has continuous task, PASS.");
        return true;
    }

    // Temporarily supports permissions with two different spellings
    // PERMISSION_START_ABILIIES_FROM_BACKGROUND will be removed later due to misspelling
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_START_ABILITIES_FROM_BACKGROUND) ||
        VerifyCallingPermission(PermissionConstants::PERMISSION_START_ABILIIES_FROM_BACKGROUND)) {
        HILOG_DEBUG("Caller has PERMISSION_START_ABILITIES_FROM_BACKGROUND, PASS.");
        return true;
    }
    HILOG_ERROR("PERMISSION_START_ABILITIES_FROM_BACKGROUND verification failed.");
    return false;
}

bool PermissionVerification::JudgeAssociatedWakeUp(const uint32_t accessTokenId, const bool associatedWakeUp) const
{
    if (IsCallFromSameAccessToken(accessTokenId)) {
        HILOG_DEBUG("TargetAbility is in same APP, PASS.");
        return true;
    }
    if (associatedWakeUp) {
        HILOG_DEBUG("TargetAbility is allowed associatedWakeUp, PASS.");
        return true;
    }
    HILOG_ERROR("The target is not allowed associatedWakeUp.");
    return false;
}

int PermissionVerification::JudgeInvisibleAndBackground(const VerificationInfo &verificationInfo) const
{
    if (IPCSkeleton::GetCallingUid() != BROKER_UID &&
        SupportSystemAbilityPermission::IsSupportSaCallPermission() && IsSACall()) {
        HILOG_DEBUG("Support SA call");
        return ERR_OK;
    }
    if (!JudgeStartInvisibleAbility(verificationInfo.accessTokenId, verificationInfo.visible)) {
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }
    if (!JudgeStartAbilityFromBackground(verificationInfo.isBackgroundCall, verificationInfo.withContinuousTask)) {
        return CHECK_PERMISSION_FAILED;
    }

    return ERR_OK;
}

bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI() const
{
    if (IsSACall() || IsShellCall()) {
        return true;
    }
    auto callerToken = IPCSkeleton::GetCallingFullTokenID();
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(callerToken);
}

bool PermissionVerification::IsSystemAppCall() const
{
    auto callerToken = IPCSkeleton::GetCallingFullTokenID();
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(callerToken);
}

bool PermissionVerification::VerifyPrepareTerminatePermission() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_PREPARE_TERMINATE)) {
        HILOG_DEBUG("%{public}s: Permission verification succeeded.", __func__);
        return true;
    }
    HILOG_DEBUG("%{public}s: Permission verification failed", __func__);
    return false;
}

bool PermissionVerification::VerifyPrepareTerminatePermission(const int &tokenId) const
{
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId,
        PermissionConstants::PERMISSION_PREPARE_TERMINATE);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        HILOG_DEBUG("permission denied.");
        return false;
    }
    HILOG_DEBUG("verify AccessToken success");
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
