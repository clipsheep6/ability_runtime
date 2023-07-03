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

#include "start_ability_sandbox_savefile.h"
#include <climits>
#include "hilog_wrapper.h"
#include "ability_manager_errors.h"
#include "ability_util.h"
#include "ability_manager_service.h"


namespace OHOS {
namespace AAFwk {
namespace {
const std::string DLP_BUNDLE_NAME = "com.ohos.dlpmanager";
const std::string DLP_ABILITY_NAME = "SaveAsAbility";
}
const std::string StartAbilitySandboxSavefile::handlerName_ = "start_ability_snadbox_savefile";

bool StartAbilitySandboxSavefile::MatchStartRequest(StartAbilityParams &params)
{
    if (params.IsCallerSandboxApp() && params.want.GetAction() == "ohos.want.action.CREATE_FILE" &&
        params.want.GetStringParam("startMode") == "save") {
        return true;
    }

    auto element = params.want.GetElement();
    if (element.GetBundleName() == DLP_BUNDLE_NAME && element.GetAbilityName() == DLP_ABILITY_NAME &&
        !ContainRecord(params.requestCode)) {
        return true;
    }
    return false;
}

int StartAbilitySandboxSavefile::HandleStartRequest(StartAbilityParams &params)
{
    HILOG_DEBUG("called");
    auto callerRecord = params.GetCallerRecord();
    if (!callerRecord) {
        HILOG_ERROR("this shouldn't happen: caller is null");
        return ERR_INVALID_CALLER;
    }

    if (!params.SandboxExternalAuth()) {
        HILOG_WARN("sandbox external auth failed");
        return CHECK_PERMISSION_FAILED;
    }

    auto reqCode = PushRecord(params.requestCode, callerRecord);
    auto &want = params.want;
    want.SetElementName(DLP_BUNDLE_NAME, DLP_ABILITY_NAME);
    want.SetParam("requestCode", reqCode);
    want.SetParam("startMode", std::string("save_redirect"));

    auto abilityMs = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (params.startOptions) {
        return abilityMs->StartAbilityForOptionInner(want, *params.startOptions, params.callerToken, params.userId,
            reqCode, params.isStartAsCaller);
    }
    return abilityMs->StartAbilityInner(want, params.callerToken, reqCode, params.userId,
        params.isStartAsCaller);
}

std::string StartAbilitySandboxSavefile::GetHandlerName()
{
    return StartAbilitySandboxSavefile::handlerName_;
}

int StartAbilitySandboxSavefile::PushRecord(int reqCode, const std::shared_ptr<AbilityRecord> &caller)
{
    std::lock_guard guard(recordsMutex_);
    requestCode_++;
    if (requestCode_ >= INT_MAX) {
        requestCode_ = 0;
    }

    auto it = fileSavingRecords_.find(requestCode_);
    if (it != fileSavingRecords_.end()) {
        HILOG_ERROR("repeated request code");
        fileSavingRecords_.erase(it);
    }

    SaveFileRecord record{reqCode, caller};
    fileSavingRecords_.emplace(requestCode_, record);
    return requestCode_;
}

bool StartAbilitySandboxSavefile::ContainRecord(int reqCode)
{
    std::lock_guard guard(recordsMutex_);
    return fileSavingRecords_.count(reqCode) > 0;
}

void StartAbilitySandboxSavefile::HandleResult(const Want &want, int resultCode, int requestCode)
{
    std::shared_ptr<AbilityRecord> callerRecord;
    int originReqCode = -1;
    {
        std::lock_guard guard(recordsMutex_);
        auto it = fileSavingRecords_.find(requestCode);
        if (it != fileSavingRecords_.end()) {
            callerRecord = it->second.caller.lock();
            originReqCode = it->second.originReqCode;
            fileSavingRecords_.erase(it);
        }
    }
    if (!callerRecord) {
        HILOG_ERROR("request code not found: %{public}d.", requestCode);
        return;
    }
    callerRecord->SendSandboxSavefileResult(want, resultCode, originReqCode);
}
} // namespace AAFwk
} // namespace OHOS