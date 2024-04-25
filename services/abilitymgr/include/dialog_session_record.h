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
#include "cpp/mutex.h"
#include "dialog_session_info.h"
#include "json_serializer.h"
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

class DialogSessionRecord {
public:
    std::string GenerateDialogSessionId();

    void SetDialogSessionInfo(const std::string dialogSessionId, sptr<DialogSessionInfo> &dilogSessionInfo,
        std::shared_ptr<DialogCallerInfo> &dialogCallerInfo);

    sptr<DialogSessionInfo> GetDialogSessionInfo(const std::string dialogSessionId) const;

    std::shared_ptr<DialogCallerInfo> GetDialogCallerInfo(const std::string dialogSessionId) const;

    void ClearDialogContext(const std::string dialogSessionId);

    void ClearAllDialogContexts();

    bool GenerateDialogSessionRecord(AbilityRequest &abilityRequest, int32_t userId,
        std::string &dialogSessionId, std::vector<DialogAppInfo> &dialogAppInfos, bool isSelector);

private:
    mutable ffrt::mutex dialogSessionRecordLock_;
    std::unordered_map<std::string, sptr<DialogSessionInfo>> dialogSessionInfoMap_;
    std::unordered_map<std::string, std::shared_ptr<DialogCallerInfo>> dialogCallerInfoMap_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DIALOG_SESSION_RECORD_H
