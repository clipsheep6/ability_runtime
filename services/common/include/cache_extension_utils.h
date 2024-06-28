/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_AAFWK_CACHE_EXTENSION_UTILS_H
#define OHOS_AAFWK_CACHE_EXTENSION_UTILS_H

#include <unordered_set>

#include "app_utils.h"
#include "extension_ability_info.h"

namespace OHOS {
namespace AAFwk {
namespace CacheExtensionUtils {
constexpr const int32_t BASE_TEN = 10;

// cache extension type list
std::unordered_set<AppExecFwk::ExtensionAbilityType> GetCacheExtensionTypeList()
{
    std::unordered_set<AppExecFwk::ExtensionAbilityType> cacheExtTypeList;
    auto cacheExtTypeListStr = AppUtils::GetInstance().GetCacheExtensionTypeList();
    if (cacheExtTypeListStr.empty()) {
        return cacheExtTypeList;
    }
    std::vector<std::string> cacheExtTypeListVec;
    SplitStr(cacheExtTypeListStr, ";", cacheExtTypeListVec);
    for (auto it = cacheExtTypeListVec.begin(); it != cacheExtTypeListVec.end(); it++) {
        cacheExtTypeList.insert(
            static_cast<AppExecFwk::ExtensionAbilityType>(std::strtol((*it).c_str(), nullptr, BASE_TEN)));
    }
    return cacheExtTypeList;
}

inline bool IsCacheExtensionType(const AppExecFwk::ExtensionAbilityType type)
{
    return GetCacheExtensionTypeList().count(type) > 0;
}
} // namespace CacheExtensionUtils
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_AAFWK_CACHE_EXTENSION_UTILS_H
