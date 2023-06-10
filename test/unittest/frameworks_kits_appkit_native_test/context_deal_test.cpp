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

#include <gtest/gtest.h>
#include <singleton.h>
#define private public
#define protected public
#include "ohos_application.h"
#include "ability.h"
#include "context_deal.h"
#include "process_info.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"
#include "mock_bundle_manager.h"
#include "mock_ability_manager_client.h"
#undef private
#undef protected

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class ContextDealTest : public testing::Test {
public:
    ContextDealTest() : context_(nullptr)
    {}
    ~ContextDealTest()
    {}
    std::shared_ptr<ContextDeal> context_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContextDealTest::SetUpTestCase(void)
{}

void ContextDealTest::TearDownTestCase(void)
{}

void ContextDealTest::SetUp(void)
{
    OHOS::sptr<OHOS::IRemoteObject> bundleObject = new (std::nothrow) BundleMgrService();
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->RegisterSystemAbility(
        OHOS::ABILITY_MGR_SERVICE_ID, bundleObject);
    context_ = std::make_shared<ContextDeal>();
}

void ContextDealTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBundleName_0100
 * @tc.name: GetBundleName
 * @tc.desc: Verify that the GetBundleName return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBundleName_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string bundleName = "BundleName";
    info->bundleName = bundleName;
    context_->SetApplicationInfo(info);

    EXPECT_STREQ(context_->GetBundleName().c_str(), bundleName.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBundleName_0200
 * @tc.name: GetBundleName
 * @tc.desc: Verify that the GetBundleName return value is empty.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBundleName_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = nullptr;
    context_->SetApplicationInfo(info);

    EXPECT_TRUE(context_->GetBundleName().empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBundleManager_0100
 * @tc.name: GetBundleManager
 * @tc.desc: Verify that the GetBundleManager return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBundleManager_0100, Function | MediumTest | Level3)
{
    sptr<IBundleMgr> ptr = context_->GetBundleManager();
    EXPECT_NE(ptr, nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBundleCodePath_0100
 * @tc.name: GetBundleCodePath
 * @tc.desc: Verify that the GetBundleCodePath return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBundleCodePath_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string codePath = "CodePath";
    info->codePath = codePath;
    context_->SetApplicationInfo(info);

    EXPECT_STREQ(context_->GetBundleCodePath().c_str(), "/data/storage/el1/bundle");
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBundleCodePath_0200
 * @tc.name: GetBundleCodePath
 * @tc.desc: Verify that the GetBundleCodePath return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBundleCodePath_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string codePath = "/data/app/el1/bundle/public";
    info->codePath = codePath;
    context_->isCreateBySystemApp_ = true;
    context_->SetApplicationInfo(info);

    EXPECT_STREQ(context_->GetBundleCodePath().c_str(), Constants::LOCAL_BUNDLES);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetApplicationInfo_0100
 * @tc.name: GetApplicationInfo
 * @tc.desc: Verify that the GetApplicationInfo return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetApplicationInfo_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string bundleName = "BundleName";
    info->bundleName = bundleName;
    context_->SetApplicationInfo(info);

    EXPECT_STREQ(context_->GetApplicationInfo()->bundleName.c_str(), bundleName.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetAbilityManager_0100
 * @tc.name: GetAbilityManager
 * @tc.desc: Verify that the GetAbilityManager return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetAbilityManager_0100, Function | MediumTest | Level3)
{
    sptr<AAFwk::IAbilityManager> ptr = context_->GetAbilityManager();
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDatabaseDir_0100
 * @tc.name: GetDatabaseDir
 * @tc.desc: Verify that the GetDatabaseDir return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetDatabaseDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string dir = "dataBaseDir";
    info->dataBaseDir = dir;
    context_->SetApplicationInfo(info);

    EXPECT_STREQ(context_->GetDatabaseDir().c_str(), "/data/storage/el2/database");
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDatabaseDir_0200
 * @tc.name: GetDatabaseDir
 * @tc.desc: Verify that the GetDatabaseDir return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetDatabaseDir_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string dir = "dataBaseDir";
    info->dataBaseDir = dir;
    context_->SetSystemAppFlag(true);
    context_->SetApplicationInfo(info);

    EXPECT_STREQ(context_->GetDatabaseDir().c_str(), "/data/app/el2/0/database/");
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetFilesDir_0100
 * @tc.name: GetFilesDir
 * @tc.desc: Verify that the GetFilesDir return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetFilesDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string dir = "codePath";
    info->dataDir = dir;
    context_->SetApplicationInfo(info);
    dir = dir + "/" + "files";

    EXPECT_STREQ(context_->GetFilesDir().c_str(), "/data/storage/el2/base/files");
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDataDir_0100
 * @tc.name: GetDataDir
 * @tc.desc: Verify that the GetDataDir return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetDataDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string dir = "dataDir";
    info->dataDir = dir;
    context_->SetApplicationInfo(info);

    EXPECT_STREQ(context_->GetDataDir().c_str(), "/data/storage/el2/base/data");
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetAppType_0100
 * @tc.name: GetAppType
 * @tc.desc: Verify that the GetAppType return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetAppType_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    info->bundleName = "hello";
    context_->SetApplicationInfo(info);

    std::string path = context_->GetAppType();
    std::string AppType = "system";

    EXPECT_NE(path.c_str(), AppType.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetAbilityInfo_0100
 * @tc.name: GetAbilityInfo
 * @tc.desc: Verify that the GetAbilityInfo return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetAbilityInfo_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    std::string codePath = "CodePath";
    info->codePath = codePath;
    context_->SetAbilityInfo(info);

    EXPECT_STREQ(context_->GetAbilityInfo()->codePath.c_str(), codePath.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetContext_0100
 * @tc.name: GetContext
 * @tc.desc: Verify that the GetContext return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetContext_0100, Function | MediumTest | Level3)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    context_->SetContext(context);

    EXPECT_NE(context_->GetContext(), nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetApplicationContext_0100
 * @tc.name: GetApplicationContext
 * @tc.desc: Verify that the GetApplicationContext return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetApplicationContext_0100, Function | MediumTest | Level3)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    context_->SetApplicationContext(application);
    EXPECT_NE(nullptr, context_->GetApplicationContext());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetApplicationInfo_0100
 * @tc.name: SetApplicationInfo
 * @tc.desc: Verify that the SetApplicationInfo input parameter is nullptr.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetApplicationInfo_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = nullptr;
    context_->SetApplicationInfo(info);
    EXPECT_TRUE(context_->GetApplicationInfo() == nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetApplicationContext_0100
 * @tc.name: SetApplicationContext
 * @tc.desc: Verify that the SetApplicationContext input parameter is nullptr.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetApplicationContext_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Context> info = nullptr;
    context_->SetApplicationContext(info);
    EXPECT_TRUE(context_->GetApplicationContext() == nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetAbilityInfo_0100
 * @tc.name: SetAbilityInfo
 * @tc.desc: Verify that the SetAbilityInfo input parameter is nullptr.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetAbilityInfo_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = nullptr;
    context_->SetAbilityInfo(info);
    EXPECT_TRUE(context_->GetAbilityInfo() == nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetAbilityInfo_0200
 * @tc.name: SetAbilityInfo
 * @tc.desc: Verify that the SetAbilityInfo input parameter is nullptr.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetAbilityInfo_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = nullptr;
    context_->SetAbilityInfo(info);
    EXPECT_TRUE(context_->GetAbilityInfo() == nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetContext_0100
 * @tc.name: SetContext
 * @tc.desc: Verify that the SetContext input parameter is nullptr.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetContext_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Context> info = nullptr;
    context_->SetContext(info);
    EXPECT_TRUE(context_->GetContext() == nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDir_0100
 * @tc.name: GetDir
 * @tc.desc: Verify that the GetDir return value is not empty.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    info->dataDir = "OHOS";
    const std::string name = "ohos";
    constexpr int32_t mode = 0;
    context_->SetApplicationInfo(info);
    auto resulft = context_->GetDir(name, mode);
    EXPECT_FALSE(resulft.empty());
    resulft = context_->GetDir(name, mode);
    EXPECT_FALSE(resulft.empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDir_0200
 * @tc.name: GetDir
 * @tc.desc: Verify that the GetDir return value is empty.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetDir_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = nullptr;
    const std::string name = "ohos";
    constexpr int32_t mode = 0;
    context_->SetApplicationInfo(info);
    auto resulft = context_->GetDir(name, mode);
    EXPECT_TRUE(resulft.empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetHapModuleInfo_0100
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Verify that the GetHapModuleInfo return value is not nullptr.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetHapModuleInfo_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    context_->SetAbilityInfo(info);

    auto resulft = context_->GetHapModuleInfo();
    EXPECT_TRUE(resulft != nullptr);
    resulft = context_->GetHapModuleInfo();
    EXPECT_TRUE(resulft != nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetHapModuleInfo_0200
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Verify that the GetHapModuleInfo return value is nullptr.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetHapModuleInfo_0200, Function | MediumTest | Level1)
{
    auto resulft = context_->GetHapModuleInfo();
    EXPECT_TRUE(resulft == nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_IsCreateBySystemApp_0100
 * @tc.name: IsCreateBySystemApp
 * @tc.desc: Verify that the IsCreateBySystemApp return value is false.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_IsCreateBySystemApp_0100, Function | MediumTest | Level1)
{
    context_->SetSystemAppFlag(false);
    EXPECT_FALSE(context_->isCreateBySystemApp_);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_IsCreateBySystemApp_0200
 * @tc.name: IsCreateBySystemApp
 * @tc.desc: Verify that the IsCreateBySystemApp return value is true.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_IsCreateBySystemApp_0200, Function | MediumTest | Level1)
{
    context_->SetSystemAppFlag(true);
    EXPECT_TRUE(context_->isCreateBySystemApp_);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBaseDir_0100
 * @tc.name: GetBaseDir
 * @tc.desc: Verify that the GetBaseDir return value is not empty.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBaseDir_0100, Function | MediumTest | Level1)
{
    EXPECT_FALSE(context_->GetBaseDir().empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBaseDir_0200
 * @tc.name: GetBaseDir
 * @tc.desc: Verify that the GetBaseDir return value is not empty.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBaseDir_0200, Function | MediumTest | Level1)
{
    context_->SetSystemAppFlag(true);
    EXPECT_FALSE(context_->GetBaseDir().empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDisplayOrientation_0100
 * @tc.name: GetDisplayOrientation
 * @tc.desc: Verify that the GetDisplayOrientation return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetDisplayOrientation_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->orientation = DisplayOrientation::LANDSCAPE;
    context_->SetAbilityInfo(info);
    EXPECT_EQ(context_->GetDisplayOrientation(), static_cast<int32_t>(DisplayOrientation::LANDSCAPE));
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetCurrentAccountId_0100
 * @tc.name: GetCurrentAccountId
 * @tc.desc: Verify that the GetCurrentAccountId return value is not empty.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetCurrentAccountId_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    auto resulft = context_->GetCurrentAccountId();
    EXPECT_EQ(resulft, 0);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_CreateDirIfNotExist_0100
 * @tc.name: CreateDirIfNotExist
 * @tc.desc: Verify that the CreateDirIfNotExist execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_CreateDirIfNotExist_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    const std::string dir = "./";
    context_->CreateDirIfNotExist(dir);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_initResourceManager_0100
 * @tc.name: initResourceManager
 * @tc.desc: Verify that the initResourceManager execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_initResourceManager_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    const std::shared_ptr<Global::Resource::ResourceManager> resMgr(Global::Resource::CreateResourceManager());
    context_->initResourceManager(resMgr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetPreferencesDir_0100
 * @tc.name: GetPreferencesDir
 * @tc.desc: Verify that the GetPreferencesDir execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetPreferencesDir_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    const std::string expectationDir = "/data/storage/el2/base/preferences";
    const std::string resDir = context_->GetPreferencesDir();
    EXPECT_TRUE(expectationDir == resDir);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_HapModuleInfoRequestInit_0100
 * @tc.name: HapModuleInfoRequestInit
 * @tc.desc: Verify that the HapModuleInfoRequestInit execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_HapModuleInfoRequestInit_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    const std::shared_ptr<EventRunner> runner;
    EXPECT_FALSE(context_->HapModuleInfoRequestInit());

    context_->abilityInfo_ = std::make_shared<AbilityInfo>();
    EXPECT_TRUE(context_->HapModuleInfoRequestInit());
}
}   // namespace AppExecFwk
}   // OHOS