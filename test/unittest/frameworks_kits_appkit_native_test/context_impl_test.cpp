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
#include "context_impl.h"
#undef private

#include "ability_constants.h"
#include "ability_local_record.h"
#include "application_context.h"
#include "context.h"
#include "hap_module_info.h"
#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "mock_ability_token.h"
#include "mock_bundle_manager.h"
#include "mock_resource_manager.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace {
const int64_t CONTEXT_CREATE_BY_SYSTEM_APP(0x00000001);
} // namespace

class ContextImplTest : public testing::Test {
public:
    ContextImplTest() : contextImpl_(nullptr)
    {}
    ~ContextImplTest()
    {}
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContextImplTest::SetUpTestCase(void)
{}

void ContextImplTest::TearDownTestCase(void)
{}

void ContextImplTest::SetUp(void)
{
    contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    sptr<IRemoteObject> bundleObject = new (std::nothrow) BundleMgrService();
    DelayedSingleton<SysMrgClient>::GetInstance()->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID,
        bundleObject);
}

void ContextImplTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetBundleName_001
 * @tc.name: GetBundleName
 * @tc.desc: Test whether GetBundleName is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetBundleName_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleName_001 start";
    std::string bundleName = contextImpl_->GetBundleName();
    EXPECT_STREQ(bundleName.c_str(), "");
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleName_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetBundleName_002
 * @tc.name: GetBundleName
 * @tc.desc: Test whether GetBundleName is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetBundleName_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleName_002 start";
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    applicationInfo->bundleName = "com.test";
    contextImpl_->SetApplicationInfo(applicationInfo);
    std::string bundleName = contextImpl_->GetBundleName();
    EXPECT_STREQ(bundleName.c_str(), "com.test");
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleName_002 end";
}

/**
 * @tc.name: AppExecFwk_ContextImpl_GetBundleName_003
 * @tc.desc: Get bundle name when parent context is not nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetBundleName_003, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    auto applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    EXPECT_NE(applicationInfo, nullptr);
    applicationInfo->bundleName = "com.test.parentcontext";
    parentContext->SetApplicationInfo(applicationInfo);

    contextImpl->SetParentContext(parentContext);
    std::string bundleName = contextImpl->GetBundleName();
    EXPECT_EQ(bundleName, "com.test.parentcontext");
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetBundleCodeDir_0100
 * @tc.desc: Get bundle code directory.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetBundleCodeDir_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // branch when application info is nullptr
    auto codeDir = contextImpl->GetBundleCodeDir();
    EXPECT_EQ(codeDir, "");

    // construct application info
    auto applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    EXPECT_NE(applicationInfo, nullptr);
    applicationInfo->codePath = "/data/app/el1/bundle/public/testCodeDir";
    contextImpl->SetApplicationInfo(applicationInfo);

    // not create by system app
    codeDir = contextImpl->GetBundleCodeDir();
    EXPECT_EQ(codeDir, AbilityRuntime::Constants::LOCAL_CODE_PATH);

    // create by system app(flag is ContextImpl::CONTEXT_CREATE_BY_SYSTEM_APP)
    contextImpl->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);
    codeDir = contextImpl->GetBundleCodeDir();
    EXPECT_EQ(codeDir, "/data/bundles/testCodeDir");

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: IsUpdatingConfigurations_0100
 * @tc.desc: IsUpdatingConfigurations basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, IsUpdatingConfigurations_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto isUpdating = contextImpl->IsUpdatingConfigurations();
    EXPECT_EQ(isUpdating, false);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: PrintDrawnCompleted_0100
 * @tc.desc: PrintDrawnCompleted basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, PrintDrawnCompleted_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto isComplete = contextImpl->PrintDrawnCompleted();
    EXPECT_EQ(isComplete, false);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetDatabaseDir_0100
 * @tc.desc: Get base directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetDatabaseDir_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // not create by system app and parent context is nullptr
    auto databaseDir = contextImpl->GetDatabaseDir();
    EXPECT_EQ(databaseDir, "/data/storage/el2/database");

    // create by system app and parent context is not nullptr
    contextImpl->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    auto applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    EXPECT_NE(applicationInfo, nullptr);
    applicationInfo->bundleName = "com.test.database";
    parentContext->SetApplicationInfo(applicationInfo);
    contextImpl->SetParentContext(parentContext);
    databaseDir = contextImpl->GetDatabaseDir();
    EXPECT_EQ(databaseDir, "/data/app/el2/0/database/com.test.database/");

    // create by system app and hap module info of parent context is not nullptr
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "test_moduleName";
    contextImpl->InitHapModuleInfo(hapModuleInfo);
    databaseDir = contextImpl->GetDatabaseDir();
    EXPECT_EQ(databaseDir, "/data/app/el2/0/database/com.test.database/test_moduleName");

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetPreferencesDir_0100
 * @tc.desc: Get preference directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetPreferencesDir_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto preferenceDir = contextImpl->GetPreferencesDir();
    EXPECT_EQ(preferenceDir, "/data/storage/el2/base/preferences");

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetTempDir_0100
 * @tc.desc: Get temp directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetTempDir_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto tempDir = contextImpl->GetTempDir();
    EXPECT_EQ(tempDir, "/data/storage/el2/base/temp");

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetFilesDir_0100
 * @tc.desc: Get files directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetFilesDir_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto filesDir = contextImpl->GetFilesDir();
    EXPECT_EQ(filesDir, "/data/storage/el2/base/files");

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetDistributedFilesDir_0100
 * @tc.desc: Get distributed directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetDistributedFilesDir_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // not create by system app
    auto distributedDir = contextImpl->GetDistributedFilesDir();
    EXPECT_EQ(distributedDir, "/data/storage/el2/distributedfiles");

    // create by system app and bundleName is empty
    contextImpl->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);
    distributedDir = contextImpl->GetDistributedFilesDir();
    EXPECT_EQ(distributedDir, "/mnt/hmdfs/0/device_view/local/data/");

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetBaseDir_0100
 * @tc.desc: Get base directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetBaseDir_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // not create by system app and parent context is nullptr
    auto baseDir = contextImpl->GetBaseDir();
    EXPECT_EQ(baseDir, "/data/storage/el2/base");

    // create by system app and parent context is not nullptr
    contextImpl->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    auto applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    EXPECT_NE(applicationInfo, nullptr);
    applicationInfo->bundleName = "com.test.base";
    parentContext->SetApplicationInfo(applicationInfo);
    contextImpl->SetParentContext(parentContext);
    baseDir = contextImpl->GetBaseDir();
    EXPECT_EQ(baseDir, "/data/app/el2/0/base/com.test.base/haps/");

    // create by system app and hap module info of parent context is not nullptr
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "test_moduleName";
    contextImpl->InitHapModuleInfo(hapModuleInfo);
    baseDir = contextImpl->GetBaseDir();
    EXPECT_EQ(baseDir, "/data/app/el2/0/base/com.test.base/haps/test_moduleName");

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: SwitchArea_0100
 * @tc.desc: Switch area basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, SwitchArea_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // invalid mode
    contextImpl->SwitchArea(-1);
    contextImpl->SwitchArea(2);

    // valid mode
    contextImpl->SwitchArea(0);
    contextImpl->SwitchArea(1);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetAreaArea_0100
 * @tc.desc: Get area basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetAreaArea_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    contextImpl->SwitchArea(0);
    auto mode = contextImpl->GetArea();
    EXPECT_EQ(mode, 0);

    contextImpl->SwitchArea(1);
    mode = contextImpl->GetArea();
    EXPECT_EQ(mode, 1);

    // invalid area_
    contextImpl->currArea_ = "invalid";
    mode = contextImpl->GetArea();
    EXPECT_EQ(mode, 1); // default is AbilityRuntime::ContextImpl::EL_DEFAULT

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetCurrentAccountId_0100
 * @tc.desc: Get current account id test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetCurrentAccountId_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto accountId = contextImpl->GetCurrentAccountId();
    EXPECT_EQ(accountId, 0); // default account id
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetCurrentActiveAccountId_0100
 * @tc.desc: Get current active account id test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetCurrentActiveAccountId_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto accountId = contextImpl->GetCurrentActiveAccountId();
    EXPECT_EQ(accountId, 100); // default active account id is 100
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: CreateBundleContext_0100
 * @tc.desc: Create bundle context test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, CreateBundleContext_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // bundle name is empty
    auto context = contextImpl->CreateBundleContext("");
    EXPECT_EQ(context, nullptr);

    // bundle name is invalid
    context = contextImpl->CreateBundleContext("invalid_bundleName");
    EXPECT_EQ(context, nullptr);

    context = contextImpl->CreateBundleContext("test_contextImpl");
    EXPECT_NE(context, nullptr);

    // parent context is not nullptr
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    contextImpl->SetParentContext(parentContext);
    context = contextImpl->CreateBundleContext("");
    EXPECT_EQ(context, nullptr);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: AppExecFwk_ContextImpl_SetApplicationInfo_001
 * @tc.name: SetApplicationInfo
 * @tc.desc: Test whether SetApplicationInfo is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_SetApplicationInfo_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetApplicationInfo_001 start";
    contextImpl_->SetApplicationInfo(nullptr);
    EXPECT_EQ(contextImpl_->GetApplicationInfo(), nullptr);
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    contextImpl_->SetApplicationInfo(applicationInfo);
    EXPECT_NE(contextImpl_->GetApplicationInfo(), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetApplicationInfo_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetApplicationInfo_001
 * @tc.name: GetApplicationInfo
 * @tc.desc: Test whether GetApplicationInfo is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetApplicationInfo_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetApplicationInfo_001 start";
    EXPECT_TRUE(contextImpl_->GetApplicationInfo() == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetApplicationInfo_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetApplicationContext_001
 * @tc.name: GetApplicationContext
 * @tc.desc: Test whether GetApplicationContext is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetApplicationContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetApplicationContext_001 start";
    EXPECT_TRUE(contextImpl_->GetApplicationContext() == nullptr);

    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    contextImpl->SetParentContext(parentContext);
    EXPECT_EQ(contextImpl->GetApplicationInfo(), nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetApplicationContext_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_SetParentContext_001
 * @tc.name: SetParentContext
 * @tc.desc: Test whether SetParentContext is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_SetParentContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetParentContext_001 start";
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    contextImpl_->SetParentContext(contextImpl_);
    EXPECT_TRUE(contextImpl_->GetApplicationContext() == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetParentContext_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetHapModuleInfo_001
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Test whether GetHapModuleInfo is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetHapModuleInfo_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetHapModuleInfo_001 start";
    EXPECT_EQ(contextImpl_->GetHapModuleInfo(), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetHapModuleInfo_001 end";
}

/**
 * @tc.number: CreateModuleContext_001
 * @tc.name: CreateModuleContext
 * @tc.desc: Test whether CreateModuleContext is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000H6I25
 */
