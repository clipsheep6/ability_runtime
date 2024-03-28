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

#ifndef OHOS_ABILITY_RUNTIME_JS_DEMO_UI_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_JS_DEMO_UI_EXTENSION_H

#include "demo_ui_extension.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
class JsDemoUIExtension : public DemoUIExtension,
                          public std::enable_shared_from_this<JsDemoUIExtension> {
public:
    explicit JsDemoUIExtension(const std::unique_ptr<Runtime> &runtime);
    ~JsDemoUIExtension() override;

    /**
     * @brief Create JsDemoUIExtension.
     *
     * @param runtime The runtime.
     * @return The JsDemoUIExtension instance.
     */
    static JsDemoUIExtension *Create(const std::unique_ptr<Runtime> &runtime);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_DEMO_UI_EXTENSION_H
