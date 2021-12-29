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

#ifndef OHOS_AAFWK_ABILITY_MOCK_BUNDLE_MANAGER_H
#define OHOS_AAFWK_ABILITY_MOCK_BUNDLE_MANAGER_H

#include <vector>
#include <gmock/gmock.h>
#include "ability_info.h"
#include "application_info.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "ohos/aafwk/content/want.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "form_info.h"
#include "shortcut_info.h"
#include "common_event_info.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string COM_IX_HIWORLD = "com.ix.hiworld";
const std::string COM_IX_HIMUSIC = "com.ix.hiMusic";
const std::string COM_IX_HIRADIO = "com.ix.hiRadio";
const std::string COM_IX_HISERVICE = "com.ix.hiService";
const std::string COM_IX_MUSICSERVICE = "com.ix.musicService";
const std::string COM_IX_HIDATA = "com.ix.hiData";
const std::string SYSTEM_UI_BUNDLE_NAME = "com.ohos.systemui";
const std::string COM_IX_HIPHONE = "com.ix.hiPhone";
const std::string COM_IX_HISELECTOR = "com.ohos.selector";
const std::string COM_IX_TEST1 = "com.ix.test1";
const std::string COM_IX_TEST2 = "com.ix.test2";
const std::string COM_IX_TEST3 = "com.ix.test3";
const std::string COM_IX_TEST4 = "com.ix.test4";
constexpr int32_t MAX_SYS_UID = 2899;
constexpr int32_t ROOT_UID = 0;
constexpr int32_t HIWORLD_APP_UID = 1;
constexpr int32_t HIMUSIC_APP_UID = 2;
constexpr int32_t HIRADIO_APP_UID = 3;
constexpr int32_t HISERVICE_UID = 4;
constexpr int32_t MUSICSERVICE_UID = 5;
constexpr int32_t HIDATA_UID = 6;
constexpr int32_t SYSTEM_UI_UID = 7;
constexpr int32_t HIPHONE_UID = 8;
constexpr int32_t HISELECTOR_UID = 9;
constexpr int32_t TEST1_UID = 10;
constexpr int32_t TEST2_UID = 11;
constexpr int32_t TEST3_UID = 12;
constexpr int32_t TEST4_UID = 13;

auto HiWordInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "Helloworld";
    abilityInfo.applicationInfo.name = "Helloworld";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = true;
    abilityInfo.applicationInfo.uid = HIWORLD_APP_UID;
    abilityInfo.visible = true;
    abilityInfo.deviceId = elementTemp.GetDeviceID();
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
    abilityInfo.applicationInfo.uid = HIMUSIC_APP_UID;
    abilityInfo.visible = true;
    abilityInfo.deviceId = elementTemp.GetDeviceID();

    if (elementTemp.GetAbilityName() == "MusicAbility") {
        abilityInfo.process = "p1";
        abilityInfo.launchMode = LaunchMode::STANDARD;
    }
    if (elementTemp.GetAbilityName() == "MusicTopAbility") {
        abilityInfo.process = "p1";
        abilityInfo.launchMode = LaunchMode::SINGLETOP;
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
    abilityInfo.applicationInfo.uid = HIRADIO_APP_UID;
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.process = "p3";
    abilityInfo.visible = true;
    abilityInfo.deviceId = elementTemp.GetDeviceID();

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
    abilityInfo.applicationInfo.uid = HISERVICE_UID;
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.process = "p4";
    abilityInfo.visible = true;
    abilityInfo.deviceId = elementTemp.GetDeviceID();
    return true;
};

auto MusicServiceInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "musicService";
    abilityInfo.applicationInfo.name = "musicService";
    abilityInfo.applicationInfo.uid = MUSICSERVICE_UID;
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.process = "p5";
    abilityInfo.visible = true;
    abilityInfo.deviceId = elementTemp.GetDeviceID();
    return true;
};

auto HiDataInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiData";
    abilityInfo.applicationInfo.name = "hiData";
    abilityInfo.applicationInfo.uid = HIDATA_UID;
    abilityInfo.type = AbilityType::DATA;
    abilityInfo.process = "p6";
    abilityInfo.visible = true;
    abilityInfo.deviceId = elementTemp.GetDeviceID();
    return true;
};

auto SystemUiInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "systemui";
    abilityInfo.applicationInfo.name = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.uid = SYSTEM_UI_UID;
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.process = "p6";
    abilityInfo.visible = true;
    abilityInfo.launchMode = LaunchMode::STANDARD;
    abilityInfo.deviceId = elementTemp.GetDeviceID();
    return true;
};

