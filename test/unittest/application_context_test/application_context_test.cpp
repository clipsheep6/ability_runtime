/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "application_context.h"
#undef private
#include "mock_ability_token.h"
#include "mock_context_impl.h"
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class ApplicationContextTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<ApplicationContext> context_ = nullptr;
    std::shared_ptr<MockContextImpl> mock_ = nullptr;
};

void ApplicationContextTest::SetUpTestCase(void)
{}

void ApplicationContextTest::TearDownTestCase(void)
{}

void ApplicationContextTest::SetUp()
{
    context_ = std::make_shared<ApplicationContext>();
    mock_ = std::make_shared<MockContextImpl>();
}

void ApplicationContextTest::TearDown()
{}

/**
 * @tc.number: RegisterAbilityLifecycleCallback_0100
 * @tc.name: RegisterAbilityLifecycleCallback
 * @tc.desc: Register Ability Lifecycle Callback
 */
HWTEST_F(ApplicationContextTest, RegisterAbilityLifecycleCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterAbilityLifecycleCallback_0100 start";
    context_->callbacks_.clear();
    std::shared_ptr<AbilityLifecycleCallback> abilityLifecycleCallback = nullptr;
    context_->RegisterAbilityLifecycleCallback(abilityLifecycleCallback);
    EXPECT_TRUE(context_->IsAbilityLifecycleCallbackEmpty());
    GTEST_LOG_(INFO) << "RegisterAbilityLifecycleCallback_0100 end";
}

/**
 * @tc.number: RegisterEnvironmentCallback_0100
 * @tc.name: RegisterEnvironmentCallback
 * @tc.desc: Register Environment Callback
 */
HWTEST_F(ApplicationContextTest, RegisterEnvironmentCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterEnvironmentCallback_0100 start";
    context_->envCallbacks_.clear();
    std::shared_ptr<EnvironmentCallback> environmentCallback = nullptr;
    context_->RegisterEnvironmentCallback(environmentCallback);
    EXPECT_TRUE(context_->envCallbacks_.empty());
    GTEST_LOG_(INFO) << "RegisterEnvironmentCallback_0100 end";
}

/**
 * @tc.number: GetBundleName_0100
 * @tc.name: GetBundleName
 * @tc.desc: Get BundleName failed
 */
HWTEST_F(ApplicationContextTest, GetBundleName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleName_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetBundleName();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetBundleName_0100 end";
}

/**
 * @tc.number: GetBundleName_0200
 * @tc.name: GetBundleName
 * @tc.desc: Get BundleName sucess
 */
HWTEST_F(ApplicationContextTest, GetBundleName_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleName_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetBundleName();
    EXPECT_EQ(ret, "com.test.bundleName");
    GTEST_LOG_(INFO) << "GetBundleName_0200 end";
}

/**
 * @tc.number: CreateBundleContext_0100
 * @tc.name: CreateBundleContext
 * @tc.desc: Create BundleContext failed
 */
HWTEST_F(ApplicationContextTest, CreateBundleContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateBundleContext_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateBundleContext(bundleName);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "CreateBundleContext_0100 end";
}

/**
 * @tc.number: CreateBundleContext_0200
 * @tc.name: CreateBundleContext
 * @tc.desc: Create BundleContext sucess
 */
HWTEST_F(ApplicationContextTest, CreateBundleContext_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateBundleContext_0200 start";
    context_->AttachContextImpl(mock_);
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateBundleContext(bundleName);
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "CreateBundleContext_0200 end";
}

/**
 * @tc.number: CreateModuleContext_0100
 * @tc.name: CreateModuleContext
 * @tc.desc: Create ModuleContext failed
 */
HWTEST_F(ApplicationContextTest, CreateModuleContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleContext_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    std::string moduleName = "moduleName";
    auto ret = context_->CreateModuleContext(moduleName);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleContext_0100 end";
}

/**
 * @tc.number: CreateModuleContext_0200
 * @tc.name: CreateModuleContext
 * @tc.desc: Create ModuleContext sucess
 */
HWTEST_F(ApplicationContextTest, CreateModuleContext_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleContext_0200 start";
    context_->AttachContextImpl(mock_);
    std::string moduleName = "moduleName";
    auto ret = context_->CreateModuleContext(moduleName);
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleContext_0200 end";
}

