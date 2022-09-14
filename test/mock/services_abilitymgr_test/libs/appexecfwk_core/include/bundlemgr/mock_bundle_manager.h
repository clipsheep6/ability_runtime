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
#include "shortcut_info.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string COM_IX_HIWORLD = "com.ix.hiworld";
const std::string COM_IX_HIMUSIC = "com.ix.hiMusic";
const std::string COM_IX_HIRADIO = "com.ix.hiRadio";
const std::string COM_IX_HISERVICE = "com.ix.hiService";
const std::string COM_IX_MUSICSERVICE = "com.ix.musicService";
const std::string COM_IX_HIDATA = "com.ix.hiData";
const std::string COM_IX_HIEXTENSION = "com.ix.hiExtension";
const std::string COM_IX_HIACCOUNT = "com.ix.hiAccount";
const std::string COM_IX_HIBACKGROUNDMUSIC = "com.ix.hiBackgroundMusic";
const std::string COM_IX_HIBACKGROUNDDATA = "com.ix.hiBackgroundData";
const std::string COM_IX_HISINGLEMUSIC = "com.ix.hiSingleMusicInfo";
const std::string COM_IX_ACCOUNTSERVICE = "com.ix.accountService";
const std::string COM_OHOS_TEST = "com.ohos.test";
constexpr int32_t MAX_SYS_UID = 2899;
const int32_t BASE_USER_RANGE = 200000;
const int32_t APPLICATIONINFO_UID = 20000000;

auto HiWordInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "Helloworld";
    abilityInfo.applicationInfo.name = "Helloworld";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = true;
    abilityInfo.applicationInfo.apiCompatibleVersion = 8;
    return true;
};

auto HiMusicInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiMusic";
    abilityInfo.applicationInfo.name = "hiMusic";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = false;

    if (elementTemp.GetAbilityName() == "MusicAbility") {
        abilityInfo.process = "p1";
        abilityInfo.launchMode = LaunchMode::STANDARD;
    }
    if (elementTemp.GetAbilityName() == "MusicTopAbility") {
        abilityInfo.process = "p1";
        abilityInfo.launchMode = LaunchMode::STANDARD;
    }
    if (elementTemp.GetAbilityName() == "MusicSAbility") {
        abilityInfo.process = "p2";
        abilityInfo.launchMode = LaunchMode::SINGLETON;
    }
    return true;
};

auto HiRadioInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiRadio";
    abilityInfo.applicationInfo.name = "hiRadio";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.process = "p3";
    if (elementTemp.GetAbilityName() == "RadioAbility") {
        abilityInfo.launchMode = LaunchMode::STANDARD;
    }
    if (elementTemp.GetAbilityName() == "RadioTopAbility") {
        abilityInfo.launchMode = LaunchMode::SINGLETON;
    }
    return true;
};

auto HiServiceInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiService";
    abilityInfo.applicationInfo.name = "hiService";
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.process = "p4";
    return true;
};

auto MusicServiceInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.uid = APPLICATIONINFO_UID;
    abilityInfo.applicationName = "musicService";
    abilityInfo.applicationInfo.name = "musicService";
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.process = "p5";
    return true;
};

auto HiDataInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiData";
    abilityInfo.applicationInfo.name = "hiData";
    abilityInfo.type = AbilityType::DATA;
    abilityInfo.process = "p6";
    return true;
};

auto HiExtensionInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiExtension";
    abilityInfo.applicationInfo.name = "hiExtension";
    abilityInfo.type = AbilityType::EXTENSION;
    return true;
};

auto HiAccountInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.uid = APPLICATIONINFO_UID;
    abilityInfo.applicationName = "AccountTest";
    abilityInfo.applicationInfo.name = "AccountTest";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = true;
    return true;
};

auto HiBAckgroundMusicInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.singleton = true;
    abilityInfo.applicationName = "hiBackgroundMusic";
    abilityInfo.applicationInfo.name = "hiBackgroundMusic";
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.process = "p4";
    return true;
};

auto HiBAckgroundDataInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.singleton = true;
    abilityInfo.applicationName = "hiBackgroundData";
    abilityInfo.applicationInfo.name = "hiBackgroundData";
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.process = "p4";
    return true;
};

auto HiSingleMusicInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.singleton = true;
    abilityInfo.applicationName = "hiSingleMusic";
    abilityInfo.applicationInfo.name = "hiSingleMusic";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.process = "p3";
    if (elementTemp.GetAbilityName() == "SingleMusicAbility") {
        abilityInfo.launchMode = LaunchMode::STANDARD;
    }
    if (elementTemp.GetAbilityName() == "SingleMusicTopAbility") {
        abilityInfo.launchMode = LaunchMode::SINGLETON;
    }
    return true;
};

auto AccountServiceInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.uid = APPLICATIONINFO_UID;
    abilityInfo.applicationName = "accountService";
    abilityInfo.applicationInfo.name = "accountService";
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.process = "p9";
    return true;
};

