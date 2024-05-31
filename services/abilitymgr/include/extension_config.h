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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_CONFIG_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_CONFIG_H

#include <map>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <unordered_set>

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
    bool IsExtensionStartThirdPartyAppEnable(std::string extensionTypeName);
    bool IsExtensionStartServiceEnable(std::string extensionTypeName, std::string targetUri);
private:
    void LoadExtensionConfig(const nlohmann::json &object);
    bool ReadFileInfoJson(const std::string &filePath, nlohmann::json &jsonBuf);

    std::string GetExtensionConfigPath() const;
    void LoadExtensionAutoDisconnectTime(const nlohmann::json &object, std::string extensionTypeName);
    void LoadExtensionThirdPartyAppBlockedList(const nlohmann::json &object, std::string extensionTypeName);
    void LoadExtensionServiceBlockedList(const nlohmann::json &, std::string extensionTypeNameobject);

    bool CheckServiceExtensionUriValid(const std::string &uri);

    std::map<std::string, int32_t> extensionAutoDisconnectTimeMap_;
    std::unordered_map<std::string, bool> thirdPartyAppBlockedFlags_;
    std::unordered_map<std::string, std::unordered_set<std::string>> serviceBlockedLists_;
};
} // OHOS
} // AAFwk

#endif // OHOS_ABILITY_RUNTIME_EXTENSION_CONFIG_H