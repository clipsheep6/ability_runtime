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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MGR_SERVICE_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MGR_SERVICE_H

#include "gmock/gmock.h"
#include "semaphore_ex.h"
#include "bundle_mgr_host.h"

namespace OHOS {
namespace AppExecFwk {
class MockBundleMgrService : public BundleMgrHost {
public:
    MOCK_METHOD4(GetApplicationInfo,
        bool(const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo));
    virtual bool GetApplicationInfos(
        const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos) override
    {
        return false;
    }
    virtual bool GetBundleInfo(
        const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId) override
    {
        return false;
    }
    virtual bool GetBundleInfos(
        const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId) override
    {
        return false;
    }
    virtual int GetUidByBundleName(const std::string &bundleName, const int userId) override
    {
        return 0;
    }
    virtual std::string GetAppIdByBundleName(const std::string &bundleName, const int userId) override
    {
        return "";
    }
    virtual bool GetBundleNameForUid(const int uid, std::string &bundleName) override
    {
        return false;
    }
    MOCK_METHOD2(GetBundleGids, bool(const std::string &bundleName, std::vector<int> &gids));
    std::string GetAppType(const std::string &bundleName)
    {
        GTEST_LOG_(INFO) << "MockBundleMgrService::GetAppType called";
        return "ModuleTestType";
    }
    virtual sptr<IBundleUserMgr> GetBundleUserMgr() override
    {
        return nullptr;
    }
    MOCK_METHOD2(GetBundleInfosByMetaData, bool(const std::string &metaData, std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD2(QueryAbilityInfo, bool(const Want &want, AbilityInfo &abilityInfo));
    MOCK_METHOD2(QueryAbilityInfoByUri, bool(const std::string &abilityUri, AbilityInfo &abilityInfo));
    MOCK_METHOD1(QueryKeepAliveBundleInfos, bool(std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD3(
        GetBundleArchiveInfo, bool(const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo));
    MOCK_METHOD2(GetHapModuleInfo, bool(const std::string &hapFilePath, HapModuleInfo &hapModuleInfo));
    MOCK_METHOD2(GetHapModuleInfo, bool(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo));
    MOCK_METHOD3(GetHapModuleInfo, bool(const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo));
    MOCK_METHOD2(GetLaunchWantForBundle, bool(const std::string &bundleName, Want &want));
    MOCK_METHOD2(CheckPublicKeys, int(const std::string &firstBundleName, const std::string &secondBundleName));
    MOCK_METHOD2(CheckPermission, int(const std::string &bundleName, const std::string &permission));
    MOCK_METHOD2(GetPermissionDef, int32_t(const std::string &permissionName, PermissionDef &permissionDef));
    MOCK_METHOD1(GetAllPermissionGroupDefs, bool(std::vector<PermissionDef> &permissionDefs));
    MOCK_METHOD2(GetAppsGrantedPermissions,
        bool(const std::vector<std::string> &permissions, std::vector<std::string> &appNames));
    MOCK_METHOD1(HasSystemCapability, bool(const std::string &capName));
    MOCK_METHOD1(GetSystemAvailableCapabilities, bool(std::vector<std::string> &systemCaps));
    MOCK_METHOD0(IsSafeMode, bool());
    MOCK_METHOD2(CleanBundleDataFiles, bool(const std::string &bundleName, const int userId));
    MOCK_METHOD1(RegisterBundleStatusCallback, bool(const sptr<IBundleStatusCallback> &bundleStatusCallback));
    MOCK_METHOD1(ClearBundleStatusCallback, bool(const sptr<IBundleStatusCallback> &bundleStatusCallback));
    MOCK_METHOD0(UnregisterBundleStatusCallback, bool());
    virtual bool DumpInfos(
        const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result) override
    {
        return false;
    }
    MOCK_METHOD0(GetBundleInstaller, sptr<IBundleInstaller>());
    MOCK_METHOD1(GetBundleInstaller, bool(const std::string &));
    MOCK_METHOD1(IsApplicationEnabled, bool(const std::string &));
    MOCK_METHOD2(GetNameForUid, bool(const int uid, std::string &name));
    MOCK_METHOD2(GetBundlesForUid, bool(const int uid, std::vector<std::string> &));
    MOCK_METHOD1(IsAbilityEnabled, bool(const AbilityInfo &));
    void Wait()
    {
        sem_.Wait();
    }

    int Post()
    {
        sem_.Post();
        return 0;
    }

    void PostVoid()
    {
        sem_.Post();
    }

private:
    Semaphore sem_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MGR_SERVICE_H
