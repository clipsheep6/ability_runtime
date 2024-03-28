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

#include "ability_auto_startup_data_manager.h"

#include <algorithm>
#include <unistd.h>

#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"
#include "types.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms
constexpr const char *AUTO_STARTUP_STORAGE_DIR = "/data/service/el1/public/database/auto_startup_service";
const std::string JSON_KEY_BUNDLE_NAME = "bundleName";
const std::string JSON_KEY_ABILITY_NAME = "abilityName";
const std::string JSON_KEY_MODULE_NAME = "moduleName";
const std::string JSON_KEY_IS_AUTO_STARTUP = "isAutoStartup";
const std::string JSON_KEY_IS_EDM_FORCE = "isEdmForce";
const std::string JSON_KEY_TYPE_NAME = "abilityTypeName";
} // namespace
const DistributedKv::AppId AbilityAutoStartupDataManager::APP_ID = { "auto_startup_storage" };
const DistributedKv::StoreId AbilityAutoStartupDataManager::STORE_ID = { "auto_startup_infos" };
AbilityAutoStartupDataManager::AbilityAutoStartupDataManager() {}

AbilityAutoStartupDataManager::~AbilityAutoStartupDataManager()
{
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(APP_ID, kvStorePtr_);
    }
}

DistributedKv::Status AbilityAutoStartupDataManager::GetKvStore()
{
    DistributedKv::Options options = { .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = AUTO_STARTUP_STORAGE_DIR };

    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Return error: %{public}d.", status);
        return status;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Get kvStore success.");
    return status;
}

bool AbilityAutoStartupDataManager::CheckKvStore()
{
    if (kvStorePtr_ != nullptr) {
        return true;
    }
    int32_t tryTimes = MAX_TIMES;
    while (tryTimes > 0) {
        DistributedKv::Status status = GetKvStore();
        if (status == DistributedKv::Status::SUCCESS && kvStorePtr_ != nullptr) {
            return true;
        }
        TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Try times: %{public}d.", tryTimes);
        usleep(CHECK_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

int32_t AbilityAutoStartupDataManager::InsertAutoStartupData(
    const AutoStartupInfo &info, bool isAutoStartup, bool isEdmForce)
{
    if (info.bundleName.empty() || info.abilityName.empty()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Invalid value.");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s.",
        info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "kvStore is nullptr.");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key = ConvertAutoStartupDataToKey(info);
    DistributedKv::Value value = ConvertAutoStartupStatusToValue(isAutoStartup, isEdmForce, info.abilityTypeName);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Insert data to kvStore error: %{public}d.", status);
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::UpdateAutoStartupData(
    const AutoStartupInfo &info, bool isAutoStartup, bool isEdmForce)
{
    if (info.bundleName.empty() || info.abilityName.empty()) {
        TAG_LOGW(AAFwkTag::AUTO_STARTUP, "Invalid value!");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s.",
        info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "kvStore is nullptr.");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key = ConvertAutoStartupDataToKey(info);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Delete data from kvStore error: %{public}d.", status);
        return ERR_INVALID_OPERATION;
    }
    DistributedKv::Value value = ConvertAutoStartupStatusToValue(isAutoStartup, isEdmForce, info.abilityTypeName);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Insert data to kvStore error: %{public}d.", status);
        return ERR_INVALID_OPERATION;
    }

    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::DeleteAutoStartupData(const AutoStartupInfo &info)
{
    if (info.bundleName.empty() || info.abilityName.empty()) {
        TAG_LOGW(AAFwkTag::AUTO_STARTUP, "Invalid value!");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s.",
        info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "kvStore is nullptr.");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key = ConvertAutoStartupDataToKey(info);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Delete data from kvStore error: %{public}d.", status);
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::DeleteAutoStartupData(const std::string &bundleName)
{
    if (bundleName.empty()) {
        TAG_LOGW(AAFwkTag::AUTO_STARTUP, "Invalid value!");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "bundleName: %{public}s.", bundleName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "kvStore is nullptr.");
            return ERR_NO_INIT;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get entries error: %{public}d.", status);
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        if (IsEqual(item.key, bundleName)) {
            {
                std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
                status = kvStorePtr_->Delete(item.key);
            }
            if (status != DistributedKv::Status::SUCCESS) {
                TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Delete data from kvStore error: %{public}d.", status);
                return ERR_INVALID_OPERATION;
            }
        }
    }

    return ERR_OK;
}

AutoStartupStatus AbilityAutoStartupDataManager::QueryAutoStartupData(const AutoStartupInfo &info)
{
    AutoStartupStatus asustatus;
    if (info.bundleName.empty() || info.abilityName.empty()) {
        TAG_LOGW(AAFwkTag::AUTO_STARTUP, "Invalid value!");
        asustatus.code = ERR_INVALID_VALUE;
        return asustatus;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s.",
        info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "kvStore is nullptr.");
            asustatus.code = ERR_NO_INIT;
            return asustatus;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get entries error: %{public}d.", status);
        asustatus.code = ERR_INVALID_OPERATION;
        return asustatus;
    }

    asustatus.code = ERR_NAME_NOT_FOUND;
    for (const auto &item : allEntries) {
        if (IsEqual(item.key, info)) {
            ConvertAutoStartupStatusFromValue(item.value, asustatus.isAutoStartup, asustatus.isEdmForce);
            asustatus.code = ERR_OK;
        }
    }

    return asustatus;
}

int32_t AbilityAutoStartupDataManager::QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Called.");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "kvStore is nullptr.");
            return ERR_NO_INIT;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get entries error: %{public}d.", status);
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        bool isAutoStartup, isEdmForce;
        ConvertAutoStartupStatusFromValue(item.value, isAutoStartup, isEdmForce);
        if (isAutoStartup) {
            infoList.emplace_back(ConvertAutoStartupInfoFromKeyAndValue(item.key, item.value));
        }
    }
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "InfoList.size: %{public}zu.", infoList.size());
    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::GetCurrentAppAutoStartupData(
    const std::string &bundleName, std::vector<AutoStartupInfo> &infoList)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Called.");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "kvStore is nullptr.");
            return ERR_NO_INIT;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get entries error: %{public}d.", status);
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        if (IsEqual(item.key, bundleName)) {
            infoList.emplace_back(ConvertAutoStartupInfoFromKeyAndValue(item.key, item.value));
        }
    }
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "InfoList.size: %{public}zu.", infoList.size());
    return ERR_OK;
}

