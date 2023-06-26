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

#ifndef OHOS_ABILITY_RUNTIME_SIMULAOTR_ABILITY_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_SIMULAOTR_ABILITY_CONTEXT_H

#include "foundation/ability/ability_runtime/frameworks/simulator/common/include/context.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityContext : public Context {
public:
    AbilityContext() = default;
    virtual ~AbilityContext() = default;

    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() override;

    Options GetOptions() override;
    void SetOptions(const Options& options) override;

private:
    Options options_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SIMULAOTR_ABILITY_CONTEXT_H