auto HiPhoneInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiPhone";
    abilityInfo.applicationInfo.name = "hiPhone";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = false;
    abilityInfo.process = "p7";
    abilityInfo.visible = true;
    abilityInfo.applicationInfo.uid = HIPHONE_UID;
    if (elementTemp.GetAbilityName() == "PhoneAbility1") {
        abilityInfo.launchMode = LaunchMode::SINGLETON;
    }
    if (elementTemp.GetAbilityName() == "PhoneAbility2") {
        abilityInfo.launchMode = LaunchMode::SINGLETON;
    }
    return true;
};

auto HiSelectorInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiSelector";
    abilityInfo.applicationInfo.name = "hiSelector";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = false;
    abilityInfo.visible = true;
    abilityInfo.applicationInfo.uid = HISELECTOR_UID;
    abilityInfo.launchMode = LaunchMode::SINGLETON;
    return true;
};

auto HiTest1Info = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.name = "hiTest1";
    abilityInfo.applicationInfo.uid = TEST1_UID;
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.process = "p10";
    abilityInfo.visible = true;
    abilityInfo.deviceId = elementTemp.GetDeviceID();
    abilityInfo.launchMode = LaunchMode::STANDARD;
    return true;
};

auto HiTest2Info = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.name = "hiTest2";
    abilityInfo.applicationInfo.uid = TEST2_UID;
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.process = "p11";
    abilityInfo.visible = true;
    abilityInfo.deviceId = elementTemp.GetDeviceID();
    abilityInfo.launchMode = LaunchMode::STANDARD;
    return true;
};

auto HiTest3Info = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.name = "hiTest3";
    abilityInfo.applicationInfo.uid = TEST3_UID;
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.process = "p12";
    abilityInfo.visible = true;
    abilityInfo.deviceId = elementTemp.GetDeviceID();
    abilityInfo.launchMode = LaunchMode::STANDARD;
    return true;
};

