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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_CONFIG_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_CONFIG_H

#include <map>
#include <nlohmann/json.hpp>

#include "extension_ability_info.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
class ExtensionConfig : public DelayedSingleton<ExtensionConfig> {
public:
    explicit ExtensionConfig() = default;
    virtual ~ExtensionConfig() = default;
    void LoadExtensionConfiguration();
    int32_t GetExtensionAutoDisconnectTime(std::string extensionTypeName);
private:
    void LoadExtensionAutoDisconnectTime(const nlohmann::json &object);
    bool ReadFileInfoJson(const std::string &filePath, nlohmann::json &jsonBuf);

    std::map<std::string, int32_t> extensionAutoDisconnectTimeMap_;
};
} // OHOS
} // AAFwk

#endif // OHOS_ABILITY_RUNTIME_EXTENSION_CONFIG_H