auto TestInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "com.ohos.test";
    abilityInfo.applicationInfo.name = "com.ohos.test";
    abilityInfo.moduleName = ".MyApplication";
    abilityInfo.package = "com.ohos.test";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = false;
    abilityInfo.kind = "page";
    abilityInfo.process = "p8";
    abilityInfo.labelId = 10001;
    abilityInfo.label = "$string:label";
    abilityInfo.descriptionId = 10002;
    abilityInfo.description = "$string:mainability_description";
    abilityInfo.iconId = 10003;
    abilityInfo.iconPath = "$media:icon";
    abilityInfo.theme = "mytheme";
    abilityInfo.readPermission = "readPermission";
    abilityInfo.writePermission = "writePermission";
    abilityInfo.resourcePath = "/data/app/com.ohos.test";
    abilityInfo.srcPath = "/resources/base/profile";
    abilityInfo.srcLanguage = "C++";
    abilityInfo.isLauncherAbility = false;
    abilityInfo.isNativeAbility = false;
    abilityInfo.enabled = false;
    abilityInfo.supportPipMode = false;
    abilityInfo.formEnabled = false;
    return true;
};
}  // namespace
class BundleMgrProxy : public IRemoteProxy<IBundleMgr> {
public:
    explicit BundleMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBundleMgr>(impl)
    {}
    virtual ~BundleMgrProxy()
    {}
    int QueryWantAbility(const AAFwk::Want &want, std::vector<AbilityInfo> &abilityInfos);

