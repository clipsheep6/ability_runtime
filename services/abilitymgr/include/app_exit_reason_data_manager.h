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
#include "distributed_kv_data_manager.h"
#include "exit_reason.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
class AppExitReasonDataManager : public DelayedSingleton<AppExitReasonDataManager> {
public:
    AppExitReasonDataManager();

    virtual ~AppExitReasonDataManager();

    int32_t SetAppExitReason(const std::string &bundleName, uint32_t accessTokenId,
        const std::vector<std::string> &abilityList, const AAFwk::ExitReason &exitReason);

    int32_t GetAppExitReason(const std::string &bundleName, uint32_t accessTokenId, const std::string &abilityName,
        bool &isSetReason, AAFwk::ExitReason &exitReason);

    int32_t DeleteAppExitReason(const std::string &bundleName, int32_t uid, int32_t appIndex);

    int32_t DeleteAppExitReason(const std::string &bundleName, uint32_t accessTokenId);

    int32_t AddAbilityRecoverInfo(uint32_t accessTokenId,
        const std::string &moduleName, const std::string &abilityName, const int &sessionId);

    int32_t DeleteAbilityRecoverInfo(
        uint32_t accessTokenId, const std::string &moduleName, const std::string &abilityName);

    int32_t DeleteAllRecoverInfoByTokenId(uint32_t tokenId);

    int32_t GetAbilityRecoverInfo(uint32_t accessTokenId,
        const std::string &moduleName, const std::string &abilityName, bool &hasRecoverInfo);

    int32_t SetUIExtensionAbilityExitReason(const std::string &bundleName,
        const std::vector<std::string> &extensionList, const AAFwk::ExitReason &exitReason);

    bool GetUIExtensionAbilityExitReason(const std::string &keyEx, AAFwk::ExitReason &exitReason);

    int32_t GetAbilitySessionId(uint32_t accessTokenId,
        const std::string &moduleName, const std::string &abilityName, int &sessionId);

private:
    DistributedKv::Status GetKvStore();
    bool CheckKvStore();
    DistributedKv::Value ConvertAppExitReasonInfoToValue(
        const std::vector<std::string> &abilityList, const AAFwk::ExitReason &exitReason);
    void ConvertAppExitReasonInfoFromValue(const DistributedKv::Value &value, AAFwk::ExitReason &exitReason,
        int64_t &time_stamp, std::vector<std::string> &abilityList);
    void UpdateAppExitReason(uint32_t accessTokenId, const std::vector<std::string> &abilityList,
        const AAFwk::ExitReason &exitReason);
    void InnerDeleteAppExitReason(const std::string &keyName);

    void UpdateAbilityRecoverInfo(uint32_t accessTokenId,
        const std::vector<std::string> &recoverInfoList, const std::vector<int> &sessionIdList);
    DistributedKv::Value ConvertAbilityRecoverInfoToValue(
        const std::vector<std::string> &recoverInfoList, const std::vector<int> &sessionIdList);
    void ConvertAbilityRecoverInfoFromValue(
        const DistributedKv::Value &value, std::vector<std::string> &recoverInfoList, std::vector<int> &sessionIdList);
    void InnerDeleteAbilityRecoverInfo(uint32_t accessTokenId);
    DistributedKv::Key GetAbilityRecoverInfoKey(uint32_t accessTokenId);
    DistributedKv::Value ConvertAppExitReasonInfoToValueOfExtensionName(
        const std::string &extensionListName, const AAFwk::ExitReason &exitReason);

    const DistributedKv::AppId appId_ { "app_exit_reason_storage" };
    const DistributedKv::StoreId storeId_ { "app_exit_reason_infos" };
    DistributedKv::DistributedKvDataManager dataManager_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_EXIT_REASON_DATA_MANAGER_H