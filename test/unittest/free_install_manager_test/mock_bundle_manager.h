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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_H

#include <vector>
#include <gmock/gmock.h>
#include "ability_info.h"
#include "application_info.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "want.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "form_info.h"
#include "mock_app_control_manager.h"
#include "shortcut_info.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t MAX_SYS_UID = 2899;
const int32_t BASE_USER_RANGE = 200000;
const int32_t APPLICATIONINFO_UID = 20000000;
}  // namespace
class BundleMgrProxy : public IRemoteProxy<IBundleMgr> {
public:
    explicit BundleMgrProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IBundleMgr>(impl)
    {}
    virtual ~BundleMgrProxy()
    {}

    bool QueryAbilityInfo(const Want& want, int32_t flags, int32_t userId, AbilityInfo& abilityInfo,
        const sptr<IRemoteObject>& callBack) override
    {
        if (userId == 1) {
            // 创建回调
            return false;
        }
        return true;
    }

    bool QueryAbilityInfo(const AAFwk::Want& want, AbilityInfo& abilityInfo) override;

    bool GetApplicationInfo(
        const std::string& appName, const ApplicationFlag flag, const int userId, ApplicationInfo& appInfo) override;

    bool GetBundleInfo(
        const std::string& bundleName, const BundleFlag flag, BundleInfo& bundleInfo, int32_t userId) override;

    virtual bool ImplicitQueryInfoByPriority(const Want& want, int32_t flags, int32_t userId,
        AbilityInfo& abilityInfo, ExtensionAbilityInfo& extensionInfo) override
    {
        return true;
    }
};

class BundleMgrStub : public IRemoteStub<IBundleMgr> {
public:
    virtual ~BundleMgrStub() {}
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
};

class BundleMgrService : public BundleMgrStub {
public:
    BundleMgrService();
    ~BundleMgrService();

    bool QueryAbilityInfo(const AAFwk::Want& want, AbilityInfo& abilityInfo) override;
    bool QueryAbilityInfo(const Want& want, int32_t flags, int32_t userId, AbilityInfo& abilityInfo) override;
    bool GetApplicationInfo(
        const std::string& appName, const ApplicationFlag flag, const int userId, ApplicationInfo& appInfo) override;
    bool GetBundleInfo(
        const std::string& bundleName, const BundleFlag flag, BundleInfo& bundleInfo, int32_t userId) override;
    int GetUidByBundleName(const std::string& bundleName, const int userId) override;

    bool CheckWantEntity(const AAFwk::Want&, AbilityInfo&);

    virtual bool ImplicitQueryInfoByPriority(const Want& want, int32_t flags, int32_t userId,
        AbilityInfo& abilityInfo, ExtensionAbilityInfo& extensionAbilityInfo) override
    {
        abilityInfo.name = "MainAbility";
        abilityInfo.bundleName = "com.ohos.launcher";
        return true;
    }

    virtual sptr<IAppControlMgr> GetAppControlProxy()
    {
        if (isAppControlProxyNull_) {
            isAppControlProxyNull_ = false;
            return nullptr;
        }
        sptr<IAppControlMgr> appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
        return appControlMgr;
    }
public:
    using QueryAbilityInfoFunType =
        std::function<bool(std::string bundleName, AbilityInfo& abilityInfo, ElementName& elementTemp)>;
    std::map<std::string, QueryAbilityInfoFunType> abilityInfoMap_;
    bool isAppControlProxyNull_ = false;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_H
