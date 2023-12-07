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

#include "extension_record_manager.h"

#include "ability_util.h"
#include "hilog_wrapper.h"
#include "ui_extension_utils.h"
#include "ui_extension_record.h"

namespace OHOS {
namespace AbilityRuntime {
std::atomic_int32_t ExtensionRecordManager::extensionRecordId_ = INVALID_EXTENSION_RECORD_ID;

ExtensionRecordManager::ExtensionRecordManager(const int32_t userId) : userId_(userId)
{
    HILOG_DEBUG("constructor.");
}

ExtensionRecordManager::~ExtensionRecordManager()
{
    HILOG_INFO("deconstructor.");
}

int32_t ExtensionRecordManager::GenerateExtensionRecordId(const int32_t extensionRecordId)
{
    HILOG_DEBUG("Input id is %{public}d.", extensionRecordId);
    std::lock_guard<std::mutex> lock(mutex_);
    if (extensionRecordId != INVALID_EXTENSION_RECORD_ID &&
        !extensionRecordIdSet_.count(extensionRecordId)) {
        extensionRecordIdSet_.insert(extensionRecordId);
        extensionRecordId_ = extensionRecordId;
        return extensionRecordId_;
    }

    if (extensionRecordId == INVALID_EXTENSION_RECORD_ID) {
        ++extensionRecordId_;
    }

    while (extensionRecordIdSet_.count(extensionRecordId_)) {
        extensionRecordId_++;
    }

    return extensionRecordId_;
}

void ExtensionRecordManager::AddExtensionRecord(const int32_t extensionRecordId,
    const std::shared_ptr<ExtensionRecord> &record)
{
    HILOG_DEBUG("extensionRecordId %{public}d.", extensionRecordId);
    std::lock_guard<std::mutex> lock(mutex_);
    extensionRecords_.emplace(extensionRecordId, record);
}

void ExtensionRecordManager::RemoveExtensionRecord(const int32_t extensionRecordId)
{
    HILOG_DEBUG("extensionRecordId %{public}d.", extensionRecordId);
    std::lock_guard<std::mutex> lock(mutex_);
    extensionRecords_.erase(extensionRecordId);
}

bool ExtensionRecordManager::CheckExtensionLoaded(const int32_t extensionRecordId,
    const std::string &hostBundleName)
{
    HILOG_DEBUG("extensionRecordId %{public}d.", extensionRecordId);
    std::lock_guard<std::mutex> lock(mutex_);
    // find target record firstly
    auto it = extensionRecords_.find(extensionRecordId);
    if (it != extensionRecords_.end() && it->second != nullptr) {
        // check bundleName
        HILOG_DEBUG("Stored host bundleName: %{public}s, input bundleName is %{public}s.",
            it->second->hostBundleName_.c_str(), hostBundleName.c_str());
        if (it->second->hostBundleName_ == hostBundleName) {
            return true;
        }
    }
    HILOG_DEBUG("Not found stored id %{public}d.", extensionRecordId);
    return false;
}

bool ExtensionRecordManager::IsBelongToManager(const AppExecFwk::AbilityInfo &abilityInfo)
{
    // only support UIExtension now
    return AAFwk::UIExtensionUtils::IsUIExtension(abilityInfo.extensionAbilityType);
}

bool ExtensionRecordManager::IsFocused(int32_t extensionRecordId, const sptr<IRemoteObject>& focusToken)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sptr<IRemoteObject> rootCallerToken = GetRootCallerTokenLocked(extensionRecordId);
    bool isFocused = rootCallerToken == focusToken;
    HILOG_DEBUG("id: %{public}d isFocused: %{public}d.", extensionRecordId, isFocused);
    return isFocused;
}

sptr<IRemoteObject> ExtensionRecordManager::GetRootCallerTokenLocked(int32_t extensionRecordId)
{
    auto it = extensionRecords_.find(extensionRecordId);
    if (it != extensionRecords_.end() && it->second != nullptr) {
        sptr<IRemoteObject> rootCallerToken = it->second->GetRootCallerToken();
        if (rootCallerToken != nullptr) {
            return rootCallerToken;
        }
        if (!it->second->ContinueToGetCallerToken()) {
            return it->second->GetCallToken();
        }
        auto callerToken = it->second->GetCallToken();
        if (callerToken == nullptr) {
            HILOG_ERROR("callerToken is null, id: %{public}d.", extensionRecordId);
            return nullptr;
        }
        auto callerAbilityRecord = AAFwk::Token::GetAbilityRecordByToken(callerToken);
        if (callerAbilityRecord == nullptr) {
            HILOG_ERROR("callerAbilityRecord is null, id: %{public}d.", extensionRecordId);
            return nullptr;
        }
        if (callerAbilityRecord->GetUIExtensionAbilityId() == INVALID_EXTENSION_RECORD_ID) {
            HILOG_DEBUG("update rootCallerToken, id: %{public}d.", extensionRecordId);
            it->second->SetRootCallerToken(callerToken);
            return callerToken;
        }
        rootCallerToken = GetRootCallerTokenLocked(callerAbilityRecord->GetUIExtensionAbilityId());
        HILOG_DEBUG("update rootCallerToken, id: %{public}d.", extensionRecordId);
        it->second->SetRootCallerToken(rootCallerToken);
        return rootCallerToken;
    }
    HILOG_ERROR("Not found id %{public}d.", extensionRecordId);
    return nullptr;
}

int32_t ExtensionRecordManager::CreateExtensionRecord(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord,
    const std::string &hostBundleName, int32_t &extensionRecordId)
{
    // factory pattern with ability request
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord is null");
        return ERR_NULL_OBJECT;
    }
    extensionRecordId = GenerateExtensionRecordId(extensionRecordId);
    if (AAFwk::UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        std::shared_ptr<ExtensionRecord> extensionRecord = std::make_shared<UIExtensionRecord>(abilityRecord,
            hostBundleName, extensionRecordId);
        std::lock_guard<std::mutex> lock(mutex_);
        HILOG_DEBUG("add UIExtension, id %{public}d.", extensionRecordId);
        extensionRecords_[extensionRecordId] = extensionRecord;
        abilityRecord->SetUIExtensionAbilityId(extensionRecordId);
        return ERR_OK;
    }
    return ERR_INVALID_VALUE;
}
} // namespace AbilityRuntime
} // namespace OHOS
