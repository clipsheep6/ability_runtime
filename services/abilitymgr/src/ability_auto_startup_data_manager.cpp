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

#include "accesstoken_kit.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"
#include "types.h"
#include "os_account_manager_wrapper.h"

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
const std::string JSON_KEY_APP_CLONE_INDEX = "appCloneIndex";
const std::string JSON_KEY_ACCESS_TOKENID = "accessTokenId";
const std::string JSON_KEY_USERID = "userId";
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
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Error: %{public}d", status);
        return status;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Get kvStore success");
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
        TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Try times: %{public}d", tryTimes);
        usleep(CHECK_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

int32_t AbilityAutoStartupDataManager::InsertAutoStartupData(
    const AutoStartupInfo &info, bool isAutoStartup, bool isEdmForce)
{
    if (info.bundleName.empty() || info.abilityName.empty() || info.accessTokenId.empty() || info.userId == -1) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, userId: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.userId);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
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
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Insert data to kvStore error: %{public}d", status);
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::UpdateAutoStartupData(
    const AutoStartupInfo &info, bool isAutoStartup, bool isEdmForce)
{
    if (info.bundleName.empty() || info.abilityName.empty() || info.accessTokenId.empty() || info.userId == -1) {
        TAG_LOGW(AAFwkTag::AUTO_STARTUP, "Invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, userId: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.userId);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
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
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Delete data from kvStore error: %{public}d", status);
        return ERR_INVALID_OPERATION;
    }
    DistributedKv::Value value = ConvertAutoStartupStatusToValue(isAutoStartup, isEdmForce, info.abilityTypeName);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Insert data to kvStore error: %{public}d", status);
        return ERR_INVALID_OPERATION;
    }

    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::DeleteAutoStartupData(const AutoStartupInfo &info)
{
    if (info.bundleName.empty() || info.abilityName.empty() || info.accessTokenId.empty() || info.userId == -1) {
        TAG_LOGW(AAFwkTag::AUTO_STARTUP, "Invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, userId: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.userId);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
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
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Delete data from kvStore error: %{public}d", status);
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::DeleteAutoStartupData(const std::string &bundleName, int32_t uid)
{
    int32_t userId;
    if (DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
            GetOsAccountLocalIdFromUid(uid, userId) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetOsAccountLocalIdFromUid failed");
        return ERR_INVALID_VALUE;
    }
    uint32_t accessTokenId = Security::AccessToken::AccessTokenKit::GetHapTokenID(userId, bundleName, 0);
    auto accessTokenIdStr = std::to_string(accessTokenId);
    if (bundleName.empty() || accessTokenIdStr.empty()) {
        TAG_LOGW(AAFwkTag::AUTO_STARTUP, "Invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "bundleName: %{public}s, accessTokenId: %{public}s",
        bundleName.c_str(), accessTokenIdStr.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get entries error: %{public}d", status);
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        if (IsEqual(item.key, accessTokenIdStr)) {
            {
                std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
                status = kvStorePtr_->Delete(item.key);
            }
            if (status != DistributedKv::Status::SUCCESS) {
                TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Delete data from kvStore error: %{public}d", status);
                return ERR_INVALID_OPERATION;
            }
        }
    }

    return ERR_OK;
}

AutoStartupStatus AbilityAutoStartupDataManager::QueryAutoStartupData(const AutoStartupInfo &info)
{
    AutoStartupStatus asustatus;
    if (info.bundleName.empty() || info.abilityName.empty() || info.accessTokenId.empty() || info.userId == -1) {
        TAG_LOGW(AAFwkTag::AUTO_STARTUP, "Invalid value");
        asustatus.code = ERR_INVALID_VALUE;
        return asustatus;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, userId: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.userId);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
            asustatus.code = ERR_NO_INIT;
            return asustatus;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get entries error: %{public}d", status);
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

int32_t AbilityAutoStartupDataManager::QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList,
    int32_t userId)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get entries error: %{public}d", status);
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        if (!IsEqual(item.key, userId)) {
            continue;
        }
        bool isAutoStartup, isEdmForce;
        ConvertAutoStartupStatusFromValue(item.value, isAutoStartup, isEdmForce);
        if (isAutoStartup) {
            infoList.emplace_back(ConvertAutoStartupInfoFromKeyAndValue(item.key, item.value));
        }
    }
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "InfoList.size: %{public}zu", infoList.size());
    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::GetCurrentAppAutoStartupData(
    const std::string &bundleName, std::vector<AutoStartupInfo> &infoList, const std::string &accessTokenId)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get entries error: %{public}d", status);
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        if (IsEqual(item.key, accessTokenId)) {
            infoList.emplace_back(ConvertAutoStartupInfoFromKeyAndValue(item.key, item.value));
        }
    }
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "InfoList.size: %{public}zu", infoList.size());
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
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "value: %{public}s", value.ToString().c_str());
    return value;
}

