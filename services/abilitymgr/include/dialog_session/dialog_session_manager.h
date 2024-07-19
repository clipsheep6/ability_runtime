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

#ifndef OHOS_ABILITY_RUNTIME_DIALOG_SESSION_MANAGEER_H
#define OHOS_ABILITY_RUNTIME_DIALOG_SESSION_MANAGEER_H
#include <list>
#include <unordered_map>
#include <string>
#include "ability_record.h"
#include "cpp/mutex.h"
#include "dialog_session_info.h"
#include "json_serializer.h"
#include "nocopyable.h"
#include "parcel.h"
#include "refbase.h"
#include "system_dialog_scheduler.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
struct DialogCallerInfo {
    int32_t userId = -1;
    int requestCode = -1;
    sptr<IRemoteObject> callerToken;
    Want targetWant;
    bool isSelector = false;
};

enum class SelectorType {
    IMPLICIT_START_SELECTOR = 0,
    APP_CLONR_SELECTOR = 1
};

class DialogSessionManager {
public:
    static DialogSessionManager &GetInstance();
    ~DialogSessionManager() = default;

    sptr<DialogSessionInfo> GetDialogSessionInfo(const std::string &dialogSessionId) const;

    std::shared_ptr<DialogCallerInfo> GetDialogCallerInfo(const std::string &dialogSessionId) const;

    int SendDialogResult(const Want &want, const std::string &dialogSessionId, bool isAllowed);

    int CreateJumpModalDialog(AbilityRequest &abilityRequest, int32_t userId, const Want &replaceWant);

    int CreateImplicitSelectorModalDialog(AbilityRequest &abilityRequest, const Want &want, int32_t userId,
        std::vector<DialogAppInfo> &dialogAppInfos);

    int CreateCloneSelectorModalDialog(AbilityRequest &abilityRequest, const Want &want, int32_t userId,
        std::vector<DialogAppInfo> &dialogAppInfos, const std::string &replaceWant);

    int HandleErmsResult(AbilityRequest &abilityRequest, int32_t userId, const Want &replaceWant);

    bool IsCreateCloneSelectorDialog(const std::string &bundleName, int32_t userId);

private:
    DialogSessionManager() = default;
    std::string GenerateDialogSessionId();

    void SetDialogSessionInfo(const std::string &dialogSessionId, sptr<DialogSessionInfo> &dilogSessionInfo,
        std::shared_ptr<DialogCallerInfo> &dialogCallerInfo);

    void ClearDialogContext(const std::string &dialogSessionId);

    void ClearAllDialogContexts();

    std::string GenerateDialogSessionRecordCommon(AbilityRequest &abilityRequest, int32_t userId,
        const AAFwk::WantParams &parameters, std::vector<DialogAppInfo> &dialogAppInfos, bool isSelector);

    void GenerateCallerAbilityInfo(AbilityRequest &abilityRequest, DialogAbilityInfo &callerAbilityInfo);

    void GenerateSelectorTargetAbilityInfos(std::vector<DialogAppInfo> &dialogAppInfos,
        std::vector<DialogAbilityInfo> &targetAbilityInfos);
    
    void GenerateJumpTargetAbilityInfos(AbilityRequest &abilityRequest,
        std::vector<DialogAbilityInfo> &targetAbilityInfos);

    void GenerateDialogCallerInfo(AbilityRequest &abilityRequest, int32_t userId,
        std::shared_ptr<DialogCallerInfo> dialogCallerInfo, bool isSelector);

    int CreateModalDialogCommon(const Want &replaceWant, sptr<IRemoteObject> callerToken,
        const std::string &dialogSessionId);

    mutable ffrt::mutex dialogSessionRecordLock_;
    std::unordered_map<std::string, sptr<DialogSessionInfo>> dialogSessionInfoMap_;
    std::unordered_map<std::string, std::shared_ptr<DialogCallerInfo>> dialogCallerInfoMap_;

    DISALLOW_COPY_AND_MOVE(DialogSessionManager);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DIALOG_SESSION_MANAGEER_H