HWTEST_F(ContextImplTest, CreateModuleContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleContext_001 start";
    EXPECT_EQ(contextImpl_->CreateModuleContext("module_name"), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleContext_001 end";
}

/**
 * @tc.name: CreateModuleContext_002
 * @tc.desc: Create module context test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, CreateModuleContext_002, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // bundleName is valid, but module name is empty
    auto moduleContext = contextImpl->CreateModuleContext("test_contextImpl", "");
    EXPECT_EQ(moduleContext, nullptr);

    // bundle name is invalid
    moduleContext = contextImpl->CreateModuleContext("invalid_bundleName", "invalid_moduleName");
    EXPECT_EQ(moduleContext, nullptr);

    // module didn't exist
    moduleContext = contextImpl->CreateModuleContext("test_contextImpl", "invalid_moduleName");
    EXPECT_EQ(moduleContext, nullptr);

    moduleContext = contextImpl->CreateModuleContext("test_contextImpl", "test_moduleName");
    EXPECT_NE(moduleContext, nullptr);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: AppExecFwk_AppContext_RegisterAbilityLifecycleCallback_001
 * @tc.name: RegisterAbilityLifecycleCallback
 * @tc.desc: Test whether RegisterAbilityLifecycleCallback is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplTest, AppExecFwk_AppContext_RegisterAbilityLifecycleCallback_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_RegisterAbilityLifecycleCallback_001 start";
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->RegisterAbilityLifecycleCallback(nullptr);
    EXPECT_NE(applicationContext, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_RegisterAbilityLifecycleCallback_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_UnregisterAbilityLifecycleCallback_001
 * @tc.name: UnregisterAbilityLifecycleCallback
 * @tc.desc: Test whether UnregisterAbilityLifecycleCallback is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplTest, AppExecFwk_AppContext_UnregisterAbilityLifecycleCallback_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_UnregisterAbilityLifecycleCallback_001 start";
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->UnregisterAbilityLifecycleCallback(nullptr);
    EXPECT_NE(applicationContext, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_UnregisterAbilityLifecycleCallback_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_InitResourceManager_001
 * @tc.name: InitResourceManager
 * @tc.desc: Test whether InitResourceManager is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5826I
 */
HWTEST_F(ContextImplTest, AppExecFwk_AppContext_InitResourceManager_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_001 start";
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ContextImpl> appContext = std::make_shared<AbilityRuntime::ContextImpl>();
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.applicationInfo.multiProjects = true;
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    bundleInfo.applicationInfo.multiProjects = false;
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    bundleInfo.applicationInfo.multiProjects = true;
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "com.test.module");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    bundleInfo.applicationInfo.multiProjects = false;
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "com.test.module");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_InitResourceManager_002
 * @tc.name: InitResourceManager
 * @tc.desc: Test whether InitResourceManager is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5826I
 */
