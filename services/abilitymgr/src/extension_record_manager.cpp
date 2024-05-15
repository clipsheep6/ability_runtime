/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ui_extension_utils.h"
#include "ui_extension_record.h"
#include "ui_extension_record_factory.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *SEPARATOR = ":";
}
std::atomic_int32_t ExtensionRecordManager::extensionRecordId_ = INVALID_EXTENSION_RECORD_ID;

ExtensionRecordManager::ExtensionRecordManager(const int32_t userId) : userId_(userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "constructor.");
}

ExtensionRecordManager::~ExtensionRecordManager()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "deconstructor.");
}

int32_t ExtensionRecordManager::GenerateExtensionRecordId(const int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Input id is %{public}d.", extensionRecordId);
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
    TAG_LOGD(AAFwkTag::ABILITYMGR, "extensionRecordId %{public}d.", extensionRecordId);
    std::lock_guard<std::mutex> lock(mutex_);
    extensionRecords_.emplace(extensionRecordId, record);
}

void ExtensionRecordManager::RemoveExtensionRecord(const int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "extensionRecordId %{public}d.", extensionRecordId);
    std::lock_guard<std::mutex> lock(mutex_);
    extensionRecords_.erase(extensionRecordId);
    terminateRecords_.erase(extensionRecordId);
}

void ExtensionRecordManager::AddExtensionRecordToTerminatedList(const int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "extensionRecordId %{public}d.", extensionRecordId);
    std::lock_guard<std::mutex> lock(mutex_);

    auto findRecord = extensionRecords_.find(extensionRecordId);
    if (findRecord == extensionRecords_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "extensionRecordId %{public}d not found.", extensionRecordId);
        return;
    }
    terminateRecords_.emplace(*findRecord);
    extensionRecords_.erase(extensionRecordId);
}

int32_t ExtensionRecordManager::GetExtensionRecord(const int32_t extensionRecordId,
    const std::string &hostBundleName, std::shared_ptr<ExtensionRecord> &extensionRecord, bool &isLoaded)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "extensionRecordId %{public}d.", extensionRecordId);
    std::lock_guard<std::mutex> lock(mutex_);
    // find target record firstly
    auto it = extensionRecords_.find(extensionRecordId);
    if (it != extensionRecords_.end() && it->second != nullptr) {
        // check bundleName
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Stored host bundleName: %{public}s, input bundleName is %{public}s.",
            it->second->hostBundleName_.c_str(), hostBundleName.c_str());
        if (it->second->hostBundleName_ == hostBundleName) {
            extensionRecord = it->second;
            isLoaded = true;
            return ERR_OK;
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Not found stored id %{public}d.", extensionRecordId);
    extensionRecord = nullptr;
    isLoaded = false;
    return ERR_NULL_OBJECT;
}

bool ExtensionRecordManager::IsBelongToManager(const AppExecFwk::AbilityInfo &abilityInfo)
{
    // only support UIExtension now
    return AAFwk::UIExtensionUtils::IsUIExtension(abilityInfo.extensionAbilityType);
}

int32_t ExtensionRecordManager::GetActiveUIExtensionList(const int32_t pid, std::vector<std::string> &extensionList)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &it : extensionRecords_) {
        if (it.second == nullptr || it.second->abilityRecord_ == nullptr ||
            pid != it.second->abilityRecord_->GetPid()) {
            continue;
        }

        extensionList.push_back(it.second->abilityRecord_->GetAbilityInfo().moduleName + SEPARATOR +
                                it.second->abilityRecord_->GetAbilityInfo().name);
    }
    return ERR_OK;
}

int32_t ExtensionRecordManager::GetActiveUIExtensionList(
    const std::string &bundleName, std::vector<std::string> &extensionList)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &it : extensionRecords_) {
        if (it.second == nullptr || it.second->abilityRecord_ == nullptr ||
            bundleName != it.second->abilityRecord_->GetAbilityInfo().bundleName) {
            continue;
        }

        extensionList.push_back(it.second->abilityRecord_->GetAbilityInfo().moduleName + SEPARATOR +
                                it.second->abilityRecord_->GetAbilityInfo().name);
    }
    return ERR_OK;
}

