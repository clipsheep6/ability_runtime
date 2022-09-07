/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_H

#include <vector>
#include <gtest/gtest.h>

#include "ability_info.h"
#include "application_info.h"
#include "bundle_mgr_interface.h"
#include "want.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
class BundleMgrProxy : public IRemoteProxy<IBundleMgr> {
public:
    explicit BundleMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBundleMgr>(impl)
    {}
    ~BundleMgrProxy() = default;

    bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;
    bool GetApplicationInfos(
        const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos) override;
    bool GetBundleInfo(const std::string &bundleName,
        const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId) override
    {
        return true;
    }
    bool GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId) override
    {
        return true;
    }
    sptr<IBundleUserMgr> GetBundleUserMgr() override
    {
        return nullptr;
    }
    int GetUidByBundleName(const std::string &bundleName, const int userId) override;
    std::string GetAppIdByBundleName(const std::string &bundleName, const int userId) override;
    bool GetBundleNameForUid(const int uid, std::string &bundleName) override;
    bool GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames) override;
    bool GetNameForUid(const int uid, std::string &name) override;
    bool GetBundleGids(const std::string &bundleName, std::vector<int> &gids) override;
    std::string GetAppType(const std::string &bundleName) override;
    bool GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos) override;
    bool QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo) override;
    bool QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos) override;
    bool QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo) override;
    bool QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos) override;
    bool GetBundleArchiveInfo(
        const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo) override;
    bool GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo) override;
    bool GetHapModuleInfo(
        const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo) override;
    bool GetLaunchWantForBundle(const std::string &bundleName, Want &want) override;
    int CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName) override;
    bool GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef) override;
    bool HasSystemCapability(const std::string &capName) override;
    bool GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps) override;
    bool IsSafeMode() override;
    bool CleanBundleDataFiles(const std::string &bundleName, const int userId) override;
    bool RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override;
    bool ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override;
    bool UnregisterBundleStatusCallback() override;
    bool DumpInfos(
        const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result) override;
    bool IsApplicationEnabled(const std::string &bundleName) override;
    bool IsAbilityEnabled(const AbilityInfo &abilityInfo) override;
    bool GetAllFormsInfo(std::vector<FormInfo> &formInfos) override;
    bool GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos) override;
    bool GetFormsInfoByModule(
        const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos) override;
    bool GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos) override;
    sptr<IBundleInstaller> GetBundleInstaller() override;

    bool ImplicitQueryInfoByPriority(const Want &want, int32_t flags, int32_t userId,
        AbilityInfo &abilityInfo, ExtensionAbilityInfo &extensionInfo) override
    {
        abilityInfo.name = "MainAbility";
        abilityInfo.bundleName = "com.ohos.launcher";
        return true;
    }
};

class BundleMgrStub : public IRemoteStub<IBundleMgr> {
public:
    int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
};

class BundleMgrService : public BundleMgrStub {
public:
    bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;
    bool GetApplicationInfos(
        const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos) override;
    bool GetBundleInfo(const std::string &bundleName,
        const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId) override
    {
        return true;
    }
    bool GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId) override
    {
        return true;
    }
    sptr<IBundleUserMgr> GetBundleUserMgr() override
    {
        return nullptr;
    }
    int GetUidByBundleName(const std::string &bundleName, const int userId) override;
    std::string GetAppIdByBundleName(const std::string &bundleName, const int userId) override;
    bool GetBundleNameForUid(const int uid, std::string &bundleName) override;
    bool GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames) override;
    bool GetNameForUid(const int uid, std::string &name) override;
    bool GetBundleGids(const std::string &bundleName, std::vector<int> &gids) override;
    std::string GetAppType(const std::string &bundleName) override;
    bool GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos) override;
    bool QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo) override;
    bool QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos) override;
    bool QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo) override;
    bool QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos) override;
    bool GetBundleArchiveInfo(
        const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo) override;
    bool GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo) override;
    bool GetHapModuleInfo(
        const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo) override;
    bool GetLaunchWantForBundle(const std::string &bundleName, Want &want) override;
    int CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName) override;
    bool GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef) override;
    bool HasSystemCapability(const std::string &capName) override;
    bool GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps) override;
    bool IsSafeMode() override;
    bool CleanBundleDataFiles(const std::string &bundleName, const int userId) override;
    bool RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override;
    bool ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override;
    bool UnregisterBundleStatusCallback() override;
    bool DumpInfos(
        const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result) override;
    bool IsApplicationEnabled(const std::string &bundleName) override;
    bool IsAbilityEnabled(const AbilityInfo &abilityInfo) override;
    bool GetAllFormsInfo(std::vector<FormInfo> &formInfos) override;
    bool GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos) override;
    bool GetFormsInfoByModule(
        const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos) override;
    bool GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos) override;
    sptr<IBundleInstaller> GetBundleInstaller() override;
    bool GetBundleGidsByUid(const std::string &bundleName, const int &uid, std::vector<int> &gids) override
    {
        return true;
    }
    bool QueryAbilityInfosByUri(const std::string &abilityUri, std::vector<AbilityInfo> &abilityInfos) override
    {
        return true;
    }
    bool GetAllCommonEventInfo(const std::string &eventKey,
        std::vector<CommonEventInfo> &commonEventInfos) override
    {
        return true;
    }
    bool GetDistributedBundleInfo(const std::string &networkId, const std::string &bundleName,
        DistributedBundleInfo &distributedBundleInfo) override
    {
        return true;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_H
