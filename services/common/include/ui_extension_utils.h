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

#ifndef OHOS_AAFwk_UI_EXTENSION_UTILS_H
#define OHOS_AAFwk_UI_EXTENSION_UTILS_H

#include <unordered_set>

#include "extension_ability_info.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AAFwk {
namespace UIExtensionUtils {
// ui extension type list
    std::unordered_set<AppExecFwk::ExtensionAbilityType> GetUiExtensionSet();
    bool IsUIExtension(const AppExecFwk::ExtensionAbilityType type);
    bool IsSystemUIExtension(const AppExecFwk::ExtensionAbilityType type);
// In this case, extension which be starting needs that caller should be the system app, otherwise not supported.
    bool IsSystemCallerNeeded(const AppExecFwk::ExtensionAbilityType type);
// In this collection, extension can be embedded by public app, which requires vertical businesses to ensure security.
    bool IsPublicForEmbedded(const AppExecFwk::ExtensionAbilityType type);
// In this collection, extension can be embedded by public app, which some UX effects are constrained
    bool IsPublicForConstrainedEmbedded(const AppExecFwk::ExtensionAbilityType type);
    bool IsEnterpriseAdmin(const AppExecFwk::ExtensionAbilityType type);
    bool IsWindowExtension(const AppExecFwk::ExtensionAbilityType type);
} // namespace UIExtensionUtils
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_AAFwk_UI_EXTENSION_UTILS_H
