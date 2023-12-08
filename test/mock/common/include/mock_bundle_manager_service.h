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

#ifndef OHOS_MOCK_BUNDLE_MANAGER_SERVICE_H
#define OHOS_MOCK_BUNDLE_MANAGER_SERVICE_H

#include "bundle_installer_interface.h"
#include "bundle_mgr_interface.h"
#include "gmock/gmock.h"
#include "iremote_object.h"
#include "iremote_stub.h"
#include "mock_overlay_manager.h"

namespace OHOS {
constexpr int32_t BASE_USER_RANGE = 200000;
class MockBundleManagerService : public IRemoteStub<AppExecFwk::IBundleMgr> {
public:
    MockBundleManagerService() {};
    virtual ~MockBundleManagerService() {};
    MOCK_METHOD0(GetBundleInstaller, sptr<AppExecFwk::IBundleInstaller>());
    MOCK_METHOD2(
        GetHapModuleInfo, bool(const AppExecFwk::AbilityInfo &abilityInfo, AppExecFwk::HapModuleInfo &hapModuleInfo));
    MOCK_METHOD3(GetHapModuleInfo,
        bool(const AppExecFwk::AbilityInfo &abilityInfo, int32_t userId, AppExecFwk::HapModuleInfo &hapModuleInfo));
    MOCK_METHOD1(GetAppType, std::string(const std::string &bundleName));
    MOCK_METHOD2(GetBaseSharedBundleInfos,
        ErrCode(const std::string &bundleName, std::vector<AppExecFwk::BaseSharedBundleInfo> &baseSharedBundleInfos));
    MOCK_METHOD2(GetBundleInfoForSelf, ErrCode(int32_t flags, AppExecFwk::BundleInfo &bundleInfo));

    sptr<AppExecFwk::IOverlayManager> GetOverlayManagerProxy()
    {
        sptr<AppExecFwk::IOverlayManager> overlayModuleProxy =
            new (std::nothrow) AppExecFwk::OverlayManagerProxy(nullptr);
        return overlayModuleProxy;
    }

    bool GetBundleGidsByUid(const std::string &bundleName, const int &uid, std::vector<int> &gids)
    {
        return true;
    }

    bool GetBundleInfo(const std::string &bundleName, const AppExecFwk::BundleFlag flag,
        AppExecFwk::BundleInfo &bundleInfo, int32_t userId) override
    {
        if (bundleName == "test_contextImpl") {
            bundleInfo.name = "test_contextImpl";
            bundleInfo.applicationInfo.name = "test_contextImpl";
            AppExecFwk::HapModuleInfo moduleInfo1;
            moduleInfo1.moduleName = "test_moduleName";
            bundleInfo.hapModuleInfos.push_back(moduleInfo1);
        }
        return true;
    }

    bool GetApplicationInfo(const std::string &appName, const AppExecFwk::ApplicationFlag flag, const int userId,
        AppExecFwk::ApplicationInfo &appInfo)
    {
        if (appName.empty()) {
            return false;
        }
        appInfo.name = appName;
        appInfo.bundleName = appName;
        appInfo.uid = userId * BASE_USER_RANGE;
        if (appName.compare("com.test.crowdtest") == 0) {
            appInfo.appDistributionType = "crowdtesting";
            appInfo.crowdtestDeadline = 0;
        }
        if (appName.compare("com.test.atomicservice") == 0) {
            appInfo.bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
        }
        return true;
    }

    bool GetBundleInfos(const AppExecFwk::BundleFlag flag, std::vector<AppExecFwk::BundleInfo> &bundleInfos,
        int32_t userId = AppExecFwk::Constants::UNSPECIFIED_USERID)
    {
        OHOS::AppExecFwk::BundleInfo bundleInfo;
        bundleInfo.name = "com.ix.residentservcie";
        bundleInfo.isKeepAlive = true;
        bundleInfo.applicationInfo.process = "com.ix.residentservcie";

        OHOS::AppExecFwk::HapModuleInfo hapModuleInfo;
        hapModuleInfo.isModuleJson = true;
        hapModuleInfo.mainElementName = "residentServiceAbility";
        hapModuleInfo.process = "com.ix.residentservcie";
        bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo);

        bundleInfos.emplace_back(bundleInfo);
        return true;
    }
};
} // namespace OHOS
#endif // OHOS_MOCK_BUNDLE_MANAGER_SERVICE_H