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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_STATE_CHANGE_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_STATE_CHANGE_CALLBACK_H

#include <memory>
#include <set>

class NativeEngine;
class NativeValue;
class NativeReference;
struct NativeCallbackInfo;

namespace OHOS {
namespace AbilityRuntime {
class ApplicationStateChangeCallback {
public:
    virtual ~ApplicationStateChangeCallback() {}

    /**
     * Called back when the application in foreground.
     *
     * @since 10
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     */
    virtual void NotifyApplicationForeground() = 0;

    /**
     * Called back when the application in background.
     *
     * @since 10
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     */
    virtual void NotifyApplicationBackground() = 0;
};

class JsApplicationStateChangeCallback : public ApplicationStateChangeCallback,
    public std::enable_shared_from_this<JsApplicationStateChangeCallback> {
public:
    explicit JsApplicationStateChangeCallback(NativeEngine* engine);
    virtual ~JsApplicationStateChangeCallback() = default;
    void NotifyApplicationForeground() override;
    void NotifyApplicationBackground() override;
    void Register(NativeValue *jsCallback);

    /**
     * @brief Unregister application state change callback.
     * @param jsCallback, if jscallback is nullptr, delete all register jscallback.
     *                    or if jscallback is specified, delete prescribed jscallback.
     * @return Returns true on unregister success, others return false.
     */
    bool UnRegister(NativeValue *jsCallback = nullptr);
    bool IsEmpty() const;
private:
    void CallJsMethodInnerCommon(
        const std::string &methodName, const std::set<std::shared_ptr<NativeReference>> callbacks);
    void CallJsMethod(const std::string &methodName);
    void CallApplicationForegroundInner(const std::string &methodName);
    NativeEngine* engine_ = nullptr;
    std::set<std::shared_ptr<NativeReference>> callbacks_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APPLICATION_STATE_CHANGE_CALLBACK_H
