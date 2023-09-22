/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_APP_STATE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_JS_APP_STATE_OBSERVER_H

#include "application_state_observer_stub.h"
#include "native_engine/native_engine.h"
#include "event_handler.h"

namespace OHOS {
namespace AbilityRuntime {
using OHOS::AppExecFwk::ApplicationStateObserverStub;
using OHOS::AppExecFwk::AppStateData;
using OHOS::AppExecFwk::AbilityStateData;
using OHOS::AppExecFwk::ProcessData;
class JSAppStateObserver : public ApplicationStateObserverStub {
public:
    explicit JSAppStateObserver(napi_env engine);
    ~JSAppStateObserver();
    void OnForegroundApplicationChanged(const AppStateData &appStateData) override;
    void OnAbilityStateChanged(const AbilityStateData &abilityStateData) override;
    void OnExtensionStateChanged(const AbilityStateData &abilityStateData) override;
    void OnProcessCreated(const ProcessData &processData) override;
    void OnProcessStateChanged(const ProcessData &processData) override;
    void OnProcessDied(const ProcessData &processData) override;
    void HandleOnForegroundApplicationChanged(const AppStateData &appStateData);
    void HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData);
    void HandleOnExtensionStateChanged(const AbilityStateData &abilityStateData);
    void HandleOnProcessCreated(const ProcessData &processData);
    void HandleOnProcessStateChanged(const ProcessData &processData);
    void HandleOnProcessDied(const ProcessData &processData);
    void CallJsFunction(napi_value value, const char *methodName, napi_value* argv, size_t argc);
    void AddJsObserverObject(const int32_t observerId, napi_value jsObserverObject);
    bool RemoveJsObserverObject(const int32_t observerId);
    bool FindObserverByObserverId(const int32_t observerId);
    size_t GetJsObserverMapSize();

private:
    napi_env env_;
    std::map<int32_t, std::shared_ptr<NativeReference>> jsObserverObjectMap_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_APP_STATE_OBSERVER_H
