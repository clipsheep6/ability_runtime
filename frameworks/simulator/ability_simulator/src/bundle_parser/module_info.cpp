/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "module_info.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "json_util.h"
#include "nlohmann/json.hpp"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string MODULE_INFO_MODULE_SOURCE_DIR = "moduleSourceDir";
const std::string MODULE_INFO_PRELOADS = "preloads";
}
void to_json(nlohmann::json &jsonObject, const ModuleInfo &moduleInfo)
{
    jsonObject = nlohmann::json {
        {Constants::MODULE_NAME, moduleInfo.moduleName},
        {MODULE_INFO_MODULE_SOURCE_DIR, moduleInfo.moduleSourceDir},
        {MODULE_INFO_PRELOADS, moduleInfo.preloads}
    };
}

void from_json(const nlohmann::json &jsonObject, ModuleInfo &moduleInfo)
{
    const auto &jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        Constants::MODULE_NAME,
        moduleInfo.moduleName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_INFO_MODULE_SOURCE_DIR,
        moduleInfo.moduleSourceDir,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        MODULE_INFO_PRELOADS,
        moduleInfo.preloads,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    if (parseResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM,
            "read module moduleInfo from jsonObject error, error code : %{public}d", parseResult);
    }
}
} // namespace AppExecFwk
} // namespace OHOS
