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

#include "proxy_authorization_uri_config.h"
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "accesstoken_kit.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* CONFIG_PATH = "/system/etc/proxy_authorization_uri.json";

const std::string PROXY_AUTHORIZATION_URI_NAME = "proxyAuthorizationUri";
const std::string BUNDLE_NAME = "bundleName";
}

void ProxyAuthorizationUriConfig::LoadConfiguration()
{
    HILOG_DEBUG("call");
    nlohmann::json jsonBuf;
    if (!ReadFileInfoJson(CONFIG_PATH, jsonBuf)) {
        HILOG_ERROR("Parse file failed.");
        return;
    }
    LoadBundleNameAllowedList(jsonBuf);
}

bool ProxyAuthorizationUriConfig::isAuthorizationUriAllowed(uint32_t fromTokenId)
{
    Security::AccessToken::NativeTokenInfo nativeInfo;
    auto result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(fromTokenId, nativeInfo);
    if (result == Security::AccessToken::AccessTokenKitRet::RET_SUCCESS &&
        bundleNameAllowedList_.find(nativeInfo.processName) != bundleNameAllowedList_.end()) {
        return true;
    }

    Security::AccessToken::HapTokenInfo hapInfo;
    result = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(fromTokenId, hapInfo);
    if (result == Security::AccessToken::AccessTokenKitRet::RET_SUCCESS &&
        bundleNameAllowedList_.find(hapInfo.bundleName) != bundleNameAllowedList_.end()) {
        return true;
    } 
    return false;
}

void ProxyAuthorizationUriConfig::LoadBundleNameAllowedList(const nlohmann::json &object)
{
    if (!object.contains(PROXY_AUTHORIZATION_URI_NAME)) {
        HILOG_ERROR("Proxy authorization uri config not existed.");
        return;
    }

    for (auto &item : object.at(PROXY_AUTHORIZATION_URI_NAME).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.contains(BUNDLE_NAME) || !jsonObject.at(BUNDLE_NAME).is_string()) {
            continue;
        }
        std::string bundleName = jsonObject.at(BUNDLE_NAME).get<std::string>();
        bundleNameAllowedList_.insert(bundleName);
    }
}

bool ProxyAuthorizationUriConfig::ReadFileInfoJson(const std::string &filePath, nlohmann::json &jsonBuf)
{
    if (access(filePath.c_str(), F_OK) != 0) {
        HILOG_DEBUG("%{public}s, not existed", filePath.c_str());
        return false;
    }

    std::fstream in;
    char errBuf[256];
    errBuf[0] = '\0';
    in.open(filePath, std::ios_base::in);
    if (!in.is_open()) {
        strerror_r(errno, errBuf, sizeof(errBuf));
        HILOG_ERROR("the file cannot be open due to  %{public}s", errBuf);
        return false;
    }

    in.seekg(0, std::ios::end);
    int64_t size = in.tellg();
    if (size <= 0) {
        HILOG_ERROR("the file is an empty file");
        in.close();
        return false;
    }

    in.seekg(0, std::ios::beg);
    jsonBuf = nlohmann::json::parse(in, nullptr, false);
    in.close();
    if (jsonBuf.is_discarded()) {
        HILOG_ERROR("bad profile file");
        return false;
    }

    return true;
}
}
}