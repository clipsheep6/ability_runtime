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

#ifndef OHOS_ABILITY_RUNTIME_STARTUP_TOPOLOGY_SORT_H
#define OHOS_ABILITY_RUNTIME_STARTUP_TOPOLOGY_SORT_H

#include <deque>
#include <map>
#include <string>
#include <vector>

#include "hilog_wrapper.h"
#include "startup_task.h"
#include "startup_sort_result.h"
#include "startup_utils.h"

namespace OHOS {
namespace AbilityRuntime {
class StartupTopologySort {
public:
    StartupTopologySort() {};
    ~StartupTopologySort() = default;

    static int32_t Sort(const std::map<std::string, std::shared_ptr<StartupTask>> &startupMap,
        std::shared_ptr<StartupSortResult> &startupSortResult);

private:
    static int32_t SortZeroDeque(const std::shared_ptr<StartupTask> &startup,
        const std::map<std::string, std::shared_ptr<StartupTask>> &startupMap,
        std::map<std::string, std::uint32_t> &inDegreeMap, std::deque<std::string> &zeroDeque,
        std::shared_ptr<StartupSortResult> &startupSortResult);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STARTUP_TOPOLOGY_SORT_H
