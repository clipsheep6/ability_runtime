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

#ifndef OHOS_ABILITY_RUNTIME_ACTION_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_ACTION_EXTENSION_H

#include "ui_extension_base.h"

namespace OHOS {
namespace AbilityRuntime {
class UIExtensionContext;
class Runtime;
/**
 * @brief Action extension components.
 */
class ActionExtension : public UIExtensionBase<UIExtensionContext>,
                        public std::enable_shared_from_this<ActionExtension> {
public:
    ActionExtension() = default;
    virtual ~ActionExtension() = default;

    /**
     * @brief Create action extension.
     *
     * @param runtime The runtime.
     * @return The action extension instance.
     */
    static ActionExtension *Create(const std::unique_ptr<Runtime> &runtime);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ACTION_EXTENSION_H