HWTEST_F(ContextImplTest, AppExecFwk_AppContext_InitResourceManager_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_002 start";
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ContextImpl> appContext = std::make_shared<AbilityRuntime::ContextImpl>();
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.applicationInfo.multiProjects = true;
    HapModuleInfo info;
    info.name = "com.ohos.contactsdataability";
    info.moduleName = "entry";
    info.description = "dataability_description";
    info.iconPath = "$media:icon";
    info.deviceTypes = {"smartVision"};
    info.bundleName = "com.ohos.contactsdataability";
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    info.resourcePath = "/data/app/el1/budle/public/com.ohos.contactsdataability"\
        "/com.ohos.contactsdataability/assets/entry/resources.index";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    info.hapPath = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    info.resourcePath = "";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_002 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_InitResourceManager_003
 * @tc.name: InitResourceManager
 * @tc.desc: Test whether InitResourceManager is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5826I
 */
HWTEST_F(ContextImplTest, AppExecFwk_AppContext_InitResourceManager_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_003 start";
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ContextImpl> appContext = std::make_shared<AbilityRuntime::ContextImpl>();
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.applicationInfo.multiProjects = true;
    HapModuleInfo info;
    info.name = "com.ohos.contactsdataability";
    info.moduleName = "entry";
    info.description = "dataability_description";
    info.iconPath = "$media:icon";
    info.deviceTypes = {"smartVision"};
    info.bundleName = "com.ohos.contactsdataability";
    info.resourcePath = "/data/app/el1/budle/public/com.ohos.contactsdataability"\
        "/com.ohos.contactsdataability/assets/entry/resources.index";
    info.hapPath = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    info.moduleName = "entry1";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_003 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_InitResourceManager_004
 * @tc.name: InitResourceManager
 * @tc.desc: Test whether InitResourceManager is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5826I
 */
HWTEST_F(ContextImplTest, AppExecFwk_AppContext_InitResourceManager_004, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_004 start";
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ContextImpl> appContext = std::make_shared<AbilityRuntime::ContextImpl>();
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.applicationInfo.multiProjects = true;
    HapModuleInfo info;
    info.name = "com.ohos.contactsdataability";
    info.moduleName = "entry";
    info.description = "dataability_description";
    info.iconPath = "$media:icon";
    info.deviceTypes = {"smartVision"};
    info.bundleName = "com.ohos.contactsdataability";
    info.resourcePath = "/data/app/el1/budle/public/com.ohos.contactsdataability"\
        "/com.ohos.contactsdataability/assets/entry/resources.index";
    info.hapPath = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    contextImpl_->InitResourceManager(bundleInfo, appContext, false, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    info.resourcePath = "resources.index";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_004 end";
}

/**
 * @tc.name: AppExecFwk_AppContext_InitResourceManager_005
 * @tc.desc: abnornal branch test for InitResourceManager.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, AppExecFwk_AppContext_InitResourceManager_005, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // branch when appContext is nullptr
    AppExecFwk::BundleInfo bundleInfo;
    contextImpl->InitResourceManager(bundleInfo, nullptr, true, "");

    // parent context is not nullptr
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    contextImpl->SetParentContext(parentContext);
    EXPECT_EQ(contextImpl->GetResourceManager(), nullptr);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetBundleCodePath_0100
 * @tc.desc: Get bundle code path test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetBundleCodePath_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto codePath = contextImpl->GetBundleCodePath();
    EXPECT_EQ(codePath, "");

    // construt application info
    auto applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    EXPECT_NE(applicationInfo, nullptr);
    applicationInfo->codePath = "/data/app/el1";
    contextImpl->SetApplicationInfo(applicationInfo);
    EXPECT_EQ(contextImpl->GetBundleCodePath(), "/data/app/el1");

    // parent context is not nullptr
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    contextImpl->SetParentContext(parentContext);
    EXPECT_EQ(contextImpl->GetBundleCodePath(), "");

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: InitHapModuleInfo_0100
 * @tc.desc: Init hap module info test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, InitHapModuleInfo_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    AppExecFwk::HapModuleInfo hapModuleInfo;
    contextImpl->InitHapModuleInfo(hapModuleInfo);
    EXPECT_NE(contextImpl->GetHapModuleInfo(), nullptr);

    // branch when hap module info has been assigned
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    contextImpl->InitHapModuleInfo(abilityInfo);
    EXPECT_NE(contextImpl->GetHapModuleInfo(), nullptr);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: InitHapModuleInfo_0200
 * @tc.desc: Init hap module info test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, InitHapModuleInfo_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    contextImpl->InitHapModuleInfo(nullptr);
    contextImpl->InitHapModuleInfo(abilityInfo);
    EXPECT_NE(contextImpl->GetHapModuleInfo(), nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: SetToken_0100
 * @tc.desc: set token and get token test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, SetToken_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    contextImpl->SetToken(nullptr);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    contextImpl->SetToken(token);
    auto after = contextImpl->GetToken();
    EXPECT_EQ(token, after);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetDeviceType_0100
 * @tc.desc: Get device type test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetDeviceType_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // branch when config is nullptr
    auto deviceType = contextImpl->GetDeviceType();
    EXPECT_EQ(deviceType, Global::Resource::DeviceType::DEVICE_PHONE);

    // get device type again
    deviceType = contextImpl->GetDeviceType();
    EXPECT_EQ(deviceType, Global::Resource::DeviceType::DEVICE_PHONE);

    // construct configuration
    auto config = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(config, nullptr);
    config->AddItem(AAFwk::GlobalConfigurationKey::DEVICE_TYPE, "phone");
    contextImpl->SetConfiguration(config);
    deviceType = contextImpl->GetDeviceType();
    EXPECT_EQ(deviceType, Global::Resource::DeviceType::DEVICE_PHONE);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: GetCacheDir_0100
 * @tc.name: GetCacheDir_0100
 * @tc.desc: Get cache dir test.
 */
HWTEST_F(ContextImplTest, GetCacheDir_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto cacheDir = contextImpl->GetCacheDir();
    EXPECT_EQ(cacheDir, "/data/storage/el2/base/cache");

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: GetConfiguration_0100
 * @tc.name: GetConfiguration_0100
 * @tc.desc: Get configuration test.
 */
HWTEST_F(ContextImplTest, GetConfiguration_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    auto configRet = contextImpl->GetConfiguration();
    EXPECT_EQ(configRet, nullptr);

     // construct configuration
    auto config = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(config, nullptr);
    config->AddItem(AAFwk::GlobalConfigurationKey::DEVICE_TYPE, "phone");
    contextImpl->SetConfiguration(config);

    configRet = contextImpl->GetConfiguration();
    EXPECT_NE(configRet, nullptr);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: IsCreateBySystemApp_0100
 * @tc.name: IsCreateBySystemApp_0100
 * @tc.desc: Is create by system app test.
 */
HWTEST_F(ContextImplTest, IsCreateBySystemApp_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    auto isSystemApp = contextImpl->IsCreateBySystemApp();
    EXPECT_EQ(isSystemApp, false);

    contextImpl->flags_ = CONTEXT_CREATE_BY_SYSTEM_APP;
    isSystemApp = contextImpl->IsCreateBySystemApp();
    EXPECT_EQ(isSystemApp, true);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: SetResourceManager_0100
 * @tc.name: SetResourceManager_0100
 * @tc.desc: Set Resource Manager test.
 */
HWTEST_F(ContextImplTest, SetResourceManager_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    EXPECT_EQ(contextImpl->GetResourceManager(), nullptr);

    auto resourceManager = std::make_shared<Global::Resource::MockResourceManager>();
    EXPECT_NE(resourceManager, nullptr);

    contextImpl->SetResourceManager(resourceManager);
    EXPECT_NE(contextImpl->GetResourceManager(), nullptr);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: GetResourceManager_0100
 * @tc.name: GetResourceManager_0100
 * @tc.desc: Get Resource Manager test.
 */
HWTEST_F(ContextImplTest, GetResourceManager_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    EXPECT_EQ(contextImpl->GetResourceManager(), nullptr);

    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    contextImpl->SetParentContext(parentContext);
    auto resourceManager = std::make_shared<Global::Resource::MockResourceManager>();
    EXPECT_NE(resourceManager, nullptr);

    parentContext->SetResourceManager(resourceManager);
    EXPECT_NE(contextImpl->GetResourceManager(), nullptr);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: GetBundleManager_0100
 * @tc.name: GetBundleManager_0100
 * @tc.desc: Get Bundle Manager test.
 */
HWTEST_F(ContextImplTest, GetBundleManager_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    EXPECT_NE(contextImpl->GetBundleManager(), nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}
}  // namespace AppExecFwk
}