int32_t ExtensionRecordManager::GetOrCreateExtensionRecord(const AAFwk::AbilityRequest &abilityRequest,
    const std::string &hostBundleName, std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord, bool &isLoaded)
{
    CHECK_POINTER_AND_RETURN(abilityRequest.sessionInfo, ERR_INVALID_VALUE);
    abilityRecord = GetAbilityRecordBySessionInfo(abilityRequest.sessionInfo);
    if (abilityRecord != nullptr) {
        isLoaded = true;
        return ERR_OK;
    }
    std::shared_ptr<ExtensionRecord> extensionRecord = nullptr;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Check Preload Extension Record.");
    auto result = IsPreloadExtensionRecord(abilityRequest, hostBundleName, extensionRecord, isLoaded);
    if (result) {
        RemovePreloadUIExtensionRecord(abilityRequest, hostBundleName, extensionRecord, isLoaded);
    } else {
        int32_t ret = GetOrCreateExtensionRecordInner(abilityRequest, hostBundleName, extensionRecord, isLoaded);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetOrCreateExtensionRecordInner error!");
            return ret;
        }
    }
    if (extensionRecord != nullptr) {
        abilityRecord = extensionRecord->abilityRecord_;
    }
    return ERR_OK;
}

std::shared_ptr<AAFwk::AbilityRecord> ExtensionRecordManager::GetAbilityRecordBySessionInfo(
    const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    CHECK_POINTER_AND_RETURN(sessionInfo, nullptr);
    if (sessionInfo->uiExtensionComponentId == INVALID_EXTENSION_RECORD_ID) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ExtensionAbility id invalid or not configured.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& it : extensionRecords_) {
        if (it.second == nullptr) {
            continue;
        }
        std::shared_ptr<AAFwk::AbilityRecord> abilityRecord = it.second->abilityRecord_;
        if (abilityRecord == nullptr) {
            continue;
        }
        sptr<AAFwk::SessionInfo> recordSessionInfo = abilityRecord->GetSessionInfo();
        if (recordSessionInfo == nullptr) {
            continue;
        }
        if (recordSessionInfo->uiExtensionComponentId == sessionInfo->uiExtensionComponentId) {
            TAG_LOGD(AAFwkTag::ABILITYMGR,
                "found record, uiExtensionComponentId: %{public}" PRIu64, sessionInfo->uiExtensionComponentId);
            return abilityRecord;
        }
    }
    return nullptr;
}

bool ExtensionRecordManager::IsHostSpecifiedProcessValid(const AAFwk::AbilityRequest &abilityRequest,
    std::shared_ptr<ExtensionRecord> &record, const std::string &process)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &iter: extensionRecords_) {
        if (iter.second == nullptr || iter.second->abilityRecord_ == nullptr) {
            continue;
        }
        if (iter.second->abilityRecord_->GetProcessName() != process) {
            continue;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "found match extension record: id %{public}d", iter.first);
        AppExecFwk::AbilityInfo abilityInfo = iter.second->abilityRecord_->GetAbilityInfo();
        if (abilityRequest.abilityInfo.bundleName != abilityInfo.bundleName) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleName is not match");
            return false;
        }
        if (abilityRequest.abilityInfo.name != abilityInfo.name) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityName is not match");
            return false;
        }
        return true;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "specified process not found, %{public}s", process.c_str());
    return false;
}

int32_t ExtensionRecordManager::UpdateProcessName(const AAFwk::AbilityRequest &abilityRequest,
    std::shared_ptr<ExtensionRecord> &record)
{
    std::shared_ptr<AAFwk::AbilityRecord> abilityRecord = record->abilityRecord_;
    switch (record->processMode_) {
        case PROCESS_MODE_INSTANCE: {
            std::string process = abilityRequest.abilityInfo.bundleName + SEPARATOR + abilityRequest.abilityInfo.name
                + SEPARATOR + std::to_string(abilityRecord->GetUIExtensionAbilityId());
            abilityRecord->SetProcessName(process);
            break;
        }
        case PROCESS_MODE_TYPE: {
            std::string process = abilityRequest.abilityInfo.bundleName + SEPARATOR + abilityRequest.abilityInfo.name;
            abilityRecord->SetProcessName(process);
            break;
        }
        case PROCESS_MODE_HOST_SPECIFIED: {
            std::string process = abilityRequest.want.GetStringParam(PROCESS_MODE_HOST_SPECIFIED_KEY);
            if (!IsHostSpecifiedProcessValid(abilityRequest, record, process)) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "host specified process name is invalid, %{public}s", process.c_str());
                return ERR_INVALID_VALUE;
            }
            abilityRecord->SetProcessName(process);
            break;
        }
        default: // AppExecFwk::ExtensionProcessMode::UNDEFINED or AppExecFwk::ExtensionProcessMode::BUNDLE
            // no need to update
            break;
    }
    return ERR_OK;
}

