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

#include "startup_utils.h"

#include <map>

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::map<int32_t, std::string> ERR_MSG_MAP = {
    { ERR_STARTUP_INVALID_VALUE,                    "invalid parameter." },
    { ERR_STARTUP_INTERNAL_ERROR,                   "internal error." },
    { ERR_STARTUP_DEPENDENCY_NOT_FOUND,             "startup task or its dependency not found." },
    { ERR_STARTUP_CIRCULAR_DEPENDENCY,              "the startup tasks have circular dependencies." },
    { ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,        "an error occurred while running the startup tasks." },
    { ERR_STARTUP_TIMEOUT,                          "running startup tasks timeout." },
};
}

std::string StartupUtils::GetErrorMessage(int32_t errCode)
{
    auto iter = ERR_MSG_MAP.find(errCode);
    if (iter == ERR_MSG_MAP.end()) {
        return ERR_MSG_MAP.at(ERR_STARTUP_INTERNAL_ERROR);
    }
    return iter->second;
}
} // namespace AbilityRuntime
} // namespace OHOS
