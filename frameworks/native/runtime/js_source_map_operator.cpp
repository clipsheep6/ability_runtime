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

#include "js_source_map_operator.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
std::string JsSourceMapOperatorImpl::TranslateBySourceMap(const std::string& stackStr)
{
    if (bindSourceMaps_ == nullptr) {
        HILOG_ERROR("Source map is invalid.");
        return "";
    }

    std::string sourceMapData;
    if (isModular_) {
        JsEnv::SourceMap::ReadSourceMapData(hapPath_, sourceMapData);
        bindSourceMaps_->Init(isModular_, sourceMapData);
    }
    return bindSourceMaps_->TranslateBySourceMap(stackStr, hapPath_);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
