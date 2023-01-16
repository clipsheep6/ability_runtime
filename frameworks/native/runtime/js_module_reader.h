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

#ifndef OHOS_ABILITY_RUNTIME_JS_MODULE_READER_H
#define OHOS_ABILITY_RUNTIME_JS_MODULE_READER_H

#include <sstream>
#include <string>

#include "js_module_searcher.h"
#include "extractor.h"

using Extractor = OHOS::AbilityBase::Extractor;

namespace OHOS {
namespace AbilityRuntime {
class JsModuleReader final : private JsModuleSearcher {
public:
    JsModuleReader(const std::string& bundleName, const std::string& hapPath,
        const std::shared_ptr<Extractor>& runtimeExtractor)
        : JsModuleSearcher(bundleName), hapPath_(hapPath), runtimeExtractor_(runtimeExtractor)
    {}
    ~JsModuleReader() = default;

    JsModuleReader(const JsModuleReader&) = default;
    JsModuleReader(JsModuleReader&&) = default;
    JsModuleReader& operator=(const JsModuleReader&) = default;
    JsModuleReader& operator=(JsModuleReader&&) = default;

    std::vector<uint8_t> operator()(const std::string& curJsModulePath, const std::string& newJsModuleUri) const;

private:
    std::string hapPath_;
    std::shared_ptr<Extractor> runtimeExtractor_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_MODULE_READER_H
