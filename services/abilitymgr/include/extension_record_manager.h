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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_MANAGER_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_MANAGER_H

#include <atomic>
#include <map>
#include <memory>
#include <set>

#include "ability_record.h"
#include "extension_record.h"
#include "ui_extension_host_info.h"

namespace OHOS {
namespace AbilityRuntime {
class ExtensionRecordManager : public std::enable_shared_from_this<ExtensionRecordManager> {
public:
    using ExtensionAbilityRecordMap = std::map<int32_t, std::shared_ptr<ExtensionRecord>>;

    explicit ExtensionRecordManager(const int32_t userId);
    virtual ~ExtensionRecordManager();

    /**
     * @brief Generate extension record id, if input id didn't exist, return it, else assign one.
     *
     * @param extensionRecordId Input extension record id.
     * @return int32_t Generated extension record id.
     */
    int32_t GenerateExtensionRecordId(const int32_t extensionRecordId);

    /**
     * @brief Add extension record by id, if record exist, replace it.
     *
     * @param extensionRecordId extension record id.
     * @param record extension record.
     */
    void AddExtensionRecord(const int32_t extensionRecordId, const std::shared_ptr<ExtensionRecord> &record);

    /**
     * @brief Remove extension record by id
     *
     * @param extensionRecordId extension record id.
     */
    void RemoveExtensionRecord(const int32_t extensionRecordId);

    /**
     * @brief Add extension record to terminate list by id
     *
     * @param extensionRecordId extension record id.
     */
    void AddExtensionRecordToTerminatedList(const int32_t extensionRecordId);

    static bool IsBelongToManager(const AppExecFwk::AbilityInfo &abilityInfo);

    bool IsFocused(int32_t extensionRecordId, const sptr<IRemoteObject>& focusToken);

    int32_t StartAbility(const AAFwk::AbilityRequest &abilityRequest);

    int32_t CreateExtensionRecord(
        const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord, const std::string &hostBundleName,
        std::shared_ptr<ExtensionRecord> &extensionRecord, int32_t &extensionRecordId);

    int32_t GetOrCreateExtensionRecord(const AAFwk::AbilityRequest &abilityRequest, const std::string &hostBundleName,
        std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord, bool &isLoaded);

    std::shared_ptr<AAFwk::AbilityRecord> GetAbilityRecordBySessionInfo(const sptr<AAFwk::SessionInfo> &sessionInfo);

    int32_t GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token, UIExtensionHostInfo &hostInfo);

    void LoadTimeout(int32_t extensionRecordId);
    void ForegroundTimeout(int32_t extensionRecordId);
    void BackgroundTimeout(int32_t extensionRecordId);
    void TerminateTimeout(int32_t extensionRecordId);
private:
    inline std::shared_ptr<ExtensionRecord> GetExtensionRecordById(int32_t extensionRecordId);

private:
    int32_t userId_;
    static std::atomic_int32_t extensionRecordId_;
    std::mutex mutex_;
    std::set<int32_t> extensionRecordIdSet_;
    ExtensionAbilityRecordMap extensionRecords_;
    ExtensionAbilityRecordMap terminateRecords_;

    sptr<IRemoteObject> GetRootCallerTokenLocked(int32_t extensionRecordId);

    int32_t GetOrCreateExtensionRecordInner(const AAFwk::AbilityRequest &abilityRequest,
        const std::string &hostBundleName, std::shared_ptr<ExtensionRecord> &extensionRecord, bool &isLoaded);

    int32_t GetExtensionRecord(const int32_t extensionRecordId, const std::string &hostBundleName,
        std::shared_ptr<ExtensionRecord> &extensionRecord, bool &isLoaded);

    int32_t UpdateProcessName(const AAFwk::AbilityRequest &abilityRequest, std::shared_ptr<ExtensionRecord> &record);

    bool IsHostSpecifiedProcessValid(const AAFwk::AbilityRequest &abilityRequest,
        std::shared_ptr<ExtensionRecord> &record, const std::string &process);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_MANAGER_H
