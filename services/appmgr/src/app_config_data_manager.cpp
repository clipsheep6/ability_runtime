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

#include "app_config_data_manager.h"

#include <unistd.h>

#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;
constexpr const char *APP_CONFIG_STORAGE_DIR = "/data/service/el1/public/database/app_config_data";
const std::string KEY_WAITING_DEBUG_INFO = "WaitingDebugInfo";
} // namespace
const DistributedKv::AppId AppConfigDataManager::APP_ID = { "app_config_data_storage" };
const DistributedKv::StoreId AppConfigDataManager::STORE_ID = { "app_config_data_infos" };
AppConfigDataManager::AppConfigDataManager() {}

AppConfigDataManager::~AppConfigDataManager()
{
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(APP_ID, kvStorePtr_);
    }
}

DistributedKv::Status AppConfigDataManager::GetKvStore()
{
    DistributedKv::Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = APP_CONFIG_STORAGE_DIR
    };

    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::APPMGR, "Return error is %{public}d.", status);
        return status;
    }

    return status;
}

bool AppConfigDataManager::CheckKvStore()
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
        usleep(CHECK_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

int32_t AppConfigDataManager::SetAppWaitingDebugInfo(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called, bundle name is %{public}s.", bundleName.c_str());
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid value.");
        return ERR_INVALID_VALUE;
    }

    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::APPMGR, "The kvStore is nullptr.");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key(KEY_WAITING_DEBUG_INFO);
    DistributedKv::Value value(bundleName);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::APPMGR, "Insert data to kvStore error is %{public}d.", status);
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

int32_t AppConfigDataManager::ClearAppWaitingDebugInfo()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::APPMGR, "The kvStore is nullptr.");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key(KEY_WAITING_DEBUG_INFO);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::APPMGR, "Delete data from kvStore error is %{public}d.", status);
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

int32_t AppConfigDataManager::GetAppWaitingDebugList(std::vector<std::string> &bundleNameList)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::APPMGR, "The kvStore is nullptr.");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Status status;
    std::vector<DistributedKv::Entry> allEntries;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->GetEntries(nullptr, allEntries);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::APPMGR, "Get entries error is %{public}d.", status);
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        if (item.key.ToString() == KEY_WAITING_DEBUG_INFO) {
            bundleNameList.emplace_back(item.value.ToString());
        }
    }
    TAG_LOGD(AAFwkTag::APPMGR, "The bundle name list size is %{public}zu.", bundleNameList.size());
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
