/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_SERVICE_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_SERVICE_EXTENSION_H

#ifdef SUPPORT_GRAPHICS
#include "display_manager.h"
#endif
#include "extension_base.h"

namespace OHOS {
namespace AbilityRuntime {
class ServiceExtensionContext;
class Runtime;
class ServiceExtension;
using CreatorFunc = std::function<ServiceExtension* (const std::unique_ptr<Runtime>& runtime)>;
/**
 * @brief Basic service components.
 */
class ServiceExtension : public ExtensionBase<ServiceExtensionContext> {
public:
    ServiceExtension() = default;
    virtual ~ServiceExtension() = default;

    /**
     * @brief Create and init context.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     * @return The created context.
     */
    virtual std::shared_ptr<ServiceExtensionContext> CreateAndInitContext(
        const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Init the extension.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Create Extension.
     *
     * @param runtime The runtime.
     * @return The ServiceExtension instance.
     */
    static ServiceExtension* Create(const std::unique_ptr<Runtime>& runtime);

    /**
     * @brief Set a creator function.
     *
     * @param creator The function for create a service-extension ability.
     */
    static void SetCreator(const CreatorFunc& creator);

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdated(const AppExecFwk::Configuration &configuration) override;

    /**
     * @brief Called when start current service extension.
     *
     * @param want Indicates parameters carried with start.
     */
    void OnStart(const AAFwk::Want &want) override;

    /**
     * @brief Called when configuration changed, including system configuration and window configuration.
     *
     */
    virtual void ConfigurationUpdated();

#ifdef SUPPORT_GRAPHICS
protected:
    class ServiceExtensionDisplayListener : public Rosen::DisplayManager::IDisplayListener {
    public:
        explicit ServiceExtensionDisplayListener(const std::weak_ptr<ServiceExtension>& serviceExtension)
        {
            serviceExtension_ = serviceExtension;
        }

        void OnCreate(Rosen::DisplayId displayId) override
        {
            auto sptr = serviceExtension_.lock();
            if (sptr != nullptr) {
                sptr->OnCreate(displayId);
            }
        }

        void OnDestroy(Rosen::DisplayId displayId) override
        {
            auto sptr = serviceExtension_.lock();
            if (sptr != nullptr) {
                sptr->OnDestroy(displayId);
            }
        }

        void OnChange(Rosen::DisplayId displayId) override
        {
            auto sptr = serviceExtension_.lock();
            if (sptr != nullptr) {
                sptr->OnChange(displayId);
            }
        }

    private:
        std::weak_ptr<ServiceExtension> serviceExtension_;
    };

    void OnCreate(Rosen::DisplayId displayId);
    void OnDestroy(Rosen::DisplayId displayId);
    void OnChange(Rosen::DisplayId displayId);

private:
    sptr<ServiceExtensionDisplayListener> displayListener_ = nullptr;
#endif

private:
    static CreatorFunc creator_;
    std::shared_ptr<AbilityHandler> handler_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_SERVICE_EXTENSION_H
