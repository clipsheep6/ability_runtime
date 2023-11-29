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

#include "ui_extension_ability_connect_manager.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
std::atomic_int32_t UIExtensionAbilityConnectManager::uiExtensionAbilityId_ = INVALID_UI_EXTENSION_ABILITY_ID;

UIExtensionAbilityConnectManager::UIExtensionAbilityConnectManager(const int userId) : userId_(userId)
{
    HILOG_DEBUG("constructor.");
}

UIExtensionAbilityConnectManager::~UIExtensionAbilityConnectManager()
{
    HILOG_INFO("deconstructor.");
}

int32_t UIExtensionAbilityConnectManager::GenerateUIExtensionAbilityId(const int32_t uiExtensionAbilityId)
{
    HILOG_DEBUG("Input id is %{public}d.", uiExtensionAbilityId);
    std::lock_guard<std::mutex> lock(mutex_);
    if (uiExtensionAbilityId != INVALID_UI_EXTENSION_ABILITY_ID &&
        !uiExtensionAbilityIdSet_.count(uiExtensionAbilityId)) {
        uiExtensionAbilityIdSet_.insert(uiExtensionAbilityId);
        uiExtensionAbilityId_ = uiExtensionAbilityId;
        return uiExtensionAbilityId_;
    }

    if (uiExtensionAbilityId == INVALID_UI_EXTENSION_ABILITY_ID) {
        ++uiExtensionAbilityId_;
    }

    while (uiExtensionAbilityIdSet_.count(uiExtensionAbilityId_)) {
        uiExtensionAbilityId_++;
    }

    return uiExtensionAbilityId_;
}

void UIExtensionAbilityConnectManager::AddUIExtensionAbilityRecord(const int32_t uiExtensionAbilityId,
    const std::shared_ptr<UIExtensionAbilityRecord> record)
{
    HILOG_DEBUG("UIExtensionAbilityId %{public}d.", uiExtensionAbilityId);
    std::lock_guard<std::mutex> lock(mutex_);
    uiExtensionAbilityRecords_.emplace(uiExtensionAbilityId, record);
}

void UIExtensionAbilityConnectManager::RemoveUIExtensionAbilityRecord(const int32_t uiExtensionAbilityId)
{
    HILOG_DEBUG("UIExtensionAbilityId %{public}d.", uiExtensionAbilityId);
    std::lock_guard<std::mutex> lock(mutex_);
    uiExtensionAbilityRecords_.erase(uiExtensionAbilityId);
}

bool UIExtensionAbilityConnectManager::CheckUIExtensionAbilityLoaded(const int32_t uiExtensionAbilityId,
    const std::string hostBundleName)
{
    HILOG_DEBUG("UIExtensionAbilityId %{public}d.", uiExtensionAbilityId);
    std::lock_guard<std::mutex> lock(mutex_);
    // find target record firstly
    auto it = uiExtensionAbilityRecords_.find(uiExtensionAbilityId);
    if (it != uiExtensionAbilityRecords_.end() && it->second != nullptr) {
        // check bundlename
        HILOG_DEBUG("Stored host bundleName: %{public}s, input bundleName is %{public}s.",
            it->second->hostBundleName_.c_str(), hostBundleName.c_str());
        if (it->second->hostBundleName_ == hostBundleName) {
            return true;
        }
    }
    HILOG_DEBUG("Not found stored id %{public}d.", uiExtensionAbilityId);
    return false;
}
} // namespace AbilityRuntime
} // namespace OHOS
