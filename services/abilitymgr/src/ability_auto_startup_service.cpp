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

#include "ability_auto_startup_service.h"

#include <algorithm>
#include <mutex>

#include "ability_auto_startup_data_manager.h"
#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "auto_startup_info.h"
#include "auto_startup_interface.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "parameters.h"
#include "permission_constants.h"
#include "permission_verification.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;
namespace {
constexpr char PRODUCT_APPBOOT_SETTING_ENABLED[] = "const.product.appboot.setting.enabled";
} // namespace

AbilityAutoStartupService::AbilityAutoStartupService() {}

AbilityAutoStartupService::~AbilityAutoStartupService() {}

int32_t AbilityAutoStartupService::RegisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
{
    HILOG_DEBUG("Called.");
    int32_t code = CheckPermissionForSystem();
    if (code != ERR_OK) {
        return code;
    }

    {
        std::lock_guard<std::mutex> lock(autoStartUpMutex_);
        bool isFound = false;
        auto item = callbackVector_.begin();
        while (item != callbackVector_.end()) {
            if (*item == callback) {
                isFound = true;
                break;
            }
            item++;
        }
        if (!isFound) {
            callbackVector_.emplace_back(callback);
            SetDeathRecipient(
                callback, new (std::nothrow) AbilityAutoStartupService::ClientDeathRecipient(weak_from_this()));
        } else {
            HILOG_DEBUG("Callback is already exist.");
        }
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupService::UnregisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
{
    HILOG_DEBUG("Called.");
    int32_t code = CheckPermissionForSystem();
    if (code != ERR_OK) {
        return code;
    }

    {
        std::lock_guard<std::mutex> lock(autoStartUpMutex_);
        bool isFound = false;
        auto item = callbackVector_.begin();
        while (item != callbackVector_.end()) {
            if (*item == callback) {
                item = callbackVector_.erase(item);
                isFound = true;
            } else {
                item++;
            }
        }
        if (!isFound) {
            HILOG_DEBUG("Callback is not exist.");
        }
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupService::SetApplicationAutoStartup(const AutoStartupInfo &info)
{
    HILOG_DEBUG("Called, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s.",
        info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str());
    int32_t code = CheckPermissionForSystem();
    if (code != ERR_OK) {
        return code;
    }

    bool isVisible;
    std::string abilityTypeName;
    if (!GetAbilityData(info, isVisible, abilityTypeName)) {
        HILOG_ERROR("Failed to get ability data.");
        return INNER_ERR;
    }

    if (!isVisible) {
        HILOG_ERROR("Current ability is not visible.");
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }

    AutoStartupInfo fullInfo(info);
    fullInfo.abilityTypeName = abilityTypeName;

    return InnerSetApplicationAutoStartup(fullInfo);
}

int32_t AbilityAutoStartupService::InnerSetApplicationAutoStartup(const AutoStartupInfo &info)
{
    AutoStartupStatus status =
        DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAutoStartupData(info);
    if (status.code != ERR_OK && status.code != ERR_NAME_NOT_FOUND) {
        HILOG_ERROR("Query auto startup data failed.");
        return status.code;
    }

    int32_t result;
    if (status.code == ERR_NAME_NOT_FOUND) {
        HILOG_INFO("Query data is not exist.");
        result =
            DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->InsertAutoStartupData(info, true, false);
        if (result == ERR_OK) {
            ExecuteCallbacks(true, info);
        }
        return result;
    }
    if (status.isEdmForce) {
        HILOG_ERROR("Edm application abnormal.");
        return ERR_EDM_APP_CONTROLLED;
    }
    if (!status.isAutoStartup) {
        result =
            DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->UpdateAutoStartupData(info, true, false);
        if (result == ERR_OK) {
            ExecuteCallbacks(true, info);
        }
        return result;
    }
    return ERR_ALREADY_EXISTS;
}

int32_t AbilityAutoStartupService::CancelApplicationAutoStartup(const AutoStartupInfo &info)
{
    HILOG_DEBUG("Called, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s.",
        info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str());
    int32_t code = CheckPermissionForSystem();
    if (code != ERR_OK) {
        return code;
    }

    bool isVisible;
    std::string abilityTypeName;
    if (!GetAbilityData(info, isVisible, abilityTypeName)) {
        HILOG_ERROR("Failed to get ability data.");
        return INNER_ERR;
    }

    if (!isVisible) {
        HILOG_ERROR("Current ability is not visible.");
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }

    AutoStartupInfo fullInfo(info);
    fullInfo.abilityTypeName = abilityTypeName;

    return InnerCancelApplicationAutoStartup(fullInfo);
}

int32_t AbilityAutoStartupService::InnerCancelApplicationAutoStartup(const AutoStartupInfo &info)
{
    AutoStartupStatus status =
        DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAutoStartupData(info);
    if (status.code != ERR_OK) {
        HILOG_ERROR("Query auto startup data failed.");
        return status.code;
    }

    if (status.isEdmForce) {
        HILOG_ERROR("Edm application abnormal.");
        return ERR_EDM_APP_CONTROLLED;
    }

    if (status.isAutoStartup) {
        int32_t result = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->DeleteAutoStartupData(info);
        if (result == ERR_OK) {
            ExecuteCallbacks(false, info);
        }
        return result;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupService::QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList)
{
    HILOG_DEBUG("Called.");
    int32_t code = CheckPermissionForEDM();
    code = code == ERR_OK ? code : CheckPermissionForSystem();
    if (code != ERR_OK) {
        HILOG_ERROR("Permission verification failed.");
        return code;
    }

    return DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAllAutoStartupApplications(infoList);
}

int32_t AbilityAutoStartupService::QueryAllAutoStartupApplicationsWithoutPermission(
    std::vector<AutoStartupInfo> &infoList)
{
    HILOG_DEBUG("Called.");
    if (!system::GetBoolParameter(PRODUCT_APPBOOT_SETTING_ENABLED, false)) {
        HILOG_DEBUG("Query EDM auto start up info list.");
        return DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAllAutoStartupApplications(
            infoList, true);
    }
    return DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAllAutoStartupApplications(infoList);
}

int32_t AbilityAutoStartupService::RegisterAutoStartupCallback(const sptr<IRemoteObject> &callback)
{
    HILOG_DEBUG("Called.");
    if (!system::GetBoolParameter(PRODUCT_APPBOOT_SETTING_ENABLED, false)) {
        HILOG_ERROR("Product configuration item is disable.");
        return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
    }

    std::string bundleName = GetSelfApplicationBundleName();
    if (bundleName.empty()) {
        HILOG_ERROR("Get self application bundleName failed.");
        return ERR_INVALID_OPERATION;
    }
    {
        std::lock_guard<std::mutex> lock(autoStartUpMutex_);
        auto item = callbackMaps_.find(bundleName);
        if (item != callbackMaps_.end()) {
            HILOG_DEBUG("Callback is already exist.");
            return ERR_OK;
        }
        callbackMaps_.emplace(bundleName, callback);
        SetDeathRecipient(
            callback, new (std::nothrow) AbilityAutoStartupService::ClientDeathRecipient(weak_from_this()));
    }

    return ERR_OK;
}

int32_t AbilityAutoStartupService::UnregisterAutoStartupCallback(const sptr<IRemoteObject> &callback)
{
    HILOG_DEBUG("Called.");
    if (!system::GetBoolParameter(PRODUCT_APPBOOT_SETTING_ENABLED, false)) {
        HILOG_ERROR("Product configuration item is disable.");
        return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
    }

    std::string bundleName = GetSelfApplicationBundleName();
    if (bundleName.empty()) {
        HILOG_ERROR("Get self application bundleName failed.");
        return ERR_INVALID_OPERATION;
    }
    {
        std::lock_guard<std::mutex> lock(autoStartUpMutex_);
        auto item = callbackMaps_.find(bundleName);
        if (item == callbackMaps_.end()) {
            HILOG_DEBUG("BundleName is not exist.");
            return ERR_NAME_NOT_FOUND;
        }
        if (item->second != callback) {
            HILOG_DEBUG("Callback is not exist.");
            return ERR_NAME_NOT_FOUND;
        }
        callbackMaps_.erase(item);
    }

    return ERR_OK;
}

int32_t AbilityAutoStartupService::SetAutoStartup(const AutoStartupInfo &info)
{
    HILOG_DEBUG("Called, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s.",
        info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str());
    int32_t code = CheckPermissionForSelf(info.bundleName);
    if (code != ERR_OK) {
        return code;
    }

    bool isVisible;
    std::string abilityTypeName;
    if (!GetAbilityData(info, isVisible, abilityTypeName)) {
        HILOG_ERROR("Failed to get ability data.");
        return INNER_ERR;
    }

    if (!isVisible) {
        HILOG_ERROR("Current ability is not visible.");
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }

    AutoStartupInfo fullInfo(info);
    fullInfo.abilityTypeName = abilityTypeName;

    return InnerSetAutoStartup(fullInfo);
}

int32_t AbilityAutoStartupService::InnerSetAutoStartup(const AutoStartupInfo &info)
{
    AutoStartupStatus status =
        DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAutoStartupData(info);
    if (status.code != ERR_OK && status.code != ERR_NAME_NOT_FOUND) {
        HILOG_ERROR("Query auto startup data failed.");
        return status.code;
    }

    int32_t result;
    if (status.code == ERR_NAME_NOT_FOUND) {
        HILOG_INFO("Query data is not exist.");
        result =
            DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->InsertAutoStartupData(info, true, false);
        if (result == ERR_OK) {
            ExecuteCallbacks(true, info);
        }
        return result;
    }

    if (status.isEdmForce) {
        HILOG_ERROR("Edm application abnormal.");
        return ERR_EDM_APP_CONTROLLED;
    }

    if (!status.isAutoStartup) {
        result =
            DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->UpdateAutoStartupData(info, true, false);
        if (result == ERR_OK) {
            ExecuteCallbacks(true, info);
        }
        return result;
    }
    return ERR_ALREADY_EXISTS;
}

int32_t AbilityAutoStartupService::CancelAutoStartup(const AutoStartupInfo &info)
{
    HILOG_DEBUG("Called, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s.",
        info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str());
    int32_t code = CheckPermissionForSelf(info.bundleName);
    if (code != ERR_OK) {
        return code;
    }

    bool isVisible;
    std::string abilityTypeName;
    if (!GetAbilityData(info, isVisible, abilityTypeName)) {
        HILOG_ERROR("Failed to get ability data.");
        return INNER_ERR;
    }

    if (!isVisible) {
        HILOG_ERROR("Current ability is not visible.");
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }

    AutoStartupInfo fullInfo(info);
    fullInfo.abilityTypeName = abilityTypeName;

    return InnerCancelAutoStartup(fullInfo);
}

int32_t AbilityAutoStartupService::InnerCancelAutoStartup(const AutoStartupInfo &info)
{
    AutoStartupStatus status =
        DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAutoStartupData(info);
    if (status.code != ERR_OK) {
        HILOG_ERROR("Query auto startup data failed.");
        return status.code;
    }

    if (status.isEdmForce) {
        HILOG_ERROR("Edm application abnormal.");
        return ERR_EDM_APP_CONTROLLED;
    }

    if (status.isAutoStartup) {
        int32_t result = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->DeleteAutoStartupData(info);
        if (result == ERR_OK) {
            ExecuteCallbacks(false, info);
        }
        return result;
    }

    return ERR_OK;
}

int32_t AbilityAutoStartupService::IsAutoStartup(const AutoStartupInfo &info, bool &isAutoStartup)
{
    HILOG_DEBUG("Called, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s.",
        info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str());
    int32_t code = CheckPermissionForSelf(info.bundleName);
    if (code != ERR_OK) {
        return code;
    }

    AutoStartupStatus status =
        DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAutoStartupData(info);
    if (status.code != ERR_OK) {
        HILOG_DEBUG("Query auto startup data failed.");
        isAutoStartup = false;
    } else {
        isAutoStartup = status.isAutoStartup;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupService::DeleteAutoStartupData(const std::string &bundleName)
{
    HILOG_DEBUG("Called.");
    return DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->DeleteAutoStartupData(bundleName);
}

int32_t AbilityAutoStartupService::CheckAutoStartupData(const std::string &bundleName, int32_t uid)
{
    std::vector<AutoStartupInfo> infoList;
    int32_t result = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->GetCurrentAppAutoStartupData(
        bundleName, infoList);
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to get auto startup data.");
        return result;
    }
    if (infoList.size() == 0) {
        return ERR_OK;
    }

    AppExecFwk::BundleInfo bundleInfo;
    if (!GetBundleInfo(bundleName, bundleInfo, uid)) {
        return INNER_ERR;
    }

    bool isFound = false;
    for (auto info : infoList) {
        for (auto abilityInfo : bundleInfo.abilityInfos) {
            if ((abilityInfo.bundleName == info.bundleName) && (abilityInfo.name == info.abilityName) &&
                (info.moduleName.empty() || (abilityInfo.moduleName == info.moduleName))) {
                isFound = true;
                break;
            }
        }
    }

    if (!isFound) {
        HILOG_DEBUG("Current bundleName not found in Datebase.");
        return DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->DeleteAutoStartupData(bundleName);
    }
    return ERR_OK;
}

void AbilityAutoStartupService::ExecuteCallbacks(bool isCallOn, const AutoStartupInfo &info)
{
    HILOG_DEBUG("bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s.", info.bundleName.c_str(),
        info.moduleName.c_str(), info.abilityName.c_str());
    for (auto item : callbackVector_) {
        auto remoteSystemCallback = iface_cast<IAutoStartupCallBack>(item);
        if (remoteSystemCallback != nullptr) {
            if (isCallOn) {
                remoteSystemCallback->OnAutoStartupOn(info);
            } else {
                remoteSystemCallback->OnAutoStartupOff(info);
            }
        }
    }

    auto it = callbackMaps_.find(info.bundleName);
    if (it != callbackMaps_.end()) {
        auto remoteCallback = iface_cast<IAutoStartupCallBack>(it->second);
        if (remoteCallback != nullptr) {
            if (isCallOn) {
                remoteCallback->OnAutoStartupOn(info);
            } else {
                remoteCallback->OnAutoStartupOff(info);
            }
        }
    }
}

void AbilityAutoStartupService::SetDeathRecipient(
    const sptr<IRemoteObject> &callback, const sptr<IRemoteObject::DeathRecipient> &deathRecipient)
{
    HILOG_DEBUG("Called.");
    if (callback == nullptr || deathRecipient == nullptr) {
        HILOG_ERROR("The callerToken or the deathRecipient is empty.");
        return;
    }
    std::lock_guard<std::mutex> lock(deathRecipientsMutex_);
    auto iter = deathRecipients_.find(callback);
    if (iter == deathRecipients_.end()) {
        deathRecipients_.emplace(callback, deathRecipient);
        callback->AddDeathRecipient(deathRecipient);
        return;
    }
    HILOG_DEBUG("The deathRecipient has been added.");
}

void AbilityAutoStartupService::CleanResource(const wptr<IRemoteObject> &remote)
{
    HILOG_DEBUG("Called.");
    auto object = remote.promote();
    if (object == nullptr) {
        HILOG_ERROR("Remote object is nullptr.");
        return;
    }

    // Clean the callbackVector_ and callbackMaps_.
    {
        std::lock_guard<std::mutex> lock(autoStartUpMutex_);
        for (auto item = callbackVector_.begin(); item != callbackVector_.end();) {
            if (*item == object) {
                item = callbackVector_.erase(item);
            } else {
                item++;
            }
        }

        for (auto it = callbackMaps_.begin(); it != callbackMaps_.end();) {
            const auto &callback = it->second;
            if (callback == object) {
                it = callbackMaps_.erase(it);
            } else {
                it++;
            }
        }
    }
    {
        std::lock_guard<std::mutex> deathLock(deathRecipientsMutex_);
        auto iter = deathRecipients_.find(object);
        if (iter != deathRecipients_.end()) {
            auto deathRecipient = iter->second;
            deathRecipients_.erase(iter);
            object->RemoveDeathRecipient(deathRecipient);
        }
    }
}

AbilityAutoStartupService::ClientDeathRecipient::ClientDeathRecipient(
    const std::weak_ptr<AbilityAutoStartupService> &weakPtr)
{
    weakPtr_ = weakPtr;
}

void AbilityAutoStartupService::ClientDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_DEBUG("Called.");
    auto abilityAutoStartupService = weakPtr_.lock();
    if (abilityAutoStartupService == nullptr) {
        HILOG_ERROR("abilityAutoStartupService is nullptr.");
        return;
    }
    abilityAutoStartupService->CleanResource(remote);
}

std::string AbilityAutoStartupService::GetSelfApplicationBundleName()
{
    auto bundleMgrClient = GetBundleMgrClient();
    if (bundleMgrClient == nullptr) {
        HILOG_ERROR("Failed to get BundleMgrClient.");
        return "";
    }

    std::string bundleName;
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (IN_PROCESS_CALL(bundleMgrClient->GetNameForUid(callerUid, bundleName)) != ERR_OK) {
        HILOG_ERROR("Get Bundle Name failed.");
        return "";
    }
    HILOG_DEBUG("Get bundle name: %{public}s.", bundleName.c_str());
    return bundleName;
}

bool AbilityAutoStartupService::CheckSelfApplication(const std::string &bundleName)
{
    HILOG_DEBUG("Called, bundleName: %{public}s.", bundleName.c_str());
    return GetSelfApplicationBundleName() == bundleName ? true : false;
}

bool AbilityAutoStartupService::GetBundleInfo(
    const std::string &bundleName, AppExecFwk::BundleInfo &bundleInfo, int32_t uid)
{
    HILOG_DEBUG("Called.");
    auto bundleMgrClient = GetBundleMgrClient();
    if (bundleMgrClient == nullptr) {
        HILOG_ERROR("Failed to get BundleMgrClient.");
        return false;
    }

    int32_t userId;
    if (uid == -1) {
        userId = IPCSkeleton::GetCallingUid() / AppExecFwk::Constants::BASE_USER_RANGE;
    } else {
        userId = uid / AppExecFwk::Constants::BASE_USER_RANGE;
    }
    if (userId == 0) {
        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        if (abilityMgr == nullptr) {
            HILOG_ERROR("The abilityMgr is nullptr.");
            return false;
        }
        userId = abilityMgr->GetUserId();
    }
    HILOG_DEBUG("Current userId: %{public}d.", userId);
    auto flags =
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES | AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO;
    if (!IN_PROCESS_CALL(bundleMgrClient->GetBundleInfo(
        bundleName, static_cast<AppExecFwk::BundleFlag>(flags), bundleInfo, userId))) {
        HILOG_ERROR("Failed to get bundle info.");
        return false;
    }

    return true;
}

bool AbilityAutoStartupService::GetAbilityData(
    const AutoStartupInfo &info, bool &isVisible, std::string &abilityTypeName)
{
    HILOG_DEBUG("Called, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s.",
        info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str());
    AppExecFwk::BundleInfo bundleInfo;
    if (!GetBundleInfo(info.bundleName, bundleInfo)) {
        return false;
    }

    for (auto abilityInfo : bundleInfo.abilityInfos) {
        if ((abilityInfo.bundleName == info.bundleName) && (abilityInfo.name == info.abilityName)) {
            if (info.moduleName.empty() || (abilityInfo.moduleName == info.moduleName)) {
                isVisible = abilityInfo.visible;
                abilityTypeName = GetAbilityTypeName(abilityInfo);
                HILOG_DEBUG("Get ability info success.");
                return true;
            }
        }
    }

    for (auto extensionInfo : bundleInfo.extensionInfos) {
        if ((extensionInfo.bundleName == info.bundleName) && (extensionInfo.name == info.abilityName)) {
            if (info.moduleName.empty() || (extensionInfo.moduleName == info.moduleName)) {
                isVisible = extensionInfo.visible;
                abilityTypeName = GetExtensionTypeName(extensionInfo);
                HILOG_DEBUG("Get extension info success.");
                return true;
            }
        }
    }
    return false;
}

std::string AbilityAutoStartupService::GetAbilityTypeName(AppExecFwk::AbilityInfo abilityInfo)
{
    std::string abilityTypeName;
    if (abilityInfo.type == AppExecFwk::AbilityType::PAGE) {
        abilityTypeName = "UIAbility";
    }
    return abilityTypeName;
}

std::string AbilityAutoStartupService::GetExtensionTypeName(AppExecFwk::ExtensionAbilityInfo extensionInfo)
{
    std::string abilityTypeName;
    if (extensionInfo.type == AppExecFwk::ExtensionAbilityType::SERVICE) {
        abilityTypeName = "ServiceExtension";
    }
    return abilityTypeName;
}

std::shared_ptr<AppExecFwk::BundleMgrClient> AbilityAutoStartupService::GetBundleMgrClient()
{
    HILOG_DEBUG("Called.");
    if (bundleMgrClient_ == nullptr) {
        bundleMgrClient_ = DelayedSingleton<AppExecFwk::BundleMgrClient>::GetInstance();
    }
    return bundleMgrClient_;
}

int32_t AbilityAutoStartupService::CheckPermissionForSystem()
{
    if (!system::GetBoolParameter(PRODUCT_APPBOOT_SETTING_ENABLED, false)) {
        HILOG_ERROR("Product configuration item is disable.");
        return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
    }

    if (!PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        HILOG_ERROR("The caller is not system-app, can not use system-api.");
        return ERR_NOT_SYSTEM_APP;
    }

    if (!PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_MANAGE_APP_BOOT)) {
        HILOG_ERROR("Not have PERMISSION_MANAGE_APP_BOOT approval.");
        return CHECK_PERMISSION_FAILED;
    }

    return ERR_OK;
}

int32_t AbilityAutoStartupService::CheckPermissionForSelf(const std::string &bundleName)
{
    if (!system::GetBoolParameter(PRODUCT_APPBOOT_SETTING_ENABLED, false)) {
        HILOG_ERROR("Product configuration item is disable.");
        return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
    }

    if (!CheckSelfApplication(bundleName)) {
        HILOG_ERROR("Not self application.");
        return ERR_NOT_SELF_APPLICATION;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupService::GetAbilityInfo(const AutoStartupInfo &info, std::string &abilityTypeName)
{
    bool isVisible = false;
    if (!GetAbilityData(info, isVisible, abilityTypeName)) {
        HILOG_ERROR("Failed to get ability data.");
        return INNER_ERR;
    }

    if (!isVisible) {
        HILOG_ERROR("Current ability is not visible.");
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }

    return ERR_OK;
}

int32_t AbilityAutoStartupService::SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
{
    int32_t errorCode = CheckPermissionForEDM();
    if (errorCode != ERR_OK) {
        return errorCode;
    }
    std::string typeName;
    errorCode = GetAbilityInfo(info, typeName);
    if (errorCode != ERR_OK) {
        return errorCode;
    }
    AutoStartupInfo fullInfo(info);
    fullInfo.abilityTypeName = typeName;
    return InnerApplicationAutoStartupByEDM(fullInfo, true, flag);
}

int32_t AbilityAutoStartupService::CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
{
    int32_t errorCode = CheckPermissionForEDM();
    if (errorCode != ERR_OK) {
        return errorCode;
    }
    std::string typeName;
    errorCode = GetAbilityInfo(info, typeName);
    if (errorCode != ERR_OK) {
        return errorCode;
    }
    AutoStartupInfo fullInfo(info);
    fullInfo.abilityTypeName = typeName;
    return InnerApplicationAutoStartupByEDM(fullInfo, false, flag);
}

int32_t AbilityAutoStartupService::SetApplicationAutoStartupByEDM(
    const std::vector<AutoStartupInfo> &infoList, bool flag)
{
    int32_t errorCode = CheckPermissionForEDM();
    if (errorCode != ERR_OK) {
        HILOG_ERROR("Permission denied.");
        return errorCode;
    }
    int32_t tempErrorCode = ERR_OK;
    for (auto info : infoList) {
        std::string typeName;
        tempErrorCode = GetAbilityInfo(info, typeName);
        if (tempErrorCode != ERR_OK) {
            HILOG_ERROR("Get ability info failed, bundleName:%{public}s,moduleName:%{public}s,abilityName:%{public}s,"
                "flag:%{public}d.", info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str(), flag);
            errorCode = tempErrorCode;
            continue;
        }
        info.abilityTypeName = typeName;
        tempErrorCode = InnerApplicationAutoStartupByEDM(info, true, flag);
        if (tempErrorCode != ERR_OK) {
            HILOG_ERROR("Failed to update the database.");
            errorCode = tempErrorCode;
        }
    }
    return errorCode;
}

int32_t AbilityAutoStartupService::CancelApplicationAutoStartupByEDM(
    const std::vector<AutoStartupInfo> &infoList, bool flag)
{
    int32_t errorCode = CheckPermissionForEDM();
    if (errorCode != ERR_OK) {
        HILOG_ERROR("Permission denied.");
        return errorCode;
    }
    int32_t tempErrorCode = ERR_OK;
    for (auto info : infoList) {
        std::string typeName;
        tempErrorCode = GetAbilityInfo(info, typeName);
        if (tempErrorCode != ERR_OK) {
            HILOG_ERROR("Get ability info failed, bundleName:%{public}s,moduleName:%{public}s,abilityName:%{public}s,"
                "flag:%{public}d.", info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str(), flag);
            errorCode = tempErrorCode;
            continue;
        }
        info.abilityTypeName = typeName;
        tempErrorCode = InnerApplicationAutoStartupByEDM(info, false, flag);
        if (tempErrorCode != ERR_OK) {
            HILOG_ERROR("Failed to update the database.");
            errorCode = tempErrorCode;
        }
    }
    return errorCode;
}

int32_t AbilityAutoStartupService::InnerApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool isSet, bool flag)
{
    HILOG_DEBUG("Called, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s, isSet: %{public}d.,"
        "flag: %{public}d.", info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str(), isSet, flag);
    AutoStartupStatus status =
        DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAutoStartupData(info);
    if (status.code != ERR_OK && status.code != ERR_NAME_NOT_FOUND) {
        HILOG_ERROR("Query auto startup data failed.");
        return status.code;
    }

    int32_t result = ERR_OK;
    if (status.code == ERR_NAME_NOT_FOUND) {
        result = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->InsertAutoStartupData(
            info, isSet, flag);
        if (result == ERR_OK && isSet) {
            ExecuteCallbacks(isSet, info);
        }
        return result;
    }

    bool isFlag = isSet ? !status.isAutoStartup : status.isAutoStartup;
    if (isFlag) {
        result =
            DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->UpdateAutoStartupData(info, isSet, flag);
        if (result == ERR_OK) {
            ExecuteCallbacks(isSet, info);
        }
        return result;
    }
    if (status.isEdmForce != flag) {
        result =
            DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->UpdateAutoStartupData(info, isSet, flag);
        return result;
    }

    return result;
}

int32_t AbilityAutoStartupService::CheckPermissionForEDM()
{
    if (!PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_MANAGE_APP_BOOT_INTERNAL)) {
        HILOG_ERROR("Not have ohos.permission.MANAGE_APP_BOOT_INTERNAL approval.");
        return CHECK_PERMISSION_FAILED;
    }
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
