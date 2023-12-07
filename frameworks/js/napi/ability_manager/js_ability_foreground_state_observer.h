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

#ifndef OHOS_ABILITY_RUNTIME_JS_ABILITY_FOREGROUND_STATE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_JS_ABILITY_FOREGROUND_STATE_OBSERVER_H

#include <set>

#include "ability_foreground_state_observer_stub.h"
#include "ability_state_data.h"
#include "event_handler.h"
#include "js_ability_manager_utils.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
using OHOS::AppExecFwk::AbilityForegroundStateObserverStub;
using OHOS::AppExecFwk::AbilityStateData;
class JSAbilityForegroundStateObserver : public AbilityForegroundStateObserverStub {
public:
    explicit JSAbilityForegroundStateObserver(napi_env engine);
    virtual ~JSAbilityForegroundStateObserver() = default;

    void OnAbilityStateChanged(const AbilityStateData &abilityStateData);
    void HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData);
    void CallJsFunction(const napi_value &value, const char *methodName, const napi_value *argv, const size_t argc);
    void AddJsObserverObject(const napi_value &jsObserverObject);
    void RemoveJsObserverObject(const napi_value &jsObserverObject);
    void RemoveAllJsObserverObject();
    std::shared_ptr<NativeReference> GetObserverObject(const napi_value &jsObserverObject);
    bool IsObserverObjectExsit(const napi_value &jsObserverObject);
    bool IsEmpty();
    void SetValid(bool valid);

private:
    napi_env env_;
    volatile bool valid_ = true;
    std::mutex mutexlock;
    std::set<std::shared_ptr<NativeReference>> jsObserverObjectSet_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_ABILITY_FOREGROUND_STATE_OBSERVER_H
