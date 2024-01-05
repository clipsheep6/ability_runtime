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

#ifndef OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
#define OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H

#include <singleton.h>

#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

class BundleMgrHelper : public std::enable_shared_from_this<BundleMgrHelper> {
public:
    DISALLOW_COPY_AND_MOVE(BundleMgrHelper);
    ErrCode GetNameForUid(const int32_t uid, std::string &name);
    bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId);
    ErrCode InstallSandboxApp(const std::string &bundleName, int32_t dlpType, int32_t userId, int32_t &appIndex);
    ErrCode UninstallSandboxApp(const std::string &bundleName, int32_t appIndex, int32_t userId);
    ErrCode GetUninstalledBundleInfo(const std::string bundleName, BundleInfo &bundleInfo);
    ErrCode GetSandboxBundleInfo(const std::string &bundleName, int32_t appIndex, int32_t userId, BundleInfo &info);
    ErrCode GetSandboxAbilityInfo(const Want &want, int32_t appIndex, int32_t flags, int32_t userId,
        AbilityInfo &abilityInfo);
    ErrCode GetSandboxExtAbilityInfos(const Want &want, int32_t appIndex, int32_t flags, int32_t userId,
        std::vector<ExtensionAbilityInfo> &extensionInfos);
    ErrCode GetSandboxHapModuleInfo(const AbilityInfo &abilityInfo, int32_t appIndex, int32_t userId,
        HapModuleInfo &hapModuleInfo);
    bool GetBundleInfo(const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId);
    bool GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo);
    std::string GetAbilityLabel(const std::string &bundleName, const std::string &abilityName);
    std::string GetAppType(const std::string &bundleName);
    ErrCode GetBaseSharedBundleInfos(
        const std::string &bundleName, std::vector<BaseSharedBundleInfo> &baseSharedBundleInfos,
        GetDependentBundleInfoFlag flag = GetDependentBundleInfoFlag::GET_APP_CROSS_HSP_BUNDLE_INFO);
    ErrCode GetBundleInfoForSelf(int32_t flags, BundleInfo &bundleInfo);
    ErrCode GetDependentBundleInfo(const std::string &sharedBundleName, BundleInfo &sharedBundleInfo,
        GetDependentBundleInfoFlag flag = GetDependentBundleInfoFlag::GET_APP_CROSS_HSP_BUNDLE_INFO);
    bool GetGroupDir(const std::string &dataGroupId, std::string &dir);
    sptr<IOverlayManager> GetOverlayManagerProxy();
    bool QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo);
    bool QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo);
    bool GetBundleInfos(int32_t flags,
        std::vector<BundleInfo> &bundleInfos, int32_t userId = Constants::UNSPECIFIED_USERID);
    sptr<IQuickFixManager> GetQuickFixManagerProxy();
    bool ProcessPreload(const Want &want);
    sptr<IAppControlMgr> GetAppControlProxy();
    bool QueryExtensionAbilityInfos(const Want &want, const int32_t &flag, const int32_t &userId,
        std::vector<ExtensionAbilityInfo> &extensionInfos);
    ErrCode GetBundleInfoV9(
        const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId);
    bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int32_t userId, ApplicationInfo &appInfo);
    bool GetApplicationInfo(const std::string &appName, int32_t flags, int32_t userId, ApplicationInfo &appInfo);
    ErrCode GetJsonProfile(ProfileType profileType, const std::string &bundleName,
        const std::string &moduleName, std::string &profile, int32_t userId = Constants::UNSPECIFIED_USERID);
    bool UnregisterBundleEventCallback(const sptr<IBundleEventCallback> &bundleEventCallback);
    bool QueryExtensionAbilityInfoByUri(
        const std::string &uri, int32_t userId, ExtensionAbilityInfo &extensionAbilityInfo);
    bool ImplicitQueryInfoByPriority(const Want &want, int32_t flags, int32_t userId,
        AbilityInfo &abilityInfo, ExtensionAbilityInfo &extensionInfo);
    bool QueryAbilityInfoByUri(const std::string &abilityUri, int32_t userId, AbilityInfo &abilityInfo);
    bool QueryAbilityInfo(const Want &want,
        int32_t flags, int32_t userId, AbilityInfo &abilityInfo, const sptr<IRemoteObject> &callBack);
    void UpgradeAtomicService(const Want &want, int32_t userId);
    bool ImplicitQueryInfos(const Want &want, int32_t flags, int32_t userId, bool withDefault,
        std::vector<AbilityInfo> &abilityInfos, std::vector<ExtensionAbilityInfo> &extensionInfos);
    bool CleanBundleDataFiles(const std::string &bundleName, const int32_t userId = Constants::DEFAULT_USERID);
    bool QueryDataGroupInfos(const std::string &bundleName, int32_t userId, std::vector<DataGroupInfo> &infos);
    bool GetBundleGidsByUid(const std::string &bundleName, const int32_t &uid, std::vector<int32_t> &gids);
    bool RegisterBundleEventCallback(const sptr<IBundleEventCallback> &bundleEventCallback);
    bool GetBundleInfos(
        const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId = Constants::UNSPECIFIED_USERID);
    bool GetHapModuleInfo(const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo);
    bool QueryAppGalleryBundleName(std::string &bundleName);
    ErrCode GetUidByBundleName(const std::string &bundleName, const int32_t userId);
    ErrCode QueryExtensionAbilityInfosOnlyWithTypeName(const std::string &extensionTypeName,
        const uint32_t flag, const int32_t userId, std::vector<ExtensionAbilityInfo> &extensionInfos);
    sptr<IDefaultApp> GetDefaultAppProxy();
    ErrCode CleanObsoleteBundleTempFiles();

private:
    sptr<IBundleMgr> Connect();
    sptr<IBundleInstaller> ConnectBundleInstaller();
    void OnDeath();
	
private:
    DECLARE_DELAYED_SINGLETON(BundleMgrHelper)
    std::mutex mutex_;
    sptr<IBundleMgr> bundleMgr_;
    sptr<IBundleInstaller> bundleInstaller_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H