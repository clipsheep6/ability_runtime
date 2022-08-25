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
#include "ipc_skeleton.h"
#include "permission_constants.h"

namespace OHOS {
namespace AAFwk {
const std::string DLP_PARAMS_INDEX = "ohos.dlp.params.index";
const std::string DLP_PARAMS_SECURITY_FLAG = "ohos.dlp.params.securityFlag";
const std::string DMS_PROCESS_NAME = "distributedsched";
bool PermissionVerification::VerifyCallingPermission(const std::string &permissionName)
{
    HILOG_DEBUG("VerifyCallingPermission permission %{public}s", permissionName.c_str());
    auto callerToken = GetCallingTokenID();
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (ret == Security::AccessToken::PermissionState::PERMISSION_DENIED) {
        HILOG_ERROR("permission %{public}s: PERMISSION_DENIED", permissionName.c_str());
        return false;
    }
    HILOG_DEBUG("verify AccessToken success");
    return true;
}

bool PermissionVerification::IsSACall()
{
    HILOG_DEBUG("AmsMgrScheduler::IsSACall is called.");
    auto callerToken = GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        HILOG_DEBUG("caller tokenType is native, verify success");
        return true;
    }
    HILOG_DEBUG("Not SA called.");
    return false;
}

bool PermissionVerification::CheckSpecificSystemAbilityAccessPermission()
{
    HILOG_DEBUG("PermissionVerification::CheckSpecifidSystemAbilityAccessToken is called.");
    if (!IsSACall()) {
        HILOG_ERROR("caller tokenType is not native, verify failed.");
        return false;
    }
    auto callerToken = GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerToken, nativeTokenInfo);
    if (result != ERR_OK || nativeTokenInfo.processName != DMS_PROCESS_NAME) {
        HILOG_ERROR("Check process name failed.");
        return false;
    }
    return true;
}

bool PermissionVerification::VerifyRunningInfoPerm()
{
    if (IsSACall()) {
        HILOG_DEBUG("%{public}s: the interface called by SA.", __func__);
        return true;
    }
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_GET_RUNNING_INFO)) {
        HILOG_DEBUG("%{public}s: Permission verification succeeded.", __func__);
        return true;
    }
    HILOG_ERROR("%{public}s: Permission verification failed.", __func__);
    return false;
}

bool PermissionVerification::VerifyControllerPerm()
{
    if (IsSACall()) {
        HILOG_DEBUG("%{public}s: the interface called by SA.", __func__);
        return true;
    }
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_SET_ABILITY_CONTROLLER)) {
        HILOG_DEBUG("%{public}s: Permission verification succeeded.", __func__);
        return true;
    }
    HILOG_ERROR("%{public}s: Permission verification failed.", __func__);
    return false;
}

bool PermissionVerification::VerifyDlpPermission(Want &want)
{
    if (want.GetIntParam(DLP_PARAMS_INDEX, 0) == 0) {
        want.RemoveParam(DLP_PARAMS_SECURITY_FLAG);
        return true;
    }

    if (IsSACall()) {
        return true;
    }
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_ACCESS_DLP)) {
        return true;
    }
    HILOG_ERROR("%{public}s: Permission verification failed", __func__);
    return false;
}

int PermissionVerification::VerifyAccountPermission()
{
    if (IsSACall()) {
        return ERR_OK;
    }
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_INTERACT_ACROSS_LOCAL_ACCOUNTS)) {
        return ERR_OK;
    }
    HILOG_ERROR("%{public}s: Permission verification failed", __func__);
    return CHECK_PERMISSION_FAILED;
}

bool PermissionVerification::VerifyMissionPermission()
{
    if (IsSACall()) {
        return true;
    }
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        HILOG_DEBUG("%{public}s: Permission verification succeeded.", __func__);
        return true;
    }
    HILOG_ERROR("%{public}s: Permission verification failed", __func__);
    return false;
}

int PermissionVerification::VerifyAppStateObserverPermission()
{
    if (IsSACall()) {
        return ERR_OK;
    }
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_RUNNING_STATE_OBSERVER)) {
        HILOG_INFO("Permission verification succeeded.");
        return ERR_OK;
    }
    HILOG_ERROR("Permission verification failed.");
    return ERR_PERMISSION_DENIED;
}

int32_t PermissionVerification::VerifyUpdateConfigurationPerm()
{
    if (IsSACall() || VerifyCallingPermission(PermissionConstants::PERMISSION_UPDATE_CONFIGURATION)) {
        HILOG_INFO("Verify permission %{public}s succeed.", PermissionConstants::PERMISSION_UPDATE_CONFIGURATION);
        return ERR_OK;
    }

    HILOG_ERROR("Verify permission %{public}s failed.", PermissionConstants::PERMISSION_UPDATE_CONFIGURATION);
    return ERR_PERMISSION_DENIED;
}

unsigned int PermissionVerification::GetCallingTokenID()
{
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    HILOG_DEBUG("callerToken : %{private}u", callerToken);
    return callerToken;
}
}  // namespace AAFwk
}  // namespace OHOS
