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

#include "app_exit_reason_data_manager.h"

#include <algorithm>
#include <chrono>
#include <unistd.h>

#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms
constexpr const char *APP_EXIT_REASON_STORAGE_DIR = "/data/service/el1/public/database/app_exit_reason";
const std::string JSON_KEY_REASON = "reason";
const std::string JSON_KEY_EXIT_MSG = "exit_msg";
const std::string JSON_KEY_TIME_STAMP = "time_stamp";
const std::string JSON_KEY_ABILITY_LIST = "ability_list";
const std::string KEY_RECOVER_INFO_PREFIX = "recover_info";
const std::string JSON_KEY_RECOVER_INFO_LIST = "recover_info_list";
const std::string JSON_KEY_SESSION_ID_LIST = "session_id_list";
const std::string JSON_KEY_EXTENSION_NAME = "extension_name";
const std::string SEPARATOR = ":";
} // namespace
AppExitReasonDataManager::AppExitReasonDataManager() {}

AppExitReasonDataManager::~AppExitReasonDataManager()
{
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(appId_, kvStorePtr_);
    }
}

DistributedKv::Status AppExitReasonDataManager::GetKvStore()
{
    DistributedKv::Options options = { .createIfMissing = true,
        .encrypt = false,
        .autoSync = true,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = APP_EXIT_REASON_STORAGE_DIR };

    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, appId_, storeId_, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "return error: %{public}d", status);
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "get kvStore success");
    }
    return status;
}

bool AppExitReasonDataManager::CheckKvStore()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AppExitReasonDataManager::CheckKvStore start");
    if (kvStorePtr_ != nullptr) {
        return true;
    }
    int32_t tryTimes = MAX_TIMES;
    while (tryTimes > 0) {
        DistributedKv::Status status = GetKvStore();
        if (status == DistributedKv::Status::SUCCESS && kvStorePtr_ != nullptr) {
            return true;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "try times: %{public}d", tryTimes);
        usleep(CHECK_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

int32_t AppExitReasonDataManager::SetAppExitReason(const std::string &bundleName,
    const std::vector<std::string> &abilityList, const AAFwk::ExitReason &exitReason)
{
    if (bundleName.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "bundleName: %{public}s", bundleName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "kvStore is nullptr");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key(bundleName);
    DistributedKv::Value value = ConvertAppExitReasonInfoToValue(abilityList, exitReason);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "insert data to kvStore error: %{public}d", status);
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

int32_t AppExitReasonDataManager::DeleteAppExitReason(const std::string &bundleName)
{
    if (bundleName.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid value.");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "bundleName: %{public}s.", bundleName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "kvStore is nullptr.");
            return ERR_NO_INIT;
        }
    }

    std::string keyUiExten = bundleName + SEPARATOR;
    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get entries error: %{public}d", status);
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        const auto &keyValue = item.key.ToString();
        if (keyValue != bundleName && keyValue.find(keyUiExten) == std::string::npos) {
            continue;
        }

        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        auto errCode = kvStorePtr_->Delete(item.key);
        status = (errCode != DistributedKv::Status::SUCCESS) ? errCode : status;
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "delete data from kvStore error: %{public}d", status);
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

int32_t AppExitReasonDataManager::GetAppExitReason(const std::string &bundleName, const std::string &abilityName,
    bool &isSetReason, AAFwk::ExitReason &exitReason)
{
    if (bundleName.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid value!");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "bundleName: %{public}s!", bundleName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "kvStore is nullptr!");
            return ERR_NO_INIT;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get entries error: %{public}d", status);
        return ERR_INVALID_VALUE;
    }

    std::vector<std::string> abilityList;
    int64_t time_stamp;
    isSetReason = false;
    for (const auto &item : allEntries) {
        if (item.key.ToString() == bundleName) {
            ConvertAppExitReasonInfoFromValue(item.value, exitReason, time_stamp, abilityList);
            auto pos = std::find(abilityList.begin(), abilityList.end(), abilityName);
            if (pos != abilityList.end()) {
                isSetReason = true;
                abilityList.erase(std::remove(abilityList.begin(), abilityList.end(), abilityName), abilityList.end());
                UpdateAppExitReason(bundleName, abilityList, exitReason);
            }
            TAG_LOGI(AAFwkTag::ABILITYMGR, "current bundle name: %{public}s reason: %{public}d exitMsg: %{public}s \
                abilityName:%{public}s isSetReason:%{public}d",
                item.key.ToString().c_str(), exitReason.reason, exitReason.exitMsg.c_str(), abilityName.c_str(),
                isSetReason);
            if (abilityList.empty()) {
                InnerDeleteAppExitReason(bundleName);
            }
            break;
        }
    }

    return ERR_OK;
}

void AppExitReasonDataManager::UpdateAppExitReason(const std::string &bundleName,
    const std::vector<std::string> &abilityList, const AAFwk::ExitReason &exitReason)
{
    if (kvStorePtr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "kvStore is nullptr.");
        return;
    }

    DistributedKv::Key key(bundleName);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "delete data from kvStore error: %{public}d.", status);
        return;
    }

    DistributedKv::Value value = ConvertAppExitReasonInfoToValue(abilityList, exitReason);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "insert data to kvStore error: %{public}d", status);
    }
}

DistributedKv::Value AppExitReasonDataManager::ConvertAppExitReasonInfoToValue(
    const std::vector<std::string> &abilityList, const AAFwk::ExitReason &exitReason)
{
    std::chrono::milliseconds nowMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_REASON, exitReason.reason },
        { JSON_KEY_EXIT_MSG, exitReason.exitMsg },
        { JSON_KEY_TIME_STAMP, nowMs.count() },
        { JSON_KEY_ABILITY_LIST, abilityList },
    };
    DistributedKv::Value value(jsonObject.dump());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "value: %{public}s", value.ToString().c_str());
    return value;
}

void AppExitReasonDataManager::ConvertAppExitReasonInfoFromValue(const DistributedKv::Value &value,
    AAFwk::ExitReason &exitReason, int64_t &time_stamp, std::vector<std::string> &abilityList)
{
    nlohmann::json jsonObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to parse json sting.");
        return;
    }
    if (jsonObject.contains(JSON_KEY_REASON) && jsonObject[JSON_KEY_REASON].is_number_integer()) {
        exitReason.reason = jsonObject.at(JSON_KEY_REASON).get<AAFwk::Reason>();
    }
    if (jsonObject.contains(JSON_KEY_EXIT_MSG) && jsonObject[JSON_KEY_EXIT_MSG].is_string()) {
        exitReason.exitMsg = jsonObject.at(JSON_KEY_EXIT_MSG).get<std::string>();
    }
    if (jsonObject.contains(JSON_KEY_TIME_STAMP) && jsonObject[JSON_KEY_TIME_STAMP].is_number_integer()) {
        time_stamp = jsonObject.at(JSON_KEY_TIME_STAMP).get<int64_t>();
    }
    if (jsonObject.contains(JSON_KEY_ABILITY_LIST) && jsonObject[JSON_KEY_ABILITY_LIST].is_array()) {
        abilityList.clear();
        auto size = jsonObject[JSON_KEY_ABILITY_LIST].size();
        for (size_t i = 0; i < size; i++) {
            if (jsonObject[JSON_KEY_ABILITY_LIST][i].is_string()) {
                abilityList.emplace_back(jsonObject[JSON_KEY_ABILITY_LIST][i]);
            }
        }
    }
}

void AppExitReasonDataManager::InnerDeleteAppExitReason(const std::string &bundleName)
{
    if (kvStorePtr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "kvStore is nullptr");
        return;
    }

    DistributedKv::Key key(bundleName);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "delete data from kvStore error: %{public}d", status);
    }
}

