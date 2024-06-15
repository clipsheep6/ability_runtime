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
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "permission_constants.h"
#include "server_constant.h"
#include "support_system_ability_permission.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AAFwk {
const std::string DLP_PARAMS_SECURITY_FLAG = "ohos.dlp.params.securityFlag";
namespace {
const int32_t SHELL_START_EXTENSION_FLOOR = 0; // FORM
const int32_t SHELL_START_EXTENSION_CEIL = 21; // EMBEDDED_UI
const int32_t BROKER_UID = 5557;
const std::string FOUNDATION_PROCESS_NAME = "foundation";
const std::set<std::string> OBSERVER_NATIVE_CALLER = {
    "memmgrservice",
    "resource_schedule_service",
};
}
bool PermissionVerification::VerifyPermissionByTokenId(const int &tokenId, const std::string &permissionName) const
{
    TAG_LOGD(AAFwkTag::DEFAULT, "VerifyPermissionByTokenId permission %{public}s", permissionName.c_str());
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, permissionName, false);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::DEFAULT, "permission %{public}s: PERMISSION_DENIED", permissionName.c_str());
        return false;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "verify AccessToken success");
    return true;
}

bool PermissionVerification::VerifyCallingPermission(
    const std::string &permissionName, const uint32_t specifyTokenId) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::DEFAULT, "VerifyCallingPermission permission %{public}s, specifyTokenId is %{public}u",
        permissionName.c_str(), specifyTokenId);
    auto callerToken = specifyTokenId == 0 ? GetCallingTokenID() : specifyTokenId;
    TAG_LOGD(AAFwkTag::DEFAULT, "callerToken is %{public}u", callerToken);
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName, false);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::DEFAULT, "permission %{public}s: PERMISSION_DENIED", permissionName.c_str());
        return false;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "verify AccessToken success");
    return true;
}

bool PermissionVerification::IsSACall() const
{
    TAG_LOGD(AAFwkTag::DEFAULT, "%{public}s: is called.", __func__);
    auto callerToken = GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        TAG_LOGD(AAFwkTag::DEFAULT, "caller tokenType is native, verify success");
        return true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "Not SA called.");
    return false;
}

bool PermissionVerification::IsShellCall() const
{
    TAG_LOGD(AAFwkTag::DEFAULT, "%{public}s: is called.", __func__);
    auto callerToken = GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL) {
        TAG_LOGD(AAFwkTag::DEFAULT, "caller tokenType is shell, verify success");
        return true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "Not shell called.");
    return false;
}

bool PermissionVerification::CheckSpecificSystemAbilityAccessPermission(const std::string &processName) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::DEFAULT, "PermissionVerification::CheckSpecifidSystemAbilityAccessToken is called.");
    if (!IsSACall()) {
        TAG_LOGE(AAFwkTag::DEFAULT, "caller tokenType is not native, verify failed.");
        return false;
    }
    auto callerToken = GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerToken, nativeTokenInfo);
    if (result != ERR_OK || nativeTokenInfo.processName != processName) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Check process name failed.");
        return false;
    }
    return true;
}

bool PermissionVerification::CheckObserverCallerPermission() const
{
    TAG_LOGD(AAFwkTag::DEFAULT, "called");
    if (!IsSACall()) {
        TAG_LOGE(AAFwkTag::DEFAULT, "caller tokenType is not native");
        return false;
    }
    auto callerToken = GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerToken, nativeTokenInfo);
    if (result != ERR_OK ||
        OBSERVER_NATIVE_CALLER.find(nativeTokenInfo.processName) == OBSERVER_NATIVE_CALLER.end()) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Check native token failed.");
        return false;
    }
    return true;
}

bool PermissionVerification::VerifyRunningInfoPerm() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_GET_RUNNING_INFO)) {
        TAG_LOGD(AAFwkTag::DEFAULT, "%{public}s: Permission verification succeeded.", __func__);
        return true;
    }
    TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s: Permission verification failed.", __func__);
    return false;
}