auto HiTest4Info = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.name = "hiTest4";
    abilityInfo.applicationInfo.uid = TEST4_UID;
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.process = "p13";
    abilityInfo.visible = true;
    abilityInfo.deviceId = elementTemp.GetDeviceID();
    abilityInfo.launchMode = LaunchMode::STANDARD;
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

    bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo) override;

    virtual bool NotifyAbilityLifeStatus(const std::string &bundleName,
        const std::string &abilityName, const int64_t launchTime, const int uid) override;
    virtual bool CheckIsSystemAppByUid(const int uid) override;
    MOCK_METHOD3(GetApplicationInfos,
        bool(const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos));
    MOCK_METHOD2(GetBundleInfos, bool(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD2(GetUidByBundleName, int(const std::string &bundleName, const int userId));
    MOCK_METHOD2(GetBundleNameForUid, bool(const int uid, std::string &bundleName));
    MOCK_METHOD2(GetBundleGids, bool(const std::string &bundleName, std::vector<int> &gids));
    MOCK_METHOD1(GetAppType, std::string(const std::string &bundleName));
    MOCK_METHOD2(GetBundleInfosByMetaData, bool(const std::string &metaData, std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD1(QueryKeepAliveBundleInfos, bool(std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD2(GetAbilityLabel, std::string(const std::string &bundleName, const std::string &className));
    MOCK_METHOD3(
        GetBundleArchiveInfo, bool(const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo));
    MOCK_METHOD2(GetHapModuleInfo, bool(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo));
    MOCK_METHOD2(GetLaunchWantForBundle, bool(const std::string &bundleName, Want &want));
    MOCK_METHOD2(CheckPublicKeys, int(const std::string &firstBundleName, const std::string &secondBundleName));
    MOCK_METHOD2(CheckPermission, int(const std::string &bundleName, const std::string &permission));
    MOCK_METHOD3(CheckPermissionByUid, int(const std::string &bundleName,
        const std::string &permission, const int userId));
    MOCK_METHOD2(GetPermissionDef, bool(const std::string &permissionName, PermissionDef &permissionDef));
    MOCK_METHOD1(GetAllPermissionGroupDefs, bool(std::vector<PermissionDef> &permissionDefs));
    MOCK_METHOD2(GetAppsGrantedPermissions,
        bool(const std::vector<std::string> &permissions, std::vector<std::string> &appNames));
    MOCK_METHOD1(HasSystemCapability, bool(const std::string &capName));
    MOCK_METHOD1(GetSystemAvailableCapabilities, bool(std::vector<std::string> &systemCaps));
    MOCK_METHOD0(IsSafeMode, bool());
    MOCK_METHOD2(CleanBundleCacheFiles,
        bool(const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback));
    MOCK_METHOD2(CleanBundleDataFiles, bool(const std::string &bundleName, const int userId));
    MOCK_METHOD3(RequestPermissionFromUser,
        bool(const std::string &bundleName, const std::string &permission, const int userId));
    MOCK_METHOD1(RegisterBundleStatusCallback, bool(const sptr<IBundleStatusCallback> &bundleStatusCallback));
    MOCK_METHOD1(ClearBundleStatusCallback, bool(const sptr<IBundleStatusCallback> &bundleStatusCallback));
    MOCK_METHOD0(UnregisterBundleStatusCallback, bool());
    MOCK_METHOD3(DumpInfos, bool(const DumpFlag flag, const std::string &bundleName, std::string &result));
    MOCK_METHOD1(IsApplicationEnabled, bool(const std::string &bundleName));
    MOCK_METHOD2(SetApplicationEnabled, bool(const std::string &bundleName, bool isEnable));
    MOCK_METHOD0(GetBundleInstaller, sptr<IBundleInstaller>());
    MOCK_METHOD2(GetAppIdByBundleName, std::string(const std::string &bundleName, const int userId));
    MOCK_METHOD2(GetBundlesForUid, bool(const int uid, std::vector<std::string> &bundleNames));
    MOCK_METHOD2(GetNameForUid, bool(const int uid, std::string &name));
    MOCK_METHOD2(QueryAbilityInfos, bool(const Want &want, std::vector<AbilityInfo> &abilityInfos));
    MOCK_METHOD2(QueryAbilityInfosForClone, bool(const Want &want, std::vector<AbilityInfo> &abilityInfos));
    MOCK_METHOD1(IsAbilityEnabled, bool(const AbilityInfo &abilityInfo));
    MOCK_METHOD2(SetAbilityEnabled, bool(const AbilityInfo &abilityInfo, bool isEnabled));
    MOCK_METHOD2(GetAbilityIcon, std::string(const std::string &bundleName, const std::string &className));
    MOCK_METHOD3(
        CanRequestPermission, bool(const std::string &bundleName, const std::string &permissionName, const int userId));
    MOCK_METHOD1(RegisterAllPermissionsChanged, bool(const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD2(RegisterPermissionsChanged,
        bool(const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD1(UnregisterPermissionsChanged, bool(const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD1(GetAllFormsInfo, bool(std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(GetFormsInfoByApp, bool(const std::string &bundleName, std::vector<FormInfo> &formInfos));
    MOCK_METHOD3(GetFormsInfoByModule,
        bool(const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(GetShortcutInfos, bool(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos));
    MOCK_METHOD1(CheckBundleNameInAllowList, bool(const std::string &bundleName));
    MOCK_METHOD3(GetBundleGidsByUid, bool(const std::string &bundleName, const int &uid, std::vector<int> &gids));
    MOCK_METHOD2(GetAllCommonEventInfo, bool(const std::string &eventKey,
        std::vector<CommonEventInfo> &commonEventInfos));
};

class BundleMgrStub : public IRemoteStub<IBundleMgr> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IBundleMgr");
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
};

class BundleMgrService : public BundleMgrStub {
public:
    BundleMgrService();
    ~BundleMgrService();

    bool QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo) override;
    bool QueryAbilityInfoByUri(const std::string &uri, AbilityInfo &abilityInfo) override;
    bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;
    bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo) override;
    int GetUidByBundleName(const std::string &bundleName, const int userId) override;
    virtual bool CheckIsSystemAppByUid(const int uid) override;
    bool CheckWantEntity(const AAFwk::Want &, AbilityInfo &);
    virtual bool NotifyAbilityLifeStatus(const std::string &bundleName,
        const std::string &abilityName, const int64_t launchTime, const int uid) override;
    bool QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos) override;
    virtual bool QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfos) override;
    virtual bool GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos) override;
    virtual bool QueryAbilityInfosByUri(const std::string &abilityUri, std::vector<AbilityInfo> &abilityInfos) override;
    virtual bool CheckBundleNameInAllowList(const std::string &bundleName);
    MOCK_METHOD2(QueryWantAbility, int(const AAFwk::Want &want, std::vector<AbilityInfo> &abilityInfos));
    MOCK_METHOD3(GetApplicationInfos,
        bool(const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos));
    MOCK_METHOD2(GetBundleNameForUid, bool(const int uid, std::string &bundleName));
    MOCK_METHOD2(GetBundleGids, bool(const std::string &bundleName, std::vector<int> &gids));
    MOCK_METHOD1(GetAppType, std::string(const std::string &bundleName));
    MOCK_METHOD2(GetBundleInfosByMetaData, bool(const std::string &metaData, std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD1(QueryKeepAliveBundleInfos, bool(std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD2(GetAbilityLabel, std::string(const std::string &bundleName, const std::string &className));
    MOCK_METHOD3(
        GetBundleArchiveInfo, bool(const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo));
    MOCK_METHOD2(GetHapModuleInfo, bool(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo));
    MOCK_METHOD2(GetLaunchWantForBundle, bool(const std::string &bundleName, Want &want));
    MOCK_METHOD2(CheckPublicKeys, int(const std::string &firstBundleName, const std::string &secondBundleName));
    MOCK_METHOD2(CheckPermission, int(const std::string &bundleName, const std::string &permission));
    MOCK_METHOD3(CheckPermissionByUid, int(const std::string &bundleName,
        const std::string &permission, const int userId));
    MOCK_METHOD2(GetPermissionDef, bool(const std::string &permissionName, PermissionDef &permissionDef));
    MOCK_METHOD1(GetAllPermissionGroupDefs, bool(std::vector<PermissionDef> &permissionDefs));
    MOCK_METHOD2(GetAppsGrantedPermissions,
        bool(const std::vector<std::string> &permissions, std::vector<std::string> &appNames));
    MOCK_METHOD1(HasSystemCapability, bool(const std::string &capName));
    MOCK_METHOD1(GetSystemAvailableCapabilities, bool(std::vector<std::string> &systemCaps));
    MOCK_METHOD0(IsSafeMode, bool());
    MOCK_METHOD2(CleanBundleCacheFiles,
        bool(const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback));
    MOCK_METHOD2(CleanBundleDataFiles, bool(const std::string &bundleName, const int userId));
    MOCK_METHOD3(RequestPermissionFromUser,
        bool(const std::string &bundleName, const std::string &permission, const int userId));
    MOCK_METHOD1(RegisterBundleStatusCallback, bool(const sptr<IBundleStatusCallback> &bundleStatusCallback));
    MOCK_METHOD1(ClearBundleStatusCallback, bool(const sptr<IBundleStatusCallback> &));
    MOCK_METHOD2(SetApplicationEnabled, bool(const std::string &bundleName, bool isEnable));
    MOCK_METHOD0(GetBundleInstaller, sptr<IBundleInstaller>());
    MOCK_METHOD2(GetAppIdByBundleName, std::string(const std::string &bundleName, const int userId));
    MOCK_METHOD2(GetBundlesForUid, bool(const int uid, std::vector<std::string> &bundleNames));
    MOCK_METHOD2(GetNameForUid, bool(const int uid, std::string &name));
    MOCK_METHOD1(IsAbilityEnabled, bool(const AbilityInfo &abilityInfo));
    MOCK_METHOD2(SetAbilityEnabled, bool(const AbilityInfo &abilityInfo, bool isEnabled));
    MOCK_METHOD2(GetAbilityIcon, std::string(const std::string &bundleName, const std::string &className));
    MOCK_METHOD3(
        CanRequestPermission, bool(const std::string &bundleName, const std::string &permissionName, const int userId));
    MOCK_METHOD1(RegisterAllPermissionsChanged, bool(const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD2(RegisterPermissionsChanged,
        bool(const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD1(UnregisterPermissionsChanged, bool(const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD2(GetModuleUsageRecords, bool(const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords));
    MOCK_METHOD1(GetAllFormsInfo, bool(std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(GetFormsInfoByApp, bool(const std::string &bundleName, std::vector<FormInfo> &formInfos));
    MOCK_METHOD3(GetFormsInfoByModule,
        bool(const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(GetShortcutInfos, bool(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos));
    MOCK_METHOD2(GetAllCommonEventInfo, bool(const std::string &eventKey,
        std::vector<CommonEventInfo> &commonEventInfos));
    MOCK_METHOD0(UnregisterBundleStatusCallback, bool());
    MOCK_METHOD3(DumpInfos, bool(const DumpFlag flag, const std::string &bundleName, std::string &result));
    MOCK_METHOD1(IsApplicationEnabled, bool(const std::string &bundleName));
    MOCK_METHOD2(RemoveClonedBundle, bool(const std::string &bundleName, const int32_t uid));
    MOCK_METHOD1(BundleClone, bool(const std::string &bundleName));
    MOCK_METHOD3(GetBundleGidsByUid, bool(const std::string &bundleName, const int &uid, std::vector<int> &gids));
public:
    using QueryAbilityInfoFunType =
        std::function<bool(std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp)>;
    std::map<std::string, QueryAbilityInfoFunType> abilityInfoMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_ABILITY_MOCK_BUNDLE_MANAGER_H