int32_t ExtensionRecordManager::AddPreloadUIExtensionRecord(const std::shared_ptr<AAFwk::AbilityRecord> abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    std::shared_ptr<ExtensionRecord> extensionRecord = nullptr;
    auto extensionRecordId = abilityRecord->GetUIExtensionAbilityId();
    if (extensionRecords_.find(extensionRecordId) != extensionRecords_.end()) {
        extensionRecord = extensionRecords_[extensionRecordId];
        CHECK_POINTER_AND_RETURN(extensionRecord, ERR_INVALID_VALUE);
        auto hostBundleName = extensionRecord->hostBundleName_;
        auto preLoadUIExtensionInfo = std::make_tuple(abilityRecord->GetWant().GetElement().GetAbilityName(),
            abilityRecord->GetWant().GetElement().GetBundleName(),
            abilityRecord->GetWant().GetElement().GetModuleName(), hostBundleName);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "hostBundleName: %{public}s, elementName:%{public}s ",
            hostBundleName.c_str(), abilityRecord->GetWant().GetElement().GetURI().c_str());
        std::lock_guard<std::mutex> lock(preloadUIExtensionMapMutex_);
        preloadUIExtensionMap_[preLoadUIExtensionInfo].push_back(extensionRecord);
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "The extensionRecordId has no corresponding extensionRecord object!");
    return ERR_INVALID_VALUE;
}

void ExtensionRecordManager::RemoveAllPreloadUIExtensionRecord(PreLoadUIExtensionMapKey &preLoadUIExtensionInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    std::lock_guard<std::mutex> lock(preloadUIExtensionMapMutex_);
    if (preloadUIExtensionMap_.find(preLoadUIExtensionInfo) != preloadUIExtensionMap_.end()) {
        preloadUIExtensionMap_.erase(preLoadUIExtensionInfo);
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "The preLoadUIExtensionInfo has no corresponding extensionRecord object!");
    }
}

bool ExtensionRecordManager::IsPreloadExtensionRecord(const AAFwk::AbilityRequest &abilityRequest,
    const std::string &hostBundleName, std::shared_ptr<ExtensionRecord> &extensionRecord, bool &isLoaded)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    std::string abilityName = abilityRequest.want.GetElement().GetAbilityName();
    std::string bundleName = abilityRequest.want.GetElement().GetBundleName();
    std::string moduleName = abilityRequest.want.GetElement().GetModuleName();
    auto extensionRecordMapKey = std::make_tuple(abilityName, bundleName, moduleName, hostBundleName);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "hostBundleName: %{public}s, bundleName: %{public}s",
        hostBundleName.c_str(), bundleName.c_str());
    std::lock_guard<std::mutex> lock(preloadUIExtensionMapMutex_);
    auto item = preloadUIExtensionMap_.find(extensionRecordMapKey);
    if (item != preloadUIExtensionMap_.end()) {
        if (!item->second.empty()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "UIExtensionAbility has been preloaded.");
            auto extensionRecords = item->second;
            extensionRecord = extensionRecords[0];
            if (extensionRecord == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "ExtensionRecord is nullptr.");
                return false;
            }
            extensionRecord->Update(abilityRequest);
            isLoaded = true;
            return true;
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "UIExtension is not preloaded.");
    return false;
}

bool ExtensionRecordManager::RemovePreloadUIExtensionRecord(const AAFwk::AbilityRequest &abilityRequest,
    const std::string &hostBundleName, std::shared_ptr<ExtensionRecord> &extensionRecord, bool &isLoaded)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    std::string abilityName = abilityRequest.want.GetElement().GetAbilityName();
    std::string bundleName = abilityRequest.want.GetElement().GetBundleName();
    std::string moduleName = abilityRequest.want.GetElement().GetModuleName();
    auto extensionRecordMapKey = std::make_tuple(abilityName, bundleName, moduleName, hostBundleName);
    std::lock_guard<std::mutex> lock(preloadUIExtensionMapMutex_);
    auto item = preloadUIExtensionMap_.find(extensionRecordMapKey);
    if (item != preloadUIExtensionMap_.end()) {
        if (!item->second.empty()) {
            item->second.erase(item->second.begin());
        }
        if (item->second.empty()) {
            preloadUIExtensionMap_.erase(extensionRecordMapKey);
        }
        return true;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "preloadUIExtensionMap_ erase key error.");
    return false;
}

int32_t ExtensionRecordManager::GetOrCreateExtensionRecordInner(const AAFwk::AbilityRequest &abilityRequest,
    const std::string &hostBundleName, std::shared_ptr<ExtensionRecord> &extensionRecord, bool &isLoaded)
{
    std::shared_ptr<ExtensionRecordFactory> factory = nullptr;
    if (AAFwk::UIExtensionUtils::IsUIExtension(abilityRequest.abilityInfo.extensionAbilityType)) {
        factory = DelayedSingleton<UIExtensionRecordFactory>::GetInstance();
    }
    if (factory == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid extensionAbilityType");
        return ERR_INVALID_VALUE;
    }
    int32_t result = factory->PreCheck(abilityRequest, hostBundleName);
    if (result != ERR_OK) {
        return result;
    }
    int32_t extensionRecordId = INVALID_EXTENSION_RECORD_ID;
    bool needReuse = factory->NeedReuse(abilityRequest, extensionRecordId);
    if (needReuse) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "reuse record, id: %{public}d", extensionRecordId);
        int32_t ret = GetExtensionRecord(extensionRecordId, hostBundleName, extensionRecord, isLoaded);
        if (ret == ERR_OK) {
            extensionRecord->Update(abilityRequest);
        }
        return ret;
    }
    result = factory->CreateRecord(abilityRequest, extensionRecord);
    if (result != ERR_OK) {
        return result;
    }
    CHECK_POINTER_AND_RETURN(extensionRecord, ERR_NULL_OBJECT);
    std::shared_ptr<AAFwk::AbilityRecord> abilityRecord = extensionRecord->abilityRecord_;
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_NULL_OBJECT);
    isLoaded = false;
    extensionRecordId = GenerateExtensionRecordId(extensionRecordId);
    extensionRecord->extensionRecordId_ = extensionRecordId;
    extensionRecord->hostBundleName_ = hostBundleName;
    abilityRecord->SetOwnerMissionUserId(userId_);
    abilityRecord->SetUIExtensionAbilityId(extensionRecordId);
    result = UpdateProcessName(abilityRequest, extensionRecord);
    if (result != ERR_OK) {
        return result;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "extensionRecordId: %{public}d, extensionProcessMode:%{public}d, process: %{public}s",
        extensionRecordId, abilityRequest.extensionProcessMode, abilityRecord->GetAbilityInfo().process.c_str());
    std::lock_guard<std::mutex> lock(mutex_);
    extensionRecords_[extensionRecordId] = extensionRecord;
    return ERR_OK;
}

int32_t ExtensionRecordManager::StartAbility(const AAFwk::AbilityRequest &abilityRequest)
{
    return ERR_OK;
}

bool ExtensionRecordManager::IsFocused(int32_t extensionRecordId, const sptr<IRemoteObject>& focusToken)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sptr<IRemoteObject> rootCallerToken = GetRootCallerTokenLocked(extensionRecordId);
    bool isFocused = rootCallerToken == focusToken;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "id: %{public}d isFocused: %{public}d.", extensionRecordId, isFocused);
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
            TAG_LOGE(AAFwkTag::ABILITYMGR, "callerToken is null, id: %{public}d.", extensionRecordId);
            return nullptr;
        }
        auto callerAbilityRecord = AAFwk::Token::GetAbilityRecordByToken(callerToken);
        if (callerAbilityRecord == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "callerAbilityRecord is null, id: %{public}d.", extensionRecordId);
            return nullptr;
        }
        if (callerAbilityRecord->GetUIExtensionAbilityId() == INVALID_EXTENSION_RECORD_ID) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "update rootCallerToken, id: %{public}d.", extensionRecordId);
            it->second->SetRootCallerToken(callerToken);
            return callerToken;
        }
        rootCallerToken = GetRootCallerTokenLocked(callerAbilityRecord->GetUIExtensionAbilityId());
        TAG_LOGD(AAFwkTag::ABILITYMGR, "update rootCallerToken, id: %{public}d.", extensionRecordId);
        it->second->SetRootCallerToken(rootCallerToken);
        return rootCallerToken;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "Not found id %{public}d.", extensionRecordId);
    return nullptr;
}