bool PermissionVerification::VerifyControllerPerm() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_SET_ABILITY_CONTROLLER)) {
        TAG_LOGD(AAFwkTag::DEFAULT, "%{public}s: Permission verification succeeded.", __func__);
        return true;
    }
    TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s: Permission verification failed.", __func__);
    return false;
}

bool PermissionVerification::VerifyDlpPermission(Want &want) const
{
    if (want.GetIntParam(AbilityRuntime::ServerConstant::DLP_INDEX, 0) == 0) {
        want.RemoveParam(DLP_PARAMS_SECURITY_FLAG);
        return true;
    }

    if (VerifyCallingPermission(PermissionConstants::PERMISSION_ACCESS_DLP)) {
        return true;
    }
    TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s: Permission verification failed", __func__);
    return false;
}

int PermissionVerification::VerifyAccountPermission() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_INTERACT_ACROSS_LOCAL_ACCOUNTS)) {
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s: Permission verification failed", __func__);
    return CHECK_PERMISSION_FAILED;
}

bool PermissionVerification::VerifyMissionPermission() const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TAG_LOGD(AAFwkTag::DEFAULT, "%{public}s: Permission verification succeeded.", __func__);
        return true;
    }
    TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s: Permission verification failed", __func__);
    return false;
}

int PermissionVerification::VerifyAppStateObserverPermission() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_RUNNING_STATE_OBSERVER)) {
        TAG_LOGD(AAFwkTag::DEFAULT, "Permission verification succeeded.");
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::DEFAULT, "Permission verification failed.");
    return ERR_PERMISSION_DENIED;
}

int32_t PermissionVerification::VerifyUpdateConfigurationPerm() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_UPDATE_CONFIGURATION)) {
        TAG_LOGI(AAFwkTag::DEFAULT,
            "Verify permission %{public}s succeed.", PermissionConstants::PERMISSION_UPDATE_CONFIGURATION);
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::DEFAULT,
        "Verify permission %{public}s failed.", PermissionConstants::PERMISSION_UPDATE_CONFIGURATION);
    return ERR_PERMISSION_DENIED;
}

int32_t PermissionVerification::VerifyUpdateAPPConfigurationPerm() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_UPDATE_APP_CONFIGURATION)) {
        HILOG_INFO("Verify permission %{public}s succeed.", PermissionConstants::PERMISSION_UPDATE_APP_CONFIGURATION);
        return ERR_OK;
    }
    HILOG_ERROR("Verify permission %{public}s failed.", PermissionConstants::PERMISSION_UPDATE_APP_CONFIGURATION);
    return ERR_PERMISSION_DENIED;
}

bool PermissionVerification::VerifyInstallBundlePermission() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_INSTALL_BUNDLE)) {
        TAG_LOGI(AAFwkTag::DEFAULT,
            "Verify permission %{public}s succeed.", PermissionConstants::PERMISSION_INSTALL_BUNDLE);
        return true;
    }

    TAG_LOGE(AAFwkTag::DEFAULT, "Verify permission %{public}s failed.", PermissionConstants::PERMISSION_INSTALL_BUNDLE);
    return false;
}

bool PermissionVerification::VerifyGetBundleInfoPrivilegedPermission() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_GET_BUNDLE_INFO_PRIVILEGED)) {
        TAG_LOGI(AAFwkTag::DEFAULT,
            "Verify permission %{public}s succeed.", PermissionConstants::PERMISSION_GET_BUNDLE_INFO_PRIVILEGED);
        return true;
    }

    TAG_LOGE(AAFwkTag::DEFAULT,
        "Verify permission %{public}s failed.", PermissionConstants::PERMISSION_GET_BUNDLE_INFO_PRIVILEGED);
    return false;
}

bool PermissionVerification::VerifyStartRecentAbilityPermission() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_START_RECENT_ABILITY)) {
        TAG_LOGI(AAFwkTag::DEFAULT,
            "Verify permission %{public}s succeed.", PermissionConstants::PERMISSION_START_RECENT_ABILITY);
        return true;
    }
    return VerifyMissionPermission();
}

int PermissionVerification::CheckCallDataAbilityPermission(const VerificationInfo &verificationInfo, bool isShell) const
{
    if ((verificationInfo.apiTargetVersion > API8 || isShell) &&
        !JudgeStartAbilityFromBackground(verificationInfo.isBackgroundCall, verificationInfo.withContinuousTask)) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Application can not start DataAbility from background after API8.");
        return CHECK_PERMISSION_FAILED;
    }
    if (!JudgeStartInvisibleAbility(verificationInfo.accessTokenId, verificationInfo.visible)) {
        TAG_LOGE(AAFwkTag::DEFAULT,
            "Target DataAbility is not visible, and caller does not have INVISIBLE permission.");
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }
    if (!JudgeAssociatedWakeUp(verificationInfo.accessTokenId, verificationInfo.associatedWakeUp)) {
        TAG_LOGE(AAFwkTag::DEFAULT,
            "Target DataAbility's associatedWakeUp is false, reject start it from other application.");
        return CHECK_PERMISSION_FAILED;
    }

    return ERR_OK;
}

int PermissionVerification::CheckCallServiceAbilityPermission(const VerificationInfo &verificationInfo) const
{
    if (CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS_NAME)) {
        TAG_LOGD(AAFwkTag::DEFAULT, "Allow fms to connect service ability.");
        return ERR_OK;
    }
    if ((verificationInfo.apiTargetVersion > API8 || IsShellCall()) &&
        !JudgeStartAbilityFromBackground(verificationInfo.isBackgroundCall, verificationInfo.withContinuousTask)) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Application can not start ServiceAbility from background after API8.");
        return CHECK_PERMISSION_FAILED;
    }
    if (!JudgeStartInvisibleAbility(verificationInfo.accessTokenId, verificationInfo.visible)) {
        TAG_LOGE(AAFwkTag::DEFAULT,
            "Target ServiceAbility is not visible, and caller does not have INVISIBLE permission.");
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }
    if (!JudgeAssociatedWakeUp(verificationInfo.accessTokenId, verificationInfo.associatedWakeUp)) {
        TAG_LOGE(AAFwkTag::DEFAULT,
            "Target ServiceAbility's associatedWakeUp is false, reject start it from other application.");
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
        TAG_LOGE(AAFwkTag::DEFAULT, "Not remote call, Caller is from same APP, StartAbilityByCall reject");
        return CHECK_PERMISSION_FAILED;
    }
    // Different APP call, check permissions
    if (!VerifyCallingPermission(PermissionConstants::PERMISSION_ABILITY_BACKGROUND_COMMUNICATION)) {
        TAG_LOGE(AAFwkTag::DEFAULT,
            "PERMISSION_ABILITY_BACKGROUND_COMMUNICATION verification failed, StartAbilityByCall reject");
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
    TAG_LOGD(AAFwkTag::DEFAULT, "callerToken : %{private}u", callerToken);
    return callerToken;
}

bool PermissionVerification::JudgeStartInvisibleAbility(const uint32_t accessTokenId, const bool visible,
    const uint32_t specifyTokenId) const
{
    if (visible) {
        TAG_LOGD(AAFwkTag::DEFAULT, "TargetAbility visible is true, PASS.");
        return true;
    }
    if (specifyTokenId > 0 && accessTokenId == specifyTokenId) {
        TAG_LOGD(AAFwkTag::DEFAULT, "AccessTokenId is the same as specifyTokenId, targetAbility is in same APP, PASS.");
        return true;
    }
    if (IsCallFromSameAccessToken(accessTokenId)) {
        TAG_LOGD(AAFwkTag::DEFAULT, "TargetAbility is in same APP, PASS.");
        return true;
    }
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_START_INVISIBLE_ABILITY, specifyTokenId)) {
        TAG_LOGD(AAFwkTag::DEFAULT, "Caller has PERMISSION_START_INVISIBLE_ABILITY, PASS.");
        return true;
    }
    TAG_LOGE(AAFwkTag::DEFAULT, "PERMISSION_START_INVISIBLE_ABILITY verification failed.");
    return false;
}

