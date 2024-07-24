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

#include "app_exit_reason_helper.h"

#include <string>
#include <vector>

#include "ability_util.h"
#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "app_exit_reason_data_manager.h"
#include "app_scheduler.h"
#include "bundle_constants.h"
#include "bundle_mgr_interface.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "os_account_manager_wrapper.h"
#include "scene_board_judgement.h"
#include "singleton.h"
#include "sub_managers_helper.h"

namespace OHOS {
namespace AAFwk {
namespace {
    constexpr int32_t U0_USER_ID = 0;
}

AppExitReasonHelper::AppExitReasonHelper(std::shared_ptr<SubManagersHelper> subManagersHelper)
    : subManagersHelper_(subManagersHelper) {}

int32_t AppExitReasonHelper::RecordAppExitReason(const ExitReason &exitReason)
{
    if (!IsExitReasonValid(exitReason)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Force exit reason invalid.");
        return ERR_INVALID_VALUE;
    }

    auto pid = IPCSkeleton::GetCallingPid();
    AppExecFwk::ApplicationInfo application;
    bool debug = false;
    auto ret = IN_PROCESS_CALL(DelayedSingleton<AppScheduler>::GetInstance()->GetApplicationInfoByProcessID(pid,
        application, debug));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetApplicationInfoByProcessID failed.");
        return ret;
    }
    auto bundleName = application.bundleName;
    int32_t resultCode = RecordProcessExtensionExitReason(NO_PID, bundleName, exitReason);
    if (resultCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Record Process Extension Exit Reason failed.code: %{public}d", resultCode);
    }

    CHECK_POINTER_AND_RETURN(subManagersHelper_, ERR_NULL_OBJECT);
    std::vector<std::string> abilityList;
    auto uid = IPCSkeleton::GetCallingUid();
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = subManagersHelper_->GetUIAbilityManagerByUid(uid);
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_NULL_OBJECT);
        uiAbilityManager->GetActiveAbilityList(uid, abilityList);
    } else {
        auto missionListManager = subManagersHelper_->GetMissionListManagerByUid(uid);
        CHECK_POINTER_AND_RETURN(missionListManager, ERR_NULL_OBJECT);
        missionListManager->GetActiveAbilityList(uid, abilityList);
    }

    ret = DelayedSingleton<AppScheduler>::GetInstance()->NotifyAppMgrRecordExitReason(pid, exitReason.reason,
        exitReason.exitMsg);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyAppMgrRecordExitReason failed.code: %{public}d", ret);
    }

    if (abilityList.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Active abilityLists empty.");
        return ERR_GET_ACTIVE_ABILITY_LIST_EMPTY;
    }
    return DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->SetAppExitReason(bundleName,
        application.accessTokenId, abilityList, exitReason);
}

int32_t AppExitReasonHelper::RecordProcessExitReason(const int32_t pid, const ExitReason &exitReason)
{
    AppExecFwk::ApplicationInfo application;
    bool debug = false;
    auto ret = IN_PROCESS_CALL(DelayedSingleton<AppScheduler>::GetInstance()->GetApplicationInfoByProcessID(pid,
        application, debug));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetApplicationInfoByProcessID failed.");
        return ret;
    }
    auto bundleName = application.bundleName;
    int32_t resultCode = RecordProcessExtensionExitReason(pid, bundleName, exitReason);
    if (resultCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Record Process Extension Exit Reason failed.code: %{public}d", resultCode);
    }

    return RecordProcessExitReason(pid, bundleName, application.uid, application.accessTokenId, exitReason);
}

int32_t AppExitReasonHelper::RecordAppExitReason(const std::string &bundleName, int32_t uid, int32_t appIndex,
    const ExitReason &exitReason)
{
    int32_t userId;
    if (DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(uid, userId) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get GetOsAccountLocalIdFromUid failed.");
        return ERR_INVALID_VALUE;
    }
    uint32_t accessTokenId = Security::AccessToken::AccessTokenKit::GetHapTokenID(userId, bundleName, appIndex);
    return RecordProcessExitReason(NO_PID, bundleName, uid, accessTokenId, exitReason);
}

