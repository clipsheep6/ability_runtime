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

#ifndef OHOS_ABILITY_RUNTIME_DLP_UTILS_H
#define OHOS_ABILITY_RUNTIME_DLP_UTILS_H

#include "ability_record.h"
#ifdef WITH_DLP
#include "dlp_permission_kit.h"
#endif // WITH_DLP
#include "iremote_object.h"
#include "permission_verification.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace DlpUtils {
#ifdef WITH_DLP
using Dlp = Security::DlpPermission::DlpPermissionKit;
#endif // WITH_DLP
static bool DlpAccessOtherAppsCheck(const sptr<IRemoteObject> &callerToken, const Want &want)
{
#ifdef WITH_DLP
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return true;
    }
    if (callerToken == nullptr) {
        return true;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("Ability has already been destroyed.");
        return true;
    }
    if (abilityRecord->GetAppIndex() == 0) {
        return true;
    }
    if (abilityRecord->GetApplicationInfo().bundleName == want.GetElement().GetBundleName()) {
        return true;
    }
    int32_t uid = abilityRecord->GetApplicationInfo().uid;
    Security::DlpPermission::SandBoxExternalAuthorType authResult;
    int result = Dlp::GetSandboxExternalAuthorization(uid, want, authResult);
    if (result != ERR_OK) {
        HILOG_ERROR("GetSandboxExternalAuthorization failed %{public}d.", result);
        return false;
    }
    if (authResult != Security::DlpPermission::SandBoxExternalAuthorType::ALLOW_START_ABILITY) {
        HILOG_ERROR("Auth failed, not allow start %{public}d.", uid);
        return false;
    }
#endif // WITH_DLP
    return true;
}

static bool OtherAppsAccessDlpCheck(const sptr<IRemoteObject> &callerToken, const Want &want)
{
    if (callerToken != nullptr) {
        auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
        if (abilityRecord != nullptr && abilityRecord->GetAppIndex() != 0) {
            return true;
        }
    }

    return PermissionVerification::GetInstance()->VerifyDlpPermission(const_cast<Want &>(want));
}
}  // namespace DlpUtils
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DLP_UTILS_H
