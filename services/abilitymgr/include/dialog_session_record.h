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

#ifndef OHOS_ABILITY_RUNTIME_DIALOG_SESSION_RECORD_H
#define OHOS_ABILITY_RUNTIME_DIALOG_SESSION_RECORD_H
#include <list>
#include <unordered_map>
#include <string>
#include "ability_record.h"
#include "json_serializer.h"
#include "system_dialog_scheduler.h"

namespace OHOS {
namespace AAFwk {

struct DialogAbilityInfo {
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    std::string bundleIconId;
    std::string bundleLabelId;
    std::string abilityIconId;
    std::string abilityLabelId;
};

struct DialogSessionInfo {
    DialogAbilityInfo callerAbilityInfo;
    std::list<DialogAbilityInfo> targetAbilityInfos; // get info form erms or bms
    nlohmann::json paramsJson;
};

struct DialogCallerInfo {
    int32_t userId;
    int requestCode;
    sptr<IRemoteObject> callerToken;
};

class DialogSessionRecord {
public:
    std::string GenerateDialogSessionId();

    void SetDialogSessionInfo(const std::string dialogSessionId, sptr<DialogSessionInfo> &dilogSessionInfo,
        std::shared_ptr<DialogCallerInfo> &dialogCallerInfo);

    sptr<DialogSessionInfo> GetDialogContext(const std::string dialogSessionId) const;

    std::shared_ptr<DialogCallerInfo> GetDialogCallerInfo(const std::string dialogSessionId) const;

    void ClearDialogContext(const std::string dialogSessionId);

    void ClearAllDialogContexts();

    int GenerateDialogSessionRecord(AbilityRequest &abilityRequest, int32_t userId,
        std::string &dialogSessionId, std::vector<DialogAppInfo> &dialogAppInfos);


private:
    std::unordered_map<std::string, sptr<DialogSessionInfo>> dialogSessionInfoMap_;
    std::unordered_map<std::string, std::shared_ptr<DialogCallerInfo>> dialogCallerInfoMap_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DIALOG_SESSION_RECORD_H