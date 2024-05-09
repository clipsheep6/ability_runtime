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

#include "parser_util.h"

#include <fstream>

#include "config_policy_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *DEFAULT_PRE_BUNDLE_ROOT_DIR = "/system";
constexpr const char *PRODUCT_SUFFIX = "/etc/app";
constexpr const char *INSTALL_LIST_CAPABILITY_CONFIG = "/install_list_capability.json";
constexpr const char *INSTALL_LIST = "install_list";
constexpr const char *BUNDLE_NAME = "bundleName";
constexpr const char *KEEP_ALIVE = "keepAlive";
constexpr const char *KEEP_ALIVE_ENABLE = "KeepAliveEnable";
constexpr const char *KEEP_ALIVE_CONFIGURED_LIST = "KeepAliveConfiguredList";

} // namespace
ParserUtil &ParserUtil::GetInstance()
{
    static ParserUtil instance;
    return instance;
}

void ParserUtil::GetResidentProcessRawData(std::vector<std::tuple<std::string, std::string, std::string>> &list)
{
    std::vector<std::string> rootDirList;
    GetPreInstallRootDirList(rootDirList);

    for (auto &root : rootDirList) {
        auto fileDir = root.append(PRODUCT_SUFFIX).append(INSTALL_LIST_CAPABILITY_CONFIG);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Search file dir : %{public}s", fileDir.c_str());
        ParsePreInstallAbilityConfig(fileDir, list);
    }
}

void ParserUtil::ParsePreInstallAbilityConfig(
    const std::string &filePath, std::vector<std::tuple<std::string, std::string, std::string>> &list)
{
    nlohmann::json jsonBuf;
    if (!ReadFileIntoJson(filePath, jsonBuf)) {
        return;
    }

    if (jsonBuf.is_discarded()) {
        return;
    }

    FilterInfoFromJson(jsonBuf, list);
}

bool ParserUtil::FilterInfoFromJson(
    nlohmann::json &jsonBuf, std::vector<std::tuple<std::string, std::string, std::string>> &list)
{
    if (jsonBuf.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Profile format error");
        return false;
    }

    if (jsonBuf.find(INSTALL_LIST) == jsonBuf.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "InstallList not exist");
        return false;
    }

    auto arrays = jsonBuf.at(INSTALL_LIST);
    if (!arrays.is_array() || arrays.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Array not found");
        return false;
    }

    std::string bundleName;
    std::string KeepAliveEnable = "1";
    std::string KeepAliveConfiguredList;
    for (const auto &array : arrays) {
        if (!array.is_object()) {
            continue;
        }

        // Judgment logic exists, not found, not bool, not resident process
        if (!(array.find(KEEP_ALIVE) != array.end() && array.at(KEEP_ALIVE).is_boolean() &&
                array.at(KEEP_ALIVE).get<bool>())) {
            continue;
        }

        if (!(array.find(BUNDLE_NAME) != array.end() && array.at(BUNDLE_NAME).is_string())) {
            continue;
        }

        bundleName = array.at(BUNDLE_NAME).get<std::string>();

        if (array.find(KEEP_ALIVE_ENABLE) != array.end() && array.at(KEEP_ALIVE_ENABLE).is_boolean()) {
            auto val = array.at(KEEP_ALIVE_ENABLE).get<bool>();
            KeepAliveEnable = std::to_string(val);
        }

        if (array.find(KEEP_ALIVE_CONFIGURED_LIST) != array.end() && array.at(KEEP_ALIVE_CONFIGURED_LIST).is_array()) {
            // Save directly in the form of an array and parse it when in use
            KeepAliveConfiguredList = array.at(KEEP_ALIVE_CONFIGURED_LIST).dump();
        }

        list.emplace_back(std::make_tuple(bundleName, KeepAliveEnable, KeepAliveConfiguredList));
        bundleName.clear();
        KeepAliveEnable = "1";
        KeepAliveConfiguredList.clear();
    }

    return true;
}

void ParserUtil::GetPreInstallRootDirList(std::vector<std::string> &rootDirList)
{
    auto cfgDirList = GetCfgDirList();
    if (cfgDirList != nullptr) {
        for (const auto &cfgDir : cfgDirList->paths) {
            if (cfgDir == nullptr) {
                continue;
            }
            rootDirList.emplace_back(cfgDir);
        }

        FreeCfgDirList(cfgDirList);
    }
    bool ret = std::find(rootDirList.begin(), rootDirList.end(), DEFAULT_PRE_BUNDLE_ROOT_DIR) != rootDirList.end();
    if (!ret) {
        rootDirList.emplace_back(DEFAULT_PRE_BUNDLE_ROOT_DIR);
    }
}

bool ParserUtil::ReadFileIntoJson(const std::string &filePath, nlohmann::json &jsonBuf)
{
    if (filePath.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "File path empty.");
        return false;
    }

    std::ifstream fin(filePath);
    if (!fin.is_open()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "File path exception.");
        return false;
    }

    fin.seekg(0, std::ios::end);
    int64_t size = fin.tellg();
    if (size <= 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The file is an empty file!");
        fin.close();
        return false;
    }

    fin.seekg(0, std::ios::beg);
    jsonBuf = nlohmann::json::parse(fin, nullptr, false);
    fin.close();
    if (jsonBuf.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Bad profile file");
        return false;
    }
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS