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

#ifndef OHOS_ABILITY_RUNTIME_JS_FREE_INSTALL_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_JS_FREE_INSTALL_OBSERVER_H

#include <memory>
#include <string>
#include <vector>

#include "native_engine/native_engine.h"
#include "free_install_observer_stub.h"

namespace OHOS {
namespace AbilityRuntime {
struct JsFreeInstallObserverObject {
    std::string bundleName;
    std::string abilityName;
    std::string startTime;
    std::shared_ptr<NativeReference> callback;
    bool isAbilityResult = false;
};

class JsFreeInstallObserver : public FreeInstallObserverStub {
public:
    explicit JsFreeInstallObserver(NativeEngine& engine);
    ~JsFreeInstallObserver();

    /**
     * OnInstallFinished, return free install result.
     *
     * @param bundleName Free install bundleName.
     * @param abilityName Free install abilityName.
     * @param startTime Free install start request time.
     * @param resultCode The result of this free install.
     */
    void OnInstallFinished(const std::string &bundleName, const std::string &abilityName,
        const std::string &startTime, const int &resultCode) override;

    /**
     * @brief Use for context to add an callback into the observer.
     *
     * @param bundleName The bundleName of want.
     * @param abilityName The abilityName of want.
     * @param startTime The startTime that want requested.
     * @param jsObserverObject The js object instance.
     */
    void AddJsObserverObject(const std::string &bundleName, const std::string &abilityName,
        const std::string &startTime, NativeValue* jsObserverObject, bool isAbilityResult = false);
private:
    void CallJsFunction(NativeValue* value, NativeValue* const *argv, size_t argc);
    void HandleOnInstallFinished(const std::string &bundleName, const std::string &abilityName,
        const std::string &startTime, const int &resultCode);
    NativeEngine& engine_;
    std::vector<JsFreeInstallObserverObject> jsObserverObjectList_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_FREE_INSTALL_OBSERVER_H