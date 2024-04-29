/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_ABILITY_FIRST_FRAME_STATE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_JS_ABILITY_FIRST_FRAME_STATE_OBSERVER_H

#ifdef SUPPORT_GRAPHICS
#include <mutex>
#include <set>

#include "ability_first_frame_state_observer_stub.h"
#include "ability_manager_interface.h"
#include "singleton.h"
#include "event_handler.h"
#include "native_engine/native_engine.h"
#include "ability_first_frame_state_data.h"

namespace OHOS {
namespace AbilityRuntime {
using OHOS::AppExecFwk::AbilityFirstFrameStateObserverStub;
using OHOS::AppExecFwk::AbilityFirstFrameStateData;
class JSAbilityFirstFrameStateObserver : public AbilityFirstFrameStateObserverStub {
public:
    explicit JSAbilityFirstFrameStateObserver(napi_env engine);
    virtual ~JSAbilityFirstFrameStateObserver() = default;
    void OnAbilityFirstFrameState(const AbilityFirstFrameStateData &abilityFirstFrameStateData) override;
    void HandleOnAbilityFirstFrameState(const AbilityFirstFrameStateData &abilityFirstFrameStateData);
    void CallJsFunction(const napi_value value, const char *methodName, const napi_value *argv, const size_t argc);
    void SetJsObserverObject(const napi_value &jsObserverObject);
    void ResetJsObserverObject();
    std::shared_ptr<NativeReference> GetNativeReference();

private:
    napi_env env_;
    std::shared_ptr<NativeReference> jsObserverObject_;
};

class JSAbilityFirstFrameStateObserverManager {
public:
    static JSAbilityFirstFrameStateObserverManager *GetInstance()
    {
        static JSAbilityFirstFrameStateObserverManager instance;
        return &instance;
    }
    ~JSAbilityFirstFrameStateObserverManager() = default;
    void Init(napi_env env);
    void AddJSAbilityFirstFrameStateObserver(const sptr<JSAbilityFirstFrameStateObserver> observer);
    bool IsObserverObjectExist(const napi_value &jsObserverObject);
    void RemoveAllJsObserverObjects(sptr<OHOS::AAFwk::IAbilityManager> &abilityManager);
    void RemoveJsObserverObject(sptr<OHOS::AAFwk::IAbilityManager> &abilityManager,
        const napi_value &jsObserverObject);

private:
    JSAbilityFirstFrameStateObserverManager() = default;
    DISALLOW_COPY_AND_MOVE(JSAbilityFirstFrameStateObserverManager);
    std::shared_ptr<NativeReference> GetObserverObject(const napi_value &jsObserverObject);

private:
    napi_env env_;
    std::mutex observerListLock_;
    std::list<sptr<JSAbilityFirstFrameStateObserver>> jsAbilityFirstFrameStateObserverList_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // SUPPORT_GRAPHICS
#endif // OHOS_ABILITY_RUNTIME_JS_ABILITY_FIRST_FRAME_STATE_OBSERVER_H
