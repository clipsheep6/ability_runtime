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

#include "js_module_reader.h"

#include "file_path_utils.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

using namespace OHOS::AbilityBase;

namespace OHOS {
namespace AbilityRuntime {
std::vector<uint8_t> JsModuleReader::operator()(
    const std::string& curJsModulePath, const std::string& newJsModuleUri) const
{
    std::vector<uint8_t> buffer;
    if (runtimeExtractor_ == nullptr) {
        HILOG_ERROR("Runtime extractor does not exist");
        return buffer;
    }

    std::string newJsModulePath = NormalizeUri(GetBundleName(), curJsModulePath, newJsModuleUri);
    if (newJsModulePath.empty()) {
        return buffer;
    }

    std::ostringstream dest;
    if (!runtimeExtractor_->GetFileBuffer(newJsModulePath, dest)) {
        HILOG_ERROR("Get abc file failed");
        return buffer;
    }

    const auto& outStr = dest.str();
    buffer.assign(outStr.begin(), outStr.end());

    return buffer;
}
} // namespace AbilityRuntime
} // namespace OHOS
