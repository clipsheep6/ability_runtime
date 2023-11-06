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

#ifndef OHOS_ABILITY_RUNTIME_VIEW_DATA_H
#define OHOS_ABILITY_RUNTIME_VIEW_DATA_H

#include <string>

namespace OHOS {
namespace AbilityRuntime {

enum class AutoFillType {
    UNSPECIFIED,
    PASSWORD,
    USER_NAME,
    NEW_PASSWORD,
};

enum ResultCode {
    INVALID_VALUE = 0,
    FILL_SUCESS,
    FILL_FAILED,
    FILL_CANCEL,
    SAVE_SUCESS,
    SAVE_FAILED,
};

struct PageNodeInfo {
    int32_t id = -1;
    int32_t depth = -1;
    AutoFillType autoFillType = AutoFillType::UNSPECIFIED;
    std::string tag;
    std::string value;
    std::string passwordRules;
    bool enableAutoFill = true;
};

struct ViewData {
    std::string ToJsonString() const;
    ViewData ToJsValue(const std::string &strData) const;

    std::string bundleName;
    std::string abilityName;
    std::string moduleName;
    std::string pageName;
    std::vector<PageNodeInfo> pageNodeInfos;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_VIEW_DATA_H