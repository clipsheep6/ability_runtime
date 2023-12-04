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

#include "bundle_mgr_helper.h"

#include "bundle_mgr_service_death_recipient.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
BundleMgrHelper::BundleMgrHelper() {}

BundleMgrHelper::~BundleMgrHelper()
{
    if (bundleMgr_ != nullptr && deathRecipient_ != nullptr) {
        bundleMgr_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
}

ErrCode BundleMgrHelper::GetNameForUid(const int32_t uid, std::string &name)
{
    HILOG_DEBUG("Called.");
    if (Connect() != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    return bundleMgr_->GetNameForUid(uid, name);
}

bool BundleMgrHelper::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo,
    int32_t userId)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->GetBundleInfo(bundleName, flag, bundleInfo, userId);
}

ErrCode BundleMgrHelper::InstallSandboxApp(const std::string &bundleName, int32_t dlpType, int32_t userId,
    int32_t &appIndex)
{
    HILOG_DEBUG("Called.");
    if (bundleName.empty()) {
        HILOG_ERROR("The bundleName is empty.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }
    ErrCode result = ConnectBundleInstaller();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_INTERNAL_ERROR;
    }

    return bundleInstaller_->InstallSandboxApp(bundleName, dlpType, userId, appIndex);
}

ErrCode BundleMgrHelper::UninstallSandboxApp(const std::string &bundleName, int32_t appIndex, int32_t userId)
{
    HILOG_DEBUG("Called.");
    if (bundleName.empty() || appIndex <= Constants::INITIAL_APP_INDEX) {
        HILOG_ERROR("The params are invalid.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_INTERNAL_ERROR;
    }

    return bundleInstaller_->UninstallSandboxApp(bundleName, appIndex, userId);
}

ErrCode BundleMgrHelper::GetUninstalledBundleInfo(const std::string bundleName, BundleInfo &bundleInfo)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    return bundleMgr_->GetUninstalledBundleInfo(bundleName, bundleInfo);
}

ErrCode BundleMgrHelper::GetSandboxBundleInfo(
    const std::string &bundleName, int32_t appIndex, int32_t userId, BundleInfo &info)
{
    HILOG_DEBUG("Called.");
    if (bundleName.empty() || appIndex <= Constants::INITIAL_APP_INDEX) {
        HILOG_ERROR("The params are invalid.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }
    ErrCode result = ConnectBundleInstaller();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_INTERNAL_ERROR;
    }

    return bundleMgr_->GetSandboxBundleInfo(bundleName, appIndex, userId, info);
}

ErrCode BundleMgrHelper::GetSandboxAbilityInfo(const Want &want, int32_t appIndex, int32_t flags, int32_t userId,
    AbilityInfo &abilityInfo)
{
    HILOG_DEBUG("Called.");
    if (appIndex <= Constants::INITIAL_APP_INDEX || appIndex > Constants::MAX_APP_INDEX) {
        HILOG_ERROR("The params are invalid.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_INTERNAL_ERROR;
    }

    return bundleMgr_->GetSandboxAbilityInfo(want, appIndex, flags, userId, abilityInfo);
}

ErrCode BundleMgrHelper::GetSandboxExtAbilityInfos(const Want &want, int32_t appIndex, int32_t flags,
    int32_t userId, std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    HILOG_DEBUG("Called.");
    if (appIndex <= Constants::INITIAL_APP_INDEX || appIndex > Constants::MAX_APP_INDEX) {
        HILOG_ERROR("The params are invalid.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_INTERNAL_ERROR;
    }

    return bundleMgr_->GetSandboxExtAbilityInfos(want, appIndex, flags, userId, extensionInfos);
}

ErrCode BundleMgrHelper::GetSandboxHapModuleInfo(const AbilityInfo &abilityInfo, int32_t appIndex, int32_t userId,
    HapModuleInfo &hapModuleInfo)
{
    HILOG_DEBUG("Called.");
    if (appIndex <= Constants::INITIAL_APP_INDEX || appIndex > Constants::MAX_APP_INDEX) {
        HILOG_ERROR("The params are invalid.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_INTERNAL_ERROR;
    }

    return bundleMgr_->GetSandboxHapModuleInfo(abilityInfo, appIndex, userId, hapModuleInfo);
}

ErrCode BundleMgrHelper::Connect()
{
    HILOG_DEBUG("Called.");
    std::lock_guard<std::mutex> lock(mutex_);
    if (bundleMgr_ == nullptr) {
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityManager == nullptr) {
            HILOG_ERROR("Failed to get system ability manager.");
            return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
        }

        sptr<IRemoteObject> remoteObject_ = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (remoteObject_ == nullptr || (bundleMgr_ = iface_cast<IBundleMgr>(remoteObject_)) == nullptr) {
            HILOG_ERROR("Failed to get bundle mgr service remote object.");
            return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
        }
        std::weak_ptr<BundleMgrHelper> weakPtr = shared_from_this();
        auto deathCallback = [weakPtr](const wptr<IRemoteObject>& object) {
            auto sharedPtr = weakPtr.lock();
            if (sharedPtr == nullptr) {
                HILOG_ERROR("Bundle helper instance is nullptr.");
                return;
            }
            sharedPtr->OnDeath();
        };
        deathRecipient_ = new (std::nothrow) BundleMgrServiceDeathRecipient(deathCallback);
        if (deathRecipient_ == nullptr) {
            HILOG_ERROR("Failed to create death recipient ptr deathRecipient_!");
            return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
        }
        bundleMgr_->AsObject()->AddDeathRecipient(deathRecipient_);
    }

    return ERR_OK;
}

ErrCode BundleMgrHelper::ConnectBundleInstaller()
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return result;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (bundleInstaller_ == nullptr) {
        bundleInstaller_ = bundleMgr_->GetBundleInstaller();
        if ((bundleInstaller_ == nullptr) || (bundleInstaller_->AsObject() == nullptr)) {
            HILOG_ERROR("Failed to get bundle installer proxy.");
            return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
        }
    }

    return ERR_OK;
}

void BundleMgrHelper::OnDeath()
{
    HILOG_DEBUG("Called.");
    std::lock_guard<std::mutex> lock(mutex_);
    if (bundleMgr_ == nullptr) {
        HILOG_ERROR("bundleMgr_ is nullptr.");
        return;
    }
    bundleMgr_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    bundleMgr_ = nullptr;
    bundleInstaller_ = nullptr;
}

bool BundleMgrHelper::GetBundleInfo(const std::string &bundleName, int32_t flags,
    BundleInfo &bundleInfo, int32_t userId)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->GetBundleInfo(bundleName, flags, bundleInfo, userId);
}

bool BundleMgrHelper::GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->GetHapModuleInfo(abilityInfo, hapModuleInfo);
}

std::string BundleMgrHelper::GetAbilityLabel(const std::string &bundleName, const std::string &abilityName)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return "";
    }

    return bundleMgr_->GetAbilityLabel(bundleName, abilityName);
}

