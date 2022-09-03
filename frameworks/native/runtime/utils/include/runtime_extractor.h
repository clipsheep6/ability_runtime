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

#ifndef OHOS_ABILITY_RUNTIME_RUNTIME_EXTRACTOR_H
#define OHOS_ABILITY_RUNTIME_RUNTIME_EXTRACTOR_H

#include "base_extractor.h"

namespace OHOS {
namespace AbilityRuntime {
class RuntimeExtractor final : public BaseExtractor {
public:
    explicit RuntimeExtractor(const std::string& source);
    RuntimeExtractor(const std::string& source, const bool isRuntime);
    std::shared_ptr<RuntimeExtractor> Create();
    ~RuntimeExtractor() override;

    bool isSameHap(const std::string& hapPath) const;

private:
    std::string hapPath_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_RUNTIME_EXTRACTOR_H
