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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_ABILITY_CONNECT_MANAGER_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_ABILITY_CONNECT_MANAGER_H

#include <atomic>
#include <map>
#include <memory>
#include <set>

#include "ui_extension_ability_record.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr int32_t INVALID_UI_EXTENSION_ABILITY_ID = 0;

class UIExtensionAbilityConnectManager : public std::enable_shared_from_this<UIExtensionAbilityConnectManager> {
public:
    using UIExtensionAbilityRecordMap = std::map<int32_t, std::shared_ptr<UIExtensionAbilityRecord>>;

    explicit UIExtensionAbilityConnectManager(const int32_t userId);
    virtual ~UIExtensionAbilityConnectManager();

    /**
     * @brief Generate uiextensionability id, if input id didn't exist, return it, else assign one.
     *
     * @param uiExtensionAbilityId Input uiextensionability id.
     * @return int32_t Generated uiextensionability id.
     */
    int32_t GenerateUIExtensionAbilityId(const int32_t uiExtensionAbilityId);

    /**
     * @brief Add uiextensionability record by id, if record exist, replace it.
     *
     * @param uiExtensionAbilityId uiextensionability id.
     * @param record uiextensionability record.
     */
    void AddUIExtensionAbilityRecord(const int32_t uiExtensionAbilityId,
        const std::shared_ptr<UIExtensionAbilityRecord> record);

    /**
     * @brief Remove uiextensionability record by id
     *
     * @param uiExtensionAbilityId uiextensionability id.
     */
    void RemoveUIExtensionAbilityRecord(const int32_t uiExtensionAbilityId);

    /**
     * @brief Check if host bundleName matched to stored record by specified id.
     *
     * @param uiExtensionAbilityId uiextensionability id.
     * @param hostBundleName bundleName of target uiextensionability.
     * @return true Matched.
     * @return false Not Match.
     */
    bool CheckUIExtensionAbilityLoaded(const int32_t uiExtensionAbilityId, const std::string hostBundleName);

private:
    int32_t userId_;
    static std::atomic_int32_t uiExtensionAbilityId_;
    std::mutex mutex_;
    std::set<int32_t> uiExtensionAbilityIdSet_;
    UIExtensionAbilityRecordMap uiExtensionAbilityRecords_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_EXTENSION_ABILITY_CONNECT_MANAGER_H
