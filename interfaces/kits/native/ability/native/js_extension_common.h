/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_EXTENSION_COMMON_H
#define OHOS_ABILITY_RUNTIME_JS_EXTENSION_COMMON_H

#include "configuration.h"
#include "service_extension.h"

class NativeReference;
class NativeValue;
class NativeObject;

namespace OHOS {
namespace AbilityRuntime {
class ServiceExtension;
class JsRuntime;
/**
 * @brief Basic js extension common components.
 */
class JsExtensionCommon : public ExtensionCommon,
                            public std::enable_shared_from_this<JsExtensionCommon> {
public:
    JsExtensionCommon(JsRuntime &jsRuntime, NativeReference &jsObj,
        const std::shared_ptr<NativeReference> &shellContextRef);

    virtual ~JsExtensionCommon() override;

    /**
     * @brief Create JsServiceExtension.
     *
     * @param jsRuntime The runtime.
     * @param jsObj The js object instance.
     * @return The JsServiceExtension instance.
     */
    static std::shared_ptr<JsExtensionCommon> Create(JsRuntime &jsRuntime, NativeReference &jsObj,
        const std::shared_ptr<NativeReference> &shellContextRef);

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration> &fullConfig) override;

    /**
     * @brief Notify current memory level.
     *
     * @param level Current memory level.
     */
    virtual void OnMemoryLevel(int level) override;

private:
    NativeValue* CallObjectMethod(const char* name, NativeValue* const * argv, size_t argc);

private:
    JsRuntime& jsRuntime_;
    NativeReference& jsObj_;
    std::shared_ptr<NativeReference> shellContextRef_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_EXTENSION_COMMON_H
