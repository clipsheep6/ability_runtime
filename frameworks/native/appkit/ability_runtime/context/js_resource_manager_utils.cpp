/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "js_resource_manager_utils.h"

#include "js_runtime_utils.h"
#include "resource_manager_addon.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJsResourceManager(napi_env env,
    std::shared_ptr<Global::Resource::ResourceManager> &resourceManager, std::shared_ptr<Context> context)
{
    return Global::Resource::ResourceManagerAddon::Create(env, "", resourceManager, context);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