    bool QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo) override;

    bool QueryAbilityInfoByUri(const std::string &uri, AbilityInfo &abilityInfo) override;

    bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;

    bool GetBundleInfo(
        const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId) override;

    MOCK_METHOD3(GetApplicationInfos,
        bool(const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos));
    MOCK_METHOD3(GetBundleInfos, bool(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId));
    MOCK_METHOD2(GetUidByBundleName, int(const std::string &bundleName, const int userId));
    MOCK_METHOD2(GetBundleNameForUid, bool(const int uid, std::string &bundleName));
    MOCK_METHOD2(GetBundleGids, bool(const std::string &bundleName, std::vector<int> &gids));
    MOCK_METHOD1(GetAppType, std::string(const std::string &bundleName));
    MOCK_METHOD2(GetBundleInfosByMetaData, bool(const std::string &metaData, std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD1(QueryKeepAliveBundleInfos, bool(std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD3(
        GetBundleArchiveInfo, bool(const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo));
    MOCK_METHOD2(GetHapModuleInfo, bool(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo));
    MOCK_METHOD3(GetHapModuleInfo, bool(const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo));
    MOCK_METHOD2(GetLaunchWantForBundle, bool(const std::string &bundleName, Want &want));
    MOCK_METHOD2(CheckPublicKeys, int(const std::string &firstBundleName, const std::string &secondBundleName));
    MOCK_METHOD2(GetPermissionDef, bool(const std::string &permissionName, PermissionDef &permissionDef));
    MOCK_METHOD1(HasSystemCapability, bool(const std::string &capName));
    MOCK_METHOD1(GetSystemAvailableCapabilities, bool(std::vector<std::string> &systemCaps));
    MOCK_METHOD0(IsSafeMode, bool());
    MOCK_METHOD2(CleanBundleDataFiles, bool(const std::string &bundleName, const int userId));
    MOCK_METHOD1(RegisterBundleStatusCallback, bool(const sptr<IBundleStatusCallback> &bundleStatusCallback));
    MOCK_METHOD1(ClearBundleStatusCallback, bool(const sptr<IBundleStatusCallback> &bundleStatusCallback));
    MOCK_METHOD0(UnregisterBundleStatusCallback, bool());
    MOCK_METHOD4(
        DumpInfos, bool(const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result));
    MOCK_METHOD2(IsApplicationEnabled, ErrCode(const std::string &bundleName, bool &isEnable));
    MOCK_METHOD0(GetBundleInstaller, sptr<IBundleInstaller>());
    MOCK_METHOD0(GetBundleUserMgr, sptr<IBundleUserMgr>());
    MOCK_METHOD2(GetAppIdByBundleName, std::string(const std::string &bundleName, const int userId));
    MOCK_METHOD2(GetBundlesForUid, bool(const int uid, std::vector<std::string> &bundleNames));
    MOCK_METHOD2(GetNameForUid, bool(const int uid, std::string &name));
    MOCK_METHOD2(QueryAbilityInfos, bool(const Want &want, std::vector<AbilityInfo> &abilityInfos));
    MOCK_METHOD2(IsAbilityEnabled, ErrCode(const AbilityInfo &abilityInfo, bool &isEnable));
    MOCK_METHOD1(GetAllFormsInfo, bool(std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(GetFormsInfoByApp, bool(const std::string &bundleName, std::vector<FormInfo> &formInfos));
    MOCK_METHOD3(GetFormsInfoByModule,
        bool(const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(GetShortcutInfos, bool(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos));

    virtual bool ImplicitQueryInfoByPriority(const Want &want, int32_t flags, int32_t userId,
        AbilityInfo &abilityInfo, ExtensionAbilityInfo &extensionInfo) override
    {
        abilityInfo.name = "MainAbility";
        abilityInfo.bundleName = "com.ohos.launcher";
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
    BundleMgrService();
    ~BundleMgrService();

    bool QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo) override;
    bool QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo) override;
    bool QueryAbilityInfoByUri(const std::string &uri, AbilityInfo &abilityInfo) override;
    bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;
    bool GetBundleInfo(
        const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId) override;
    int GetUidByBundleName(const std::string &bundleName, const int userId) override;

    bool CheckWantEntity(const AAFwk::Want &, AbilityInfo &);

    MOCK_METHOD2(QueryWantAbility, int(const AAFwk::Want &want, std::vector<AbilityInfo> &abilityInfos));
    MOCK_METHOD3(GetApplicationInfos,
        bool(const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos));
    MOCK_METHOD2(GetBundleNameForUid, bool(const int uid, std::string &bundleName));
    MOCK_METHOD2(GetBundleGids, bool(const std::string &bundleName, std::vector<int> &gids));
    MOCK_METHOD1(GetAppType, std::string(const std::string &bundleName));
    MOCK_METHOD2(GetBundleInfosByMetaData, bool(const std::string &metaData, std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD1(QueryKeepAliveBundleInfos, bool(std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD3(
        GetBundleArchiveInfo, bool(const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo));
    MOCK_METHOD2(GetHapModuleInfo, bool(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo));
    MOCK_METHOD3(GetHapModuleInfo, bool(const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo));
    MOCK_METHOD2(GetLaunchWantForBundle, bool(const std::string &bundleName, Want &want));
    MOCK_METHOD2(CheckPublicKeys, int(const std::string &firstBundleName, const std::string &secondBundleName));
    MOCK_METHOD2(GetPermissionDef, bool(const std::string &permissionName, PermissionDef &permissionDef));
    MOCK_METHOD1(HasSystemCapability, bool(const std::string &capName));
    MOCK_METHOD1(GetSystemAvailableCapabilities, bool(std::vector<std::string> &systemCaps));
    MOCK_METHOD0(IsSafeMode, bool());
    MOCK_METHOD2(CleanBundleDataFiles, bool(const std::string &bundleName, const int userId));
    MOCK_METHOD1(RegisterBundleStatusCallback, bool(const sptr<IBundleStatusCallback> &bundleStatusCallback));
    MOCK_METHOD1(ClearBundleStatusCallback, bool(const sptr<IBundleStatusCallback> &));
    MOCK_METHOD0(GetBundleInstaller, sptr<IBundleInstaller>());
    MOCK_METHOD0(GetBundleUserMgr, sptr<IBundleUserMgr>());
    MOCK_METHOD2(GetAppIdByBundleName, std::string(const std::string &bundleName, const int userId));
    MOCK_METHOD2(GetBundlesForUid, bool(const int uid, std::vector<std::string> &bundleNames));
    MOCK_METHOD2(GetNameForUid, bool(const int uid, std::string &name));
    MOCK_METHOD2(QueryAbilityInfos, bool(const Want &want, std::vector<AbilityInfo> &abilityInfos));
    MOCK_METHOD2(IsAbilityEnabled, ErrCode(const AbilityInfo &abilityInfo, bool &isEnable));
    MOCK_METHOD1(GetAllFormsInfo, bool(std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(GetFormsInfoByApp, bool(const std::string &bundleName, std::vector<FormInfo> &formInfos));
    MOCK_METHOD3(GetFormsInfoByModule,
        bool(const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(GetShortcutInfos, bool(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos));
    MOCK_METHOD0(UnregisterBundleStatusCallback, bool());
    MOCK_METHOD4(
        DumpInfos, bool(const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result));
    MOCK_METHOD2(IsApplicationEnabled, ErrCode(const std::string &bundleName, bool &isEnable));
    virtual bool GetBundleGidsByUid(const std::string &bundleName, const int &uid, std::vector<int> &gids) override
    {
        return true;
    }
    virtual bool QueryAbilityInfosByUri(const std::string &abilityUri, std::vector<AbilityInfo> &abilityInfos) override
    {
        return true;
    }
    virtual bool GetAllCommonEventInfo(const std::string &eventKey,
        std::vector<CommonEventInfo> &commonEventInfos) override
    {
        return true;
    }
    virtual bool GetDistributedBundleInfo(const std::string &networkId, const std::string &bundleName,
        DistributedBundleInfo &distributedBundleInfo) override
    {
        return true;
    }
    virtual bool GetBundleInfos(const BundleFlag flags,
        std::vector<BundleInfo> &bundleInfos, int32_t userId = Constants::UNSPECIFIED_USERID)
    {
        return false;
    }
    virtual bool ImplicitQueryInfoByPriority(const Want &want, int32_t flags, int32_t userId,
        AbilityInfo &abilityInfo, ExtensionAbilityInfo &extensionAbilityInfo) override
    {
        abilityInfo.name = "MainAbility";
        abilityInfo.bundleName = "com.ohos.launcher";
        return true;
    }
public:
    using QueryAbilityInfoFunType =
        std::function<bool(std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp)>;
    std::map<std::string, QueryAbilityInfoFunType> abilityInfoMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_H
