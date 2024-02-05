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

#ifndef OHOS_ABILITY_RUNTIME_PERMISSION_VERIFICATION_H
#define OHOS_ABILITY_RUNTIME_PERMISSION_VERIFICATION_H

#include "ipc_skeleton.h"
#include "singleton.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class PermissionVerification : public DelayedSingleton<PermissionVerification> {
public:
struct VerificationInfo {
    bool visible = false;
    bool isBackgroundCall = true;
    bool associatedWakeUp = false;
    uint32_t accessTokenId = 0;
    int32_t apiTargetVersion = 0;
    bool withContinuousTask = false;
};

    PermissionVerification() = default;
    ~PermissionVerification() = default;

    bool VerifyPermissionByTokenId(const int &tokenId, const std::string &permissionName) const;

    bool VerifyCallingPermission(const std::string &permissionName) const;

    bool IsSACall() const;

    bool IsShellCall() const;

    bool CheckSpecificSystemAbilityAccessPermission(const std::string &processName) const;

    bool CheckObserverCallerPermission() const;

    bool VerifyRunningInfoPerm() const;

    bool VerifyControllerPerm() const;

    bool VerifyDlpPermission(Want &want) const;

    int VerifyAccountPermission() const;

    bool VerifyMissionPermission() const;

    int VerifyAppStateObserverPermission() const;

    int32_t VerifyUpdateConfigurationPerm() const;

    bool VerifyInstallBundlePermission() const;

    bool VerifyGetBundleInfoPrivilegedPermission() const;

    bool VerifyStartRecentAbilityPermission() const;

    int CheckCallDataAbilityPermission(const VerificationInfo &verificationInfo, bool isShell) const;

    int CheckCallServiceAbilityPermission(const VerificationInfo &verificationInfo) const;

    int CheckCallAbilityPermission(const VerificationInfo &verificationInfo) const;

    /**
     * Check if Caller is allowed to start ServiceExtension(Stage) or DataShareExtension(Stage)
     *
     * @param verificationInfo, verificationInfo.
     * @return Returns ERR_OK on check success, others on check failure.
     */
    int CheckCallServiceExtensionPermission(const VerificationInfo &verificationInfo) const;

    int CheckStartByCallPermission(const VerificationInfo &verificationInfo) const;

    bool JudgeCallerIsAllowedToUseSystemAPI() const;

    bool IsSystemAppCall() const;

    bool VerifyPrepareTerminatePermission() const;

    bool VerifyPrepareTerminatePermission(const int &tokenId) const;

private:
    DISALLOW_COPY_AND_MOVE(PermissionVerification);

    constexpr static int32_t API8 = 8;

    unsigned int GetCallingTokenID() const;

    bool JudgeStartInvisibleAbility(const uint32_t accessTokenId, const bool visible) const;

    bool JudgeStartAbilityFromBackground(const bool isBackgroundCall, bool withContinuousTask = false) const;

    bool JudgeAssociatedWakeUp(const uint32_t accessTokenId, const bool associatedWakeUp) const;

    int JudgeInvisibleAndBackground(const VerificationInfo &verificationInfo) const;

    inline bool IsCallFromSameAccessToken(const uint32_t accessTokenId) const
    {
        return IPCSkeleton::GetCallingTokenID() == accessTokenId;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PERMISSION_VERIFICATION_H