/**
 * @tc.number: CreateModuleContext_0300
 * @tc.name: CreateModuleContext
 * @tc.desc: Create ModuleContext failed
 */
HWTEST_F(ApplicationContextTest, CreateModuleContext_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleContext_0300 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    std::string moduleName = "moduleName";
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateModuleContext(bundleName, moduleName);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleContext_0300 end";
}

/**
 * @tc.number: CreateModuleContext_0400
 * @tc.name: CreateModuleContext
 * @tc.desc: Create ModuleContext sucess
 */
HWTEST_F(ApplicationContextTest, CreateModuleContext_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleContext_0400 start";
    context_->AttachContextImpl(mock_);
    std::string moduleName = "moduleName";
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateModuleContext(bundleName, moduleName);
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleContext_0400 end";
}

/**
 * @tc.number: GetApplicationInfo_0100
 * @tc.name: GetApplicationInfo
 * @tc.desc: Get ApplicationInfo failed
 */
HWTEST_F(ApplicationContextTest, GetApplicationInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetApplicationInfo_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetApplicationInfo();
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "GetApplicationInfo_0100 end";
}

/**
 * @tc.number: GetApplicationInfo_0200
 * @tc.name: GetApplicationInfo
 * @tc.desc:Get ApplicationInfo sucess
 */
HWTEST_F(ApplicationContextTest, GetApplicationInfo_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetApplicationInfo_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetApplicationInfo();
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "GetApplicationInfo_0200 end";
}

/**
 * @tc.number: GetResourceManager_0100
 * @tc.name: GetResourceManager
 * @tc.desc: Get ResourceManager failed
 */
HWTEST_F(ApplicationContextTest, GetResourceManager_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetResourceManager_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetResourceManager();
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "GetResourceManager_0100 end";
}

/**
 * @tc.number: GetApplicationInfo_0200
 * @tc.name: GetResourceManager
 * @tc.desc:Get ResourceManager sucess
 */
HWTEST_F(ApplicationContextTest, GetResourceManager_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetResourceManager_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetResourceManager();
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "GetResourceManager_0200 end";
}

/**
 * @tc.number: GetBundleCodePath_0100
 * @tc.name: GetBundleCodePath
 * @tc.desc: Get BundleCode Path failed
 */
HWTEST_F(ApplicationContextTest, GetBundleCodePath_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleCodePath_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetBundleCodePath();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetBundleCodePath_0100 end";
}

/**
 * @tc.number: GetBundleCodePath_0200
 * @tc.name: GetBundleCodePath
 * @tc.desc:Get BundleCode Path sucess
 */
HWTEST_F(ApplicationContextTest, GetBundleCodePath_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleCodePath_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetBundleCodePath();
    EXPECT_EQ(ret, "codePath");
    GTEST_LOG_(INFO) << "GetBundleCodePath_0200 end";
}

/**
 * @tc.number: GetHapModuleInfo_0100
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Get HapModuleInfo failed
 */
HWTEST_F(ApplicationContextTest, GetHapModuleInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetHapModuleInfo_0100 start";
    auto ret = context_->GetHapModuleInfo();
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "GetHapModuleInfo_0100 end";
}

/**
 * @tc.number: GetBundleCodeDir_0100
 * @tc.name: GetBundleCodeDir
 * @tc.desc: Get Bundle Code Dir failed
 */
HWTEST_F(ApplicationContextTest, GetBundleCodeDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleCodeDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetBundleCodeDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetBundleCodeDir_0100 end";
}

/**
 * @tc.number: GetBundleCodeDir_0200
 * @tc.name: GetBundleCodeDir
 * @tc.desc:Get Bundle Code Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetBundleCodeDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleCodeDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetBundleCodeDir();
    EXPECT_EQ(ret, "/code");
    GTEST_LOG_(INFO) << "GetBundleCodeDir_0200 end";
}

/**
 * @tc.number: GetCacheDir_0100
 * @tc.name: GetCacheDir
 * @tc.desc: Get Cache Dir failed
 */
HWTEST_F(ApplicationContextTest, GetCacheDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCacheDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetCacheDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetCacheDir_0100 end";
}

