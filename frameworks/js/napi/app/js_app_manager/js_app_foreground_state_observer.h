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

#ifndef OHOS_ABILITY_RUNTIME_JS_APP_FOREGROUND_STATE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_JS_APP_FOREGROUND_STATE_OBSERVER_H

#include <mutex>
#include <set>

#include "app_foreground_state_observer_stub.h"
#include "event_handler.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
using OHOS::AppExecFwk::AppForegroundStateObserverStub;
using OHOS::AppExecFwk::AppStateData;
class JSAppForegroundStateObserver : public AppForegroundStateObserverStub {
public:
    explicit JSAppForegroundStateObserver(napi_env engine);
    virtual ~JSAppForegroundStateObserver() = default;
    void OnAppStateChanged(const AppStateData &appStateData);
    void HandleOnAppStateChanged(const AppStateData &appStateData);
    void CallJsFunction(const napi_value value, const char *methodName, const napi_value *argv, const size_t argc);
    void AddJsObserverObject(const napi_value &jsObserverObject);
    void RemoveJsObserverObject(const napi_value &jsObserverObject);
    void RemoveAllJsObserverObjects();
    std::shared_ptr<NativeReference> GetObserverObject(const napi_value &jsObserverObject);
    bool IsEmpty();
    void SetValid(bool valid);

private:
    napi_env env_;
    volatile bool valid_ = true;
    std::set<std::shared_ptr<NativeReference>> jsObserverObjectSet_;
    std::mutex jsObserverObjectSetLock_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_APP_FOREGROUND_STATE_OBSERVER_H
