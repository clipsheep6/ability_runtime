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

#ifndef OHOS_ABILITY_RUNTIME_START_ABILITY_SANDBOX_SAVEFILE_H
#define OHOS_ABILITY_RUNTIME_START_ABILITY_SANDBOX_SAVEFILE_H

#include <memory>
#include <mutex>
#include <unordered_map>
#include "start_ability_handler.h"

namespace OHOS {
namespace AAFwk {
struct SaveFileRecord {
    int32_t originReqCode = 0;
    std::weak_ptr<AbilityRecord> caller;
};

class StartAbilitySandboxSavefile : public StartAbilityHandler,
                                    public std::enable_shared_from_this<StartAbilitySandboxSavefile> {
public:
    static const std::string handlerName_;
    bool MatchStartRequest(StartAbilityParams &params) override;
    int HandleStartRequest(StartAbilityParams &params) override;
    std::string GetHandlerName() override;

    void HandleResult(const Want &want, int resultCode, int requestCode);
protected:
    int PushRecord(int reqCode, const std::shared_ptr<AbilityRecord> &caller);
    bool ContainRecord(int reqCode);

    int StartAbility(StartAbilityParams &params, int requestCode);
private:
    std::mutex recordsMutex_;
    std::unordered_map<int, SaveFileRecord> fileSavingRecords_;
    int requestCode_ = 0;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_START_ABILITY_SANDBOX_SAVEFILE_H