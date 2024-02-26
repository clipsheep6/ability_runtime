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

#include "embedded_ui_extension_module_loader.h"

#include "embedded_ui_extension.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
EmbeddedUIExtensionModuleLoader::EmbeddedUIExtensionModuleLoader() = default;
EmbeddedUIExtensionModuleLoader::~EmbeddedUIExtensionModuleLoader() = default;

Extension *EmbeddedUIExtensionModuleLoader::Create(const std::unique_ptr<Runtime> &runtime) const
{
    HILOG_DEBUG("called");
    return EmbeddedUIExtension::Create(runtime);
}

std::map<std::string, std::string> EmbeddedUIExtensionModuleLoader::GetParams()
{
    HILOG_DEBUG("called");
    std::map<std::string, std::string> params;
    // type means extension type in ExtensionAbilityType of extension_ability_info.h, 21 means embedded_ui_extension.
    params.insert(std::pair<std::string, std::string>("type", "21"));
    params.insert(std::pair<std::string, std::string>("name", "EmbeddedUIExtensionAbility"));
    return params;
}

extern "C" __attribute__((visibility("default"))) void *OHOS_EXTENSION_GetExtensionModule()
{
    return &EmbeddedUIExtensionModuleLoader::GetInstance();
}
} // namespace AbilityRuntime
} // namespace OHOS
