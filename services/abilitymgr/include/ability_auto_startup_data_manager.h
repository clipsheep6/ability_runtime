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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_DATA_MANAGER_H

#include <mutex>
#include <vector>

#include "auto_startup_info.h"
#include "distributed_kv_data_manager.h"
#include "nlohmann/json.hpp"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityAutoStartupDataManager : public DelayedSingleton<AbilityAutoStartupDataManager> {
public:
    AbilityAutoStartupDataManager();

    virtual ~AbilityAutoStartupDataManager();

    int32_t InsertAutoStartupData(const AutoStartupInfo &info, bool isAutoStartup, bool isEdmForce);

    int32_t UpdateAutoStartupData(const AutoStartupInfo &info, bool isAutoStartup, bool isEdmForce);

    int32_t DeleteAutoStartupData(const AutoStartupInfo &info);

    int32_t DeleteAutoStartupData(const std::string &bundleName, int32_t accessTokenId);

    AutoStartupStatus QueryAutoStartupData(const AutoStartupInfo &info);

    int32_t QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList, int32_t userId);

    int32_t GetCurrentAppAutoStartupData(const std::string &bundleName,
        std::vector<AutoStartupInfo> &infoList, const std::string &accessTokenId);

private:
    DistributedKv::Status GetKvStore();
    bool CheckKvStore();
    DistributedKv::Value ConvertAutoStartupStatusToValue(
        bool isAutoStartup, bool isEdmForce, const std::string &abilityTypeName);
    void ConvertAutoStartupStatusFromValue(const DistributedKv::Value &value, bool &isAutoStartup, bool &isEdmForce);
    DistributedKv::Key ConvertAutoStartupDataToKey(const AutoStartupInfo &info);
    AutoStartupInfo ConvertAutoStartupInfoFromKeyAndValue(
        const DistributedKv::Key &key, const DistributedKv::Value &value);
    bool IsEqual(nlohmann::json &jsonObject, const std::string &key,
        const std::string &value, bool checkEmpty = false);
    bool IsEqual(nlohmann::json &jsonObject, const std::string &key, int32_t value);
    bool IsEqual(const DistributedKv::Key &key, const AutoStartupInfo &info);
    bool IsEqual(const DistributedKv::Key &key, const std::string &accessTokenId);
    bool IsEqual(const DistributedKv::Key &key, int32_t userId);

    static const DistributedKv::AppId APP_ID;
    static const DistributedKv::StoreId STORE_ID;
    DistributedKv::DistributedKvDataManager dataManager_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_DATA_MANAGER_H