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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_RECORD_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_RECORD_H

#include <atomic>
#include <map>
#include <memory>
#include <string>

#include "extension_record.h"

namespace OHOS {
namespace AbilityRuntime {
class UIExtensionRecord : public ExtensionRecord {
public:
    UIExtensionRecord(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord);

    ~UIExtensionRecord() override;

    bool ContinueToGetCallerToken() override;

    void Update(const AAFwk::AbilityRequest &abilityRequest) override;

    void LoadTimeout();
    void ForegroundTimeout();
    void BackgroundTimeout();
    void TerminateTimeout();
private:
    enum ErrorCode {
        LOAD_TIMEOUT = 0,
        FOREGROUND_TIMEOUT = 1,
        BACKGROUND_TIMEOUT = 2,
        TERMINATE_TIMEOUT = 3
    };
    void HandleNotifyUIExtensionTimeout(ErrorCode code);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_EXTENSION_RECORD_H
