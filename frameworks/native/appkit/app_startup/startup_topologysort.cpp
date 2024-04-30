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

#include "hilog_tag_wrapper.h"
#include "startup_topologysort.h"

namespace OHOS {
namespace AbilityRuntime {
int32_t StartupTopologySort::Sort(const std::map<std::string, std::shared_ptr<StartupTask>> &startupMap,
    std::shared_ptr<StartupSortResult> &startupSortResult)
{
    startupSortResult = std::make_shared<StartupSortResult>();
    if (startupSortResult == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "Create StartupSortResult obj fail.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    std::deque<std::string> zeroDeque;
    std::map<std::string, std::uint32_t> inDegreeMap;
    for (auto &iter : startupMap) {
        if (iter.second == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "StartupTask is nullptr.");
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        int32_t result = SortZeroDeque(iter.second, startupMap, inDegreeMap, zeroDeque, startupSortResult);
        if (result != ERR_OK) {
            return result;
        }
    }

    uint32_t mainStartupCount = 0;
    uint32_t threadStartupCount = 0;
    while (!zeroDeque.empty()) {
        std::string key = zeroDeque.front();
        zeroDeque.pop_front();
        auto it = startupMap.find(key);
        if (it == startupMap.end()) {
            TAG_LOGE(AAFwkTag::STARTUP, "startup not found: %{public}s", key.c_str());
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        if (it->second->GetCallCreateOnMainThread()) {
            mainStartupCount++;
        } else {
            threadStartupCount++;
        }

        std::vector<std::string> &childrenStartupVector = startupSortResult->startupChildrenMap_[key];
        for (auto &child : childrenStartupVector) {
            inDegreeMap[child]--;
            if (inDegreeMap[child] == 0) {
                zeroDeque.push_back(child);
            }
        }
    }

    if (mainStartupCount + threadStartupCount != startupMap.size()) {
        TAG_LOGE(AAFwkTag::STARTUP, "circular dependency, main: %{public}u, thread: %{public}u, startupMap %{public}zu",
            mainStartupCount, threadStartupCount, startupMap.size());
        return ERR_STARTUP_CIRCULAR_DEPENDENCY;
    }
    TAG_LOGD(AAFwkTag::STARTUP, "main: %{public}u, thread: %{public}u", mainStartupCount, threadStartupCount);
    return ERR_OK;
}

int32_t StartupTopologySort::SortZeroDeque(const std::shared_ptr<StartupTask> &startup,
    const std::map<std::string, std::shared_ptr<StartupTask>> &startupMap,
    std::map<std::string, std::uint32_t> &inDegreeMap, std::deque<std::string> &zeroDeque,
    std::shared_ptr<StartupSortResult> &startupSortResult)
{
    std::string key = startup->GetName();
    auto result = inDegreeMap.emplace(key, startup->getDependenciesCount());
    if (!result.second) {
        TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, failed to emplace to inDegreeMap.", key.c_str());
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    std::vector<std::string> dependencies = startup->GetDependencies();
    if (dependencies.empty()) {
        zeroDeque.push_back(key);
        startupSortResult->zeroDequeResult_.push_back(key);
    } else {
        for (auto &parentName : dependencies) {
            if (startupMap.find(parentName) == startupMap.end()) {
                TAG_LOGE(AAFwkTag::STARTUP,
                    "%{public}s, failed to find dep: %{public}s.", key.c_str(), parentName.c_str());
                return ERR_STARTUP_DEPENDENCY_NOT_FOUND;
            }
            auto &childStartVector = startupSortResult->startupChildrenMap_[parentName];
            childStartVector.push_back(key);
        }
    }
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
