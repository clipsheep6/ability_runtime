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
    static constexpr char ABS_CODE_PATH[] = "/data/storage/el1/";
    static constexpr char MERGE_ABC_PATH[] = "ets/modules.abc";
    static constexpr char SHARED_FILE_SUFFIX[] = ".hsp";
    static constexpr char HAP_SUFFIX[] = ".hap";
    static constexpr char OH_TEST_FLAG[] = "_test";
    explicit JsModuleReader(const std::string& bundleName) : JsModuleSearcher(bundleName) {}
    ~JsModuleReader() = default;

    JsModuleReader(const JsModuleReader&) = default;
    JsModuleReader(JsModuleReader&&) = default;
    JsModuleReader& operator=(const JsModuleReader&) = default;
    JsModuleReader& operator=(JsModuleReader&&) = default;

    std::vector<uint8_t> operator()(const std::string& hapPath) const;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_MODULE_READER_H
