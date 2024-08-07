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
#define protected public
#include "ability_auto_startup_service.h"
#include "ability_manager_errors.h"
#include "distributed_kv_data_manager.h"
#include "mock_my_flag.h"
#include "mock_permission_verification.h"
#include "mock_sa_call.h"
#include "nativetoken_kit.h"
#include "parameters.h"
#include "token_setproc.h"
#undef private
#undef protected

namespace {
const char *perms[] = { "ohos.permission.MANAGE_APP_BOOT_INTERNAL" };
const std::string AUTO_STARTUP_SERVICE_EMPTY = "";
const std::string AUTO_STARTUP_SERVICE_BUNDLENAME = "bundleName";
const std::string AUTO_STARTUP_SERVICE_ABILITYNAME = "abilityName";
const bool AUTO_STARTUP_SERVICE_TRUE = true;
const bool AUTO_STARTUP_SERVICE_FALSE = false;
} // namespace

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace AAFwk {
class AbilityAutoStartupServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    static void SetNativeToken()
    {
        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = static_cast<int32_t>(sizeof(perms) / sizeof(perms[0])),
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };

        infoInstance.processName = "SetUpTestCase";
        auto tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
    }
    void SetUp();
    void TearDown();
};

void AbilityAutoStartupServiceTest::SetUpTestCase() {}

void AbilityAutoStartupServiceTest::TearDownTestCase() {}

void AbilityAutoStartupServiceTest::SetUp() {}

void AbilityAutoStartupServiceTest::TearDown() {}

/*
 * Feature: AbilityAutoStartupService
 * Function: RegisterAutoStartupSystemCallback
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService RegisterAutoStartupSystemCallback
 */
HWTEST_F(AbilityAutoStartupServiceTest, RegisterAutoStartupSystemCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest RegisterAutoStartupSystemCallback_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    sptr<IRemoteObject> callback;
    auto result = abilityAutoStartupService->RegisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest RegisterAutoStartupSystemCallback_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: UnregisterAutoStartupSystemCallback
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService UnregisterAutoStartupSystemCallback
 */
HWTEST_F(AbilityAutoStartupServiceTest, UnregisterAutoStartupSystemCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest UnregisterAutoStartupSystemCallback_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    sptr<IRemoteObject> callback;
    auto result = abilityAutoStartupService->UnregisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest UnregisterAutoStartupSystemCallback_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceTest, SetApplicationAutoStartup_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SetApplicationAutoStartup_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AutoStartupInfo info;
    auto result = abilityAutoStartupService->SetApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SetApplicationAutoStartup_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerSetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerSetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceTest, InnerSetApplicationAutoStartup_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerSetApplicationAutoStartup_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AutoStartupInfo info;
    info.abilityName = AUTO_STARTUP_SERVICE_EMPTY;
    auto result = abilityAutoStartupService->InnerSetApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerSetApplicationAutoStartup_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerSetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerSetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceTest, InnerSetApplicationAutoStartup_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerSetApplicationAutoStartup_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AutoStartupInfo info;
    info.abilityName = AUTO_STARTUP_SERVICE_ABILITYNAME;
    info.bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    info.accessTokenId = "123";
    info.userId = 100;
    auto result = abilityAutoStartupService->InnerSetApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerSetApplicationAutoStartup_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CancelApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceTest, CancelApplicationAutoStartup_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CancelApplicationAutoStartup start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AutoStartupInfo info;
    auto result = abilityAutoStartupService->CancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CancelApplicationAutoStartup end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerCancelApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerCancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceTest, InnerCancelApplicationAutoStartup_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerCancelApplicationAutoStartup_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AutoStartupInfo info;
    info.abilityName = AUTO_STARTUP_SERVICE_EMPTY;
    auto result = abilityAutoStartupService->InnerCancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerCancelApplicationAutoStartup_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerCancelApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerCancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceTest, InnerCancelApplicationAutoStartup_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerCancelApplicationAutoStartup_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AutoStartupInfo info;
    info.abilityName = AUTO_STARTUP_SERVICE_ABILITYNAME;
    info.bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    info.accessTokenId = "123";
    info.userId = 100;
    auto result = abilityAutoStartupService->InnerCancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerCancelApplicationAutoStartup_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: QueryAllAutoStartupApplications
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService QueryAllAutoStartupApplications
 */
HWTEST_F(AbilityAutoStartupServiceTest, QueryAllAutoStartupApplications_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest QueryAllAutoStartupApplications_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    std::vector<AutoStartupInfo> infoList;
    int32_t userId = 100;
    auto result = abilityAutoStartupService->QueryAllAutoStartupApplications(infoList, userId);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest QueryAllAutoStartupApplications_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: QueryAllAutoStartupApplicationsWithoutPermission
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService QueryAllAutoStartupApplicationsWithoutPermission
 */
HWTEST_F(AbilityAutoStartupServiceTest, QueryAllAutoStartupApplicationsWithoutPermission_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest QueryAllAutoStartupApplicationsWithoutPermission_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    std::vector<AutoStartupInfo> infoList;
    int32_t userId = 100;
    auto result = abilityAutoStartupService->QueryAllAutoStartupApplicationsWithoutPermission(infoList, userId);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest QueryAllAutoStartupApplicationsWithoutPermission_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupServiceTest, DeleteAutoStartupData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest DeleteAutoStartupData_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    std::string bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    int32_t accessTokenId = 0;
    auto result = abilityAutoStartupService->DeleteAutoStartupData(bundleName, accessTokenId);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest DeleteAutoStartupData_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckAutoStartupData
 */
HWTEST_F(AbilityAutoStartupServiceTest, CheckAutoStartupData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckAutoStartupData_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    std::string bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    auto result = abilityAutoStartupService->CheckAutoStartupData(bundleName, -1);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckAutoStartupData_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: ExecuteCallbacks
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService ExecuteCallbacks
 */
HWTEST_F(AbilityAutoStartupServiceTest, ExecuteCallbacks_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest ExecuteCallbacks_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    bool isCallOn = AUTO_STARTUP_SERVICE_TRUE;
    abilityAutoStartupService->ExecuteCallbacks(isCallOn, info);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest ExecuteCallbacks_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetDeathRecipient
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetDeathRecipient
 */
HWTEST_F(AbilityAutoStartupServiceTest, SetDeathRecipient_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SetDeathRecipient_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    sptr<IRemoteObject> callback;
    sptr<IRemoteObject::DeathRecipient> deathRecipient;
    abilityAutoStartupService->SetDeathRecipient(callback, deathRecipient);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SetDeathRecipient_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CleanResource
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CleanResource
 */
HWTEST_F(AbilityAutoStartupServiceTest, CleanResource_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CleanResource_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    wptr<IRemoteObject> remote;
    abilityAutoStartupService->CleanResource(remote);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CleanResource_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: OnRemoteDied
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService OnRemoteDied
 */
HWTEST_F(AbilityAutoStartupServiceTest, OnRemoteDied_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest OnRemoteDied_001 start";
    std::weak_ptr<AbilityAutoStartupService> weakPtr;
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService::ClientDeathRecipient>(weakPtr);
    EXPECT_NE(abilityAutoStartupService, nullptr);
    wptr<IRemoteObject> remote;
    abilityAutoStartupService->OnRemoteDied(remote);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest OnRemoteDied_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetSelfApplicationBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetSelfApplicationBundleName
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetSelfApplicationBundleName_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetSelfApplicationBundleName_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    auto result = abilityAutoStartupService->GetSelfApplicationBundleName();
    EXPECT_EQ(result, AUTO_STARTUP_SERVICE_EMPTY);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetSelfApplicationBundleName_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetSelfApplicationBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetSelfApplicationBundleName
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetSelfApplicationBundleName_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetSelfApplicationBundleName_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AbilityAutoStartupService autoStartupService;
    autoStartupService.bundleMgrClient_ = std::make_shared<AppExecFwk::BundleMgrClient>();
    auto result = abilityAutoStartupService->GetSelfApplicationBundleName();
    EXPECT_EQ(result, AUTO_STARTUP_SERVICE_EMPTY);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetSelfApplicationBundleName_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckSelfApplication
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckSelfApplication
 */
HWTEST_F(AbilityAutoStartupServiceTest, CheckSelfApplication_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckSelfApplication_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    std::string bundleName;
    auto result = abilityAutoStartupService->CheckSelfApplication(bundleName);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckSelfApplication_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetBundleInfo
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetBundleInfo
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetBundleInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetBundleInfo_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    std::string bundleName;
    int32_t userId = 100;
    int32_t appIndex = 0;
    AppExecFwk::BundleInfo bundleInfo;
    auto result = abilityAutoStartupService->GetBundleInfo(bundleName, bundleInfo, -1, userId, appIndex);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetBundleInfo_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetAbilityData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetAbilityData
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetAbilityData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAbilityData_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AutoStartupInfo info;
    std::string abilityTypeName;
    bool isVisible = AUTO_STARTUP_SERVICE_FALSE;
    std::string accessTokenId = "0";
    int32_t userId = 100;
    auto result = abilityAutoStartupService->GetAbilityData(info, isVisible, abilityTypeName, accessTokenId, userId);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAbilityData_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetAbilityTypeName
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetAbilityTypeName
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetAbilityTypeName_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAbilityTypeName_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    auto result = abilityAutoStartupService->GetAbilityTypeName(abilityInfo);
    EXPECT_EQ(result, "UIAbility");
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAbilityTypeName_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetExtensionTypeName
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetExtensionTypeName
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetExtensionTypeName_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetExtensionTypeName_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    auto result = abilityAutoStartupService->GetExtensionTypeName(extensionInfo);
    EXPECT_EQ(result, "ServiceExtension");
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetExtensionTypeName_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetBundleMgrClient
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetBundleMgrClient
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetBundleMgrClient_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetBundleMgrClient_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    auto result = abilityAutoStartupService->GetBundleMgrClient();
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetBundleMgrClient_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckPermissionForSystem
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckPermissionForSystem
 */
HWTEST_F(AbilityAutoStartupServiceTest, CheckPermissionForSystem_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckPermissionForSystem_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    auto result = abilityAutoStartupService->CheckPermissionForSystem();
    EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckPermissionForSystem_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckPermissionForSelf
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckPermissionForSelf
 */
HWTEST_F(AbilityAutoStartupServiceTest, CheckPermissionForSelf_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckPermissionForSelf_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    std::string bundleName = AUTO_STARTUP_SERVICE_EMPTY;
    auto result = abilityAutoStartupService->CheckPermissionForSelf(bundleName);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckPermissionForSelf_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetAbilityInfo
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetAbilityInfo
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetAbilityInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAbilityInfo_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AutoStartupInfo info;
    std::string abilityTypeName = AUTO_STARTUP_SERVICE_ABILITYNAME;
    std::string accessTokenId = "0";
    int32_t userId = 100;
    auto result = abilityAutoStartupService->GetAbilityInfo(info, abilityTypeName, accessTokenId, userId);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAbilityInfo_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceTest, SetApplicationAutoStartupByEDM_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SetApplicationAutoStartupByEDM_001 start";
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    MyFlag::flag_ = 1;
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AutoStartupInfo info;
    info.abilityName = "abilityName";
    info.bundleName = "bundleName";
    bool flag = false;
    auto result = abilityAutoStartupService->SetApplicationAutoStartupByEDM(info, flag);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SetApplicationAutoStartupByEDM_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CancelApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CancelApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceTest, CancelApplicationAutoStartupByEDM_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CancelApplicationAutoStartupByEDM_001 start";
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    MyFlag::flag_ = 1;
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AutoStartupInfo info;
    info.abilityName = "abilityName";
    info.bundleName = "bundleName";
    bool flag = false;
    auto result = abilityAutoStartupService->CancelApplicationAutoStartupByEDM(info, flag);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SCancelApplicationAutoStartupByEDM_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceTest, InnerApplicationAutoStartupByEDM_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerApplicationAutoStartupByEDM_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AutoStartupInfo info;
    bool isSet = AUTO_STARTUP_SERVICE_FALSE;
    bool flag = AUTO_STARTUP_SERVICE_FALSE;
    auto result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(info, isSet, flag);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerApplicationAutoStartupByEDM_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceTest, InnerApplicationAutoStartupByEDM_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerApplicationAutoStartupByEDM_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    AutoStartupInfo info;
    info.abilityName = AUTO_STARTUP_SERVICE_ABILITYNAME;
    info.bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    info.accessTokenId = "123";
    info.userId = 100;
    bool isSet = AUTO_STARTUP_SERVICE_FALSE;
    bool flag = AUTO_STARTUP_SERVICE_FALSE;
    auto result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(info, isSet, flag);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerApplicationAutoStartupByEDM_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckPermissionForEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckPermissionForEDM
 */
HWTEST_F(AbilityAutoStartupServiceTest, CheckPermissionForEDM_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckPermissionForEDM_001 start";
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    MyFlag::flag_ = 1;
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    auto result = abilityAutoStartupService->CheckPermissionForEDM();
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckPermissionForEDM_001 end";
}
} // namespace AAFwk
} // namespace OHOS
