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
#include <set>
#include <string>

#include "js_env_logger.h"
#include "source_map.h"

namespace OHOS {
namespace JsEnv {
class SourceMapOperator {
public:
    SourceMapOperator(const std::string bundleName, bool isModular)
        : bundleName_(bundleName), isModular_(isModular) {}

    ~SourceMapOperator() = default;

    std::string TranslateBySourceMap(const std::string& stackStr)
    {
        SourceMap sourceMapObj;
        std::vector<std::string> res;
        std::set<std::string> hapNames;
        sourceMapObj.ExtractStackInfo(stackStr, res);
        for (uint32_t i = 0; i < res.size(); i++) {
            size_t start = res[i].find_first_of("(");
            size_t end = res[i].find_first_of("/");
            if (start != std::string::npos && end != std::string::npos) {
                hapNames.insert(res[i].substr(start + 1, end - start - 1));
            }
        }
        for (auto &hapName : hapNames) {
            std::string hapPath = sourceMapObj.GetHapPath(hapName, bundleName_);
            if (!hapPath.empty()) {
                sourceMapObj.Init(isModular_, hapPath);
            }
        }
        return sourceMapObj.TranslateBySourceMap(stackStr);
    }

    bool TranslateUrlPositionBySourceMap(std::string& url, int& line, int& column)
    {
        SourceMap sourceMapObj;
        size_t pos = url.find_first_of("/");
        if (pos != std::string::npos) {
            std::string hapPath = sourceMapObj.GetHapPath(url.substr(0, pos), bundleName_);
            sourceMapObj.Init(isModular_, hapPath);
        }
        return sourceMapObj.TranslateUrlPositionBySourceMap(url, line, column);
    }

private:
    const std::string bundleName_;
    bool isModular_ = false;
};
} // namespace JsEnv
} // namespace OHOS
#endif // OHOS_ABILITY_JS_ENVIRONMENT_SOURCE_MAP_OPERATOR_H
