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

#ifndef OHOS_ABILITY_RUNTIME_UI_SERVICE_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_UI_SERVICE_EXTENSION_H

#include "extension_base.h"
#include "js_runtime.h"
#include "ability_lifecycle_executor.h"
#ifdef SUPPORT_GRAPHICS
#include "window_scene.h"
#endif


class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
class UIServiceExtensionContext;
class Runtime;
class UIServiceExtension;

/**
 * @brief Basic ui extension components.
 */
class UIServiceExtension : public ExtensionBase<UIServiceExtensionContext> {
public:
    UIServiceExtension() = default;
    virtual ~UIServiceExtension() = default;

    /**
     * @brief Create and init context.
     *
     * @param record the ui service extension record.
     * @param application the application info.
     * @param handler the ui service extension handler.
     * @param token the remote token.
     * @return The created context.
     */
    virtual std::shared_ptr<UIServiceExtensionContext> CreateAndInitContext(
        const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Init the ui service extension.
     *
     * @param record the ui service extension record.
     * @param application the application info.
     * @param handler the ui service extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Create ui service extension.
     *
     * @param runtime The runtime.
     * @return The ui service extension instance.
     */
    static UIServiceExtension* Create(const std::unique_ptr<Runtime>& runtime);

    /**
     * @brief Obtains the class name in this ability name, without the prefixed bundle name.
     * @return Returns the class name of this ability.
     */
    std::string GetAbilityName();

    /**
     * @brief Obtains the module name in this ability name, without the prefixed bundle name.
     * @return Returns the module name of this ability.
     */
    std::string GetModuleName();
    AppExecFwk::AbilityLifecycleExecutor::LifecycleState GetState();

#ifdef SUPPORT_GRAPHICS
    /**
     * @brief get the scene belong to the ability.
     * @return Returns a WindowScene object pointer.
     */
    std::shared_ptr<Rosen::WindowScene> GetScene();
#endif
protected:
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo_ = nullptr;
#ifdef SUPPORT_GRAPHICS
    std::shared_ptr<Rosen::WindowScene> scene_ = nullptr;
#endif
private:
    std::shared_ptr<AppExecFwk::AbilityLifecycleExecutor> abilityLifecycleExecutor_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_SERVICE_EXTENSION_H
