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
    bool isBackgroundCall = false;
    bool associatedWakeUp = false;
    uint32_t accessTokenId = 0;
    int32_t apiTargetVersion = 0;
};

    PermissionVerification() = default;
    ~PermissionVerification() = default;

    bool VerifyCallingPermission(const std::string &permissionName);

    bool IsSACall();

    bool IsShellCall();

    bool CheckSpecificSystemAbilityAccessPermission();

    bool VerifyRunningInfoPerm();

    bool VerifyControllerPerm();

    bool VerifyDlpPermission(Want &want);

    int VerifyAccountPermission();

    bool VerifyMissionPermission();

    int VerifyAppStateObserverPermission();

    int32_t VerifyUpdateConfigurationPerm();

    bool VerifyInstallBundlePermission();

    bool VerifyGetBundleInfoPrivilegedPermission();

    int CheckCallDataAbilityPermission(const VerificationInfo &verificationInfo);

    int CheckCallServiceAbilityPermission(const VerificationInfo &verificationInfo);

    int CheckCallAbilityPermission(const VerificationInfo &verificationInfo);

    /**
     * Check if Caller is allowed to start ServiceExtension(Stage) or DataShareExtension(Stage)
     *
     * @param verificationInfo, verificationInfo.
     * @return Returns ERR_OK on check success, others on check failure.
     */
    int CheckCallServiceExtensionPermission(const VerificationInfo &verificationInfo);

    int CheckCallOtherExtensionPermission(const VerificationInfo &verificationInfo);

    int CheckStartByCallPermission(const VerificationInfo &verificationInfo);

private:
    DISALLOW_COPY_AND_MOVE(PermissionVerification);

    constexpr static int32_t API8 = 8;

    unsigned int GetCallingTokenID();

    bool JudgeStartInvisibleAbility(const uint32_t accessTokenId, const bool visible);

    bool JudgeStartAbilityFromBackground(const bool isBackgroundCall);

    bool JudgeAssociatedWakeUp(const uint32_t accessTokenId, const bool associatedWakeUp);

    int JudgeInvisibleAndBackground(const VerificationInfo &verificationInfo);

    inline bool IsCallFromSameAccessToken(const uint32_t accessTokenId)
    {
        return IPCSkeleton::GetCallingTokenID() == accessTokenId;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PERMISSION_VERIFICATION_H
