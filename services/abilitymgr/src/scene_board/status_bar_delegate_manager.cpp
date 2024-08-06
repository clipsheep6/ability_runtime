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

#include "scene_board/status_bar_delegate_manager.h"

#include "ability_util.h"
#include "hitrace_meter.h"
#include "process_options.h"

namespace OHOS {
namespace AAFwk {
int32_t StatusBarDelegateManager::RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate)
{
    std::lock_guard<ffrt::mutex> lock(statusBarDelegateMutex_);
    statusBarDelegate_ = delegate;
    return ERR_OK;
}

sptr<AbilityRuntime::IStatusBarDelegate> StatusBarDelegateManager::GetStatusBarDelegate()
{
    std::lock_guard<ffrt::mutex> lock(statusBarDelegateMutex_);
    return statusBarDelegate_;
}

bool StatusBarDelegateManager::IsCallerInStatusBar()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto statusBarDelegate = GetStatusBarDelegate();
    CHECK_POINTER_AND_RETURN(statusBarDelegate, false);
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    bool isExist = false;
    auto ret = statusBarDelegate->CheckIfStatusBarItemExists(callingTokenId, isExist);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckIfStatusBarItemExists failed, ret: %{public}d", ret);
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "isExist: %{public}d", isExist);
    return isExist;
}

int32_t StatusBarDelegateManager::DoProcessAttachment(std::shared_ptr<AbilityRecord> abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    auto sessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER_AND_RETURN(sessionInfo, ERR_INVALID_VALUE);
    auto processOptions = sessionInfo->processOptions;
    if (processOptions == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "no need to attach process.");
        return ERR_OK;
    }
    if (processOptions->processMode == ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT) {
        auto callerRecord = abilityRecord->GetCallerRecord();
        CHECK_POINTER_AND_RETURN(callerRecord, ERR_INVALID_VALUE);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "attach pid to parent.");
        IN_PROCESS_CALL_WITHOUT_RET(DelayedSingleton<AppScheduler>::GetInstance()->AttachPidToParent(
            abilityRecord->GetToken(), callerRecord->GetToken()));
    }
    if (ProcessOptions::IsAttachToStatusBarMode(processOptions->processMode)) {
        auto statusBarDelegate = GetStatusBarDelegate();
        CHECK_POINTER_AND_RETURN(statusBarDelegate, ERR_INVALID_VALUE);
        auto accessTokenId = abilityRecord->GetApplicationInfo().accessTokenId;
        auto ret = statusBarDelegate->AttachPidToStatusBarItem(accessTokenId, abilityRecord->GetPid());
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "AttachPidToStatusBarItem failed, ret: %{public}d", ret);
            return ret;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "AttachPidToStatusBarItem success.");
    }
    if (processOptions->processMode == ProcessMode::NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM) {
        IN_PROCESS_CALL_WITHOUT_RET(DelayedSingleton<AppScheduler>::GetInstance()->AttachedToStatusBar(
            abilityRecord->GetToken()));
    }
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS