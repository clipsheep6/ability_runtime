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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_H

#include <atomic>
#include <map>
#include <memory>
#include <string>

#include "ability_record.h"
#include "preload_uiext_state_observer.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INVALID_EXTENSION_RECORD_ID = 0;
}
class ExtensionRecord : public std::enable_shared_from_this<ExtensionRecord> {
public:
    ExtensionRecord(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord);

    virtual ~ExtensionRecord();

    sptr<IRemoteObject> GetCallToken() const;

    sptr<IRemoteObject> GetRootCallerToken() const;

    void SetRootCallerToken(sptr<IRemoteObject> &rootCallerToken);

    sptr<IRemoteObject> GetFocusedCallerToken() const;

    void SetFocusedCallerToken(sptr<IRemoteObject> &rootCallerToken);

    virtual bool ContinueToGetCallerToken();

    virtual void Update(const AAFwk::AbilityRequest &abilityRequest);

    void UnloadUIExtensionAbility();

    int32_t RegisterStateObserver(const std::string &hostBundleName);

    std::shared_ptr<AAFwk::AbilityRecord> abilityRecord_ = nullptr;
    std::string hostBundleName_;
    int32_t extensionRecordId_ = INVALID_EXTENSION_RECORD_ID;
    uint32_t processMode_ = 0;
    bool isHostSpecified_ = false;
    pid_t hostPid_ = 0;
private:
    sptr<IRemoteObject> rootCallerToken_ = nullptr;
    sptr<IRemoteObject> focusedCallerToken_ = nullptr;
    sptr<AAFwk::PreLoadUIExtStateObserver> preLoadUIExtStateObserver_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_H
