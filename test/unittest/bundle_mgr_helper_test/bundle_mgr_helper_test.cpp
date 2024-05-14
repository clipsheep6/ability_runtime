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

#include <gtest/gtest.h>

#define private public
#include "bundle_mgr_helper.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t ABILITY_INFO_FLAG = 4;
const int32_t DEFAULT_USERID = 100;
const int32_t FIRST_APP_INDEX = 1000;
const int32_t SECOND_APP_INDEX = 2000;
const int32_t ERR_COD1 = 8519801;
const int32_t ERR_COD3 = 8519802;
const int32_t ERR_COD4 = 8519921;
const int32_t ERR_COD5 = 8519816;
const int32_t ERR_COD7 = 8521219;
} // namespace

class BundleMgrHelperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static std::shared_ptr<BundleMgrHelper> bundleMgrHelper;
};

std::shared_ptr<BundleMgrHelper> BundleMgrHelperTest::bundleMgrHelper =
    DelayedSingleton<BundleMgrHelper>::GetInstance();

void BundleMgrHelperTest::SetUpTestCase(void)
{}

void BundleMgrHelperTest::TearDownTestCase(void)
{}

void BundleMgrHelperTest::SetUp()
{}

void BundleMgrHelperTest::TearDown()
{}

/**
 * @tc.name: BundleMgrHelperTest_GetBundleInfo_001
 * @tc.desc: GetBundleInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetBundleInfo_001, TestSize.Level1)
{
    std::string bundleName = "ohos.global.systemres";
    int32_t flags = 0;
    BundleInfo bundleInfo;
    int32_t userId = DEFAULT_USERID;
    auto ret = bundleMgrHelper->GetBundleInfo(bundleName, flags, bundleInfo, userId);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: BundleMgrHelperTest_GetHapModuleInfo_001
 * @tc.desc: GetHapModuleInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetHapModuleInfo_001, TestSize.Level1)
{
    AbilityInfo abilityInfo;
    HapModuleInfo hapModuleInfo;
    auto ret = bundleMgrHelper->GetHapModuleInfo(abilityInfo, hapModuleInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_GetAbilityLabel_001
 * @tc.desc: GetAbilityLabel
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetAbilityLabel_001, TestSize.Level1)
{
    std::string bundleName;
    std::string abilityName;
    auto ret = bundleMgrHelper->GetAbilityLabel(bundleName, abilityName);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: BundleMgrHelperTest_GetAppType_001
 * @tc.desc: GetAppType
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetAppType_001, TestSize.Level1)
{
    std::string bundleName;
    auto ret = bundleMgrHelper->GetAppType(bundleName);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: BundleMgrHelperTest_GetBaseSharedBundleInfos_001
 * @tc.desc: GetBaseSharedBundleInfos
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetBaseSharedBundleInfos_001, TestSize.Level1)
{
    std::string bundleName;
    std::vector<BaseSharedBundleInfo> baseSharedBundleInfos;
    auto ret = bundleMgrHelper->GetBaseSharedBundleInfos(bundleName, baseSharedBundleInfos);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: BundleMgrHelperTest_GetBundleInfoForSelf_001
 * @tc.desc: GetBundleInfoForSelf
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetBundleInfoForSelf_001, TestSize.Level1)
{
    int32_t flags = 0;
    BundleInfo bundleInfo;
    auto ret = bundleMgrHelper->GetBundleInfoForSelf(flags, bundleInfo);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: BundleMgrHelperTest_GetDependentBundleInfo_001
 * @tc.desc: GetDependentBundleInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetDependentBundleInfo_001, TestSize.Level1)
{
    std::string sharedBundleName;
    BundleInfo sharedBundleInfo;
    auto ret = bundleMgrHelper->GetDependentBundleInfo(sharedBundleName, sharedBundleInfo);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: BundleMgrHelperTest_GetGroupDir_001
 * @tc.desc: GetGroupDir
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetGroupDir_001, TestSize.Level1)
{
    std::string dataGroupId;
    std::string dir;
    auto ret = bundleMgrHelper->GetGroupDir(dataGroupId, dir);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_GetOverlayManagerProxy_001
 * @tc.desc: GetOverlayManagerProxy
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetOverlayManagerProxy_001, TestSize.Level1)
{
    auto ret = bundleMgrHelper->GetOverlayManagerProxy();
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: BundleMgrHelperTest_QueryAbilityInfo_002
 * @tc.desc: QueryAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_QueryAbilityInfo_002, TestSize.Level1)
{
    Want want;
    AbilityInfo abilityInfo;
    auto ret = bundleMgrHelper->QueryAbilityInfo(want, abilityInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_QueryAbilityInfo_001
 * @tc.desc: QueryAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_QueryAbilityInfo_001, TestSize.Level1)
{
    Want want;
    int32_t flags = 0;
    int32_t userId = DEFAULT_USERID;
    AbilityInfo abilityInfo;
    const sptr<IRemoteObject> callBack = nullptr;
    auto ret = bundleMgrHelper->QueryAbilityInfo(want, flags, userId, abilityInfo, callBack);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_GetBundleInfos_001
 * @tc.desc: GetBundleInfos
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetBundleInfos_001, TestSize.Level1)
{
    BundleFlag flag = BundleFlag::GET_BUNDLE_WITH_ABILITIES;
    std::vector<BundleInfo> bundleInfos;
    int32_t userId = DEFAULT_USERID;
    auto ret = bundleMgrHelper->GetBundleInfos(flag, bundleInfos, userId);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: BundleMgrHelperTest_ImplicitQueryInfos_001
 * @tc.desc: ImplicitQueryInfos
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_ImplicitQueryInfos_001, TestSize.Level1)
{
    Want want;
    int32_t flags = 0;
    int32_t userId = DEFAULT_USERID;
    bool withDefault = false;
    std::vector<AbilityInfo> abilityInfos;
    std::vector<ExtensionAbilityInfo> extensionInfos;
    auto ret = bundleMgrHelper->ImplicitQueryInfos(want, flags, userId, withDefault, abilityInfos, extensionInfos);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_CleanBundleDataFiles_001
 * @tc.desc: CleanBundleDataFiles
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_CleanBundleDataFiles_001, TestSize.Level1)
{
    std::string bundleName;
    int32_t userId = DEFAULT_USERID;
    auto ret = bundleMgrHelper->CleanBundleDataFiles(bundleName, userId);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_QueryDataGroupInfos_001
 * @tc.desc: QueryDataGroupInfos
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_QueryDataGroupInfos_001, TestSize.Level1)
{
    std::string bundleName;
    int32_t userId = DEFAULT_USERID;
    std::vector<DataGroupInfo> infos;
    auto ret = bundleMgrHelper->QueryDataGroupInfos(bundleName, userId, infos);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_GetBundleGidsByUid_001
 * @tc.desc: GetBundleGidsByUid
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetBundleGidsByUid_001, TestSize.Level1)
{
    std::string bundleName;
    int32_t uid = DEFAULT_USERID;
    std::vector<int> gids;
    auto ret = bundleMgrHelper->GetBundleGidsByUid(bundleName, uid, gids);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: BundleMgrHelperTest_RegisterBundleEventCallback_001
 * @tc.desc: RegisterBundleEventCallback
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_RegisterBundleEventCallback_001, TestSize.Level1)
{
    const sptr<IBundleEventCallback> bundleEventCallback = nullptr;
    auto ret = bundleMgrHelper->RegisterBundleEventCallback(bundleEventCallback);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_GetQuickFixManagerProxy_001
 * @tc.desc: GetQuickFixManagerProxy
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetQuickFixManagerProxy_001, TestSize.Level1)
{
    auto ret = bundleMgrHelper->GetQuickFixManagerProxy();
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: BundleMgrHelperTest_QueryExtensionAbilityInfos_002
 * @tc.desc: QueryExtensionAbilityInfos
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_QueryExtensionAbilityInfos_002, TestSize.Level1)
{
    Want want;
    int32_t flag = 0;
    int32_t userId = DEFAULT_USERID;
    std::vector<ExtensionAbilityInfo> extensionInfos;
    auto ret = bundleMgrHelper->QueryExtensionAbilityInfos(want, flag, userId, extensionInfos);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_GetAppControlProxy_001
 * @tc.desc: GetAppControlProxy
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetAppControlProxy_001, TestSize.Level1)
{
    auto ret = bundleMgrHelper->GetAppControlProxy();
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: BundleMgrHelperTest_QueryExtensionAbilityInfos_001
 * @tc.desc: QueryExtensionAbilityInfos
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_QueryExtensionAbilityInfos_001, TestSize.Level1)
{
    Want want;
    int32_t flag = 0;
    int32_t userId = DEFAULT_USERID;
    std::vector<ExtensionAbilityInfo> extensionInfos;
    auto ret = bundleMgrHelper->QueryExtensionAbilityInfos(want, flag, userId, extensionInfos);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_GetApplicationInfo_001
 * @tc.desc: GetApplicationInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetApplicationInfo_001, TestSize.Level1)
{
    std::string appName;
    ApplicationFlag flag = ApplicationFlag::GET_ALL_APPLICATION_INFO;
    int userId = DEFAULT_USERID;
    ApplicationInfo appInfo;
    auto ret = bundleMgrHelper->GetApplicationInfo(appName, flag, userId, appInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_UnregisterBundleEventCallback_001
 * @tc.desc: UnregisterBundleEventCallback
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_UnregisterBundleEventCallback_001, TestSize.Level1)
{
    sptr<IBundleEventCallback> bundleEventCallback = nullptr;
    auto ret = bundleMgrHelper->UnregisterBundleEventCallback(bundleEventCallback);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_QueryExtensionAbilityInfoByUri_001
 * @tc.desc: QueryExtensionAbilityInfoByUri
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_QueryExtensionAbilityInfoByUri_001, TestSize.Level1)
{
    std::string uri;
    int32_t userId = DEFAULT_USERID;
    ExtensionAbilityInfo extensionAbilityInfo;
    auto ret = bundleMgrHelper->QueryExtensionAbilityInfoByUri(uri, userId, extensionAbilityInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_QueryAbilityInfoByUri_001
 * @tc.desc: QueryAbilityInfoByUri
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_QueryAbilityInfoByUri_001, TestSize.Level1)
{
    std::string abilityUri;
    int32_t userId = DEFAULT_USERID;
    AbilityInfo abilityInfo;
    auto ret = bundleMgrHelper->QueryAbilityInfoByUri(abilityUri, userId, abilityInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_ImplicitQueryInfoByPriority_001
 * @tc.desc: ImplicitQueryInfoByPriority
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_ImplicitQueryInfoByPriority_001, TestSize.Level1)
{
    Want want;
    int32_t flags = 0;
    int32_t userId = DEFAULT_USERID;
    AbilityInfo abilityInfo;
    ExtensionAbilityInfo extensionInfo;
    auto ret = bundleMgrHelper->ImplicitQueryInfoByPriority(want, flags, userId, abilityInfo, extensionInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_GetBundleInfos_002
 * @tc.desc: GetBundleInfos
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetBundleInfos_002, TestSize.Level1)
{
    int32_t flags = 0;
    std::vector<BundleInfo> bundleInfos;
    int32_t userId = DEFAULT_USERID;
    auto ret = bundleMgrHelper->GetBundleInfos(flags, bundleInfos, userId);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: BundleMgrHelperTest_GetHapModuleInfo_002
 * @tc.desc: GetHapModuleInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetHapModuleInfo_002, TestSize.Level1)
{
    AbilityInfo abilityInfo;
    int32_t userId = DEFAULT_USERID;
    HapModuleInfo hapModuleInfo;
    auto ret = bundleMgrHelper->GetHapModuleInfo(abilityInfo, userId, hapModuleInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_GetUidByBundleName_001
 * @tc.desc: GetUidByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetUidByBundleName_001, TestSize.Level1)
{
    std::string bundleName;
    int32_t userId = DEFAULT_USERID;
    auto ret = bundleMgrHelper->GetUidByBundleName(bundleName, userId);
    EXPECT_EQ(ret, Constants::INVALID_UID);
}

/**
 * @tc.name: BundleMgrHelperTest_GetApplicationInfo_002
 * @tc.desc: GetApplicationInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetApplicationInfo_002, TestSize.Level1)
{
    std::string appName;
    int32_t flags = 0;
    int32_t userId = DEFAULT_USERID;
    ApplicationInfo appInfo;
    auto ret = bundleMgrHelper->GetApplicationInfo(appName, flags, userId, appInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_ProcessPreload_001
 * @tc.desc: ProcessPreload
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_ProcessPreload_001, TestSize.Level1)
{
    Want want;
    auto ret = bundleMgrHelper->ProcessPreload(want);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_UpgradeAtomicService_001
 * @tc.desc: UpgradeAtomicService
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_UpgradeAtomicService_001, TestSize.Level1)
{
    Want want;
    int32_t userId = DEFAULT_USERID;
    bundleMgrHelper->UpgradeAtomicService(want, userId);
    EXPECT_NE(bundleMgrHelper->bundleMgr_, nullptr);
}

/**
 * @tc.name: BundleMgrHelperTest_QueryAbilityInfo_003
 * @tc.desc: QueryAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_QueryAbilityInfo_003, TestSize.Level1)
{
    Want want;
    int32_t flags = DEFAULT_USERID;
    int32_t userId = DEFAULT_USERID;
    AbilityInfo abilityInfo;
    auto ret = bundleMgrHelper->QueryAbilityInfo(want, flags, userId, abilityInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperTest_GetDefaultAppProxy_001
 * @tc.desc: GetDefaultAppProxy
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetDefaultAppProxy_001, TestSize.Level1)
{
    auto ret = bundleMgrHelper->GetDefaultAppProxy();
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: BundleMgrHelperTest_InstallSandboxApp_001
 * @tc.desc: InstallSandboxApp
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_InstallSandboxApp_001, TestSize.Level1)
{
    std::string bundleName = "";
    int32_t dlpType = 1;
    int32_t userId = 1;
    int32_t appIndex = 1;
    auto ret = bundleMgrHelper->InstallSandboxApp(bundleName, dlpType, userId, appIndex);
    EXPECT_EQ(ret, ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR);
}

/**
 * @tc.name: BundleMgrHelperTest_InstallSandboxApp_002
 * @tc.desc: InstallSandboxApp
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_InstallSandboxApp_002, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    int32_t dlpType = 1;
    int32_t userId = 1;
    int32_t appIndex = 1;
    auto ret = bundleMgrHelper->InstallSandboxApp(bundleName, dlpType, userId, appIndex);
    EXPECT_EQ(ret, ERR_COD1);
}

/**
 * @tc.name: BundleMgrHelperTest_UninstallSandboxApp_001
 * @tc.desc: UninstallSandboxApp
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_UninstallSandboxApp_001, TestSize.Level1)
{
    std::string bundleName = "";
    int32_t userId = 1;
    int32_t appIndex = 1;
    auto ret = bundleMgrHelper->UninstallSandboxApp(bundleName, userId, appIndex);
    EXPECT_EQ(ret, ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR);
}

/**
 * @tc.name: BundleMgrHelperTest_UninstallSandboxApp_002
 * @tc.desc: UninstallSandboxApp
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_UninstallSandboxApp_002, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    int32_t appIndex = 1;
    int32_t userId = 1;
    auto ret = bundleMgrHelper->UninstallSandboxApp(bundleName, appIndex, userId);
    EXPECT_EQ(ret, ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR);
}

/**
 * @tc.name: BundleMgrHelperTest_UninstallSandboxApp_003
 * @tc.desc: UninstallSandboxApp
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_UninstallSandboxApp_003, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    int32_t userId = 1;
    int32_t appIndex = -1;
    auto ret = bundleMgrHelper->UninstallSandboxApp(bundleName, userId, appIndex);
    EXPECT_EQ(ret, ERR_COD3);
}

/**
 * @tc.name: BundleMgrHelperTest_GetUninstalledBundleInfo_001
 * @tc.desc: GetUninstalledBundleInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetUninstalledBundleInfo_001, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    BundleInfo bundleInfo;
    auto ret = bundleMgrHelper->GetUninstalledBundleInfo(bundleName, bundleInfo);
    EXPECT_EQ(ret, ERR_COD4);
}

/**
 * @tc.name: BundleMgrHelperTest_GetSandboxBundleInfo_001
 * @tc.desc: GetSandboxBundleInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetSandboxBundleInfo_001, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    int32_t appIndex = -1;
    int32_t userId = 1;
    BundleInfo bundleInfo;
    auto ret = bundleMgrHelper->GetSandboxBundleInfo(bundleName, appIndex, userId, bundleInfo);
    EXPECT_EQ(ret, ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR);
}

/**
 * @tc.name: BundleMgrHelperTest_GetSandboxBundleInfo_002
 * @tc.desc: GetSandboxBundleInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetSandboxBundleInfo_002, TestSize.Level1)
{
    std::string bundleName = "";
    int32_t appIndex = 10;
    int32_t userId = 1;
    BundleInfo bundleInfo;
    auto ret = bundleMgrHelper->GetSandboxBundleInfo(bundleName, appIndex, userId, bundleInfo);
    EXPECT_EQ(ret, ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR);
}

/**
 * @tc.name: BundleMgrHelperTest_GetSandboxAbilityInfo_001
 * @tc.desc: GetSandboxAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetSandboxAbilityInfo_001, TestSize.Level1)
{
    Want want;
    int32_t appIndex = 0;
    int32_t flags = 1;
    int32_t userId = 1;
    AbilityInfo abilityInfo;
    auto ret = bundleMgrHelper->GetSandboxAbilityInfo(want, appIndex, flags, userId, abilityInfo);
    EXPECT_EQ(ret, ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR);
}

/**
 * @tc.name: BundleMgrHelperTest_GetSandboxAbilityInfo_002
 * @tc.desc: GetSandboxAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetSandboxAbilityInfo_002, TestSize.Level1)
{
    Want want;
    int32_t appIndex = FIRST_APP_INDEX;
    int32_t flags = 1;
    int32_t userId = 1;
    AbilityInfo abilityInfo;
    auto ret = bundleMgrHelper->GetSandboxAbilityInfo(want, appIndex, flags, userId, abilityInfo);
    EXPECT_EQ(ret, ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR);
}

/**
 * @tc.name: BundleMgrHelperTest_GetSandboxAbilityInfo_003
 * @tc.desc: GetSandboxAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetSandboxAbilityInfo_003, TestSize.Level1)
{
    Want want;
    int32_t appIndex = SECOND_APP_INDEX;
    int32_t flags = 1;
    int32_t userId = 1;
    AbilityInfo abilityInfo;
    auto ret = bundleMgrHelper->GetSandboxAbilityInfo(want, appIndex, flags, userId, abilityInfo);
    EXPECT_EQ(ret, ERR_COD5);
}

/**
 * @tc.name: BundleMgrHelperTest_GetSandboxExtAbilityInfos_001
 * @tc.desc: GetSandboxExtAbilityInfos
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetSandboxExtAbilityInfos_001, TestSize.Level1)
{
    Want want;
    int32_t appIndex = SECOND_APP_INDEX;
    int32_t flags = 1;
    int32_t userId = 1;
    std::vector<ExtensionAbilityInfo> extensionInfos;
    auto ret = bundleMgrHelper->GetSandboxExtAbilityInfos(want, appIndex, flags, userId, extensionInfos);
    EXPECT_EQ(ret, ERR_COD5);
}

/**
 * @tc.name: BundleMgrHelperTest_GetSandboxHapModuleInfo_001
 * @tc.desc: GetSandboxHapModuleInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetSandboxHapModuleInfo_001, TestSize.Level1)
{
    AbilityInfo abilityInfo;
    int32_t appIndex = 0;
    int32_t userId = 1;
    HapModuleInfo hapModuleInfo;
    auto ret = bundleMgrHelper->GetSandboxHapModuleInfo(abilityInfo, appIndex, userId, hapModuleInfo);
    EXPECT_EQ(ret, ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR);
}

/**
 * @tc.name: BundleMgrHelperTest_GetSandboxHapModuleInfo_002
 * @tc.desc: GetSandboxHapModuleInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetSandboxHapModuleInfo_002, TestSize.Level1)
{
    AbilityInfo abilityInfo;
    int32_t appIndex = FIRST_APP_INDEX;
    int32_t userId = 1;
    HapModuleInfo hapModuleInfo;
    auto ret = bundleMgrHelper->GetSandboxHapModuleInfo(abilityInfo, appIndex, userId, hapModuleInfo);
    EXPECT_EQ(ret, ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR);
}

/**
 * @tc.name: BundleMgrHelperTest_GetSandboxHapModuleInfo_003
 * @tc.desc: GetSandboxHapModuleInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetSandboxHapModuleInfo_003, TestSize.Level1)
{
    AbilityInfo abilityInfo;
    int32_t appIndex = SECOND_APP_INDEX;
    int32_t userId = 1;
    HapModuleInfo hapModuleInfo;
    auto ret = bundleMgrHelper->GetSandboxHapModuleInfo(abilityInfo, appIndex, userId, hapModuleInfo);
    EXPECT_EQ(ret, ERR_COD5);
}

/**
 * @tc.name: BundleMgrHelperTest_ConnectBundleInstaller_001
 * @tc.desc: ConnectBundleInstaller
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_ConnectBundleInstaller_001, TestSize.Level1)
{
    bundleMgrHelper->OnDeath();
    auto ret = bundleMgrHelper->ConnectBundleInstaller();
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: BundleMgrHelperTest_GetBundleInfoV9_001
 * @tc.desc: GetBundleInfoV9
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetBundleInfoV9_001, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    int32_t flags = 1;
    BundleInfo bundleInfo;
    int32_t userId = 1;
    auto ret = bundleMgrHelper->GetBundleInfoV9(bundleName, flags, bundleInfo, userId);
    EXPECT_EQ(ret, ERR_COD7);
}

/**
 * @tc.name: QueryExtensionAbilityInfosOnlyWithTypeName_001
 * @tc.desc: QueryExtensionAbilityInfosOnlyWithTypeName
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, QueryExtensionAbilityInfosOnlyWithTypeName_001, TestSize.Level1)
{
    std::string extensionTypeName = "extensionTypeName";
    uint32_t flag = 1;
    int32_t userId = 1;
    std::vector<ExtensionAbilityInfo> extensionInfos;
    auto ret = bundleMgrHelper->QueryExtensionAbilityInfosOnlyWithTypeName(extensionTypeName,
        flag, userId, extensionInfos);
    EXPECT_EQ(ret, ERR_COD7);
}

/**
 * @tc.name: GetJsonProfile_001
 * @tc.desc: GetJsonProfile
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, GetJsonProfile_001, TestSize.Level1)
{
    ProfileType profileType = AppExecFwk::PKG_CONTEXT_PROFILE;
    std::string bundleName = "bundleName";
    std::string moduleName = "moduleName";
    std::string profile = "profile";
    int32_t userId = 1;
    auto ret = bundleMgrHelper->GetJsonProfile(profileType, bundleName, moduleName, profile, userId);
    EXPECT_EQ(ret, ERR_COD7);
}

/**
 * @tc.name: BundleMgrHelperTest_QueryCloneAbilityInfo_001
 * @tc.desc: QueryCloneAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_QueryCloneAbilityInfo_001, TestSize.Level1)
{
    ElementName element;
    AbilityInfo abilityInfo;
    int32_t flags = ABILITY_INFO_FLAG;
    int32_t appTwinIndex = 1;
    int32_t userId = DEFAULT_USERID;
    auto ret = bundleMgrHelper->QueryCloneAbilityInfo(element, flags, appTwinIndex, abilityInfo, userId);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: BundleMgrHelperTest_GetCloneBundleInfo_001
 * @tc.desc: GetCloneBundleInfo
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperTest, BundleMgrHelperTest_GetCloneBundleInfo_001, TestSize.Level1)
{
    std::string bundleName;
    BundleInfo bundleInfo;
    int32_t flags = 1;
    int32_t appTwinIndex = 1;
    int32_t userId = DEFAULT_USERID;
    auto ret = bundleMgrHelper->GetCloneBundleInfo(bundleName, flags, appTwinIndex, bundleInfo, userId);
    EXPECT_NE(ret, ERR_OK);
}
}  // namespace AppExecFwk
}  // namespace OHOS