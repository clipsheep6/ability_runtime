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

#include "hilog_wrapper.h"
#include "ability_manager_errors.h"
#include "ability_util.h"
#include "ability_manager_service.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string DLP_BUNDLE_NAME = "com.ohos.dlpmanager";
const std::string DLP_ABILITY_NAME = "SaveAsAbility";
constexpr int REQUEST_TIME_OUT = 3000;
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
    auto reqCode = params.requestCode;
    PushRecord(reqCode, callerRecord);

    auto &want = params.want;
    want.SetElementName(DLP_BUNDLE_NAME, DLP_ABILITY_NAME);
    want.SetParam("requestCode", reqCode);
    want.SetParam("startMode", std::string("save_redirect"));

    auto abilityMs = DelayedSingleton<AbilityManagerService>::GetInstance();
    auto handler = abilityMs->GetEventHandler();
    if (handler) {
        auto task = [pthis = shared_from_this(), reqCode]() {
            HILOG_DEBUG("clear record: %{public}d", reqCode);
            pthis->RemoveRecord(reqCode);
        };
        handler->PostTask(task, REQUEST_TIME_OUT);
    }
    if (params.startOptions) {
        return abilityMs->StartAbilityForOptionInner(want, *params.startOptions, params.callerToken, reqCode,
            params.userId, params.isStartAsCaller);
    }
    return abilityMs->StartAbilityInner(want, params.callerToken, reqCode, params.userId,
        params.isStartAsCaller);
}

std::string StartAbilitySandboxSavefile::GetHandlerName()
{
    return StartAbilitySandboxSavefile::handlerName_;
}

void StartAbilitySandboxSavefile::PushRecord(int reqCode, const std::shared_ptr<AbilityRecord> &callerRecord)
{
    std::lock_guard guard(recordsMutex_);
    auto it = fileSavingRecords_.find(reqCode);
    if (it != fileSavingRecords_.end()) {
        HILOG_ERROR("repeated request code");
        if (it->second.lock()) {
            return;
        }
        fileSavingRecords_.erase(it);
    }
    fileSavingRecords_.emplace(reqCode, callerRecord);
}

void StartAbilitySandboxSavefile::RemoveRecord(int reqCode)
{
    std::lock_guard guard(recordsMutex_);
    fileSavingRecords_.erase(reqCode);
}

bool StartAbilitySandboxSavefile::ContainRecord(int reqCode)
{
    std::lock_guard guard(recordsMutex_);
    return fileSavingRecords_.count(reqCode) > 0;
}

void StartAbilitySandboxSavefile::HandleResult(const Want &want, int resultCode, int requestCode)
{
    std::shared_ptr<AbilityRecord> callerRecord;
    {
        std::lock_guard guard(recordsMutex_);
        auto it = fileSavingRecords_.find(requestCode);
        if (it != fileSavingRecords_.end()) {
            callerRecord = it->second.lock();
            fileSavingRecords_.erase(it);
        }
    }
    if (!callerRecord) {
        HILOG_ERROR("request code not found: %{public}d.", requestCode);
        return;
    }
    callerRecord->SendSandboxSavefileResult(want, resultCode, requestCode);
}

} // namespace AAFwk
} // namespace OHOS