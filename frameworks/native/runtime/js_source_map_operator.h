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

#ifndef OHOS_ABILITY_RUNTIME_JS_SOURCE_MAP_OPERATOR_H
#define OHOS_ABILITY_RUNTIME_JS_SOURCE_MAP_OPERATOR_H

#include "source_map.h"
#include "source_map_operator.h"

namespace OHOS {
namespace AbilityRuntime {
class JsSourceMapOperatorImpl : public JsEnv::SourceMapOperatorImpl {
public:
    JsSourceMapOperatorImpl(const std::string hapPath, bool isModular, std::shared_ptr<JsEnv::SourceMap> bindSourceMaps)
        : hapPath_(hapPath), isModular_(isModular), bindSourceMaps_(bindSourceMaps)
    {}

    ~JsSourceMapOperatorImpl() = default;

std::string TranslateBySourceMap(const std::string& stackStr) override;

private:
    std::string hapPath_;
    bool isModular_ = false;
    std::shared_ptr<JsEnv::SourceMap> bindSourceMaps_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_JS_SOURCE_MAP_OPERATOR_H
