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

#include "app_recovery/default_recovery_config.h"
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <regex>

#include "config_policy_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* CONFIG_PATH = "/etc/default_recovery_config.json";
constexpr const char* DEFAULT_RESERVE_CONFIG_PATH = "/system/etc/default_recovery_config.json";
constexpr const char* ITEM_DEFAULT_RECOVERY_NAME = "default_recovery";
constexpr const char* SUPPORT_BUNDLE_NAME_LIST = "support_bundle_name_list";
constexpr const char* RESERVE_NUMBER_WHEN_TIMEOUT = "reserve_number_when_timeout";
constexpr const char* RECOVERY_DATA_TIMEOUT_DELETE_TIME = "recovery_data_timeout_delete_time";
}

std::string DefaultRecoveryConfig::GetConfigPath()
{
    char buf[MAX_PATH_LEN] = { 0 };
    char *configPath = GetOneCfgFile(CONFIG_PATH, buf, MAX_PATH_LEN);
    if (configPath == nullptr || configPath[0] == '\0' || strlen(configPath) > MAX_PATH_LEN) {
        return DEFAULT_RESERVE_CONFIG_PATH;
    }
    return configPath;
}

bool DefaultRecoveryConfig::LoadConfiguration()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Load configuration.");
    std::string configPath = GetConfigPath();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Default recovery config path is: %{public}s.", configPath.c_str());
    nlohmann::json jsonBuf;
    if (ReadFileInfoJson(configPath, jsonBuf)) {
        if (!LoadDefaultRecovery(jsonBuf)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Load configuration failed.");
            return false;
        }
    }

    return true;
}

bool DefaultRecoveryConfig::IsBundleDefaultRecoveryEnabled(const std::string &bundleName)
{
    if (bundleNameList_.find(bundleName) != bundleNameList_.end()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Find bundleName %{public}s.", bundleName.c_str());
        return true;
    }
    return false;
}

int32_t DefaultRecoveryConfig::GetReserveNumber()
{
    return reserveNumber_;
}

int32_t DefaultRecoveryConfig::GetTimeoutDeleteTime()
{
    return timeoutDeleteTime_;
}

bool DefaultRecoveryConfig::LoadDefaultRecovery(const nlohmann::json &object)
{
    if (!object.contains(ITEM_DEFAULT_RECOVERY_NAME)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Default recovery config not existed.");
        return false;
    }

    const nlohmann::json &jsonObject = object.at(ITEM_DEFAULT_RECOVERY_NAME);
    if (jsonObject.contains(SUPPORT_BUNDLE_NAME_LIST) && jsonObject[SUPPORT_BUNDLE_NAME_LIST].is_array()) {
        for (const auto &value : jsonObject.at(SUPPORT_BUNDLE_NAME_LIST)) {
            if (value.is_string()) {
                auto bundleName = value.get<std::string>();
                TAG_LOGD(AAFwkTag::ABILITYMGR, "Bundle name is %{public}s.", bundleName.c_str());
                bundleNameList_.emplace(bundleName);
            }
        }
    }

    if (jsonObject.contains(RESERVE_NUMBER_WHEN_TIMEOUT) && jsonObject[RESERVE_NUMBER_WHEN_TIMEOUT].is_number()) {
        reserveNumber_ = jsonObject.at(RESERVE_NUMBER_WHEN_TIMEOUT).get<int32_t>();
    }

    if (jsonObject.contains(RECOVERY_DATA_TIMEOUT_DELETE_TIME) &&
        jsonObject[RECOVERY_DATA_TIMEOUT_DELETE_TIME].is_number()) {
        timeoutDeleteTime_ = jsonObject.at(RECOVERY_DATA_TIMEOUT_DELETE_TIME).get<int32_t>();
    }

    return true;
}

bool DefaultRecoveryConfig::ReadFileInfoJson(const std::string &filePath, nlohmann::json &jsonBuf)
{
    if (access(filePath.c_str(), F_OK) != 0) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s, not existed.", filePath.c_str());
        return false;
    }

    if (filePath.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "File path is empty.");
        return false;
    }

    std::fstream in;
    in.open(filePath, std::ios_base::in);
    if (!in.is_open()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Open file failed with %{public}d.", errno);
        return false;
    }

    in.seekg(0, std::ios::end);
    int64_t size = in.tellg();
    if (size <= 0) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "The file is empty.");
        in.close();
        return false;
    }

    in.seekg(0, std::ios::beg);
    jsonBuf = nlohmann::json::parse(in, nullptr, false);
    in.close();
    if (jsonBuf.is_discarded()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "bad profile file.");
        return false;
    }

    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