/**
 * @tc.number: GetCacheDir_0200
 * @tc.name: GetCacheDir
 * @tc.desc:Get Cache Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetCacheDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCacheDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetCacheDir();
    EXPECT_EQ(ret, "/cache");
    GTEST_LOG_(INFO) << "GetCacheDir_0200 end";
}

/**
 * @tc.number: GetTempDir_0100
 * @tc.name: GetTempDir
 * @tc.desc: Get Temp Dir failed
 */
HWTEST_F(ApplicationContextTest, GetTempDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTempDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetTempDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetTempDir_0100 end";
}

/**
 * @tc.number: GetTempDir_0200
 * @tc.name: GetTempDir
 * @tc.desc:Get Temp Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetTempDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTempDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetTempDir();
    EXPECT_EQ(ret, "/temp");
    GTEST_LOG_(INFO) << "GetTempDir_0200 end";
}

/**
 * @tc.number: GetFilesDir_0100
 * @tc.name: GetFilesDir
 * @tc.desc: Get Files Dir failed
 */
HWTEST_F(ApplicationContextTest, GetFilesDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilesDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetFilesDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetFilesDir_0100 end";
}

/**
 * @tc.number: GetFilesDir_0200
 * @tc.name: GetFilesDir
 * @tc.desc:Get Files Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetFilesDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilesDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetFilesDir();
    EXPECT_EQ(ret, "/files");
    GTEST_LOG_(INFO) << "GetFilesDir_0200 end";
}

/**
 * @tc.number: IsUpdatingConfigurations_0100
 * @tc.name: IsUpdatingConfigurations
 * @tc.desc: Is Updating Configurations failed
 */
HWTEST_F(ApplicationContextTest, IsUpdatingConfigurations_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsUpdatingConfigurations_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->IsUpdatingConfigurations();
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "IsUpdatingConfigurations_0100 end";
}

/**
 * @tc.number: IsUpdatingConfigurations_0200
 * @tc.name: IsUpdatingConfigurations
 * @tc.desc:Is Updating Configurations sucess
 */
HWTEST_F(ApplicationContextTest, IsUpdatingConfigurations_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsUpdatingConfigurations_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->IsUpdatingConfigurations();
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "IsUpdatingConfigurations_0200 end";
}

/**
 * @tc.number: PrintDrawnCompleted_0100
 * @tc.name: PrintDrawnCompleted
 * @tc.desc: Print Drawn Completed failed
 */
HWTEST_F(ApplicationContextTest, PrintDrawnCompleted_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PrintDrawnCompleted_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->PrintDrawnCompleted();
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "PrintDrawnCompleted_0100 end";
}

/**
 * @tc.number: PrintDrawnCompleted_0200
 * @tc.name: PrintDrawnCompleted
 * @tc.desc:Print Drawn Completed sucess
 */
HWTEST_F(ApplicationContextTest, PrintDrawnCompleted_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PrintDrawnCompleted_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->PrintDrawnCompleted();
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "PrintDrawnCompleted_0200 end";
}

/**
 * @tc.number: GetDatabaseDir_0100
 * @tc.name: GetDatabaseDir
 * @tc.desc: Get Data base Dir failed
 */
HWTEST_F(ApplicationContextTest, GetDatabaseDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDatabaseDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetDatabaseDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetDatabaseDir_0100 end";
}

/**
 * @tc.number: GetDatabaseDir_0200
 * @tc.name: GetDatabaseDir
 * @tc.desc:Get Data base Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetDatabaseDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDatabaseDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetDatabaseDir();
    EXPECT_EQ(ret, "/data/app/database");
    GTEST_LOG_(INFO) << "GetDatabaseDir_0200 end";
}

/**
 * @tc.number: GetPreferencesDir_0100
 * @tc.name: GetPreferencesDir
 * @tc.desc: Get Preferences Dir failed
 */
HWTEST_F(ApplicationContextTest, GetPreferencesDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPreferencesDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetPreferencesDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetPreferencesDir_0100 end";
}

/**
 * @tc.number: GetPreferencesDir_0200
 * @tc.name: GetPreferencesDir
 * @tc.desc:Get Preferences Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetPreferencesDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPreferencesDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetPreferencesDir();
    EXPECT_EQ(ret, "/preferences");
    GTEST_LOG_(INFO) << "GetPreferencesDir_0200 end";
}

/**
 * @tc.number: GetDistributedFilesDir_0100
 * @tc.name: GetDistributedFilesDir
 * @tc.desc: Get Distributed Files Dir failed
 */
