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

#include "datashare_extension_module_loader.h"
#include "datashare_extension.h"

namespace OHOS::AbilityRuntime {
DataShareExtensionModuleLoader::DataShareExtensionModuleLoader() = default;
DataShareExtensionModuleLoader::~DataShareExtensionModuleLoader() = default;

Extension *DataShareExtensionModuleLoader::Create(const std::unique_ptr<Runtime>& runtime) const
{
    return DataShareExtension::Create(runtime);
}

extern "C" __attribute__((visibility("default"))) void* OHOS_EXTENSION_GetExtensionModule()
{
    return &DataShareExtensionModuleLoader::GetInstance();
}
} // namespace OHOS::AbilityRuntime