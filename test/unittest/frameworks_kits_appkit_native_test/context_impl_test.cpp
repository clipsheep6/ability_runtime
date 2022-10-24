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

#include "ability_local_record.h"
#include "application_context.h"
#include "context.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

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
 * @tc.number: AppExecFwk_ContextImpl_SetApplicationInfo_001
 * @tc.name: SetApplicationInfo
 * @tc.desc: Test whether SetApplicationInfo is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_SetApplicationInfo_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetApplicationInfo_001 start";
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
 * @tc.number: AppExecFwk_ContextImpl_CreateModuleContext_001
 * @tc.name: CreateModuleContext
 * @tc.desc: Test whether CreateModuleContext is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000H6I25
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_CreateModuleContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleContext_001 start";
    EXPECT_EQ(contextImpl_->CreateModuleContext("module_name"), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleContext_001 end";
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
        std::make_shared<AbilityRuntime::ApplicationContext>();
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
        std::make_shared<AbilityRuntime::ApplicationContext>();
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
}  // namespace AppExecFwk
}
