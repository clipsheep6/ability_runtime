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

#include "start_ability_handler.h"

#include "ability_manager_service.h"
#include "dlp_utils.h"

namespace OHOS {
namespace AAFwk {
namespace {
#ifdef WITH_DLP
using Dlp = Security::DlpPermission::DlpPermissionKit;
#endif // WITH_DLP
bool SandboxAuthCheck(const AbilityRecord &callerRecord, const Want &want)
{
#ifdef WITH_DLP
    int32_t uid = callerRecord.GetApplicationInfo().uid;
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
}

int StartAbilityParams::VerifyAccountPermission()
{
    if (accountPermissionVerify.has_value()) {
        return accountPermissionVerify.value();
    }
    accountPermissionVerify = abilityMs->VerifyAccountPermission(userId);
    return accountPermissionVerify.value();
}

int32_t StartAbilityParams::GetValidUserId()
{
    if (validUserId.has_value()) {
        return validUserId.value();
    }
    validUserId = abilityMs->GetValidUserId(userId);
    return validUserId.value();
}

bool StartAbilityParams::IsCallerSandboxApp()
{
    return GetCallerAppIndex() > 0;
}

bool StartAbilityParams::OtherAppsAccessDlp()
{
    if (otherAppsAccessDlp.has_value()) {
        return otherAppsAccessDlp.value();
    }
    otherAppsAccessDlp = DlpUtils::OtherAppsAccessDlpCheck(callerToken, want);
    return otherAppsAccessDlp.value();
}

bool StartAbilityParams::DlpAccessOtherApps()
{
    if (dlpAccessOtherApps.has_value()) {
        return dlpAccessOtherApps.value();
    }
    dlpAccessOtherApps = DlpUtils::DlpAccessOtherAppsCheck(callerToken, want);
    return dlpAccessOtherApps.value();
}

bool StartAbilityParams::SandboxExternalAuth()
{
    if (sandboxExternalAuth.has_value()) {
        return sandboxExternalAuth.value();
    }
    auto record = GetCallerRecord();
    if (!record) {
        sandboxExternalAuth = false;
        return false;
    }
    sandboxExternalAuth = SandboxAuthCheck(*record, want);
    return sandboxExternalAuth.value();
}

bool StartAbilityParams::IsCallerSysApp()
{
    if (isCallerSysApp.has_value()) {
        return isCallerSysApp.value();
    }
    isCallerSysApp = PermissionVerification::GetInstance()->IsSystemAppCall();
    return isCallerSysApp.value();
}

bool StartAbilityParams::OperateRemote()
{
    if (operateRemote.has_value()) {
        return operateRemote.value();
    }
    operateRemote = abilityMs->CheckIfOperateRemote(want);
    return operateRemote.value();
}

std::shared_ptr<AbilityRecord> StartAbilityParams::GetCallerRecord()
{
    if (callerRecord.has_value()) {
        return callerRecord.value();
    }

    if (callerToken) {
        callerRecord = Token::GetAbilityRecordByToken(callerToken);
    } else {
        callerRecord = nullptr;
    }
    return callerRecord.value();
}

int32_t StartAbilityParams::GetCallerAppIndex()
{
    if (callerAppIndex.has_value()) {
        return callerAppIndex.value();
    }
    auto record = GetCallerRecord();
    callerAppIndex = record ? record->GetAppIndex() : 0;
    return callerAppIndex.value();
}

EventInfo StartAbilityParams::BuildEventInfo()
{
    EventInfo eventInfo;
    eventInfo.userId = userId;
    eventInfo.bundleName = want.GetElement().GetBundleName();
    eventInfo.moduleName = want.GetElement().GetModuleName();
    eventInfo.abilityName = want.GetElement().GetAbilityName();
    return eventInfo;
}
} // namespace AAFwk
} // namespace OHOS