void AbilityAutoStartupDataManager::ConvertAutoStartupStatusFromValue(
    const DistributedKv::Value &value, bool &isAutoStartup, bool &isEdmForce)
{
    nlohmann::json jsonObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Parse json string failed");
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
        { JSON_KEY_APP_CLONE_INDEX, info.appCloneIndex },
		{ JSON_KEY_ACCESS_TOKENID, info.accessTokenId },
        { JSON_KEY_USERID, info.userId },
    };
    DistributedKv::Key key(jsonObject.dump());
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "key: %{public}s", key.ToString().c_str());
    return key;
}

AutoStartupInfo AbilityAutoStartupDataManager::ConvertAutoStartupInfoFromKeyAndValue(
    const DistributedKv::Key &key, const DistributedKv::Value &value)
{
    AutoStartupInfo info;
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Parse jsonObject failed");
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

    if (jsonObject.contains(JSON_KEY_APP_CLONE_INDEX) && jsonObject[JSON_KEY_APP_CLONE_INDEX].is_number()) {
        info.appCloneIndex = jsonObject.at(JSON_KEY_APP_CLONE_INDEX).get<int32_t>();
    }

    if (jsonObject.contains(JSON_KEY_ACCESS_TOKENID) && jsonObject[JSON_KEY_ACCESS_TOKENID].is_string()) {
        info.accessTokenId = jsonObject.at(JSON_KEY_ACCESS_TOKENID).get<std::string>();
    }

    if (jsonObject.contains(JSON_KEY_USERID) && jsonObject[JSON_KEY_USERID].is_number()) {
        info.userId = jsonObject.at(JSON_KEY_USERID).get<int32_t>();
    }

    nlohmann::json jsonValueObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (jsonValueObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Parse jsonValueObject failed");
        return info;
    }

    if (jsonValueObject.contains(JSON_KEY_TYPE_NAME) && jsonValueObject[JSON_KEY_TYPE_NAME].is_string()) {
        info.abilityTypeName = jsonValueObject.at(JSON_KEY_TYPE_NAME).get<std::string>();
    }
    return info;
}

bool AbilityAutoStartupDataManager::IsEqual(
    nlohmann::json &jsonObject, const std::string &key, const std::string &value, bool checkEmpty)
{
    if (jsonObject.contains(key) && jsonObject[key].is_string()) {
        std::string  jsonValue = jsonObject.at(key).get<std::string>();
        if (checkEmpty && !jsonValue.empty() && jsonValue != value) {
            return false;
        } else if (value != jsonValue) {
            return false;
        }
    }
    return true;
}

bool AbilityAutoStartupDataManager::IsEqual(nlohmann::json &jsonObject, const std::string &key, int32_t value)
{
    if (jsonObject.contains(key) && jsonObject[key].is_number()) {
        if (value != jsonObject.at(key).get<int32_t>()) {
            return false;
        }
    }
    return true;
}

bool AbilityAutoStartupDataManager::IsEqual(const DistributedKv::Key &key, const AutoStartupInfo &info)
{
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Parse json string failed");
        return false;
    }

    if (!IsEqual(jsonObject, JSON_KEY_BUNDLE_NAME, info.bundleName)
        || !IsEqual(jsonObject, JSON_KEY_ABILITY_NAME, info.abilityName)
        || !IsEqual(jsonObject, JSON_KEY_MODULE_NAME, info.moduleName, true)
        || !IsEqual(jsonObject, JSON_KEY_APP_CLONE_INDEX, info.appCloneIndex)
        || !IsEqual(jsonObject, JSON_KEY_ACCESS_TOKENID, info.accessTokenId)
        || !IsEqual(jsonObject, JSON_KEY_USERID, info.userId)) {
        return false;
    }
    return true;
}

bool AbilityAutoStartupDataManager::IsEqual(const DistributedKv::Key &key, const std::string &accessTokenId)
{
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Parse json string failed");
        return false;
    }

    if (jsonObject.contains(JSON_KEY_ACCESS_TOKENID) && jsonObject[JSON_KEY_ACCESS_TOKENID].is_string()) {
        if (accessTokenId == jsonObject.at(JSON_KEY_ACCESS_TOKENID).get<std::string>()) {
            return true;
        }
    }
    return false;
}

bool AbilityAutoStartupDataManager::IsEqual(const DistributedKv::Key &key, int32_t userId)
{
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Parse json string failed");
        return false;
    }

    if (jsonObject.contains(JSON_KEY_USERID) && jsonObject[JSON_KEY_USERID].is_number()) {
        if (userId == jsonObject.at(JSON_KEY_USERID).get<int32_t>()) {
            return true;
        }
    }
    return false;
}
} // namespace AbilityRuntime
} // namespace OHOS
