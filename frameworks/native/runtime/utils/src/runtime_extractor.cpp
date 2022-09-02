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

#include "runtime_extractor.h"

#include "ability_constants.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
inline bool StringStartWith(const std::string& str, const char* startStr, size_t startStrLen)
{
    return ((str.length() >= startStrLen) && (str.compare(0, startStrLen, startStr) == 0));
}
} // namespace
RuntimeExtractor::RuntimeExtractor(const std::string& source) : BaseExtractor(source)
{
    hapPath_ = source;
    zipFile_.SetIsRuntime(true);
}

std::shared_ptr<RuntimeExtractor> RuntimeExtractor::Create()
{
    if (sourceFile_.empty()) {
        HILOG_ERROR("source is nullptr");
        return std::shared_ptr<RuntimeExtractor>();
    }

    std::string loadPath;
    if (StringStartWith(sourceFile_, Constants::ABS_CODE_PATH, std::string(Constants::ABS_CODE_PATH).length())) {
        loadPath = GetLoadPath(sourceFile_);
    } else {
        loadPath = sourceFile_;
    }
    std::shared_ptr<RuntimeExtractor> runtimeExtractor = std::make_shared<RuntimeExtractor>(loadPath);
    if (!runtimeExtractor->Init()) {
        HILOG_ERROR("RuntimeExtractor create failed");
        return std::shared_ptr<RuntimeExtractor>();
    }

    return runtimeExtractor;
}

RuntimeExtractor::~RuntimeExtractor()
{}

bool RuntimeExtractor::isSameHap(const std::string& hapPath) const
{
    return !hapPath_.empty() && !hapPath.empty() && hapPath_ == hapPath;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
