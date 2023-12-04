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

#include "auto_fill_extension_module_loader.h"

#include "auto_fill_extension.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string AUTO_FILL_EXTENSION_PARAMS_NAME_KEY = "name";
const std::string AUTO_FILL_EXTENSION_PARAMS_NAME = "AutoFillExtensionAbility";
const std::string AUTO_FILL_EXTENSION_PARAMS_TYPE_KEY = "type";
const std::string AUTO_FILL_EXTENSION_PARAMS_TYPE = "501";
}
AutoFillExtensionModuleLoader::AutoFillExtensionModuleLoader() = default;
AutoFillExtensionModuleLoader::~AutoFillExtensionModuleLoader() = default;

Extension *AutoFillExtensionModuleLoader::Create(const std::unique_ptr<Runtime> &runtime) const
{
    HILOG_DEBUG("Called");
    return AutoFillExtension::Create(runtime);
}

std::map<std::string, std::string> AutoFillExtensionModuleLoader::GetParams()
{
    HILOG_DEBUG("Called");
    std::map<std::string, std::string> params;
    // type means extension type in ExtensionAbilityType of
    // extension_ability_info.h, 21 means autoFill/password extension.
    params.insert(
        std::pair<std::string, std::string>(AUTO_FILL_EXTENSION_PARAMS_TYPE_KEY, AUTO_FILL_EXTENSION_PARAMS_TYPE));
    params.insert(
        std::pair<std::string, std::string>(AUTO_FILL_EXTENSION_PARAMS_NAME_KEY, AUTO_FILL_EXTENSION_PARAMS_NAME));
    return params;
}

extern "C" __attribute__((visibility("default"))) void *OHOS_EXTENSION_GetExtensionModule()
{
    return &AutoFillExtensionModuleLoader::GetInstance();
}
} // namespace AbilityRuntime
} // namespace OHOS
