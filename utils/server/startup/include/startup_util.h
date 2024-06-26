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

#ifndef OHOS_ABILITY_RUNTIME_STARTUP_UTIL_H
#define OHOS_ABILITY_RUNTIME_STARTUP_UTIL_H

#include <cstdint>

#include "ability_info.h"
#include "extension_ability_info.h"

namespace OHOS {
namespace AAFwk {
class Want;
}  // namespace AAFwk
namespace AppExecFwk {
enum class ExtensionAbilityType;
}  // namespace AppExecFwk
namespace AbilityRuntime {
class StartupUtil {
public:
    static bool GetAppIndex(const AAFwk::Want &want, int32_t &appIndex);
    static int32_t BuildAbilityInfoFlag();
    static bool IsSupportAppClone(AppExecFwk::ExtensionAbilityType type);
    static void InitAbilityInfoFromExtension(AppExecFwk::ExtensionAbilityInfo &extensionInfo,
        AppExecFwk::AbilityInfo &abilityInfo);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_STARTUP_UTIL_H