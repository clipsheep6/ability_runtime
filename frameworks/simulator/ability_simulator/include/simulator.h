/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_SIMULAOTR_H
#define OHOS_ABILITY_RUNTIME_SIMULAOTR_H

#include <functional>
#include <memory>
#include <string>

namespace OHOS {
namespace AbilityRuntime {
class Simulator {
public:
    struct Options {
    };

    using TerminateCallback = std::function<void(int64_t)>;
    using FormUpdateCallback = std::function<void(int64_t, const std::string&)>;

    static std::unique_ptr<Simulator> Create(const Options& options);

    virtual ~Simulator() = default;

    virtual int64_t StartAbility(const std::string& abilitySrcPath, TerminateCallback callback) = 0;
    virtual void TerminateAbility(int64_t abilityId) = 0;

    virtual int64_t CreateForm(const std::string& formSrcPath, FormUpdateCallback callback) = 0;
    virtual void RequestUpdateForm(int64_t formId) = 0;
    virtual void DestroyForm(int64_t formId) = 0;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SIMULAOTR_H
