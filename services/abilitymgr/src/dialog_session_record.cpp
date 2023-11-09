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
#include "ability_record.h"
#include "ability_util.h"
#include "hilog_wrapper.h"


namespace OHOS {
namespace AAFwk {
std::string DialogSessionRecord::GenerateDialogSessionId()//123
{
    std::random_device seed;
    std::mt19937 rng(seed());
    std::uniform_int_distribution<int> uni(0, INT_MAX);
    int num = uni(rng);
    return std::to_string(num) + "" + "";
}

void DialogSessionRecord::SetDialogSessionInfo(const std::string dialogSessionId, sptr<DialogSessionInfo> &dilogSessionInfo,
    std::shared_ptr<DialogCallerInfo> &dialogCallerInfo)
{
    dialogSessionInfoMap_[dialogSessionId] = dilogSessionInfo;
    dialogCallerInfoMap_[doalogSessionId] = dialogCallerInfo;
}

sptr<DialogSessionInfo> DialogSessionRecord::GetDialogContext(const std::string dialogSessionId) const
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

int DialogSessionRecord::GenerateDialogSessionRecord(AbilityRequest &abilityRequest, int32_t userId,
    std::string &dialogSessionId, std::vector<DialogAppInfo> &dialogAppInfos) {
    dialogSessionId = GenerateDialogSessionId();
    std::shared_ptr<DialogSessionInfo> dialogSessionInfo = std::make_shared<DialogSessionInfo>();
    sptr<IRemoteObject> callerToken = abilityRequest.callerToken;
    CHECK_POINTER_AND_RETURN(callerToken, ERR_INVALID_VALUE);
    auto callerRecord = Token::GetAbilityRecordByToken(callerToken);
    CHECK_POINTER_AND_RETURN(callerRecord, ERR_INVALID_VALUE);
    dialogSessionInfo->callerAbilityInfo.bundleName = callerRecord->GetAbilityInfo().bundleName;
    dialogSessionInfo->callerAbilityInfo.moduleName = callerRecord->GetAbilityInfo().moduleName;
    dialogSessionInfo->callerAbilityInfo.abilityName = callerRecord->GetAbilityInfo().name;

    dialogSessionInfo->paramsJson = nlohmann::json {
        {"deviceType", OHOS::system::GetDeviceType()},
        {"userId", std::to_string(userId)},
    };

    for (auto &dialogAppInfo:dialogAppInfos) {
        DialogAbilityInfo targetDialogAbilityInfo;
        targetDialogAbilityInfo.bundleName = dialogAppInfo.bundleName;
        targetDialogAbilityInfo.moduleName = dialogAppInfo.moduleName;
        targetDialogAbilityInfo.abilityName = dialogAppInfo.abilityName;
        targetDialogAbilityInfo.abilityIconId = dialogAppInfo.iconId;
        targetDialogAbilityInfo.abilityLabelId = dialogAppInfo.labelId;
        dialogSessionInfo->targetAbilityInfos.push_back(targetDialogAbilityInfo);
    }
    if (dialogAppInfos.size() == 1) {
        auto bms = AbilityUtil::GetBundleManager();
        CHECK_POINTER_AND_RETURN(bms, ERR_INVALID_VALUE);
        AppExecFwk::ApplicationInfo appInfo;
        bool ret = IN_PROCESS_CALL(bms->GetApplicationInfo(dialogAppInfos.front().bundleName,
            AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo));
        if (ret != ERR_OK) {
            HILOG_ERROR("get application info failed, err:%{public}d.", ret);
            return ret;
        }
        dialogSessionInfo->targetAbilityInfos.front().bundleIconId = appInfo.iconId;
        dialogSessionInfo->targetAbilityInfos.front().bundleLabelId = appInfo.labelId;
    } else {
        dialogSessionInfo->paramsJson["action", abilityRequest.want.GetAction()];
        dialogSessionInfo->paramsJson["wantType", abilityRequest.want.GetType()];
    }

    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    dialogCallerInfo->callerToken = callerToken;
    dialogCallerInfo->requestCode = abilityRequest.requestCode;
    dialogCallerInfo->userId = userId;
    SetDialogSessionInfo(dialogSessionId, dialogSessionInfo, dialogCallerInfo);
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
