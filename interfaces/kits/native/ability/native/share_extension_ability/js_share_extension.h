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

#ifndef OHOS_ABILITY_RUNTIME_JS_SHARE_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_JS_SHARE_EXTENSION_H

#include "configuration.h"
#include "share_extension.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
class ShareExtension;
class JsRuntime;
class JsUIExtensionBase;
/**
 * @brief Basic share extension components.
 */
class JsShareExtension : public ShareExtension, public std::enable_shared_from_this<JsShareExtension> {
public:
    explicit JsShareExtension(const std::unique_ptr<Runtime> &runtime);
    virtual ~JsShareExtension() override;

    /**
     * @brief Create JsShareExtension.
     *
     * @param runtime The runtime.
     * @return The JsShareExtension instance.
     */
    static JsShareExtension *Create(const std::unique_ptr<Runtime> &runtime);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_SHARE_EXTENSION_H