bool PermissionVerification::JudgeStartAbilityFromBackground(
    const bool isBackgroundCall, bool withContinuousTask) const
{
    if (!isBackgroundCall) {
        TAG_LOGD(AAFwkTag::DEFAULT, "Caller is not background, PASS.");
        return true;
    }

    if (withContinuousTask) {
        TAG_LOGD(AAFwkTag::DEFAULT, "Caller has continuous task, PASS.");
        return true;
    }

    // Temporarily supports permissions with two different spellings
    // PERMISSION_START_ABILIIES_FROM_BACKGROUND will be removed later due to misspelling
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_START_ABILITIES_FROM_BACKGROUND) ||
        VerifyCallingPermission(PermissionConstants::PERMISSION_START_ABILIIES_FROM_BACKGROUND)) {
        TAG_LOGD(AAFwkTag::DEFAULT, "Caller has PERMISSION_START_ABILITIES_FROM_BACKGROUND, PASS.");
        return true;
    }
    TAG_LOGE(AAFwkTag::DEFAULT, "PERMISSION_START_ABILITIES_FROM_BACKGROUND verification failed.");
    return false;
}

bool PermissionVerification::JudgeAssociatedWakeUp(const uint32_t accessTokenId, const bool associatedWakeUp) const
{
    if (IsCallFromSameAccessToken(accessTokenId)) {
        TAG_LOGD(AAFwkTag::DEFAULT, "TargetAbility is in same APP, PASS.");
        return true;
    }
    if (associatedWakeUp) {
        TAG_LOGD(AAFwkTag::DEFAULT, "TargetAbility is allowed associatedWakeUp, PASS.");
        return true;
    }
    TAG_LOGE(AAFwkTag::DEFAULT, "The target is not allowed associatedWakeUp.");
    return false;
}

int PermissionVerification::JudgeInvisibleAndBackground(const VerificationInfo &verificationInfo) const
{
    uint32_t specifyTokenId = verificationInfo.specifyTokenId;
    TAG_LOGI(AAFwkTag::DEFAULT, "specifyTokenId = %{public}u", specifyTokenId);
    if (specifyTokenId == 0 && IPCSkeleton::GetCallingUid() != BROKER_UID &&
        SupportSystemAbilityPermission::IsSupportSaCallPermission() && IsSACall()) {
        TAG_LOGD(AAFwkTag::DEFAULT, "Support SA call");
        return ERR_OK;
    }
    if (!JudgeStartInvisibleAbility(verificationInfo.accessTokenId, verificationInfo.visible,
        specifyTokenId)) {
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
        TAG_LOGD(AAFwkTag::DEFAULT, "%{public}s: Permission verification succeeded.", __func__);
        return true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "%{public}s: Permission verification failed", __func__);
    return false;
}

bool PermissionVerification::VerifyPrepareTerminatePermission(const int &tokenId) const
{
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId,
        PermissionConstants::PERMISSION_PREPARE_TERMINATE, false);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        TAG_LOGD(AAFwkTag::DEFAULT, "permission denied.");
        return false;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "verify AccessToken success");
    return true;
}

bool PermissionVerification::VerifyShellStartExtensionType(int32_t type) const
{
    if (IsShellCall() && type >= SHELL_START_EXTENSION_FLOOR && type <= SHELL_START_EXTENSION_CEIL) {
        return true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "VerifyShellStartExtensionType, reject start.");
    return false;
}

bool PermissionVerification::VerifyPreloadApplicationPermission() const
{
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_PRELOAD_APPLICATION)) {
        HILOG_DEBUG("Verify permission %{public}s succeed.", PermissionConstants::PERMISSION_PRELOAD_APPLICATION);
        return true;
    }
    HILOG_ERROR("Verify permission %{public}s failed.", PermissionConstants::PERMISSION_PRELOAD_APPLICATION);
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS
