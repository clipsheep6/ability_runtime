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

#include "app_exit_reason_data_manager.h"

#include <algorithm>
#include <chrono>
#include <unistd.h>

#include "errors.h"
#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms
constexpr const char *APP_EXIT_REASON_STORAGE_DIR = "/data/service/el1/public/database/app_exit_reason";
const std::string JSON_KEY_REASON = "reason";
const std::string JSON_KEY_TIME_STAMP = "time_stamp";
const std::string JSON_KEY_ABILITY_LIST = "ability_list";
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
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = APP_EXIT_REASON_STORAGE_DIR };

    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, appId_, storeId_, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS) {
        HILOG_ERROR("return error: %{public}d", status);
    } else {
        HILOG_INFO("get kvStore success");
    }
    return status;
}

bool AppExitReasonDataManager::CheckKvStore()
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
        HILOG_INFO("try times: %{public}d", tryTimes);
        usleep(CHECK_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

int32_t AppExitReasonDataManager::SetAppExitReason(
    const std::string &bundleName, const std::vector<std::string> &abilityList, const Reason &reason)
{
    if (bundleName.empty()) {
        HILOG_WARN("invalid value!");
        return ERR_INVALID_VALUE;
    }

    HILOG_DEBUG("bundleName: %{public}s", bundleName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            HILOG_ERROR("kvStore is nullptr");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key(bundleName);
    DistributedKv::Value value = ConvertAppExitReasonInfoToValue(abilityList, reason);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        HILOG_ERROR("insert data to kvStore error: %{public}d", status);
        return ERR_INVALID_OPERATION;
    }

    dataManager_.CloseKvStore(appId_, kvStorePtr_);
    kvStorePtr_ = nullptr;

    return ERR_OK;
}

int32_t AppExitReasonDataManager::DeleteAppExitReason(const std::string &bundleName)
{
    if (bundleName.empty()) {
        HILOG_WARN("invalid value!");
        return ERR_INVALID_VALUE;
    }

    HILOG_DEBUG("bundleName: %{public}s", bundleName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            HILOG_ERROR("kvStore is nullptr");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key(bundleName);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        HILOG_ERROR("delete data from kvStore error: %{public}d", status);
        return ERR_INVALID_OPERATION;
    }

    dataManager_.CloseKvStore(appId_, kvStorePtr_);
    kvStorePtr_ = nullptr;

    return ERR_OK;
}

int32_t AppExitReasonDataManager::GetAppExitReason(
    const std::string &bundleName, const std::string &abilityName, bool &isSetReason, Reason &reason)
{
    if (bundleName.empty()) {
        HILOG_WARN("invalid value!");
        return ERR_INVALID_VALUE;
    }

    HILOG_DEBUG("bundleName: %{public}s", bundleName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            HILOG_ERROR("kvStore is nullptr");
            return ERR_NO_INIT;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        HILOG_ERROR("get entries error: %{public}d", status);
        return ERR_INVALID_VALUE;
    }

    std::vector<std::string> abilityList;
    int64_t time_stamp;
    isSetReason = false;
    for (const auto &item : allEntries) {
        if (item.key.ToString() == bundleName) {
            ConvertAppExitReasonInfoFromValue(item.value, reason, time_stamp, abilityList);
            for (auto ability : abilityList) {
                HILOG_INFO("current ability: %{public}s in database", ability.c_str());
            }
            auto pos = std::find(abilityList.begin(), abilityList.end(), abilityName);
            if (pos != abilityList.end()) {
                isSetReason = true;
                abilityList.erase(std::remove(abilityList.begin(), abilityList.end(), abilityName), abilityList.end());
                UpdateAppExitReason(bundleName, abilityList, reason);
            }
            HILOG_INFO(
                "current bundle name: %{public}s reason: %{public}d abilityName:%{public}s isSetReason:%{public}d",
                item.key.ToString().c_str(), reason, abilityName.c_str(), isSetReason);
            if (abilityList.empty()) {
                InnerDeleteAppExitReason(bundleName);
            }
            break;
        }
    }

    dataManager_.CloseKvStore(appId_, kvStorePtr_);
    kvStorePtr_ = nullptr;

    return ERR_OK;
}

void AppExitReasonDataManager::UpdateAppExitReason(
    const std::string &bundleName, const std::vector<std::string> &abilityList, const Reason &reason)
{
    if (kvStorePtr_ == nullptr) {
        HILOG_ERROR("kvStore is nullptr");
        return;
    }

    DistributedKv::Key key(bundleName);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        HILOG_ERROR("delete data from kvStore error: %{public}d", status);
        return;
    }

    DistributedKv::Value value = ConvertAppExitReasonInfoToValue(abilityList, reason);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        HILOG_ERROR("insert data to kvStore error: %{public}d", status);
    }
}

DistributedKv::Value AppExitReasonDataManager::ConvertAppExitReasonInfoToValue(
    const std::vector<std::string> &abilityList, const Reason &reason)
{
    std::chrono::milliseconds nowMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_REASON, reason },
        { JSON_KEY_TIME_STAMP, nowMs.count() },
        { JSON_KEY_ABILITY_LIST, abilityList },
    };
    DistributedKv::Value value(jsonObject.dump());
    HILOG_INFO("value: %{public}s", value.ToString().c_str());
    return value;
}

void AppExitReasonDataManager::ConvertAppExitReasonInfoFromValue(
    const DistributedKv::Value &value, Reason &reason, int64_t &time_stamp, std::vector<std::string> &abilityList)
{
    nlohmann::json jsonObject = nlohmann::json::parse(value.ToString());
    if (!jsonObject.at(JSON_KEY_REASON).is_null()) {
        reason = jsonObject.at(JSON_KEY_REASON).get<Reason>();
    }
    if (!jsonObject.at(JSON_KEY_TIME_STAMP).is_null()) {
        time_stamp = jsonObject.at(JSON_KEY_TIME_STAMP).get<int64_t>();
    }
    if (!jsonObject.at(JSON_KEY_ABILITY_LIST).is_null()) {
        abilityList = jsonObject.at(JSON_KEY_ABILITY_LIST).get<std::vector<std::string>>();
    }
}

void AppExitReasonDataManager::InnerDeleteAppExitReason(const std::string &bundleName)
{
    if (kvStorePtr_ == nullptr) {
        HILOG_ERROR("kvStore is nullptr");
        return;
    }

    DistributedKv::Key key(bundleName);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        HILOG_ERROR("delete data from kvStore error: %{public}d", status);
    }
}
} // namespace AAFwk
} // namespace OHOS
