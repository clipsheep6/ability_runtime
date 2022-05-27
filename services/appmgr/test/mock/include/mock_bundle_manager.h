/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_AAFWK_ABILITY_MOCK_BUNDLE_MANAGER_H
#define OHOS_AAFWK_ABILITY_MOCK_BUNDLE_MANAGER_H

#include <vector>
#include "gmock/gmock.h"
#include "ability_info.h"
#include "application_info.h"
#include "want.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string COM_OHOS_HELLO = "com.ohos.test.helloworld";
const int32_t APPLICATION_NUMHELLO = 104;
const std::string COM_OHOS_SPECIAL = "com.ohos.test.special";
}  // namespace
class BundleMgrProxy : public IRemoteProxy<IBundleMgr> {
public:
    explicit BundleMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBundleMgr>(impl)
    {}
    virtual ~BundleMgrProxy()
    {}
    MOCK_METHOD2(GetNameForUid, bool(const int uid, std::string &name));
    MOCK_METHOD2(GetBundlesForUid, bool(const int uid, std::vector<std::string> &));
    MOCK_METHOD1(IsAbilityEnabled, bool(const AbilityInfo &));
    MOCK_METHOD2(QueryAbilityInfosByUri, bool(const std::string &abilityUri, std::vector<AbilityInfo> &abilityInfos));
    MOCK_METHOD2(RemoveClonedBundle, bool(const std::string &bundleName, const int32_t uid));
    MOCK_METHOD1(BundleClone, bool(const std::string &bundleName));
    MOCK_METHOD3(GetBundleGidsByUid, bool(const std::string &bundleName, const int &uid, std::vector<int> &gids));
    MOCK_METHOD1(CheckBundleNameInAllowList, bool(const std::string &bundleName));
    bool QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo) override;
    bool QueryAbilityInfoByUri(const std::string &uri, AbilityInfo &abilityInfo) override;
    std::string GetAppType(const std::string &bundleName) override;

    virtual bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;
    virtual bool GetApplicationInfos(
        const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos) override
    {
        return true;
    };
    virtual bool QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos) override
    {
        return true;
    };
    virtual bool QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfos) override
    {
        return true;
    };
    virtual bool GetBundleInfo(
        const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId) override
    {
        return true;
    };
    virtual bool GetBundleInfos(
        const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId) override
    {
        return true;
    };
    virtual int GetUidByBundleName(const std::string &bundleName, const int userId) override
    {
        return 0;
    };
    virtual std::string GetAppIdByBundleName(const std::string &bundleName, const int userId) override
    {
        return "";
    };
    virtual bool GetBundleNameForUid(const int uid, std::string &bundleName) override
    {
        return true;
    };
    virtual bool GetBundleGids(const std::string &bundleName, std::vector<int> &gids) override
    {
        return true;
    };
    virtual bool GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos) override
    {
        return true;
    };
    virtual bool QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos) override
    {
        int appUid = 2100;
        GTEST_LOG_(INFO) << "QueryKeepAliveBundleInfos()";
        ApplicationInfo info;
        info.name = "KeepAliveApp";
        info.bundleName = "KeepAliveApplication";
        info.uid = appUid;

        BundleInfo bundleInfo;
        bundleInfo.applicationInfo = info;
        HapModuleInfo hapModuleInfo;
        hapModuleInfo.name = "Module";
        HapModuleInfo hapModuleInfo1;
        hapModuleInfo1.name = "Module1";
        bundleInfo.hapModuleInfos.push_back(hapModuleInfo);
        bundleInfo.hapModuleInfos.push_back(hapModuleInfo1);

        bundleInfos.push_back(bundleInfo);
        GTEST_LOG_(INFO) << "bundleInfos size : "<<bundleInfos.size();
        return true;
    };
    virtual bool GetBundleArchiveInfo(
        const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo) override
    {
        return true;
    };
    virtual bool GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo) override;
    virtual bool GetHapModuleInfo(
        const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo) override;
    virtual bool GetLaunchWantForBundle(const std::string &bundleName, Want &want) override
    {
        return true;
    };
    virtual int CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName) override
    {
        return 0;
    };

    virtual bool GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef) override
    {
        return true;
    };
    virtual bool HasSystemCapability(const std::string &capName) override
    {
        return true;
    };
    virtual bool GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps) override
    {
        return true;
    };
    virtual bool IsSafeMode() override
    {
        return true;
    };
    virtual bool CleanBundleDataFiles(const std::string &bundleName, const int userId = 0) override
    {
        return true;
    };
    virtual bool RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override
    {
        return true;
    };
    virtual bool ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override
    {
        return true;
    };
    // unregister callback of all application
    virtual bool UnregisterBundleStatusCallback() override
    {
        return true;
    };
    virtual bool DumpInfos(
        const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result) override
    {
        return true;
    };
    virtual sptr<IBundleInstaller> GetBundleInstaller() override
    {
        return nullptr;
    };
    virtual sptr<IBundleUserMgr> GetBundleUserMgr() override
    {
        return nullptr;
    }
    virtual bool GetModuleUsageRecords(
        const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords) override
    {
        return true;
    }
    virtual bool NotifyAbilityLifeStatus(const std::string &bundleName,
        const std::string &abilityName, const int64_t launchTime, const int uid = 0) override
    {
        return true;
    }

    virtual bool GetDistributedBundleInfo(const std::string &networkId, const std::string &bundleName,
        DistributedBundleInfo &distributedBundleInfo)
    {
        return true;
    }
};

