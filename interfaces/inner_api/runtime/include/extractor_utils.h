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

#ifndef OHOS_ABILITY_RUNTIME_EXTRACTOR_UTILS_H
#define OHOS_ABILITY_RUNTIME_EXTRACTOR_UTILS_H

#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<RuntimeExtractor> InitRuntimeExtractor(const std::string& hapPath);
bool GetFileBuffer(
    const std::shared_ptr<RuntimeExtractor>& runtimeExtractor, const std::string& srcPath, std::ostringstream &dest);
bool GetFileBufferFromHap(const std::string& hapPath, const std::string& srcPath, std::ostringstream &dest);
bool GetFileListFromHap(const std::string& hapPath, const std::string& srcPath, std::vector<std::string>& assetList);
}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_EXTRACTOR_UTILS_H
