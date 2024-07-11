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
#include "freeze_util.h"
#include "js_runtime.h"
#include "context/context.h"

namespace OHOS {
namespace AbilityRuntime {
class UIServiceExtensionContext;
class Runtime;
using AbilityHandler = AppExecFwk::AbilityHandler;
using OHOSApplication = AppExecFwk::OHOSApplication;

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
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_SERVICE_EXTENSION_H
