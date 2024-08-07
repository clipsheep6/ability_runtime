/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
 
#include "live_form_extension_module_loader.h"
 
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "live_form_extension.h"
 
namespace OHOS {
namespace AbilityRuntime {
LiveFormExtensionModuleLoader::LiveFormExtensionModuleLoader() = default;
LiveFormExtensionModuleLoader::~LiveFormExtensionModuleLoader() = default;
 
Extension *LiveFormExtensionModuleLoader::Create(const std::unique_ptr<Runtime> &runtime) const
{
    TAG_LOGD(AAFwkTag::LIVE_FORM_EXT, "called");
    return LiveFormExtension::Create(runtime);
}
 
std::map<std::string, std::string> LiveFormExtensionModuleLoader::GetParams()
{
    TAG_LOGD(AAFwkTag::LIVE_FORM_EXT, "called");
    std::map<std::string, std::string> params;
    // type means extension type in ExtensionAbilityType of
    // extension_ability_info.h, 24 means liveformextension.
    params.insert(std::pair<std::string, std::string>("type", "24"));
    params.insert(std::pair<std::string, std::string>("name", "LiveFormExtensionAbility"));
    return params;
}
 
extern "C" __attribute__((visibility("default"))) void *OHOS_EXTENSION_GetExtensionModule()
{
    return &LiveFormExtensionModuleLoader::GetInstance();
}
} // namespace AbilityRuntime
} // namespace OHOS