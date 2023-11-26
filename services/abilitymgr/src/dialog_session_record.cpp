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

#include "dialog_session_record.h"

#include <random>
#include <string>
#include <chrono>
#include "ability_record.h"
#include "ability_util.h"
#include "hilog_wrapper.h"
#include "want_params_wrapper.h"


namespace OHOS {
namespace AAFwk {
std::string DialogSessionRecord::GenerateDialogSessionId()
{
    auto timestamp = std::chrono::system_clock::now().time_since_epoch();
    auto time = std::chrono::duration_cast<std::chrono::seconds>(timestamp).count();
    std::random_device seed;
    std::mt19937 rng(seed());
    std::uniform_int_distribution<int> uni(0, INT_MAX);
    int randomDigit = uni(rng);
    return std::to_string(time) + "_" + std::to_string(randomDigit);
}

void DialogSessionRecord::SetDialogSessionInfo(const std::string dialogSessionId,
    sptr<DialogSessionInfo> &dilogSessionInfo, std::shared_ptr<DialogCallerInfo> &dialogCallerInfo)
{
    dialogSessionInfoMap_[dialogSessionId] = dilogSessionInfo;
    dialogCallerInfoMap_[dialogSessionId] = dialogCallerInfo;
}

sptr<DialogSessionInfo> DialogSessionRecord::GetDialogSessionInfo(const std::string dialogSessionId) const
{
    auto it = dialogSessionInfoMap_.find(dialogSessionId);
    if (it != dialogSessionInfoMap_.end()) {
        return it->second;
    }
    HILOG_INFO("not find");
    return nullptr;
}

std::shared_ptr<DialogCallerInfo> DialogSessionRecord::GetDialogCallerInfo(const std::string dialogSessionId) const
{
    auto it = dialogCallerInfoMap_.find(dialogSessionId);
    if (it != dialogCallerInfoMap_.end()) {
        return it->second;
    }
    HILOG_INFO("not find");
    return nullptr;
}

void DialogSessionRecord::ClearDialogContext(const std::string dialogSessionId)
{
    auto it = dialogSessionInfoMap_.find(dialogSessionId);
    if (it != dialogSessionInfoMap_.end()) {
        dialogSessionInfoMap_.erase(it);
    }
    return;
}

void DialogSessionRecord::ClearAllDialogContexts()
{
    dialogSessionInfoMap_.clear();
}

bool DialogSessionRecord::QueryDialogAppInfo(DialogAbilityInfo &dialogAbilityInfo, int32_t userId)
{
    std::string bundleName = dialogAbilityInfo.bundleName;
    auto bms = AbilityUtil::GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, ERR_INVALID_VALUE);
    AppExecFwk::ApplicationInfo appInfo;
    bool ret = IN_PROCESS_CALL(bms->GetApplicationInfo(bundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo));
    if (!ret) {
        HILOG_ERROR("get application info failed, err:%{public}d.", ret);
        return false;
    }
    dialogAbilityInfo.bundleIconId = appInfo.iconId;
    dialogAbilityInfo.bundleLabelId = appInfo.labelId;
    return true;
}

bool DialogSessionRecord::GenerateDialogSessionRecord(AbilityRequest &abilityRequest, int32_t userId,
    std::string &dialogSessionId, std::vector<DialogAppInfo> &dialogAppInfos, const std::string &deviceType)
{
    sptr<DialogSessionInfo> dialogSessionInfo(new (std::nothrow) DialogSessionInfo());
    CHECK_POINTER_AND_RETURN(dialogSessionInfo, ERR_INVALID_VALUE);
    sptr<IRemoteObject> callerToken = abilityRequest.callerToken;
    CHECK_POINTER_AND_RETURN(callerToken, ERR_INVALID_VALUE);
    auto callerRecord = Token::GetAbilityRecordByToken(callerToken);
    CHECK_POINTER_AND_RETURN(callerRecord, ERR_INVALID_VALUE);
    dialogSessionInfo->callerAbilityInfo.bundleName = callerRecord->GetAbilityInfo().bundleName;
    dialogSessionInfo->callerAbilityInfo.moduleName = callerRecord->GetAbilityInfo().moduleName;
    dialogSessionInfo->callerAbilityInfo.abilityName = callerRecord->GetAbilityInfo().name;
    dialogSessionInfo->callerAbilityInfo.abilityIconId = callerRecord->GetAbilityInfo().iconId;
    dialogSessionInfo->callerAbilityInfo.abilityLabelId = callerRecord->GetAbilityInfo().labelId;
    bool ret = QueryDialogAppInfo(dialogSessionInfo->callerAbilityInfo, userId);
    if (!ret) {
        HILOG_ERROR("query dialog app info failed");
        return false;
    }
    dialogSessionInfo->parameters.SetParam("deviceType", String::Box(deviceType));
    dialogSessionInfo->parameters.SetParam("userId", Integer::Box(userId));

    for (auto &dialogAppInfo : dialogAppInfos) {
        DialogAbilityInfo targetDialogAbilityInfo;
        targetDialogAbilityInfo.bundleName = dialogAppInfo.bundleName;
        targetDialogAbilityInfo.moduleName = dialogAppInfo.moduleName;
        targetDialogAbilityInfo.abilityName = dialogAppInfo.abilityName;
        targetDialogAbilityInfo.abilityIconId = dialogAppInfo.iconId;
        targetDialogAbilityInfo.abilityLabelId = dialogAppInfo.labelId;
        int ret = QueryDialogAppInfo(targetDialogAbilityInfo, userId);
        if (!ret) {
            HILOG_ERROR("query dialog app infos failed");
            return false;
        }
        dialogSessionInfo->targetAbilityInfos.emplace_back(targetDialogAbilityInfo);
    }
    if (dialogAppInfos.size() > 1) {
        dialogSessionInfo->parameters.SetParam("action", String::Box(abilityRequest.want.GetAction()));
        dialogSessionInfo->parameters.SetParam("wantType", String::Box(abilityRequest.want.GetType()));
    }
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    dialogCallerInfo->callerToken = callerToken;
    dialogCallerInfo->requestCode = abilityRequest.requestCode;
    dialogCallerInfo->targetWant = abilityRequest.want;
    dialogCallerInfo->userId = userId;
    dialogSessionId = GenerateDialogSessionId();
    SetDialogSessionInfo(dialogSessionId, dialogSessionInfo, dialogCallerInfo);
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