int32_t AppExitReasonHelper::RecordProcessExitReason(const int32_t pid, const std::string bundleName,
    const int32_t uid, const uint32_t accessTokenId, const ExitReason &exitReason)
{
    if (!IsExitReasonValid(exitReason)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Force exit reason invalid.");
        return ERR_INVALID_VALUE;
    }

    int32_t targetUserId;
    if (DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(uid, targetUserId) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get GetOsAccountLocalIdFromUid failed.");
        return ERR_INVALID_VALUE;
    }
    std::vector<std::string> abilityLists;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        GetActiveAbilityListFromUIAbilityManager(uid, abilityLists, pid);
    } else  {
        GetActiveAbilityList(uid, abilityLists, pid);
    }

    auto ret = DelayedSingleton<AppScheduler>::GetInstance()->NotifyAppMgrRecordExitReason(pid, exitReason.reason,
        exitReason.exitMsg);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyAppMgrRecordExitReason failed.code: %{public}d", ret);
    }

    if (abilityLists.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Active abilityLists empty.");
        return ERR_GET_ACTIVE_ABILITY_LIST_EMPTY;
    }
    return DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->SetAppExitReason(bundleName,
        accessTokenId, abilityLists, exitReason);
}

int32_t AppExitReasonHelper::RecordProcessExtensionExitReason(
    const int32_t pid, const std::string &bundleName, const ExitReason &exitReason)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER_AND_RETURN(subManagersHelper_, ERR_NULL_OBJECT);
    auto connectManager = subManagersHelper_->GetCurrentConnectManager();
    CHECK_POINTER_AND_RETURN(connectManager, ERR_NULL_OBJECT);
    std::vector<std::string> extensionList;
    int32_t resultCode = ERR_OK;
    if (pid <= NO_PID) {
        resultCode = connectManager->GetActiveUIExtensionList(bundleName, extensionList);
    } else {
        resultCode = connectManager->GetActiveUIExtensionList(pid, extensionList);
    }
    if (resultCode != ERR_OK) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ResultCode: %{public}d", resultCode);
        return ERR_GET_ACTIVE_EXTENSION_LIST_EMPTY;
    }

    if (extensionList.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ExtensionList is empty.");
        return ERR_GET_ACTIVE_EXTENSION_LIST_EMPTY;
    }

    auto appExitReasonDataMgr = DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance();
    if (appExitReasonDataMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get app exit reason data mgr instance is nullptr.");
        return ERR_INVALID_VALUE;
    }

    return appExitReasonDataMgr->SetUIExtensionAbilityExitReason(bundleName, extensionList, exitReason);
}

void AppExitReasonHelper::GetActiveAbilityList(int32_t uid, std::vector<std::string> &abilityLists,
    const int32_t pid)
{
    int32_t targetUserId;
    if (DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(uid, targetUserId) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get GetOsAccountLocalIdFromUid failed.");
        return;
    }
    CHECK_POINTER(subManagersHelper_);
    if (targetUserId == U0_USER_ID) {
        auto missionListManagers = subManagersHelper_->GetMissionListManagers();
        for (auto& item: missionListManagers) {
            CHECK_POINTER_CONTINUE(item.second);
            std::vector<std::string> abilityList;
            item.second->GetActiveAbilityList(uid, abilityList, pid);
            if (!abilityList.empty()) {
                abilityLists.insert(abilityLists.end(), abilityList.begin(), abilityList.end());
            }
        }
        return;
    }

    auto listManager = subManagersHelper_->GetMissionListManagerByUserId(targetUserId);
    CHECK_POINTER(listManager);
    listManager->GetActiveAbilityList(uid, abilityLists, pid);
}

void AppExitReasonHelper::GetActiveAbilityListFromUIAbilityManager(int32_t uid, std::vector<std::string> &abilityLists,
    const int32_t pid)
{
    CHECK_POINTER(subManagersHelper_);
    int32_t targetUserId;
    if (DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(uid, targetUserId) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get GetOsAccountLocalIdFromUid failed.");
        return;
    }
    if (targetUserId == U0_USER_ID) {
        auto uiAbilityManagers = subManagersHelper_->GetUIAbilityManagers();
        for (auto& item: uiAbilityManagers) {
            CHECK_POINTER_CONTINUE(item.second);
            std::vector<std::string> abilityList;
            item.second->GetActiveAbilityList(uid, abilityList, pid);
            if (!abilityList.empty()) {
                abilityLists.insert(abilityLists.end(), abilityList.begin(), abilityList.end());
            }
        }
        return;
    }

    auto uiAbilityManager = subManagersHelper_->GetUIAbilityManagerByUserId(targetUserId);
    CHECK_POINTER(uiAbilityManager);
    uiAbilityManager->GetActiveAbilityList(uid, abilityLists, pid);
}

bool AppExitReasonHelper::IsExitReasonValid(const ExitReason &exitReason)
{
    const Reason reason = exitReason.reason;
    return reason >= REASON_MIN && reason <= REASON_MAX;
}
}  // namespace AppExecFwk
}  // namespace OHOS
