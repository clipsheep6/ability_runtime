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

#ifndef OHOS_ABILITY_RUNTIME_JS_ACTION_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_JS_ACTION_EXTENSION_H

#include "action_extension.h"
#include "configuration.h"

namespace OHOS {
namespace AbilityRuntime {
class ActionExtension;
class JsRuntime;
class JsUIExtensionBase;
/**
 * @brief Basic action extension components.
 */
class JsActionExtension : public ActionExtension, public std::enable_shared_from_this<JsActionExtension> {
public:
    explicit JsActionExtension(const std::unique_ptr<Runtime> &runtime);
    virtual ~JsActionExtension() override;

    /**
     * @brief Create JsActionExtension.
     *
     * @param runtime The runtime.
     * @return The JsActionExtension instance.
     */
    static JsActionExtension *Create(const std::unique_ptr<Runtime> &runtime);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_ACTION_EXTENSION_H
