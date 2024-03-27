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


#include "startup_topologysort.h"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<StartupSortResult> StartupTopologySort::Sort(std::vector<std::shared_ptr<StartupTask>> vectors)
{
    std::shared_ptr<StartupSortResult> startupSortResult = std::make_shared<StartupSortResult>();
    if (startupSortResult == nullptr) {
        HILOG_ERROR("Create StartupSortResult obj fail.");
        return nullptr;
    }
    std::deque<std::string> zeroDeque;
    std::map<std::string, std::uint32_t> inDegreeMap;
    for (auto up : vectors) {
        if (up == nullptr) {
            HILOG_ERROR("StartupTask is nullptr.");
            return nullptr;
        }
        if (SortZeroDeque(up, inDegreeMap, zeroDeque, startupSortResult) != ERR_OK) {
            HILOG_ERROR("StartupTask is add.");
            return nullptr;
        }
    }

    uint32_t mainStartupCount = 0;
    uint32_t threadStartupCount = 0;
    while (!zeroDeque.empty()) {
        std::string key = zeroDeque.front();
        zeroDeque.pop_front();
        auto it = startupSortResult->startupMap_.find(key);
        if (it != startupSortResult->startupMap_.end()) {
            if (it->second->GetCallCreateOnMainThread()) {
                mainStartupCount++;
            } else {
                threadStartupCount++;
            }
        } else {
            HILOG_ERROR("Dependency does not exist.");
            return nullptr;
        }

        std::vector<std::string> &childStartVector = startupSortResult->startupChildrenMap_[key];
        for (std::string dep: childStartVector) {
            inDegreeMap[dep]--;
            if (inDegreeMap[dep] == 0) {
                zeroDeque.push_back(dep);
            }
        }
    }

    if (mainStartupCount + threadStartupCount != vectors.size()) {
        HILOG_ERROR("Circle dependencies.");
        return nullptr;
    }
    return startupSortResult;
}

int32_t StartupTopologySort::SortZeroDeque(const std::shared_ptr<StartupTask> &startup,
    std::map<std::string, std::uint32_t> &inDegreeMap,
    std::deque<std::string> &zeroDeque, std::shared_ptr<StartupSortResult> &startupSortResult)
{
    std::string key = startup->GetName();
    auto it = startupSortResult->startupMap_.find(key);
    if (it == startupSortResult->startupMap_.end()) {
        startupSortResult->startupMap_.emplace(key, startup);
        inDegreeMap.emplace(key, startup->getDependenceCount());

        std::vector<std::string> depenedcies = startup->GetDependencies();
        if (depenedcies.empty()) {
            zeroDeque.push_back(key);
            startupSortResult->zeroDequeResult_.push_back(key);
        } else {
            for (auto parentName : depenedcies) {
                auto &childStartVector = startupSortResult->startupChildrenMap_[parentName];
                childStartVector.push_back(key);
            }
        }
        return ERR_OK;
    }  else {
        HILOG_ERROR("StartupTask is add.");
        return ERR_INVALID_VALUE;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
