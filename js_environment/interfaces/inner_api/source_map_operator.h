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

#ifndef OHOS_ABILITY_JS_ENVIRONMENT_SOURCE_MAP_OPERATOR_H
#define OHOS_ABILITY_JS_ENVIRONMENT_SOURCE_MAP_OPERATOR_H
#include <memory>
#include <string>

namespace OHOS {
namespace JsEnv {
class SourceMapOperatorImpl {
public:
    SourceMapOperatorImpl() = default;
    virtual ~SourceMapOperatorImpl() = default;
    virtual std::string TranslateBySourceMap(const std::string& stackStr) = 0;
};

class SourceMapOperator {
public:
    SourceMapOperator(std::shared_ptr<SourceMapOperatorImpl> impl) : impl_(impl)
    {}

    ~SourceMapOperator() = default;

    std::string TranslateBySourceMap(const std::string& stackStr)
    {
        if (impl_ == nullptr) {
            return "";
        }
        return impl_->TranslateBySourceMap(stackStr);
    }

private:
    std::shared_ptr<SourceMapOperatorImpl> impl_ = nullptr;
};
} // namespace JsEnv
} // namespace OHOS
#endif // OHOS_ABILITY_JS_ENVIRONMENT_SOURCE_MAP_OPERATOR_H