int32_t AppExitReasonDataManager::AddAbilityRecoverInfo(const std::string &bundleName,
    const std::string &moduleName, const std::string &abilityName, const int &sessionId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "AddAbilityRecoverInfo bundle %{public}s module %{public}s ability %{public}s id %{public}d ",
        bundleName.c_str(), moduleName.c_str(), abilityName.c_str(), sessionId);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "kvStore is nullptr");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key(KEY_RECOVER_INFO_PREFIX + bundleName);
    DistributedKv::Value value;
    DistributedKv::Status status = kvStorePtr_->Get(key, value);
    if (status != DistributedKv::Status::SUCCESS && status != DistributedKv::Status::KEY_NOT_FOUND) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AddAbilityRecoverInfo get error: %{public}d", status);
        return ERR_INVALID_VALUE;
    }

    std::vector<std::string> recoverInfoList;
    std::vector<int> sessionIdList;
    std::string recoverInfo = moduleName + abilityName;
    if (status == DistributedKv::Status::SUCCESS) {
        ConvertAbilityRecoverInfoFromValue(value, recoverInfoList, sessionIdList);
        auto pos = std::find(recoverInfoList.begin(), recoverInfoList.end(), recoverInfo);
        if (pos != recoverInfoList.end()) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "AddAbilityRecoverInfo recoverInfo already record");
            int index = std::distance(recoverInfoList.begin(), pos);
            sessionIdList[index] = sessionId;
            return ERR_OK;
        }
    }

    recoverInfoList.emplace_back(recoverInfo);
    sessionIdList.emplace_back(sessionId);
    value = ConvertAbilityRecoverInfoToValue(recoverInfoList, sessionIdList);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "insert data to kvStore error : %{public}d", status);
        return ERR_INVALID_OPERATION;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "AddAbilityRecoverInfo finish");
    return ERR_OK;
}

int32_t AppExitReasonDataManager::DeleteAbilityRecoverInfo(
    const std::string &bundleName, const std::string &moduleName, const std::string &abilityName)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DeleteAbilityRecoverInfo bundle %{public}s module %{public}s ability %{public}s ",
        bundleName.c_str(), moduleName.c_str(), abilityName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "kvStore is nullptr.");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key(KEY_RECOVER_INFO_PREFIX + bundleName);
    DistributedKv::Value value;
    DistributedKv::Status status = kvStorePtr_->Get(key, value);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "DeleteAbilityRecoverInfo get error: %{public}d", status);
        return ERR_INVALID_VALUE;
    }

    std::vector<std::string> recoverInfoList;
    std::vector<int> sessionIdList;
    std::string recoverInfo = moduleName + abilityName;
    ConvertAbilityRecoverInfoFromValue(value, recoverInfoList, sessionIdList);
    auto pos = std::find(recoverInfoList.begin(), recoverInfoList.end(), recoverInfo);
    if (pos != recoverInfoList.end()) {
        recoverInfoList.erase(std::remove(recoverInfoList.begin(), recoverInfoList.end(), recoverInfo),
            recoverInfoList.end());
        int index = std::distance(recoverInfoList.begin(), pos);
        sessionIdList.erase(std::remove(sessionIdList.begin(), sessionIdList.end(), sessionIdList[index]),
            sessionIdList.end());
        UpdateAbilityRecoverInfo(bundleName, recoverInfoList, sessionIdList);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "DeleteAbilityRecoverInfo remove recoverInfo succeed");
    }
    if (recoverInfoList.empty()) {
        InnerDeleteAbilityRecoverInfo(bundleName);
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DeleteAbilityRecoverInfo finished");
    return ERR_OK;
}

int32_t AppExitReasonDataManager::GetAbilityRecoverInfo(
    const std::string &bundleName, const std::string &moduleName, const std::string &abilityName, bool &hasRecoverInfo)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetAbilityRecoverInfo bundle %{public}s module %{public}s abillity %{public}s ",
        bundleName.c_str(), moduleName.c_str(), abilityName.c_str());
    hasRecoverInfo = false;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "kvStore is nullptr!");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key(KEY_RECOVER_INFO_PREFIX + bundleName);
    DistributedKv::Value value;
    DistributedKv::Status status = kvStorePtr_->Get(key, value);
    if (status != DistributedKv::Status::SUCCESS) {
        if (status == DistributedKv::Status::KEY_NOT_FOUND) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "GetAbilityRecoverInfo KEY_NOT_FOUND.");
        } else {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetAbilityRecoverInfo error: %{public}d.", status);
        }
        return ERR_INVALID_VALUE;
    }

    std::vector<std::string> recoverInfoList;
    std::vector<int> sessionIdList;
    std::string recoverInfo = moduleName + abilityName;
    ConvertAbilityRecoverInfoFromValue(value, recoverInfoList, sessionIdList);
    auto pos = std::find(recoverInfoList.begin(), recoverInfoList.end(), recoverInfo);
    if (pos != recoverInfoList.end()) {
        hasRecoverInfo = true;
        TAG_LOGI(AAFwkTag::ABILITYMGR, "GetAbilityRecoverInfo hasRecoverInfo found info");
    }
    return ERR_OK;
}

int32_t AppExitReasonDataManager::GetAbilitySessionId(const std::string &bundleName,
    const std::string &moduleName, const std::string &abilityName, int &sessionId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetAbilityRecoverInfo bundle %{public}s bundle %{public}s bundle %{public}s  ",
        bundleName.c_str(), moduleName.c_str(), abilityName.c_str());
    sessionId = 0;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "the kvStore is nullptr.");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key(KEY_RECOVER_INFO_PREFIX + bundleName);
    DistributedKv::Value value;
    DistributedKv::Status status = kvStorePtr_->Get(key, value);
    if (status != DistributedKv::Status::SUCCESS) {
        if (status == DistributedKv::Status::KEY_NOT_FOUND) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "GetAbilityRecoverInfo KEY_NOT_FOUND");
        } else {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetAbilityRecoverInfo error: %{public}d", status);
        }
        return ERR_INVALID_VALUE;
    }

    std::vector<std::string> recoverInfoList;
    std::vector<int> sessionIdList;
    std::string recoverInfo = moduleName + abilityName;
    ConvertAbilityRecoverInfoFromValue(value, recoverInfoList, sessionIdList);
    auto pos = std::find(recoverInfoList.begin(), recoverInfoList.end(), recoverInfo);
    if (pos != recoverInfoList.end()) {
        int index = std::distance(recoverInfoList.begin(), pos);
        sessionId = sessionIdList[index];
        TAG_LOGI(AAFwkTag::ABILITYMGR, "GetAbilityRecoverInfo sessionId found info %{public}d ", sessionId);
    }
    return ERR_OK;
}

int32_t AppExitReasonDataManager::SetUIExtensionAbilityExitReason(
    const std::string &bundleName, const std::vector<std::string> &extensionList, const AAFwk::ExitReason &exitReason)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    if (bundleName.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Invalid bundle name.");
        return ERR_INVALID_VALUE;
    }

    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "kvStorePtr_ is nullptr.");
            return ERR_NO_INIT;
        }
    }

    for (const auto &extension : extensionList) {
        std::string keyEx = bundleName + SEPARATOR + extension;
        DistributedKv::Key key(keyEx);
        DistributedKv::Value value = ConvertAppExitReasonInfoToValueOfExtensionName(extension, exitReason);
        DistributedKv::Status status;
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            status = kvStorePtr_->Put(key, value);
        }

        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "Insert data to kvStore error: %{public}d", status);
        }
    }

    return ERR_OK;
}