DistributedKv::Value AbilityAutoStartupDataManager::ConvertAutoStartupStatusToValue(
    bool isAutoStartup, bool isEdmForce, const std::string &abilityTypeName)
{
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_IS_AUTO_STARTUP, isAutoStartup },
        { JSON_KEY_IS_EDM_FORCE, isEdmForce },
        { JSON_KEY_TYPE_NAME, abilityTypeName },
    };
    DistributedKv::Value value(jsonObject.dump());
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "value: %{public}s.", value.ToString().c_str());
    return value;
}

void AbilityAutoStartupDataManager::ConvertAutoStartupStatusFromValue(
    const DistributedKv::Value &value, bool &isAutoStartup, bool &isEdmForce)
{
    nlohmann::json jsonObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to parse json string.");
        return;
    }
    if (jsonObject.contains(JSON_KEY_IS_AUTO_STARTUP) && jsonObject[JSON_KEY_IS_AUTO_STARTUP].is_boolean()) {
        isAutoStartup = jsonObject.at(JSON_KEY_IS_AUTO_STARTUP).get<bool>();
    }
    if (jsonObject.contains(JSON_KEY_IS_EDM_FORCE) && jsonObject[JSON_KEY_IS_EDM_FORCE].is_boolean()) {
        isEdmForce = jsonObject.at(JSON_KEY_IS_EDM_FORCE).get<bool>();
    }
}

DistributedKv::Key AbilityAutoStartupDataManager::ConvertAutoStartupDataToKey(const AutoStartupInfo &info)
{
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_BUNDLE_NAME, info.bundleName },
        { JSON_KEY_MODULE_NAME, info.moduleName },
        { JSON_KEY_ABILITY_NAME, info.abilityName },
    };
    DistributedKv::Key key(jsonObject.dump());
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "key: %{public}s.", key.ToString().c_str());
    return key;
}

AutoStartupInfo AbilityAutoStartupDataManager::ConvertAutoStartupInfoFromKeyAndValue(
    const DistributedKv::Key &key, const DistributedKv::Value &value)
{
    AutoStartupInfo info;
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to parse jsonObject.");
        return info;
    }

    if (jsonObject.contains(JSON_KEY_BUNDLE_NAME) && jsonObject[JSON_KEY_BUNDLE_NAME].is_string()) {
        info.bundleName = jsonObject.at(JSON_KEY_BUNDLE_NAME).get<std::string>();
    }

    if (jsonObject.contains(JSON_KEY_MODULE_NAME) && jsonObject[JSON_KEY_MODULE_NAME].is_string()) {
        info.moduleName = jsonObject.at(JSON_KEY_MODULE_NAME).get<std::string>();
    }

    if (jsonObject.contains(JSON_KEY_ABILITY_NAME) && jsonObject[JSON_KEY_ABILITY_NAME].is_string()) {
        info.abilityName = jsonObject.at(JSON_KEY_ABILITY_NAME).get<std::string>();
    }

    nlohmann::json jsonValueObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (jsonValueObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to parse jsonValueObject.");
        return info;
    }

    if (jsonValueObject.contains(JSON_KEY_TYPE_NAME) && jsonValueObject[JSON_KEY_TYPE_NAME].is_string()) {
        info.abilityTypeName = jsonValueObject.at(JSON_KEY_TYPE_NAME).get<std::string>();
    }

    return info;
}

bool AbilityAutoStartupDataManager::IsEqual(const DistributedKv::Key &key, const AutoStartupInfo &info)
{
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to parse json string.");
        return false;
    }

    if (jsonObject.contains(JSON_KEY_BUNDLE_NAME) && jsonObject[JSON_KEY_BUNDLE_NAME].is_string()) {
        if (info.bundleName != jsonObject.at(JSON_KEY_BUNDLE_NAME).get<std::string>()) {
            return false;
        }
    }

    if (jsonObject.contains(JSON_KEY_ABILITY_NAME) && jsonObject[JSON_KEY_ABILITY_NAME].is_string()) {
        if (info.abilityName != jsonObject.at(JSON_KEY_ABILITY_NAME).get<std::string>()) {
            return false;
        }
    }

    if (jsonObject.contains(JSON_KEY_MODULE_NAME) && jsonObject[JSON_KEY_MODULE_NAME].is_string()) {
        std::string moduleName = jsonObject.at(JSON_KEY_MODULE_NAME).get<std::string>();
        if (!moduleName.empty() && info.moduleName != moduleName) {
            return false;
        }
    }
    return true;
}

bool AbilityAutoStartupDataManager::IsEqual(const DistributedKv::Key &key, const std::string &bundleName)
{
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to parse json string.");
        return false;
    }

    if (jsonObject.contains(JSON_KEY_BUNDLE_NAME) && jsonObject[JSON_KEY_BUNDLE_NAME].is_string()) {
        if (bundleName == jsonObject.at(JSON_KEY_BUNDLE_NAME).get<std::string>()) {
            return true;
        }
    }
    return false;
}
} // namespace AbilityRuntime
} // namespace OHOS
