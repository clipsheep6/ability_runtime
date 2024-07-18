/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_SIMULATOR_H
#define OHOS_ABILITY_RUNTIME_SIMULATOR_H

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "configuration.h"
#include "options.h"

#ifdef _WIN32
#define ABILITY_EXPORT __attribute__((dllexport))
#else
#define ABILITY_EXPORT __attribute__((visibility("default")))
#endif
namespace OHOS {
namespace AbilityRuntime {
class ABILITY_EXPORT Simulator {
public:
    using TerminateCallback = std::function<void(int64_t)>;
    using FormUpdateCallback = std::function<void(int64_t, const std::string&)>;
    using ResolveBufferTrackerCallback = std::function<bool(
        const std::string&, uint8_t **, size_t *, std::string& errorMsg)>;

    /**
     * Create a simulator instance.
     *
     * @param options The simulator options.
     */
    static std::shared_ptr<Simulator> Create(const Options &options);

    virtual ~Simulator() = default;

    virtual int64_t StartAbility(
        const std::string &abilitySrcPath, TerminateCallback callback, const std::string &abilityName = "") = 0;
    virtual void TerminateAbility(int64_t abilityId) = 0;
    virtual void UpdateConfiguration(const AppExecFwk::Configuration &config) = 0;
    virtual void SetMockList(const std::map<std::string, std::string> &mockList) = 0;
    virtual void SetHostResolveBufferTracker(ResolveBufferTrackerCallback cb) = 0;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SIMULATOR_H
