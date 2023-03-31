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
    explicit BundleMgrProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IBundleMgr>(impl)
    {}
    ~BundleMgrProxy() = default;

    bool GetApplicationInfo(
        const std::string& appName, const ApplicationFlag flag, const int userId, ApplicationInfo& appInfo) override;

    bool GetBundleInfo(const std::string& bundleName,
        const BundleFlag flag, BundleInfo& bundleInfo, int32_t userId) override
    {
        return true;
    }
    std::string GetAppType(const std::string& bundleName) override;
    bool QueryAbilityInfo(const Want& want, AbilityInfo& abilityInfo) override;
    bool GetHapModuleInfo(const AbilityInfo& abilityInfo, HapModuleInfo& hapModuleInfo) override;
    bool GetHapModuleInfo(
        const AbilityInfo& abilityInfo, int32_t userId, HapModuleInfo& hapModuleInfo) override;

    bool ImplicitQueryInfoByPriority(const Want& want, int32_t flags, int32_t userId,
        AbilityInfo& abilityInfo, ExtensionAbilityInfo& extensionInfo) override
    {
        abilityInfo.name = "MainAbility";
        abilityInfo.bundleName = "com.ohos.launcher";
        return true;
    }

    ErrCode GetBaseSharedBundleInfos(const std::string &bundleName,
        std::vector<BaseSharedBundleInfo> &baseSharedBundleInfos) override
    {
        return ERR_OK;
    }
};

class BundleMgrStub : public IRemoteStub<IBundleMgr> {
public:
    int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
};

class BundleMgrService : public BundleMgrStub {
public:
    bool GetApplicationInfo(
        const std::string& appName, const ApplicationFlag flag, const int userId, ApplicationInfo& appInfo) override;
    bool GetBundleInfo(const std::string& bundleName,
        const BundleFlag flag, BundleInfo& bundleInfo, int32_t userId) override
    {
        if (bundleName == "test_contextImpl") {
            bundleInfo.name = "test_contextImpl";
            bundleInfo.applicationInfo.name = "test_contextImpl";
            HapModuleInfo moduleInfo1;
            moduleInfo1.moduleName = "test_moduleName";
            bundleInfo.hapModuleInfos.push_back(moduleInfo1);
        }
        return true;
    }
    std::string GetAppType(const std::string& bundleName) override;
    bool QueryAbilityInfo(const Want& want, AbilityInfo& abilityInfo) override;
    bool GetHapModuleInfo(const AbilityInfo& abilityInfo, HapModuleInfo& hapModuleInfo) override;
    bool GetHapModuleInfo(
        const AbilityInfo& abilityInfo, int32_t userId, HapModuleInfo& hapModuleInfo) override;
    ErrCode GetBaseSharedBundleInfos(const std::string &bundleName,
        std::vector<BaseSharedBundleInfo> &baseSharedBundleInfos) override
    {
        return ERR_OK;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_H