bool AppExitReasonDataManager::GetUIExtensionAbilityExitReason(const std::string &keyEx,
    AAFwk::ExitReason &exitReason)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "kvStorePtr_ is nullptr.");
            return false;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get entries error: %{public}d", status);
        return false;
    }
    std::vector<std::string> abilityList;
    int64_t time_stamp;
    bool isHaveReason = false;
    for (const auto &item : allEntries) {
        if (item.key.ToString() == keyEx) {
            ConvertAppExitReasonInfoFromValue(item.value, exitReason, time_stamp, abilityList);
            isHaveReason = true;
            InnerDeleteAppExitReason(keyEx);
            break;
        }
    }

    return isHaveReason;
}

void AppExitReasonDataManager::UpdateAbilityRecoverInfo(const std::string &bundleName,
    const std::vector<std::string> &recoverInfoList, const std::vector<int> &sessionIdList)
{
    if (kvStorePtr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "kvStore is nullptr.");
        return;
    }

    DistributedKv::Key key(KEY_RECOVER_INFO_PREFIX + bundleName);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "delete data from kvStore error: %{public}d", status);
        return;
    }

    DistributedKv::Value value = ConvertAbilityRecoverInfoToValue(recoverInfoList, sessionIdList);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "insert data to kvStore failed: %{public}d", status);
    }
}

DistributedKv::Value AppExitReasonDataManager::ConvertAbilityRecoverInfoToValue(
    const std::vector<std::string> &recoverInfoList, const std::vector<int> &sessionIdList)
{
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_RECOVER_INFO_LIST, recoverInfoList },
        { JSON_KEY_SESSION_ID_LIST, sessionIdList },
    };
    DistributedKv::Value value(jsonObject.dump());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ConvertAbilityRecoverInfoToValue value: %{public}s", value.ToString().c_str());
    return value;
}

void AppExitReasonDataManager::ConvertAbilityRecoverInfoFromValue(const DistributedKv::Value &value,
    std::vector<std::string> &recoverInfoList, std::vector<int> &sessionIdList)
{
    nlohmann::json jsonObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to parse json sting.");
        return;
    }
    if (jsonObject.contains(JSON_KEY_RECOVER_INFO_LIST)
        && jsonObject[JSON_KEY_RECOVER_INFO_LIST].is_array()) {
        recoverInfoList.clear();
        auto size = jsonObject[JSON_KEY_RECOVER_INFO_LIST].size();
        for (size_t i = 0; i < size; i++) {
            if (jsonObject[JSON_KEY_RECOVER_INFO_LIST][i].is_string()) {
                recoverInfoList.emplace_back(jsonObject[JSON_KEY_RECOVER_INFO_LIST][i]);
            }
        }
    }
    if (jsonObject.contains(JSON_KEY_SESSION_ID_LIST)
        && jsonObject[JSON_KEY_SESSION_ID_LIST].is_array()) {
        sessionIdList.clear();
        auto size = jsonObject[JSON_KEY_SESSION_ID_LIST].size();
        for (size_t i = 0; i < size; i++) {
            if (jsonObject[JSON_KEY_SESSION_ID_LIST][i].is_number_integer()) {
                sessionIdList.emplace_back(jsonObject[JSON_KEY_SESSION_ID_LIST][i]);
            }
        }
    }
}

void AppExitReasonDataManager::InnerDeleteAbilityRecoverInfo(const std::string &bundleName)
{
    if (kvStorePtr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "kvStore is nullptr");
        return;
    }

    DistributedKv::Key key(KEY_RECOVER_INFO_PREFIX + bundleName);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "delete data from kvStore error: %{public}d", status);
    }
}

DistributedKv::Value AppExitReasonDataManager::ConvertAppExitReasonInfoToValueOfExtensionName(
    const std::string &extensionListName, const AAFwk::ExitReason &exitReason)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    std::chrono::milliseconds nowMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_REASON, exitReason.reason },
        { JSON_KEY_EXIT_MSG, exitReason.exitMsg },
        { JSON_KEY_TIME_STAMP, nowMs.count() },
        { JSON_KEY_EXTENSION_NAME, extensionListName },
    };
    DistributedKv::Value value(jsonObject.dump());
    return value;
}
} // namespace AbilityRuntime
} // namespace OHOS
