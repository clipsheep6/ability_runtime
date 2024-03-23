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

#ifndef OHOS_ABILITY_RUNTIME_APP_CONFIG_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_APP_CONFIG_DATA_MANAGER_H

#include <mutex>
#include <vector>

#include "distributed_kv_data_manager.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
class AppConfigDataManager : public DelayedSingleton<AppConfigDataManager> {
public:
    AppConfigDataManager();

    virtual ~AppConfigDataManager();

    int32_t SetAppWaitingDebugInfo(const std::string &bundleName);

    int32_t ClearAppWaitingDebugInfo();

    int32_t GetAppWaitingDebugList(std::vector<std::string> &bundleNameList);

private:
    DistributedKv::Status GetKvStore();
    bool CheckKvStore();

    static const DistributedKv::AppId APP_ID;
    static const DistributedKv::StoreId STORE_ID;
    DistributedKv::DistributedKvDataManager dataManager_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_CONFIG_DATA_MANAGER_H