HWTEST_F(ApplicationContextTest, GetDistributedFilesDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDistributedFilesDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetDistributedFilesDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetDistributedFilesDir_0100 end";
}

/**
 * @tc.number: GetDistributedFilesDir_0200
 * @tc.name: GetDistributedFilesDir
 * @tc.desc:Get Distributed Files Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetDistributedFilesDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDistributedFilesDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetDistributedFilesDir();
    EXPECT_EQ(ret, "/mnt/hmdfs/device_view/local/data/bundleName");
    GTEST_LOG_(INFO) << "GetDistributedFilesDir_0200 end";
}

/**
 * @tc.number: GetToken_0100
 * @tc.name: GetToken
 * @tc.desc: Get Token failed
 */
HWTEST_F(ApplicationContextTest, GetToken_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetToken_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetToken();
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "GetToken_0100 end";
}

/**
 * @tc.number: GetToken_0200
 * @tc.name: GetToken
 * @tc.desc:Get Token sucess
 */
HWTEST_F(ApplicationContextTest, GetToken_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetToken_0200 start";
    std::shared_ptr<ContextImpl> contextImpl = std::make_shared<ContextImpl>();
    context_->AttachContextImpl(contextImpl);
    sptr<IRemoteObject> token = new OHOS::AppExecFwk::MockAbilityToken();
    context_->SetToken(token);
    auto ret = context_->GetToken();
    EXPECT_EQ(ret, token);
    GTEST_LOG_(INFO) << "GetToken_0200 end";
}

/**
 * @tc.number: GetArea_0100
 * @tc.name: GetArea
 * @tc.desc: Get Area failed
 */
HWTEST_F(ApplicationContextTest, GetArea_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetArea_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetArea();
    EXPECT_EQ(ret, 1);
    GTEST_LOG_(INFO) << "GetArea_0100 end";
}

/**
 * @tc.number: GetArea_0200
 * @tc.name: GetArea
 * @tc.desc:Get Area sucess
 */
HWTEST_F(ApplicationContextTest, GetArea_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetArea_0200 start";
    std::shared_ptr<ContextImpl> contextImpl = std::make_shared<ContextImpl>();
    context_->AttachContextImpl(contextImpl);
    int32_t mode = 1;
    context_->SwitchArea(mode);
    auto ret = context_->GetArea();
    EXPECT_EQ(ret, mode);
    GTEST_LOG_(INFO) << "GetArea_0200 end";
}

/**
 * @tc.number: GetConfiguration_0100
 * @tc.name: GetConfiguration
 * @tc.desc: Get Configuration failed
 */
HWTEST_F(ApplicationContextTest, GetConfiguration_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConfiguration_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetConfiguration();
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "GetConfiguration_0100 end";
}

/**
 * @tc.number: GetConfiguration_0200
 * @tc.name: GetConfiguration
 * @tc.desc:Get Configuration sucess
 */
HWTEST_F(ApplicationContextTest, GetConfiguration_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConfiguration_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetConfiguration();
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "GetConfiguration_0200 end";
}

/**
 * @tc.number: GetBaseDir_0100
 * @tc.name: GetBaseDir
 * @tc.desc:Get Base Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetBaseDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBaseDir_0100 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetBaseDir();
    EXPECT_EQ(ret, "/data/app/base");
    GTEST_LOG_(INFO) << "GetBaseDir_0100 end";
}

/**
 * @tc.number: GetDeviceType_0100
 * @tc.name: GetDeviceType
 * @tc.desc: Get DeviceType failed
 */
HWTEST_F(ApplicationContextTest, GetDeviceType_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDeviceType_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetDeviceType();
    EXPECT_EQ(ret, Global::Resource::DeviceType::DEVICE_PHONE);
    GTEST_LOG_(INFO) << "GetDeviceType_0100 end";
}

/**
 * @tc.number: GetDeviceType_0200
 * @tc.name: GetDeviceType
 * @tc.desc:Get DeviceType sucess
 */
HWTEST_F(ApplicationContextTest, GetDeviceType_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDeviceType_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetDeviceType();
    EXPECT_EQ(ret, Global::Resource::DeviceType::DEVICE_NOT_SET);
    GTEST_LOG_(INFO) << "GetDeviceType_0200 end";
}
}  // namespace AbilityRuntime
}  // namespace OHOS