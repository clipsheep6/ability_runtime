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

#ifndef OHOS_ABILITY_RUNTIME_JS_ABILITY_AUTO_STARTUP_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_JS_ABILITY_AUTO_STARTUP_CALLBACK_H

#include <chrono>
#include <iremote_object.h>

#include "auto_startup_callback_stub.h"
#include "auto_startup_info.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "parcel.h"

class NativeReference;
class NativeValue;
namespace OHOS {
namespace AbilityRuntime {
/**
 *
 * @class JsAbilityAutoStartupCallBack
 */
class JsAbilityAutoStartupCallBack : public AutoStartupCallBackStub {
public:
    explicit JsAbilityAutoStartupCallBack(NativeEngine &engine);
    virtual ~JsAbilityAutoStartupCallBack();
    void Register(NativeValue *jsCallback);
    void UnRegister(NativeValue *jsCallback);
    void OnAutoStartupOn(const AutoStartupInfo &info) override;
    void OnAutoStartupOff(const AutoStartupInfo &info) override;
    bool IsCallbacksEmpty();

private:
    void JSCallFunction(const AutoStartupInfo &info, const std::string &methodName);
    bool IsJsCallbackEquals(std::shared_ptr<NativeReference> callback, NativeValue *jsCallback);

    NativeEngine &engine_;
    std::vector<std::shared_ptr<NativeReference>> callbacks_;
    std::mutex mutexlock;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_ABILITY_AUTO_STARTUP_CALLBACK_H