class BundleMgrStub : public IRemoteStub<IBundleMgr> {
public:
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
};

class BundleMgrService : public BundleMgrStub {
public:
    MOCK_METHOD2(GetUidByBundleName, int(const std::string &bundleName, const int userId));
    MOCK_METHOD2(GetAppIdByBundleName, std::string(const std::string &bundleName, const int userId));
    MOCK_METHOD2(CleanBundleDataFiles, bool(const std::string &bundleName, const int userId));
    MOCK_METHOD2(GetNameForUid, bool(const int uid, std::string &name));
    MOCK_METHOD2(GetBundlesForUid, bool(const int uid, std::vector<std::string> &));
    MOCK_METHOD1(IsAbilityEnabled, bool(const AbilityInfo &));
    MOCK_METHOD2(QueryAbilityInfosByUri, bool(const std::string &abilityUri, std::vector<AbilityInfo> &abilityInfos));
    MOCK_METHOD2(RemoveClonedBundle, bool(const std::string &bundleName, const int32_t uid));
    MOCK_METHOD1(BundleClone, bool(const std::string &bundleName));
    MOCK_METHOD1(CheckBundleNameInAllowList, bool(const std::string &bundleName));
    bool QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo) override;
    bool QueryAbilityInfoByUri(const std::string &uri, AbilityInfo &abilityInfo) override;

    std::string GetAppType(const std::string &bundleName) override;

    virtual bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;
    virtual bool GetApplicationInfos(
        const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos) override
    {
        return true;
    };
    virtual bool QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos) override
    {
        return true;
    };
    virtual bool QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfos) override
    {
        return true;
    };
    virtual bool GetBundleInfo(
        const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId) override;
    virtual bool GetBundleInfos(
        const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId) override;
    bool GetBundleGidsByUid(
        const std::string &bundleName, const int &uid, std::vector<int> &gids) override;
    virtual bool GetBundleNameForUid(const int uid, std::string &bundleName) override
    {
        return true;
    };
    virtual bool GetBundleGids(const std::string &bundleName, std::vector<int> &gids) override;
    virtual bool GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos) override
    {
        return true;
    };
    // obtains information about an application bundle contained in a ohos Ability Package (HAP).
    virtual bool GetBundleArchiveInfo(
        const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo) override
    {
        return true;
    };
    virtual bool GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo);
    virtual bool GetHapModuleInfo(
        const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo) override;
    // obtains the Want for starting the main ability of an application based on the given bundle name.
    virtual bool GetLaunchWantForBundle(const std::string &bundleName, Want &want) override
    {
        return true;
    };
    // checks whether the publickeys of two bundles are the same.
    virtual int CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName) override
    {
        return 0;
    };
    // checks whether a specified bundle has been granted a specific permission.
    virtual bool GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef) override
    {
        return true;
    };
    virtual bool HasSystemCapability(const std::string &capName) override
    {
        return true;
    };
    virtual bool GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps) override
    {
        return true;
    };
    virtual bool IsSafeMode() override
    {
        return true;
    };

    virtual bool RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override
    {
        return true;
    };
    virtual bool ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override
    {
        return true;
    };
    // unregister callback of all application
    virtual bool UnregisterBundleStatusCallback() override
    {
        return true;
    };
    virtual bool DumpInfos(
        const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result) override
    {
        return true;
    };
    virtual sptr<IBundleInstaller> GetBundleInstaller() override
    {
        return nullptr;
    };
    virtual sptr<IBundleUserMgr> GetBundleUserMgr() override
    {
        return nullptr;
    }
    virtual bool IsApplicationEnabled(const std::string &bundleName) override
    {
        return true;
    };
    virtual bool GetAllFormsInfo(std::vector<FormInfo> &formInfos) override
    {
        return true;
    }
    virtual bool GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos) override
    {
        return true;
    }
    virtual bool GetFormsInfoByModule(
        const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos) override
    {
        return true;
    }
    virtual bool GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos) override
    {
        return true;
    }
    virtual bool GetAllCommonEventInfo(const std::string &eventKey,
        std::vector<CommonEventInfo> &commonEventInfos) override
    {
        return true;
    }
    virtual bool GetModuleUsageRecords(
        const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords) override
    {
        return true;
    }
    virtual bool NotifyAbilityLifeStatus(
        const std::string &bundleName, const std::string &abilityName,
        const int64_t launchTime, const int uid = 0) override
    {
        return true;
    }
    virtual bool QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos) override
    {
        int appUid = 2100;
        GTEST_LOG_(INFO) << "QueryKeepAliveBundleInfos()";
        ApplicationInfo info;
        info.name = "KeepAliveApp";
        info.bundleName = "KeepAliveApplication";
        info.uid = appUid;

        BundleInfo bundleInfo;
        bundleInfo.applicationInfo = info;
        HapModuleInfo hapModuleInfo;
        hapModuleInfo.name = "Module";
        HapModuleInfo hapModuleInfo1;
        hapModuleInfo1.name = "Module1";
        bundleInfo.hapModuleInfos.push_back(hapModuleInfo);
        bundleInfo.hapModuleInfos.push_back(hapModuleInfo1);

        bundleInfos.push_back(bundleInfo);
        GTEST_LOG_(INFO) << "bundleInfos size : "<<bundleInfos.size();
        return true;
    };
    virtual bool GetDistributedBundleInfo(const std::string &networkId, const std::string &bundleName,
        DistributedBundleInfo &distributedBundleInfo)
    {
        return true;
    }

    virtual bool ImplicitQueryInfoByPriority(const Want &want, int32_t flags, int32_t userId,
        AbilityInfo &abilityInfo, ExtensionAbilityInfo &extensionInfo) override
    {
        abilityInfo.name = "MainAbility";
        abilityInfo.bundleName = "com.ohos.launcher";
        return true;
    }

    BundleMgrService();
    virtual ~BundleMgrService() {}
    void MakingPackageData();
    void PushTestHelloIndexAbility(int index);
    void PushTestSpecialAbility();
    void PushTestHelloAbility();
    void MakingResidentProcData();
private:
    std::vector<BundleInfo> bundleInfos_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_ABILITY_MOCK_BUNDLE_MANAGER_H
