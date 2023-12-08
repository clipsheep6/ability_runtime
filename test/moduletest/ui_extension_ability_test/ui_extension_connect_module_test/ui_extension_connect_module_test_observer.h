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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONNECT_MODULE_TEST_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONNECT_MODULE_TEST_OBSERVER_H

#include "application_state_observer_stub.h"

namespace OHOS {
namespace AAFwk {
class UIExtensionConnectModuleTestObserver : public AppExecFwk::ApplicationStateObserverStub {
public:
    UIExtensionConnectModuleTestObserver() = default;
    ~UIExtensionConnectModuleTestObserver() = default;

    std::condition_variable observerCondation_;
    std::mutex observerMutex_;
    bool processCreated_ = false;
    bool processForegrounded_ = false;
    bool processBackgrounded_ = false;
    bool processDied_ = false;

private:
    void OnProcessCreated(const AppExecFwk::ProcessData &processData) override;
    void OnProcessStateChanged(const AppExecFwk::ProcessData &processData) override;
    void OnProcessDied(const AppExecFwk::ProcessData &processData) override;
    void OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override;
    void OnExtensionStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONNECT_MODULE_TEST_OBSERVER_H
