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

#include "mock_permission_verification.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* PERMISSION_FILE_ACCESS_MANAGER = "ohos.permission.FILE_ACCESS_MANAGER";
constexpr const char* PERMISSION_WRITE_IMAGEVIDEO = "ohos.permission.WRITE_IMAGEVIDEO";
constexpr const char* PERMISSION_READ_IMAGEVIDEO = "ohos.permission.READ_IMAGEVIDEO";
constexpr const char* PERMISSION_WRITE_AUDIO = "ohos.permission.WRITE_AUDIO";
constexpr const char* PERMISSION_READ_AUDIO = "ohos.permission.READ_AUDIO";
constexpr const char* PERMISSION_PROXY_AUTHORIZATION_URI = "ohos.permission.PROXY_AUTHORIZATION_URI";
constexpr const char* PERMISSION_GRANT_URI_PERMISSION_PRIVILEGED = "ohos.permission.GRANT_URI_PERMISSION_PRIVILEGED";
} // namespace

bool PermissionVerification::VerifyPermissionByTokenId(const int &tokenId, const std::string &permissionName) const
{
    if (MyFlag::permissionAll_) {
        return true;
    }
    if (permissionName == PERMISSION_FILE_ACCESS_MANAGER) {
        return MyFlag::permissionFileAccessManager_;
    }
    if (permissionName == PERMISSION_WRITE_IMAGEVIDEO) {
        return MyFlag::permissionWriteImageVideo_;
    }
    if (permissionName == PERMISSION_READ_IMAGEVIDEO) {
        return MyFlag::permissionReadImageVideo_;
    }
    if (permissionName == PERMISSION_WRITE_AUDIO) {
        return MyFlag::permissionWriteAudio_;
    }
    if (permissionName == PERMISSION_READ_AUDIO) {
        return MyFlag::permissionReadAudio_;
    }
    if (permissionName == PERMISSION_PROXY_AUTHORIZATION_URI) {
        return MyFlag::permissionProxyAuthorization_;
    }
    if (permissionName == PERMISSION_GRANT_URI_PERMISSION_PRIVILEGED) {
        return MyFlag::permissionPrivileged_;
    }
    return false;
}
bool PermissionVerification::VerifyCallingPermission(const std::string &permissionName) const
{
    return !!(MyFlag::flag_);
}
bool PermissionVerification::IsSACall() const
{
    return (MyFlag::flag_ & MyFlag::FLAG::IS_SA_CALL);
}
bool PermissionVerification::IsShellCall() const
{
    return (MyFlag::flag_ & MyFlag::FLAG::IS_SHELL_CALL);
}
bool PermissionVerification::CheckSpecificSystemAbilityAccessPermission() const
{
    return !!(MyFlag::flag_);
}
bool PermissionVerification::VerifyRunningInfoPerm() const
{
    return !!(MyFlag::flag_);
}
bool PermissionVerification::VerifyControllerPerm() const
{
    return !!(MyFlag::flag_);
}
bool PermissionVerification::VerifyDlpPermission(Want &want) const
{
    return !!(MyFlag::flag_);
}
int PermissionVerification::VerifyAccountPermission() const
{
    return MyFlag::flag_;
}
bool PermissionVerification::VerifyMissionPermission() const
{
    return !!(MyFlag::flag_);
}
int PermissionVerification::VerifyAppStateObserverPermission() const
{
    return MyFlag::flag_;
}
int32_t PermissionVerification::VerifyUpdateConfigurationPerm() const
{
    return static_cast<int32_t>(MyFlag::flag_);
}
bool PermissionVerification::VerifyInstallBundlePermission() const
{
    return !!(MyFlag::flag_);
}
bool PermissionVerification::VerifyGetBundleInfoPrivilegedPermission() const
{
    return !!(MyFlag::flag_);
}
int PermissionVerification::CheckCallDataAbilityPermission(const VerificationInfo &verificationInfo, bool isShell) const
{
    return MyFlag::flag_;
}
int PermissionVerification::CheckCallServiceAbilityPermission(const VerificationInfo &verificationInfo) const
{
    return MyFlag::flag_;
}
int PermissionVerification::CheckCallAbilityPermission(const VerificationInfo &verificationInfo) const
{
    return MyFlag::flag_;
}
int PermissionVerification::CheckCallServiceExtensionPermission(const VerificationInfo &verificationInfo) const
{
    return MyFlag::flag_;
}
int PermissionVerification::CheckStartByCallPermission(const VerificationInfo &verificationInfo) const
{
    return MyFlag::flag_;
}
unsigned int PermissionVerification::GetCallingTokenID() const
{
    return static_cast<unsigned int>(MyFlag::flag_);
}
bool PermissionVerification::JudgeStartInvisibleAbility(const uint32_t accessTokenId, const bool visible) const
{
    return !!(MyFlag::flag_);
}
bool PermissionVerification::JudgeStartAbilityFromBackground(const bool isBackgroundCall) const
{
    return !!(MyFlag::flag_);
}
bool PermissionVerification::JudgeAssociatedWakeUp(const uint32_t accessTokenId, const bool associatedWakeUp) const
{
    return !!(MyFlag::flag_);
}
int PermissionVerification::JudgeInvisibleAndBackground(const VerificationInfo &verificationInfo) const
{
    return MyFlag::flag_;
}
bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI() const
{
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS