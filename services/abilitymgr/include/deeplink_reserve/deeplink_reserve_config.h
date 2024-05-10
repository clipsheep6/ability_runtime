/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_DEEPLINK_RESERVE_CONFIG_H
#define OHOS_ABILITY_RUNTIME_DEEPLINK_RESERVE_CONFIG_H

#include <nlohmann/json.hpp>
#include <string>
#include <map>

#include "singleton.h"

namespace OHOS {
namespace AAFwk {

struct ReserveUri {
    std::string scheme;
    std::string host;
    std::string port;
    std::string path;
    std::string pathStartWith;
    std::string pathRegex;
    std::string type;
    std::string utd;
};

class DeepLinkReserveConfig {
public:
    static DeepLinkReserveConfig &GetInstance()
    {
        static DeepLinkReserveConfig instance;
        return instance;
    }
    ~DeepLinkReserveConfig() = default;
    bool LoadConfiguration();
    bool isLinkReserved(const std::string &linkString, std::string &bundleName);

private:
    std::string GetConfigPath();
    bool ReadFileInfoJson(const std::string &filePath, nlohmann::json &jsonBuf);
    bool LoadReservedUriList(const nlohmann::json &object);
    bool isUriMatched(const ReserveUri &reservedUri, const std::string &link);
    void LoadReservedUrilItem(const nlohmann::json &jsonUriObject, std::vector<ReserveUri> &uriList);
    DeepLinkReserveConfig() = default;
    DISALLOW_COPY_AND_MOVE(DeepLinkReserveConfig);

private:
    std::map<std::string, std::vector<ReserveUri>> deepLinkReserveUris_;
};
} // OHOS
} // AAFwk

#endif // OHOS_ABILITY_RUNTIME_DEEPLINK_RESERVE_CONFIG_H