std::string BundleMgrHelper::GetAppType(const std::string &bundleName)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return "";
    }

    return bundleMgr_->GetAppType(bundleName);
}

ErrCode BundleMgrHelper::GetBaseSharedBundleInfos(
    const std::string &bundleName, std::vector<BaseSharedBundleInfo> &baseSharedBundleInfos)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    return bundleMgr_->GetBaseSharedBundleInfos(bundleName, baseSharedBundleInfos);
}

ErrCode BundleMgrHelper::GetBundleInfoForSelf(int32_t flags, BundleInfo &bundleInfo)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    return bundleMgr_->GetBundleInfoForSelf(flags, bundleInfo);
}

ErrCode BundleMgrHelper::GetDependentBundleInfo(const std::string &sharedBundleName, BundleInfo &sharedBundleInfo)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    return bundleMgr_->GetDependentBundleInfo(sharedBundleName, sharedBundleInfo);
}

bool BundleMgrHelper::GetGroupDir(const std::string &dataGroupId, std::string &dir)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->GetGroupDir(dataGroupId, dir);
}

sptr<IOverlayManager> BundleMgrHelper::GetOverlayManagerProxy()
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return nullptr;
    }

    return bundleMgr_->GetOverlayManagerProxy();
}

bool BundleMgrHelper::QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->QueryAbilityInfo(want, abilityInfo);
}

bool BundleMgrHelper::QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->QueryAbilityInfo(want, flags, userId, abilityInfo);
}

bool BundleMgrHelper::GetBundleInfos(int32_t flags, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->GetBundleInfos(flags, bundleInfos, userId);
}

bool BundleMgrHelper::GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->GetBundleInfos(flag, bundleInfos, userId);
}

sptr<IQuickFixManager> BundleMgrHelper::GetQuickFixManagerProxy()
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return nullptr;
    }

    return bundleMgr_->GetQuickFixManagerProxy();
}

