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

#ifndef OHOS_ABILITY_RUNTIME_APP_EXIT_REASON_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_APP_EXIT_REASON_DATA_MANAGER_H

#include <mutex>
#include <string>
#include <vector>

#include "ability_util.h"
// #include "distributed_kv_data_manager.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
class AppExitReasonDataManager : public DelayedSingleton<AppExitReasonDataManager> {
public:
    AppExitReasonDataManager();

    virtual ~AppExitReasonDataManager();

    int32_t SetAppExitReason(
        const std::string &bundleName, const std::vector<std::string> &abilityList, const AAFwk::Reason &reason);

    int32_t GetAppExitReason(
        const std::string &bundleName, const std::string &abilityName, bool &isSetReason, AAFwk::Reason &reason);

    int32_t DeleteAppExitReason(const std::string &bundleName);

private:
    DistributedKv::Status GetKvStore();
    bool CheckKvStore();
    DistributedKv::Value ConvertAppExitReasonInfoToValue(
        const std::vector<std::string> &abilityList, const AAFwk::Reason &reason);
    void ConvertAppExitReasonInfoFromValue(const DistributedKv::Value &value, AAFwk::Reason &reason,
        int64_t &time_stamp, std::vector<std::string> &abilityList);
    void UpdateAppExitReason(
        const std::string &bundleName, const std::vector<std::string> &abilityList, const AAFwk::Reason &reason);
    void InnerDeleteAppExitReason(const std::string &bundleName);

    // const DistributedKv::AppId appId_ { "app_exit_reason_storage" };
    // const DistributedKv::StoreId storeId_ { "app_exit_reason_infos" };
    // DistributedKv::DistributedKvDataManager dataManager_;
    // std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_EXIT_REASON_DATA_MANAGER_H