int32_t ExtensionRecordManager::CreateExtensionRecord(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord,
    const std::string &hostBundleName, std::shared_ptr<ExtensionRecord> &extensionRecord, int32_t &extensionRecordId)
{
    // factory pattern with ability request
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is null");
        return ERR_NULL_OBJECT;
    }
    extensionRecordId = GenerateExtensionRecordId(extensionRecordId);
    if (AAFwk::UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        extensionRecord = std::make_shared<UIExtensionRecord>(abilityRecord);
        extensionRecord->hostBundleName_ = hostBundleName;
        extensionRecord->extensionRecordId_ = extensionRecordId;
        std::lock_guard<std::mutex> lock(mutex_);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "add UIExtension, id %{public}d.", extensionRecordId);
        extensionRecords_[extensionRecordId] = extensionRecord;
        abilityRecord->SetUIExtensionAbilityId(extensionRecordId);
        //add uiextension record register state observer object.
        auto ret = extensionRecord->RegisterStateObserver(hostBundleName);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Register extensionRecord state observer failed, err: %{public}d.", ret);
            return ERR_INVALID_VALUE;
        }
        return ERR_OK;
    }
    return ERR_INVALID_VALUE;
}

std::shared_ptr<AAFwk::AbilityRecord> ExtensionRecordManager::GetUIExtensionRootHostInfo(
    const sptr<IRemoteObject> token)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Input param invalid.");
        return nullptr;
    }

    auto abilityRecord = AAFwk::Token::GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get ability record failed.");
        return nullptr;
    }

    if (!AAFwk::UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Not ui extension ability.");
        return nullptr;
    }

    sptr<IRemoteObject> rootCallerToken = nullptr;
    auto extensionRecordId = abilityRecord->GetUIExtensionAbilityId();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        rootCallerToken = GetRootCallerTokenLocked(extensionRecordId);
    }

    if (rootCallerToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get root caller record failed.");
        return nullptr;
    }

    return AAFwk::Token::GetAbilityRecordByToken(rootCallerToken);
}

std::shared_ptr<ExtensionRecord> ExtensionRecordManager::GetExtensionRecordById(int32_t extensionRecordId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto findRecord = extensionRecords_.find(extensionRecordId);
    if (findRecord != extensionRecords_.end()) {
        return findRecord->second;
    }
    findRecord = terminateRecords_.find(extensionRecordId);
    if (findRecord == terminateRecords_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ui extension record not found, id: %{public}d.", extensionRecordId);
        return nullptr;
    }

    return findRecord->second;
}

void ExtensionRecordManager::LoadTimeout(int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    auto uiExtensionRecord = std::static_pointer_cast<UIExtensionRecord>(GetExtensionRecordById(extensionRecordId));
    if (uiExtensionRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Parsing ui extension record failed.");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start load timeout.");
    uiExtensionRecord->LoadTimeout();
}

void ExtensionRecordManager::ForegroundTimeout(int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    auto uiExtensionRecord = std::static_pointer_cast<UIExtensionRecord>(GetExtensionRecordById(extensionRecordId));
    if (uiExtensionRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Parsing ui extension record failed.");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start foreground timeout.");
    uiExtensionRecord->ForegroundTimeout();
}

void ExtensionRecordManager::BackgroundTimeout(int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    auto uiExtensionRecord = std::static_pointer_cast<UIExtensionRecord>(GetExtensionRecordById(extensionRecordId));
    if (uiExtensionRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Parsing ui extension record failed.");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start background timeout.");
    uiExtensionRecord->BackgroundTimeout();
}

void ExtensionRecordManager::TerminateTimeout(int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    auto uiExtensionRecord = std::static_pointer_cast<UIExtensionRecord>(GetExtensionRecordById(extensionRecordId));
    if (uiExtensionRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Parsing ui extension record failed.");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start terminate timeout.");
    uiExtensionRecord->TerminateTimeout();
}
} // namespace AbilityRuntime
} // namespace OHOS
