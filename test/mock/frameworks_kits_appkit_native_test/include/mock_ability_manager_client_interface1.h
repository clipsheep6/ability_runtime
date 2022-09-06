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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MANAGER_CLIENT_INTERFACE1_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MANAGER_CLIENT_INTERFACE1_H

#include <mutex>

#include "ability_connect_callback_interface.h"
#include "ability_context.h"
#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "ability_manager_interface.h"
#include "ability_scheduler_interface.h"
#include "iremote_object.h"
#include "iremote_stub.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class MockAbilityManagerClient : public AbilityManagerClient {
public:
    MockAbilityManagerClient();
    virtual ~MockAbilityManagerClient();

    ErrCode GetStartAbility();
    ErrCode GetTerminateAbility();
    ErrCode GetTerminateAbilityResult();

    void SetStartAbility(ErrCode tValue);
    void SetTerminateAbility(ErrCode tValue);
    void SetTerminateAbilityResult(ErrCode tValue);

    int GetTerminateAbilityValue();
    void SetTerminateAbilityValue(int nValue);

    static std::shared_ptr<MockAbilityManagerClient> mock_instance_;
    static bool mock_intanceIsNull_;

    static std::shared_ptr<MockAbilityManagerClient> GetInstance();
    static void SetInstanceNull(bool flag);

private:
    ErrCode startAbility_;
    ErrCode terminateAbility_;
    ErrCode terminateAbilityResult_;

    int terminateAbilityValue_;
};
}  // namespace AAFwk
}  // namespace OHOS

namespace OHOS {
namespace AppExecFwk {
class MockIBundleMgr : public IRemoteStub<IBundleMgr> {
public:
    MockIBundleMgr() {};
    virtual ~MockIBundleMgr() {};

    virtual bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
    {
        return true;
    };
    virtual bool GetApplicationInfos(
        const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos)
    {
        return true;
    };
    virtual bool GetBundleInfo(
        const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
    {
        return true;
    };
    virtual bool GetBundleInfos(
        const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId)
    {
        return true;
    };
    virtual int GetUidByBundleName(const std::string &bundleName, const int userId)
    {
        return 0;
    };
    virtual std::string GetAppIdByBundleName(const std::string &bundleName, const int userId)
    {
        return std::string("");
    };
    virtual bool GetBundleNameForUid(const int uid, std::string &bundleName)
    {
        return true;
    };
    virtual bool GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames)
    {
        return true;
    };
    virtual bool GetNameForUid(const int uid, std::string &name)
    {
        return true;
    };
    virtual bool GetBundleGids(const std::string &bundleName, std::vector<int> &gids)
    {
        return true;
    };
    virtual std::string GetAppType(const std::string &bundleName)
    {
        return std::string("");
    };
    virtual bool GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos)
    {
        return true;
    };
    virtual bool QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo)
    {
        return true;
    };
    virtual bool QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo)
    {
        return true;
    };
    virtual bool QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos)
    {
        return true;
    };
    virtual bool GetBundleArchiveInfo(const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo)
    {
        return true;
    };
    virtual bool GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
    {
        return true;
    };
    virtual bool GetHapModuleInfo(const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo) override
    {
        return true;
    }
    virtual bool GetLaunchWantForBundle(const std::string &bundleName, Want &want)
    {
        return true;
    };
    virtual int CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName)
    {
        return 0;
    };
    virtual int CheckPermission(const std::string &bundleName, const std::string &permission)
    {
        return 0;
    };
    virtual ErrCode GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef)
    {
        return 1;
    };
    virtual bool GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs)
    {
        return true;
    };
    virtual bool GetAppsGrantedPermissions(
        const std::vector<std::string> &permissions, std::vector<std::string> &appNames)
    {
        return true;
    };
    virtual bool HasSystemCapability(const std::string &capName)
    {
        return true;
    };
    virtual bool GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps)
    {
        return true;
    };
    virtual bool IsSafeMode()
    {
        return true;
    };
    virtual bool CleanBundleDataFiles(const std::string &bundleName, const int userId = 0)
    {
        return true;
    };
    virtual bool RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
    {
        return true;
    };
    virtual bool ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
    {
        return true;
    };
    virtual bool UnregisterBundleStatusCallback()
    {
        return true;
    };
    virtual bool DumpInfos(
        const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result)
    {
        return true;
    };
    virtual bool IsApplicationEnabled(const std::string &bundleName)
    {
        return true;
    };
    virtual bool IsAbilityEnabled(const AbilityInfo &abilityInfo)
    {
        return true;
    };
    virtual bool CanRequestPermission(
        const std::string &bundleName, const std::string &permissionName, const int userId)
    {
        return true;
    };
    virtual bool RequestPermissionFromUser(
        const std::string &bundleName, const std::string &permission, const int userId)
    {
        return true;
    };
    virtual bool GetAllFormsInfo(std::vector<FormInfo> &formInfos)
    {
        return true;
    };
    virtual bool GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos)
    {
        return true;
    };
    virtual bool GetFormsInfoByModule(
        const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos)
    {
        return true;
    };
    virtual sptr<IBundleInstaller> GetBundleInstaller()
    {
        return nullptr;
    };
    bool QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos)
    {
        return true;
    }
    bool GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos)
    {
        return true;
    }
    virtual sptr<IBundleUserMgr> GetBundleUserMgr() override
    {
        return nullptr;
    }
};

class MockAbilityContextDeal : public ContextDeal {
public:
    MockAbilityContextDeal() {};
    virtual ~MockAbilityContextDeal() {};

    sptr<IBundleMgr> GetBundleManager() const override
    {
        return sptr<IBundleMgr>(new (std::nothrow) MockIBundleMgr());
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MANAGER_CLIENT_INTERFACE1_H
