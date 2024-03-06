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
namespace {
enum InitStatus { NOT_EXECUTED, EXECUTED_SUCCESSFULLY };
}
class SourceMapOperator {
public:
    SourceMapOperator(const std::string bundleName, bool isModular)
        : bundleName_(bundleName), isModular_(isModular), initStatus_(NOT_EXECUTED) {}

    ~SourceMapOperator() = default;

    void InitSourceMap()
    {
        sourceMapObj_ = std::make_shared<SourceMap>();
        std::vector<std::string> hapList;
        sourceMapObj_->GetHapPath(bundleName_, hapList);
        for (auto &hapInfo : hapList) {
            if (!hapInfo.empty()) {
                sourceMapObj_->Init(isModular_, hapInfo);
            }
        }
        initStatus_ = EXECUTED_SUCCESSFULLY;
    }

    std::string TranslateBySourceMap(const std::string& stackStr)
    {
        if (sourceMapObj_ == nullptr) {
            JSENV_LOG_E("sourceMapObj_ is nullptr");
            return "";
        }
        return sourceMapObj_->TranslateBySourceMap(stackStr);
    }

    bool TranslateUrlPositionBySourceMap(std::string& url, int& line, int& column)
    {
        if (sourceMapObj_ == nullptr) {
            JSENV_LOG_E("sourceMapObj_ is nullptr");
            return false;
        }
        return sourceMapObj_->TranslateUrlPositionBySourceMap(url, line, column);
    }

    bool GetInitStatus() const
    {
        return (initStatus_ == InitStatus::EXECUTED_SUCCESSFULLY);
    }

private:
    const std::string bundleName_;
    bool isModular_ = false;
    std::shared_ptr<SourceMap> sourceMapObj_;
    InitStatus initStatus_;
};
} // namespace JsEnv
} // namespace OHOS
#endif // OHOS_ABILITY_JS_ENVIRONMENT_SOURCE_MAP_OPERATOR_H
