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
#include "ohos_application.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"
#include "ability_context.h"
#include "ability.h"
#include "context_container.h"
#include "context_deal.h"
#include "mock_bundle_manager.h"
#undef private

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class ContextContainerTest : public testing::Test {
public:
    ContextContainerTest() : context_(nullptr), contextDeal_(nullptr)
    {}
    ~ContextContainerTest()
    {}
    std::shared_ptr<AbilityContext> context_ = nullptr;
    std::shared_ptr<ContextDeal> contextDeal_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContextContainerTest::SetUpTestCase(void)
{
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void ContextContainerTest::TearDownTestCase(void)
{
    OHOS::DelayedSingleton<SysMrgClient>::DestroyInstance();
}

void ContextContainerTest::SetUp(void)
{
    context_ = std::make_shared<AbilityContext>();
    contextDeal_ = std::make_shared<ContextDeal>();
}

void ContextContainerTest::TearDown(void)
{
    context_ = nullptr;
    contextDeal_ = nullptr;
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetBundleName_0100
 * @tc.name: GetBundleName
 * @tc.desc: Test whether attachbasecontext is called normally,
 *           and verify whether the return value of getbundlename is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetBundleName_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string bundleName = "BundleName";
    info->bundleName = bundleName;
    contextDeal_->SetApplicationInfo(info);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_STREQ(context_->GetBundleName().c_str(), bundleName.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetBundleName_0200
 * @tc.name: GetBundleName
 * @tc.desc: Test getbundlename exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetBundleName_0200, Function | MediumTest | Level3)
{
    std::string bundleName = "";

    EXPECT_STREQ(context_->GetBundleName().c_str(), bundleName.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetBundleManager_0100
 * @tc.name: GetBundleManager
 * @tc.desc: Test whether attachbasecontext is called normally,
 *           and verify whether the return value of getbundlemanager is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetBundleManager_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);

    sptr<IBundleMgr> ptr = context_->GetBundleManager();

    EXPECT_NE(ptr, nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetBundleManager_0200
 * @tc.name: GetBundleManager
 * @tc.desc: Test getbundlemanager exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetBundleManager_0200, Function | MediumTest | Level3)
{
    sptr<IBundleMgr> ptr = context_->GetBundleManager();
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetBundleCodePath_0100
 * @tc.name: GetBundleCodePath
 * @tc.desc: Test whether attachbasecontext is called normally,
 *           and verify whether the return value of getbundlecodepath is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetBundleCodePath_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string codePath = "CodePath";
    info->codePath = codePath;
    contextDeal_->SetApplicationInfo(info);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_STREQ(context_->GetBundleCodePath().c_str(), "/data/storage/el1/bundle");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetBundleCodePath_0200
 * @tc.name: GetBundleCodePath
 * @tc.desc: Test getbundlecodepath exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetBundleCodePath_0200, Function | MediumTest | Level3)
{
    std::string codePath = "";
    std::string path = context_->GetBundleCodePath();

    EXPECT_STREQ(path.c_str(), codePath.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetApplicationInfo_0100
 * @tc.name: GetApplicationInfo
 * @tc.desc: Test whether attachbasecontext is called normally,
 *           and verify whether the return value of getapplicationinfo is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetApplicationInfo_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string bundleName = "BundleName";
    info->bundleName = bundleName;
    contextDeal_->SetApplicationInfo(info);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_STREQ(context_->GetApplicationInfo()->bundleName.c_str(), bundleName.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetApplicationInfo_0200
 * @tc.name: GetApplicationInfo
 * @tc.desc: Test getapplicationinfo exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetApplicationInfo_0200, Function | MediumTest | Level3)
{
    std::shared_ptr<ApplicationInfo> info = context_->GetApplicationInfo();
    EXPECT_EQ(info, nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetBundleResourcePath_0100
 * @tc.name: GetBundleResourcePath
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetBundleResourcePath is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetBundleResourcePath_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    std::string resourcePath = "ResourcePath";
    info->resourcePath = resourcePath;
    contextDeal_->SetAbilityInfo(info);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_STREQ(context_->GetBundleResourcePath().c_str(), resourcePath.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetBundleResourcePath_0200
 * @tc.name: GetBundleResourcePath
 * @tc.desc: Test GetBundleResourcePath exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetBundleResourcePath_0200, Function | MediumTest | Level3)
{
    std::string path = context_->GetBundleResourcePath();
    std::string empty = "";
    EXPECT_STREQ(context_->GetBundleResourcePath().c_str(), empty.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetAppType_0100
 * @tc.name: GetAppType
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetAppType is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetAppType_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    info->bundleName = "hello";
    contextDeal_->SetApplicationInfo(info);
    context_->AttachBaseContext(contextDeal_);
    std::string path = context_->GetAppType();
    std::string appType = "system";

    EXPECT_STREQ(context_->GetAppType().c_str(), appType.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetAppType_0200
 * @tc.name: GetAppType
 * @tc.desc: Test GetAppType exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetAppType_0200, Function | MediumTest | Level3)
{
    std::string path = context_->GetAppType();
    std::string empty = "";
    EXPECT_STREQ(context_->GetAppType().c_str(), empty.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetAbilityManager_0100
 * @tc.name: GetAbilityManager
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetAbilityManager is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetAbilityManager_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);

    sptr<AAFwk::IAbilityManager> ptr = context_->GetAbilityManager();
    EXPECT_NE(ptr, nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetAbilityManager_0200
 * @tc.name: GetAbilityManager
 * @tc.desc: Test GetAbilityManager exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetAbilityManager_0200, Function | MediumTest | Level3)
{
    sptr<AAFwk::IAbilityManager> ptr = context_->GetAbilityManager();
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetCodeCacheDir_0100
 * @tc.name: GetCodeCacheDir
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetCodeCacheDir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetCodeCacheDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string dir = "CodeCacheDir";
    info->dataDir = dir;
    contextDeal_->SetApplicationInfo(info);
    context_->AttachBaseContext(contextDeal_);
    std::string dirCompare = "CodeCacheDir/code_cache";
    EXPECT_STREQ(context_->GetCodeCacheDir().c_str(), "/data/storage/el2/base/data/code_cache");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetCodeCacheDir_0200
 * @tc.name: GetCodeCacheDir
 * @tc.desc: Test GetCodeCacheDir exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetCodeCacheDir_0200, Function | MediumTest | Level3)
{
    std::string empty = "";
    EXPECT_STREQ(context_->GetCodeCacheDir().c_str(), empty.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetCacheDir_0100
 * @tc.name: GetCacheDir
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetCacheDir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetCacheDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string dir = "CacheDir";
    info->cacheDir = dir;
    contextDeal_->SetApplicationInfo(info);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_STREQ(context_->GetCacheDir().c_str(), "/data/storage/el2/base/cache");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetCacheDir_0200
 * @tc.name: GetCacheDir
 * @tc.desc: Test GetCacheDir exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetCacheDir_0200, Function | MediumTest | Level3)
{
    std::string empty = "";
    EXPECT_STREQ(context_->GetCacheDir().c_str(), empty.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetDatabaseDir_0100
 * @tc.name: GetDatabaseDir
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetDatabaseDir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetDatabaseDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string dir = "dataBaseDir";
    info->dataBaseDir = dir;
    contextDeal_->SetApplicationInfo(info);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_STREQ(context_->GetDatabaseDir().c_str(), "/data/storage/el2/database");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetCacheDir_0200
 * @tc.name: GetDatabaseDir
 * @tc.desc: Test GetDatabaseDir exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetDatabaseDir_0200, Function | MediumTest | Level3)
{
    std::string empty = "";
    EXPECT_STREQ(context_->GetDatabaseDir().c_str(), empty.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetDataDir_0100
 * @tc.name: GetDataDir
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetDataDir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetDataDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string dir = "dataDir";
    info->dataDir = dir;
    contextDeal_->SetApplicationInfo(info);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_STREQ(context_->GetDataDir().c_str(), "/data/storage/el2/base/data");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetDataDir_0200
 * @tc.name: GetDataDir
 * @tc.desc: Test GetDataDir exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetDataDir_0200, Function | MediumTest | Level3)
{
    std::string empty = "";
    EXPECT_STREQ(context_->GetDataDir().c_str(), empty.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetDir_0100
 * @tc.name: GetDir
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetDir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string dir = "dataDir";
    info->dataDir = dir;
    contextDeal_->SetApplicationInfo(info);
    context_->AttachBaseContext(contextDeal_);

    std::string name = "name";
    std::string dirCompare = "dataDir/name";
    int mode = 0;
    EXPECT_STREQ(context_->GetDir(name, mode).c_str(), dirCompare.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetDir_0200
 * @tc.name: GetDir
 * @tc.desc: Test GetDir exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetDir_0200, Function | MediumTest | Level3)
{
    std::string empty = "";
    std::string name = "name";
    int mode = 0;
    EXPECT_STREQ(context_->GetDir(name, mode).c_str(), empty.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetFilesDir_0100
 * @tc.name: GetFilesDir
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetFilesDir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetFilesDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string dir = "codePath";
    info->dataDir = dir;
    contextDeal_->SetApplicationInfo(info);
    context_->AttachBaseContext(contextDeal_);
    std::string dirCompare = "codePath/files";
    EXPECT_STREQ(context_->GetFilesDir().c_str(), "/data/storage/el2/base/files");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetFilesDir_0200
 * @tc.name: GetFilesDir
 * @tc.desc: Test GetFilesDir exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetFilesDir_0200, Function | MediumTest | Level3)
{
    std::string empty = "";
    EXPECT_STREQ(context_->GetFilesDir().c_str(), empty.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetAbilityInfo_0100
 * @tc.name: GetAbilityInfo
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetAbilityInfo is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetAbilityInfo_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    std::string resourcePath = "ResourcePath";
    info->resourcePath = resourcePath;
    contextDeal_->SetAbilityInfo(info);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_STREQ(context_->GetAbilityInfo()->resourcePath.c_str(), resourcePath.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetAbilityInfo_0200
 * @tc.name: GetAbilityInfo
 * @tc.desc: Test GetAbilityInfo exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetAbilityInfo_0200, Function | MediumTest | Level3)
{
    std::shared_ptr<AbilityInfo> ptr = context_->GetAbilityInfo();
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetContext_0100
 * @tc.name: GetContext
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetContext is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetContext_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_NE(context_->GetContext(), nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetContext_0200
 * @tc.name: GetContext
 * @tc.desc: Test GetContext exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetContext_0200, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> ptr = context_->GetContext();
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetApplicationContext_0100
 * @tc.name: GetApplicationContext
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetApplicationContext is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetApplicationContext_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    contextDeal_->SetApplicationContext(application);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_NE(nullptr, context_->GetApplicationContext());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetApplicationContext_0200
 * @tc.name: GetApplicationContext
 * @tc.desc: Test GetApplicationContext exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetApplicationContext_0200, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> ptr = context_->GetApplicationContext();
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetProcessInfo_0100
 * @tc.name: GetProcessInfo
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetProcessInfo is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetProcessInfo_0100, Function | MediumTest | Level1)
{
    std::string name = "OHOS";
    pid_t id = 0;
    ProcessInfo info(name, id);
    std::shared_ptr<ProcessInfo> processinfo = std::make_shared<ProcessInfo>(info);
    contextDeal_->SetProcessInfo(processinfo);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_STREQ(name.c_str(), context_->GetProcessInfo()->GetProcessName().c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetProcessInfo_0200
 * @tc.name: GetProcessInfo
 * @tc.desc: Test GetProcessInfo exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetProcessInfo_0200, Function | MediumTest | Level3)
{
    std::shared_ptr<ProcessInfo> ptr = context_->GetProcessInfo();
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetProcessName_0100
 * @tc.name: GetProcessName
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetProcessName is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextDeal_GetProcessName_0100, Function | MediumTest | Level1)
{
    std::string name = "OHOS";
    pid_t id = 0;
    ProcessInfo info(name, id);
    std::shared_ptr<ProcessInfo> processinfo = std::make_shared<ProcessInfo>(info);
    contextDeal_->SetProcessInfo(processinfo);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_STREQ(name.c_str(), context_->GetProcessName().c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetProcessName_0200
 * @tc.name: GetProcessName
 * @tc.desc: Test GetProcessName exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetProcessName_0200, Function | MediumTest | Level3)
{
    std::string empty = "";
    std::string name = context_->GetProcessName();
    EXPECT_STREQ(empty.c_str(), name.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_InitResourceManager_0100
 * @tc.name: GetProcessName
 * @tc.desc: Test GetProcessName exception status.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_InitResourceManager_0100, Function | MediumTest | Level3)
{
    std::shared_ptr<AppExecFwk::ContextDeal> appContext = std::make_shared<AppExecFwk::ContextDeal>(true);
    AppExecFwk::BundleInfo bundleInfo;
    context_->InitResourceManager(bundleInfo, appContext);
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    HapModuleInfo info;
    info.name = "com.ohos.contactsdataability";
    info.moduleName = "entry";
    info.description = "dataability_description";
    info.iconPath = "$media:icon";
    info.deviceTypes = {"smartVision"};
    info.bundleName = "com.ohos.contactsdataability";
    bundleInfo.hapModuleInfos.push_back(info);
    context_->InitResourceManager(bundleInfo, appContext);
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    info.resourcePath = "/data/app/el1/budle/public/com.ohos.contactsdataability"\
        "/com.ohos.contactsdataability/assets/entry/resources.index";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    context_->InitResourceManager(bundleInfo, appContext);
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    info.hapPath = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    context_->InitResourceManager(bundleInfo, appContext);
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    info.resourcePath = "";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    context_->InitResourceManager(bundleInfo, appContext);
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);
}

/**
 * @tc.number: AttachBaseContext_0100
 * @tc.name: AttachBaseContext
 * @tc.desc: AttachBaseContext
 */
HWTEST_F(ContextContainerTest, AttachBaseContext_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_EQ(context_->baseContext_, nullptr);
}

/**
 * @tc.number: GetResourceManager_0100
 * @tc.name: GetResourceManager
 * @tc.desc: GetResourceManager
 */
HWTEST_F(ContextContainerTest, GetResourceManager_0100, Function | MediumTest | Level1)
{
    context_->baseContext_ = nullptr;
    EXPECT_EQ(context_->GetResourceManager(), nullptr);
}

/**
 * @tc.number: DeleteFile_0100
 * @tc.name: DeleteFile
 * @tc.desc: DeleteFile
 */
HWTEST_F(ContextContainerTest, DeleteFile_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_TRUE(context_->DeleteFile("test_file"));
}

/**
 * @tc.number: DeleteFile_0200
 * @tc.name: DeleteFile
 * @tc.desc: DeleteFile
 */
HWTEST_F(ContextContainerTest, DeleteFile_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_FALSE(context_->DeleteFile("test_file"));
}

/**
 * @tc.number: GetExternalCacheDir_0100
 * @tc.name: GetExternalCacheDir
 * @tc.desc: GetExternalCacheDir
 */
HWTEST_F(ContextContainerTest, GetExternalCacheDir_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_EQ(context_->GetExternalCacheDir(), "");
}

/**
 * @tc.number: GetExternalCacheDir_0200
 * @tc.name: GetExternalCacheDir
 * @tc.desc: GetExternalCacheDir
 */
HWTEST_F(ContextContainerTest, GetExternalCacheDir_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_EQ(context_->GetExternalCacheDir(), "");
}

/**
 * @tc.number: GetExternalFilesDir_0100
 * @tc.name: GetExternalFilesDir
 * @tc.desc: GetExternalFilesDir
 */
HWTEST_F(ContextContainerTest, GetExternalFilesDir_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    string type = "type_test";
    EXPECT_EQ(context_->GetExternalFilesDir(type), "");
}

/**
 * @tc.number: GetExternalFilesDir_0200
 * @tc.name: GetExternalFilesDir
 * @tc.desc: GetExternalFilesDir
 */
HWTEST_F(ContextContainerTest, GetExternalFilesDir_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    string type = "type_test";
    EXPECT_EQ(context_->GetExternalFilesDir(type), "");
}

/**
 * @tc.number: GetNoBackupFilesDir_0100
 * @tc.name: GetNoBackupFilesDir
 * @tc.desc: GetNoBackupFilesDir
 */
HWTEST_F(ContextContainerTest, GetNoBackupFilesDir_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_NE(context_->GetNoBackupFilesDir(), "");
}

/**
 * @tc.number: GetNoBackupFilesDir_0200
 * @tc.name: GetNoBackupFilesDir
 * @tc.desc: GetNoBackupFilesDir
 */
HWTEST_F(ContextContainerTest, GetNoBackupFilesDir_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_EQ(context_->GetNoBackupFilesDir(), "");
}

/**
 * @tc.number: VerifySelfPermission_0100
 * @tc.name: VerifySelfPermission
 * @tc.desc: VerifySelfPermission
 */
HWTEST_F(ContextContainerTest, VerifySelfPermission_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    std::string permission = "test_permission";
    EXPECT_NE(context_->VerifySelfPermission(permission), AppExecFwk::Constants::PERMISSION_NOT_GRANTED);
}

/**
 * @tc.number: VerifySelfPermission_0200
 * @tc.name: VerifySelfPermission
 * @tc.desc: VerifySelfPermission
 */
HWTEST_F(ContextContainerTest, VerifySelfPermission_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    std::string permission = "test_permission";
    EXPECT_EQ(context_->VerifySelfPermission(permission), AppExecFwk::Constants::PERMISSION_NOT_GRANTED);
}

/**
 * @tc.number: UnauthUriPermission_0100
 * @tc.name: UnauthUriPermission
 * @tc.desc: UnauthUriPermission
 */
HWTEST_F(ContextContainerTest, UnauthUriPermission_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    std::string permission = "test_permission";
    Uri uri("");
    int uid = 0;
    context_->UnauthUriPermission(permission, uri, uid);
}

/**
 * @tc.number: UnauthUriPermission_0200
 * @tc.name: UnauthUriPermission
 * @tc.desc: UnauthUriPermission
 */
HWTEST_F(ContextContainerTest, UnauthUriPermission_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    std::string permission = "test_permission";
    Uri uri("");
    int uid = 0;
    context_->UnauthUriPermission(permission, uri, uid);
}

/**
 * @tc.number: VerifyPermission_0100
 * @tc.name: VerifyPermission
 * @tc.desc: VerifyPermission
 */
HWTEST_F(ContextContainerTest, VerifyPermission_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    std::string permission = "test_permission";
    int pid = 0;
    int uid = 0;
    EXPECT_EQ(context_->VerifyPermission(permission, pid, uid), AppExecFwk::Constants::PERMISSION_NOT_GRANTED);
}

/**
 * @tc.number: VerifyPermission_0200
 * @tc.name: VerifyPermission
 * @tc.desc: VerifyPermission
 */
HWTEST_F(ContextContainerTest, VerifyPermission_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    std::string permission = "test_permission";
    int pid = 0;
    int uid = 0;
    EXPECT_EQ(context_->VerifyPermission(permission, pid, uid), AppExecFwk::Constants::PERMISSION_NOT_GRANTED);
}

/**
 * @tc.number: GetDistributedDir_0100
 * @tc.name: GetDistributedDir
 * @tc.desc: GetDistributedDir
 */
HWTEST_F(ContextContainerTest, GetDistributedDir_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_NE(context_->GetDistributedDir(), "");
}

/**
 * @tc.number: GetDistributedDir_0200
 * @tc.name: GetDistributedDir
 * @tc.desc: GetDistributedDir
 */
HWTEST_F(ContextContainerTest, GetDistributedDir_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_EQ(context_->GetDistributedDir(), "");
}

/**
 * @tc.number: SetPattern_0100
 * @tc.name: SetPattern
 * @tc.desc: SetPattern
 */
HWTEST_F(ContextContainerTest, SetPattern_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    int patternId = 0;
    context_->SetPattern(patternId);
}

/**
 * @tc.number: SetPattern_0200
 * @tc.name: SetPattern
 * @tc.desc: SetPattern
 */
HWTEST_F(ContextContainerTest, SetPattern_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    int patternId = 0;
    context_->SetPattern(patternId);
}

/**
 * @tc.number: GetAbilityPackageContext_0100
 * @tc.name: GetAbilityPackageContext
 * @tc.desc: GetAbilityPackageContext
 */
HWTEST_F(ContextContainerTest, GetAbilityPackageContext_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_EQ(context_->GetAbilityPackageContext(), nullptr);
}

/**
 * @tc.number: GetAbilityPackageContext_0200
 * @tc.name: GetAbilityPackageContext
 * @tc.desc: GetAbilityPackageContext
 */
HWTEST_F(ContextContainerTest, GetAbilityPackageContext_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_EQ(context_->GetAbilityPackageContext(), nullptr);
}

/**
 * @tc.number: RequestPermissionsFromUser_0100
 * @tc.name: RequestPermissionsFromUser
 * @tc.desc: RequestPermissionsFromUser
 */
HWTEST_F(ContextContainerTest, RequestPermissionsFromUser_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    std::vector<std::string> permissions;
    std::vector<int> permissionsState;
    PermissionRequestTask task;
    context_->RequestPermissionsFromUser(permissions, permissionsState, std::move(task));
}

/**
 * @tc.number: RequestPermissionsFromUser_0200
 * @tc.name: RequestPermissionsFromUser
 * @tc.desc: RequestPermissionsFromUser
 */
HWTEST_F(ContextContainerTest, RequestPermissionsFromUser_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    std::vector<std::string> permissions;
    std::vector<int> permissionsState;
    PermissionRequestTask task;
    context_->RequestPermissionsFromUser(permissions, permissionsState, std::move(task));
}

/**
 * @tc.number: CreateBundleContext_0100
 * @tc.name: CreateBundleContext
 * @tc.desc: CreateBundleContext
 */
HWTEST_F(ContextContainerTest, CreateBundleContext_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    std::string bundleName = "test_bundle";
    int flag = 0;
    int accountId = 0;
    EXPECT_EQ(context_->CreateBundleContext(bundleName, flag, accountId), nullptr);
}

/**
 * @tc.number: CreateBundleContext_0200
 * @tc.name: CreateBundleContext
 * @tc.desc: CreateBundleContext
 */
HWTEST_F(ContextContainerTest, CreateBundleContext_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    std::string bundleName = "";
    int flag = 0;
    int accountId = 0;
    EXPECT_EQ(context_->CreateBundleContext(bundleName, flag, accountId), nullptr);
}

/**
 * @tc.number: CreateBundleContext_0300
 * @tc.name: CreateBundleContext
 * @tc.desc: CreateBundleContext
 */
HWTEST_F(ContextContainerTest, CreateBundleContext_0300, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    std::string bundleName = "test_bundle";
    int flag = 0;
    int accountId = 0;
    EXPECT_EQ(context_->CreateBundleContext(bundleName, flag, accountId), nullptr);
}

/**
 * @tc.number: GetString_0100
 * @tc.name: GetString
 * @tc.desc: GetString
 */
HWTEST_F(ContextContainerTest, GetString_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_EQ(context_->GetString(1), "");
}

/**
 * @tc.number: GetString_0200
 * @tc.name: GetString
 * @tc.desc: GetString
 */
HWTEST_F(ContextContainerTest, GetString_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_EQ(context_->GetString(1), "");
}

/**
 * @tc.number: GetStringArray_0100
 * @tc.name: GetStringArray
 * @tc.desc: GetStringArray
 */
HWTEST_F(ContextContainerTest, GetStringArray_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_EQ(context_->GetStringArray(1).size(), 0);
}

/**
 * @tc.number: GetStringArray_0200
 * @tc.name: GetStringArray
 * @tc.desc: GetStringArray
 */
HWTEST_F(ContextContainerTest, GetStringArray_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_EQ(context_->GetStringArray(1).size(), 0);
}

/**
 * @tc.number: GetIntArray_0100
 * @tc.name: GetIntArray
 * @tc.desc: GetIntArray
 */
HWTEST_F(ContextContainerTest, GetIntArray_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_EQ(context_->GetIntArray(1).size(), 0);
}

/**
 * @tc.number: GetIntArray_0200
 * @tc.name: GetIntArray
 * @tc.desc: GetIntArray
 */
HWTEST_F(ContextContainerTest, GetIntArray_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_EQ(context_->GetIntArray(1).size(), 0);
}

/**
 * @tc.number: GetTheme_0100
 * @tc.name: GetTheme
 * @tc.desc: GetTheme
 */
HWTEST_F(ContextContainerTest, GetTheme_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_EQ(context_->GetTheme().size(), 0);
}

/**
 * @tc.number: GetTheme_0200
 * @tc.name: GetTheme
 * @tc.desc: GetTheme
 */
HWTEST_F(ContextContainerTest, GetTheme_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_EQ(context_->GetTheme().size(), 0);
}

/**
 * @tc.number: SetTheme_0100
 * @tc.name: SetTheme
 * @tc.desc: SetTheme
 */
HWTEST_F(ContextContainerTest, SetTheme_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    context_->SetTheme(1);
}

/**
 * @tc.number: SetTheme_0200
 * @tc.name: SetTheme
 * @tc.desc: SetTheme
 */
HWTEST_F(ContextContainerTest, SetTheme_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    context_->SetTheme(1);
}

/**
 * @tc.number: GetPattern_0100
 * @tc.name: GetPattern
 * @tc.desc: GetPattern
 */
HWTEST_F(ContextContainerTest, GetPattern_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_EQ(context_->GetPattern().size(), 0);
}

/**
 * @tc.number: GetPattern_0200
 * @tc.name: GetPattern
 * @tc.desc: GetPattern
 */
HWTEST_F(ContextContainerTest, GetPattern_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_EQ(context_->GetPattern().size(), 0);
}

/**
 * @tc.number: GetColor_0100
 * @tc.name: GetColor
 * @tc.desc: GetColor
 */
HWTEST_F(ContextContainerTest, GetColor_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_EQ(context_->GetColor(1), -1);
}

/**
 * @tc.number: GetThemeId_0100
 * @tc.name: GetThemeId
 * @tc.desc: GetThemeId
 */
HWTEST_F(ContextContainerTest, GetThemeId_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_EQ(context_->GetThemeId(), -1);
}

/**
 * @tc.number: GetDisplayOrientation_0100
 * @tc.name: GetDisplayOrientation
 * @tc.desc: GetDisplayOrientation
 */
HWTEST_F(ContextContainerTest, GetDisplayOrientation_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_EQ(context_->GetDisplayOrientation(), 0);
}

/**
 * @tc.number: GetDisplayOrientation_0200
 * @tc.name: GetDisplayOrientation
 * @tc.desc: GetDisplayOrientation
 */
HWTEST_F(ContextContainerTest, GetDisplayOrientation_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_EQ(context_->GetDisplayOrientation(), 0);
}

/**
 * @tc.number: GetPreferencesDir_0100
 * @tc.name: GetPreferencesDir
 * @tc.desc: GetPreferencesDir
 */
HWTEST_F(ContextContainerTest, GetPreferencesDir_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_NE(context_->GetPreferencesDir(), "");
}

/**
 * @tc.number: GetPreferencesDir_0200
 * @tc.name: GetPreferencesDir
 * @tc.desc: GetPreferencesDir
 */
HWTEST_F(ContextContainerTest, GetPreferencesDir_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_EQ(context_->GetPreferencesDir(), "");
}

/**
 * @tc.number: SetColorMode_0100
 * @tc.name: SetColorMode
 * @tc.desc: SetColorMode
 */
HWTEST_F(ContextContainerTest, SetColorMode_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    context_->SetColorMode(1);
}

/**
 * @tc.number: SetColorMode_0200
 * @tc.name: SetColorMode
 * @tc.desc: SetColorMode
 */
HWTEST_F(ContextContainerTest, SetColorMode_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    context_->SetColorMode(1);
}

/**
 * @tc.number: GetColorMode_0100
 * @tc.name: GetColorMode
 * @tc.desc: GetColorMode
 */
HWTEST_F(ContextContainerTest, GetColorMode_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_EQ(context_->GetColorMode(), -1);
}

/**
 * @tc.number: GetColorMode_0200
 * @tc.name: GetColorMode
 * @tc.desc: GetColorMode
 */
HWTEST_F(ContextContainerTest, GetColorMode_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_EQ(context_->GetColorMode(), -1);
}

/**
 * @tc.number: GetMissionId_0100
 * @tc.name: GetMissionId
 * @tc.desc: GetMissionId
 */
HWTEST_F(ContextContainerTest, GetMissionId_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_EQ(context_->GetMissionId(), -1);
}

/**
 * @tc.number: GetMissionId_0200
 * @tc.name: GetMissionId
 * @tc.desc: GetMissionId
 */
HWTEST_F(ContextContainerTest, GetMissionId_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_EQ(context_->GetMissionId(), -1);
}

/**
 * @tc.number: IsUpdatingConfigurations_0100
 * @tc.name: IsUpdatingConfigurations
 * @tc.desc: IsUpdatingConfigurations
 */
HWTEST_F(ContextContainerTest, IsUpdatingConfigurations_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_FALSE(context_->IsUpdatingConfigurations());
}

/**
 * @tc.number: IsUpdatingConfigurations_0200
 * @tc.name: IsUpdatingConfigurations
 * @tc.desc: IsUpdatingConfigurations
 */
HWTEST_F(ContextContainerTest, IsUpdatingConfigurations_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_FALSE(context_->IsUpdatingConfigurations());
}

/**
 * @tc.number: PrintDrawnCompleted_0100
 * @tc.name: PrintDrawnCompleted
 * @tc.desc: PrintDrawnCompleted
 */
HWTEST_F(ContextContainerTest, PrintDrawnCompleted_0100, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(contextDeal_);
    EXPECT_FALSE(context_->PrintDrawnCompleted());
}

/**
 * @tc.number: PrintDrawnCompleted_0200
 * @tc.name: PrintDrawnCompleted
 * @tc.desc: PrintDrawnCompleted
 */
HWTEST_F(ContextContainerTest, PrintDrawnCompleted_0200, Function | MediumTest | Level1)
{
    context_->AttachBaseContext(nullptr);
    EXPECT_FALSE(context_->PrintDrawnCompleted());
}

/**
 * @tc.number: AppExecFwk_ContextContainer_DeleteFile_0100
 * @tc.name: Delete File
 * @tc.desc: Test Delete File When baseContext is not null,
 *           and verify whether the return value of DeleteFile is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_DeleteFile_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_TRUE(context_->DeleteFile("test"));
}

/**
 * @tc.number: AppExecFwk_ContextContainer_DeleteFile_0200
 * @tc.name: Delete File
 * @tc.desc: Test Delete File When baseContext is null.
 *           and verify whether the return value of DeleteFile is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_DeleteFile_0200, Function | MediumTest | Level3)
{
    EXPECT_FALSE(context_->DeleteFile("test"));
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetExternalCacheDir_0100
 * @tc.name: Get External Cache Dir
 * @tc.desc: Test Get External Cache Dir When baseContext is not null,
 *           and verify whether the return value of Get External Cache Dir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetExternalCacheDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->GetExternalCacheDir(), "");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetExternalCacheDir_0200
 * @tc.name: Get External Cache Dir
 * @tc.desc: Test Get External Cache Dir When baseContext is null.
 *           and verify whether the return value of Get External Cache Dir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetExternalCacheDir_0200, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->GetExternalCacheDir(), "");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetExternalFilesDir_0100
 * @tc.name: Get External Files Dir
 * @tc.desc: Test Get External Files Dir When baseContext is not null,
 *           and verify whether the return value of Get External Files Dir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetExternalFilesDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    std::string type = "type";
    EXPECT_EQ(context_->GetExternalFilesDir(type), "");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetExternalFilesDir_0200
 * @tc.name: Get External FilesDir
 * @tc.desc: Test Get External Files Dir When baseContext is null.
 *           and verify whether the return value of Get External Files Dir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetExternalFilesDir_0200, Function | MediumTest | Level3)
{
    std::string type = "type";
    EXPECT_EQ(context_->GetExternalFilesDir(type), "");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetNoBackupFilesDir_0100
 * @tc.name: Get No Backup Files Dir
 * @tc.desc: Test Get No Backup Files Dir When baseContext is not null,
 *           and verify whether the return value of Get No Backup Files Dir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetNoBackupFilesDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->GetNoBackupFilesDir(), "/data/storage/el2/base/data/no_backup");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetNoBackupFilesDir_0200
 * @tc.name: Get No Backup FilesDir
 * @tc.desc: Test Get No Backup Files Dir When baseContext is null.
 *           and verify whether the return value of Get No Backup Files Dir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetNoBackupFilesDir_0200, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->GetNoBackupFilesDir(), "");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_VerifySelfPermission_0100
 * @tc.name: Verify Self Permission
 * @tc.desc: Test Verify Self Permission When baseContext is not null,
 *           and verify whether the return value of Verify Self Permission is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_VerifySelfPermission_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->VerifySelfPermission("permission"), AppExecFwk::Constants::PERMISSION_GRANTED);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_VerifySelfPermission_0200
 * @tc.name: Verify Self Permission
 * @tc.desc: Test Verify Self Permission When baseContext is null.
 *           and verify whether the return value of Verify Self Permission is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_VerifySelfPermission_0200, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->VerifySelfPermission("permission"), AppExecFwk::Constants::PERMISSION_NOT_GRANTED);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_UnauthUriPermission_0100
 * @tc.name: Unauth Uri Permission
 * @tc.desc: Test Unauth Uri Permission When baseContext is not null
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_UnauthUriPermission_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    Uri urivalue("");
    context_->UnauthUriPermission("permission", urivalue, 0);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_UnauthUriPermission_0200
 * @tc.name: Unauth Uri Permission
 * @tc.desc: TestUnauth Uri Permission When baseContext is null.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_UnauthUriPermission_0200, Function | MediumTest | Level3)
{
    Uri urivalue("");
    context_->UnauthUriPermission("permission", urivalue, 0);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetDistributedDir_0100
 * @tc.name: Get Distributed Dir
 * @tc.desc: Test Get Distributed Dir When baseContext is not null,
 *           and verify whether the return value of Get Distributed Dir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetDistributedDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->GetDistributedDir(), "/data/storage/el2/distributedfiles");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetDistributedDir_0200
 * @tc.name: Get Distributed Dir
 * @tc.desc: Test Get Distributed Dir When baseContext is null.
 *           and verify whether the return value of Get Distributed Dir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetDistributedDir_0200, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->GetDistributedDir(), "");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetAbilityPackageContext_0100
 * @tc.name: Get Ability Package Context
 * @tc.desc: Test Get Ability Package Context When baseContext is not null,
 *           and verify whether the return value of Get Ability Package Context is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetAbilityPackageContext_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->GetAbilityPackageContext(), nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetAbilityPackageContext_0200
 * @tc.name: Get Ability Package Context
 * @tc.desc: Test Get Ability Package Context When baseContext is null.
 *           and verify whether the return value of Get Ability Package Context is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetAbilityPackageContext_0200, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->GetAbilityPackageContext(), nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetHapModuleInfo_0100
 * @tc.name: Get Hap Module Info
 * @tc.desc: Test Get Hap Module Info When baseContext is not null,
 *           and verify whether the return value of Get Hap Module Info is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetHapModuleInfo_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    contextDeal_->hapModuleInfoLocal_ = std::make_shared<HapModuleInfo>();
    context_->AttachBaseContext(contextDeal_);

    EXPECT_NE(context_->GetHapModuleInfo(), nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetHapModuleInfo_0200
 * @tc.name: Get Hap Module Info
 * @tc.desc: Test Get Hap Module Info When baseContext is null.
 *           and verify whether the return value of Get Hap Module Info is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetHapModuleInfo_0200, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->GetHapModuleInfo(), nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetCaller_0100
 * @tc.name: Get Caller
 * @tc.desc: Test Get Caller When baseContext is not null,
 *           and verify whether the return value of Get Caller is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetCaller_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    contextDeal_->SerUriString("test");
    context_->AttachBaseContext(contextDeal_);

    Uri uri("test");
    EXPECT_EQ(context_->GetCaller(), uri);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetCaller_0200
 * @tc.name: Request Permissions From User
 * @tc.desc: Test Get Caller When baseContext is null.
 *           and verify whether the return value of Get Caller is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetCaller_0200, Function | MediumTest | Level3)
{
    Uri uri("");
    EXPECT_EQ(context_->GetCaller(), uri);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetString_0100
 * @tc.name: Get String
 * @tc.desc: Test Get String When baseContext is not null,
 *           and verify whether the return value of Get String is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetString_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->GetString(0), "");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetString_0200
 * @tc.name: Get String
 * @tc.desc: Test Get String When baseContext is null.
 *           and verify whether the return value of Get String is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetString_0200, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->GetString(0), "");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetStringArray_0100
 * @tc.name: Get String Array
 * @tc.desc: Test Get String Array When baseContext is not null,
 *           and verify whether the return value of Get String Array is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetStringArray_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    std::vector<std::string> ret = context_->GetStringArray(0);
    EXPECT_EQ(ret.size(), 0);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetStringArray_0200
 * @tc.name: Get String Array
 * @tc.desc: Test Get String Array When baseContext is null.
 *           and verify whether the return value of Get String Array is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetStringArray_0200, Function | MediumTest | Level3)
{
    std::vector<std::string> ret = context_->GetStringArray(0);
    EXPECT_EQ(ret.size(), 0);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetIntArray_0100
 * @tc.name: Get Int Array
 * @tc.desc: Test Get Int Array When baseContext is not null,
 *           and verify whether the return value of Get Int Array is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetIntArray_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    std::vector<int> ret = context_->GetIntArray(0);
    EXPECT_EQ(ret.size(), 0);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetIntArray_0200
 * @tc.name: Get Int Array
 * @tc.desc: Test Get Int Array When baseContext is null.
 *           and verify whether the return value of Get Int Array is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetIntArray_0200, Function | MediumTest | Level3)
{
    std::vector<int> ret = context_->GetIntArray(0);
    EXPECT_EQ(ret.size(), 0);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetTheme_0100
 * @tc.name: Get Theme
 * @tc.desc: Test Get Theme When baseContext is not null,
 *           and verify whether the return value of Get Theme is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetTheme_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);
    std::map<std::string, std::string> ret = context_->GetTheme();
    EXPECT_EQ(ret.size(), 0);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetTheme_0200
 * @tc.name: Get Theme
 * @tc.desc: Test Get Theme When baseContext is null.
 *           and verify whether the return value of Get Theme is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetTheme_0200, Function | MediumTest | Level3)
{
    std::map<std::string, std::string> ret = context_->GetTheme();
    EXPECT_EQ(ret.size(), 0);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_SetTheme_0100
 * @tc.name: Set Theme
 * @tc.desc: Test Set Theme When baseContext is not null,
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_SetTheme_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);
    context_->SetTheme(0);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_SetTheme_0200
 * @tc.name: Set Theme
 * @tc.desc: Test Set Theme When baseContext is null.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_SetTheme_0200, Function | MediumTest | Level3)
{
    context_->SetTheme(0);
}

/**
 * @tc.number: AppExecFwk_ContextContainerGetPattern_0100
 * @tc.name: Get Pattern
 * @tc.desc: Test Get Pattern When baseContext is not null,
 *           and verify whether the return value of Get Pattern is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetPattern_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    std::map<std::string, std::string> ret = context_->GetPattern();
    EXPECT_EQ(ret.size(), 0);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetPattern_0200
 * @tc.name: Get Pattern
 * @tc.desc: Test Get Pattern When baseContext is null.
 *           and verify whether the return value of Get Pattern is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetPattern_0200, Function | MediumTest | Level3)
{
    std::map<std::string, std::string> ret = context_->GetPattern();
    EXPECT_EQ(ret.size(), 0);
}

/**
 * @tc.number: AppExecFwk_ContextContainerSetPattern_0100
 * @tc.name: Set Pattern
 * @tc.desc: Test Get Pattern When baseContext is not null,
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_SetPattern_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    context_->SetPattern(0);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_SetPattern_0200
 * @tc.name: Set Pattern
 * @tc.desc: Test Set Pattern When baseContext is null.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_SetPattern_0200, Function | MediumTest | Level3)
{
    context_->SetPattern(0);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetColor_0100
 * @tc.name: Get Color
 * @tc.desc: Test Get Color When baseContext is not null,
 *           and verify whether the return value of Get Color is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetColor_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->GetColor(0), -1);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetColor_0200
 * @tc.name: Get Color
 * @tc.desc: Test Get Color When baseContext is null.
 *           and verify whether the return value of Get Color is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetColor_0200, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->GetColor(0), -1);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetThemeId_0100
 * @tc.name: Get Theme Id
 * @tc.desc: Test Get Theme Id When baseContext is not null,
 *           and verify whether the return value of Get Theme Id is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetThemeId_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->GetThemeId(), -1);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetThemeId_0200
 * @tc.name: Get Theme Id
 * @tc.desc: Test Get Theme Id When baseContext is null.
 *           and verify whether the return value of Get Theme Id is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetThemeId_0200, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->GetThemeId(), -1);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetDisplayOrientation_0100
 * @tc.name: Get Display Orientation
 * @tc.desc: Test Get Display Orientation When baseContext is not null,
 *           and verify whether the return value of Get Display Orientation is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetDisplayOrientation_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    contextDeal_-> abilityInfo_ = std::make_shared<AbilityInfo>();
    contextDeal_->abilityInfo_->orientation = DisplayOrientation::LANDSCAPE;
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->GetDisplayOrientation(), 1);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetDisplayOrientation_0200
 * @tc.name: Get Display Orientation
 * @tc.desc: Test Get Display Orientation When baseContext is null.
 *           and verify whether the return value of Get Display Orientation is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetDisplayOrientation_0200, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->GetDisplayOrientation(), 0);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetPreferencesDir_0100
 * @tc.name: Get Preferences Dir
 * @tc.desc: Test Get Preferences Dir When baseContext is not null,
 *           and verify whether the return value of Get Preferences Dir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetPreferencesDire_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->GetPreferencesDir(), "/data/storage/el2/base/preferences");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetPreferencesDir_0200
 * @tc.name: Get Preferences Dir
 * @tc.desc: Test Get Preferences Dir When baseContext is null.
 *           and verify whether the return value of Get Preferences Dir is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetPreferencesDir_0200, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->GetPreferencesDir(), "");
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetColorMode_0100
 * @tc.name: Get Color Mode
 * @tc.desc: Test Get Color Mode When baseContext is not null,
 *           and verify whether the return value of Get Color Mode is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetColorMode_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    contextDeal_->hapModuleInfoLocal_ = std::make_shared<HapModuleInfo>();
    context_->AttachBaseContext(contextDeal_);

    context_->SetColorMode(static_cast<int>(ModuleColorMode::DARK));
    EXPECT_EQ(context_->GetColorMode(), static_cast<int>(ModuleColorMode::DARK));
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetColorMode_0200
 * @tc.name: Get Color Mode
 * @tc.desc: Test Get Color Mode When baseContext is null.
 *           and verify whether the return value of Get Color Mode is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetColorMode_0200, Function | MediumTest | Level3)
{
    context_->SetColorMode(-1);
    EXPECT_EQ(context_->GetColorMode(), -1);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetMissionId_0100
 * @tc.name: Get Mission Id
 * @tc.desc: Test Get Mission Id When baseContext is not null,
 *           and verify whether the return value of Get Mission Id is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetMissionId_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);

    AAFwk::LifeCycleStateInfo lifeCycleStateInfo;
    lifeCycleStateInfo.missionId = 0;
    contextDeal_->SetLifeCycleStateInfo(lifeCycleStateInfo);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->GetMissionId(), 0);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetMissionId_0200
 * @tc.name: Get Mission Id
 * @tc.desc: Test Get Mission Id When baseContext is null.
 *           and verify whether the return value of Get Mission Id is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_GetMissionId_0200, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->GetMissionId(), -1);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_IsUpdatingConfigurations_0100
 * @tc.name: Is Updating Configurations
 * @tc.desc: Test Is Updating Configurations When baseContext is not null,
 *           and verify whether the return value of Is Updating Configurations is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_IsUpdatingConfigurations_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->IsUpdatingConfigurations(), false);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_IsUpdatingConfigurations_0200
 * @tc.name: Is Updating Configurations
 * @tc.desc: Test Is Updating Configurations When baseContext is null.
 *           and verify whether the return value of Is Updating Configurations is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_IsUpdatingConfigurations_0200, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->IsUpdatingConfigurations(), false);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_PrintDrawnCompleted_0100
 * @tc.name: Print Drawn Completed
 * @tc.desc: Test Print Drawn Completed When baseContext is not null,
 *           and verify whether the return value of Print Drawn Completed is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_PrintDrawnCompleted_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->PrintDrawnCompleted(), false);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_PrintDrawnCompleted_0200
 * @tc.name: Print Drawn Completed
 * @tc.desc: Test Print Drawn Completed When baseContext is null.
 *           and verify whether the return value of Print Drawn Completed is correct.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_PrintDrawnCompleted_0200, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->PrintDrawnCompleted(), false);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_CreateBundleContext_0100
* @tc.name: Create Bundle Context
 * @tc.desc: Test Create Bundle Context return is not null.
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_CreateBundleContext_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string bundleName = "BundleName";
    info->bundleName = bundleName;
    contextDeal_->SetContext(context);
    contextDeal_->SetApplicationInfo(info);
    contextDeal_->SetApplicationContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_NE(context_->CreateBundleContext("BundleName", 0, 0), nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_CreateBundleContext_0200
 * @tc.name: Create Bundle Context
 * @tc.desc: Test Create Bundle Context return is null because bundle info is error account id is not DEFAULT_ACCOUNT_ID
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_CreateBundleContext_0200, Function | MediumTest | Level3)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->CreateBundleContext("bundlename", 0, 0), nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_CreateBundleContext_0300
 * @tc.name: Create Bundle Context
 * @tc.desc: Test Create Bundle Context return is null because bundle info is error account id is DEFAULT_ACCOUNT_ID
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_CreateBundleContext_0300, Function | MediumTest | Level3)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    contextDeal_->SetContext(context);
    context_->AttachBaseContext(contextDeal_);

    EXPECT_EQ(context_->CreateBundleContext("bundlename", 0, DEFAULT_ACCOUNT_ID), nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextContainer_CreateBundleContext_0400
 * @tc.name: Create Bundle Context
 * @tc.desc: Test Create Bundle Context return is null because bundle name is empty
 */
HWTEST_F(ContextContainerTest, AppExecFwk_ContextContainer_CreateBundleContext_0400, Function | MediumTest | Level3)
{
    EXPECT_EQ(context_->CreateBundleContext("", 0, 0), nullptr);
}
}  // namespace AppExecFwk
}  // namespace OHOS
