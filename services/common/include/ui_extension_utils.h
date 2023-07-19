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

#ifndef OHOS_AAFwk_UI_EXTENSION_UTILS_H
#define OHOS_AAFwk_UI_EXTENSION_UTILS_H

#include <unordered_set>
#include "extension_ability_info.h"

namespace OHOS {
namespace AAFwk {
namespace UIExtensionUtils {
// ui extension type list
const std::unordered_set<AppExecFwk::ExtensionAbilityType> UI_EXTENSION_SET = {
    AppExecFwk::ExtensionAbilityType::UI,
    AppExecFwk::ExtensionAbilityType::SYSPICKER_MEDIACONTROL,
    AppExecFwk::ExtensionAbilityType::SYSDIALOG_USERAUTH,
    AppExecFwk::ExtensionAbilityType::SYSDIALOG_ATOMICSERVICEPANEL
};

inline bool IsUIExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return UI_EXTENSION_SET.find(type) != UI_EXTENSION_SET.end();
}

inline bool IsWindowExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return type == AppExecFwk::ExtensionAbilityType::WINDOW;
}
} // namespace UIExtensionUtils
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_AAFwk_UI_EXTENSION_UTILS_H