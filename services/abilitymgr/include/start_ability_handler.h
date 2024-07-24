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

#ifndef OHOS_ABILITY_RUNTIME_START_ABILITY_HANDLER_H
#define OHOS_ABILITY_RUNTIME_START_ABILITY_HANDLER_H
#include <memory>
#include <optional>

#include "ability_record.h"
#include "event_report.h"
#include "want.h"
#include "refbase.h"

namespace OHOS {
class IRemoteObject;
namespace AAFwk {
struct StartAbilityParams {
    StartAbilityParams(Want &reqWant) : want(reqWant) {}
    Want &want;
    sptr<IRemoteObject> callerToken;
    sptr<IRemoteObject> asCallerSourceToken;
    int32_t userId = -1;
    int requestCode = 0;
    bool isStartAsCaller = false;
    const StartOptions* startOptions = nullptr;

    int32_t GetValidUserId()
    {
        return validUserId;
    }
    void SetValidUserId(int32_t value)
    {
        validUserId = value;
    }
    bool IsCallerSandboxApp();

#ifdef WITH_DLP
    bool OtherAppsAccessDlp();
    bool DlpAccessOtherApps();
    bool SandboxExternalAuth();
#endif // WITH_DLP
    bool IsCallerSysApp();
    std::shared_ptr<AbilityRecord> GetCallerRecord();
    int32_t GetCallerAppIndex();

    EventInfo BuildEventInfo();
private:
    int32_t validUserId = 0;
#ifdef WITH_DLP
    std::optional<bool> otherAppsAccessDlp;
    std::optional<bool> dlpAccessOtherApps;
#endif // WITH_DLP
    std::optional<bool> sandboxExternalAuth;
    std::optional<bool> isCallerSysApp;
    std::optional<std::shared_ptr<AbilityRecord>> callerRecord;
    std::optional<int32_t> callerAppIndex;
};

class StartAbilityHandler {
public:
    StartAbilityHandler() = default;
    StartAbilityHandler(StartAbilityHandler &) = delete;
    void operator=(StartAbilityHandler &) = delete;
    virtual ~StartAbilityHandler() = default;
    virtual bool MatchStartRequest(StartAbilityParams &params);
    virtual int HandleStartRequest(StartAbilityParams &params);
    virtual int GetPriority()
    {
        return 0;
    }
    virtual std::string GetHandlerName()
    {
        return "";
    }
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_START_ABILITY_HANDLER_H