bool BundleMgrHelper::ProcessPreload(const Want &want)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->ProcessPreload(want);
}

sptr<IAppControlMgr> BundleMgrHelper::GetAppControlProxy()
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return nullptr;
    }

    return bundleMgr_->GetAppControlProxy();
}

bool BundleMgrHelper::QueryExtensionAbilityInfos(const Want &want, const int32_t &flag, const int32_t &userId,
    std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->QueryExtensionAbilityInfos(want, flag, userId, extensionInfos);
}

bool BundleMgrHelper::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int32_t userId, ApplicationInfo &appInfo)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->GetApplicationInfo(appName, flag, userId, appInfo);
}

bool BundleMgrHelper::GetApplicationInfo(
    const std::string &appName, int32_t flags, int32_t userId, ApplicationInfo &appInfo)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->GetApplicationInfo(appName, flags, userId, appInfo);
}

bool BundleMgrHelper::UnregisterBundleEventCallback(const sptr<IBundleEventCallback> &bundleEventCallback)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->UnregisterBundleEventCallback(bundleEventCallback);
}

bool BundleMgrHelper::QueryExtensionAbilityInfoByUri(
    const std::string &uri, int32_t userId, ExtensionAbilityInfo &extensionAbilityInfo)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->QueryExtensionAbilityInfoByUri(uri, userId, extensionAbilityInfo);
}

bool BundleMgrHelper::ImplicitQueryInfoByPriority(
    const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo, ExtensionAbilityInfo &extensionInfo)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->ImplicitQueryInfoByPriority(want, flags, userId, abilityInfo, extensionInfo);
}

bool BundleMgrHelper::QueryAbilityInfoByUri(const std::string &abilityUri, int32_t userId, AbilityInfo &abilityInfo)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->QueryAbilityInfoByUri(abilityUri, userId, abilityInfo);
}

bool BundleMgrHelper::QueryAbilityInfo(
    const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo, const sptr<IRemoteObject> &callBack)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->QueryAbilityInfo(want, flags, userId, abilityInfo, callBack);
}

void BundleMgrHelper::UpgradeAtomicService(const Want &want, int32_t userId)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return;
    }

    bundleMgr_->UpgradeAtomicService(want, userId);
}

bool BundleMgrHelper::ImplicitQueryInfos(const Want &want, int32_t flags, int32_t userId, bool withDefault,
    std::vector<AbilityInfo> &abilityInfos, std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->ImplicitQueryInfos(want, flags, userId, withDefault, abilityInfos, extensionInfos);
}

bool BundleMgrHelper::CleanBundleDataFiles(const std::string &bundleName, const int32_t userId)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->CleanBundleDataFiles(bundleName, userId);
}

bool BundleMgrHelper::QueryDataGroupInfos(
    const std::string &bundleName, int32_t userId, std::vector<DataGroupInfo> &infos)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->QueryDataGroupInfos(bundleName, userId, infos);
}

bool BundleMgrHelper::GetBundleGidsByUid(const std::string &bundleName, const int32_t &uid, std::vector<int32_t> &gids)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->GetBundleGidsByUid(bundleName, uid, gids);
}

bool BundleMgrHelper::RegisterBundleEventCallback(const sptr<IBundleEventCallback> &bundleEventCallback)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->RegisterBundleEventCallback(bundleEventCallback);
}

bool BundleMgrHelper::GetHapModuleInfo(const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->GetHapModuleInfo(abilityInfo, userId, hapModuleInfo);
}

bool BundleMgrHelper::QueryAppGalleryBundleName(std::string &bundleName) {
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return false;
    }

    return bundleMgr_->QueryAppGalleryBundleName(bundleName);
}

ErrCode BundleMgrHelper::GetUidByBundleName(const std::string &bundleName, const int32_t userId)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    return bundleMgr_->GetUidByBundleName(bundleName, userId);
}

sptr<IDefaultApp> BundleMgrHelper::GetDefaultAppProxy()
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return nullptr;
    }

    return bundleMgr_->GetDefaultAppProxy();
}

ErrCode BundleMgrHelper::GetJsonProfile(ProfileType profileType, const std::string &bundleName,
    const std::string &moduleName, std::string &profile, int32_t userId)
{
    HILOG_DEBUG("Called.");
    ErrCode result = Connect();
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to connect.");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    return bundleMgr_->GetUidByBundleName(bundleName, userId);
}
}  // namespace AppExecFwk
}  // namespace OHOS
