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

#include "ability_handler.h"
#define private public
#include "context_impl.h"
#include "auto_fill_extension.h"
#include "auto_fill_extension_context.h"
#undef private
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace testing::ext;
namespace {
    const std::string BUNDLE_NAME = "com.ohos.settingsdata";
    const std::string MODULE_NAME = "entry";
}
class AutoFillExtensionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AutoFillExtensionTest::SetUpTestCase(void)
{}

void AutoFillExtensionTest::TearDownTestCase(void)
{}

void AutoFillExtensionTest::SetUp(void)
{}

void AutoFillExtensionTest::TearDown(void)
{}

/**
 * @tc.name: Create_0100
 * @tc.desc: The runtime is nullptr, and the verification of create is successful.
 * @tc.type: FUNC
 */
HWTEST_F(AutoFillExtensionTest, Create_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Create_0100 start";
    std::unique_ptr<Runtime> runtime;
    auto autoFillExtension = AutoFillExtension::Create(runtime);
    EXPECT_TRUE(autoFillExtension != nullptr);
    GTEST_LOG_(INFO) << "Create_0100 end";
}

/**
 * @tc.name: Create_0200
 * @tc.desc: The language is js, and the verification of create is successful.
 * @tc.type: FUNC
 */
HWTEST_F(AutoFillExtensionTest, Create_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Create_0200 start";
    Runtime::Options options;
    auto runtime = Runtime::Create(options);
    auto autoFillExtension = AutoFillExtension::Create(runtime);
    EXPECT_TRUE(autoFillExtension != nullptr);
    GTEST_LOG_(INFO) << "Create_0200 end";
}

/**
 * @tc.name: Init_0100
 * @tc.desc: Verify the initialization is successful.
 * @tc.type: FUNC
 */
HWTEST_F(AutoFillExtensionTest, Init_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Init_0100 start";

    Runtime::Options options;
    std::unique_ptr<Runtime> runtime = Runtime::Create(options);
    auto autoFillExtension = AutoFillExtension::Create(runtime);
    EXPECT_NE(autoFillExtension, nullptr);

    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    EXPECT_NE(token, nullptr);
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(abilityInfo, token);
    EXPECT_NE(record, nullptr);
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    EXPECT_NE(application, nullptr);
    auto contextImpl = std::make_shared<ContextImpl>();
    auto applicationContext = ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    EXPECT_NE(handler, nullptr);
    
    autoFillExtension->Init(record, application, handler, token);
    EXPECT_TRUE(autoFillExtension->context_ != nullptr);
    GTEST_LOG_(INFO) << "Init_0100 end";
}

/**
 * @tc.name: CreateAndInitContext_0100
 * @tc.desc: Verify that CreateAndInitContext function calls normally.
 * @tc.type: FUNC
 */
HWTEST_F(AutoFillExtensionTest, CreateAndInitContext_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateAndInitContext_0100 start";

    Runtime::Options options;
    std::unique_ptr<Runtime> runtime = Runtime::Create(options);
    auto autoFillExtension = AutoFillExtension::Create(runtime);
    EXPECT_NE(autoFillExtension, nullptr);

    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->applicationInfo.multiProjects = true;
    abilityInfo->moduleName = MODULE_NAME;
    abilityInfo->bundleName = BUNDLE_NAME;
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    EXPECT_NE(token, nullptr);
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(abilityInfo, token);
    EXPECT_NE(record, nullptr);
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    EXPECT_NE(application, nullptr);
    auto contextImpl = std::make_shared<ContextImpl>();
    auto applicationContext = ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    EXPECT_NE(handler, nullptr);
    
    auto context = autoFillExtension->CreateAndInitContext(record, application, handler, token);
    EXPECT_NE(context, nullptr);

    EXPECT_EQ(context->GetToken(), token);
    auto appContext = Context::GetApplicationContext();
    auto appInfo = appContext->GetApplicationInfo();
    EXPECT_EQ(context->GetApplicationInfo(), appInfo);

    auto resourceManager = appContext->GetResourceManager();
    EXPECT_EQ(context->GetResourceManager(), resourceManager);
    EXPECT_EQ(context->parentContext_, appContext);
    
    EXPECT_EQ(context->GetAbilityInfo(), abilityInfo);
    EXPECT_EQ(context->GetConfiguration(), appContext->GetConfiguration());

    GTEST_LOG_(INFO) << "CreateAndInitContext_0100 end";
}

/**
 * @tc.name: CreateAndInitContext_0200
 * @tc.desc: Check nullptr AbilityLocalRecord.
 * @tc.type: FUNC
 */
HWTEST_F(AutoFillExtensionTest, CreateAndInitContext_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateAndInitContext_0200 start";

    Runtime::Options options;
    std::unique_ptr<Runtime> runtime = Runtime::Create(options);
    auto autoFillExtension = AutoFillExtension::Create(runtime);
    EXPECT_NE(autoFillExtension, nullptr);

    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->moduleName = MODULE_NAME;
    abilityInfo->bundleName = BUNDLE_NAME;
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    EXPECT_NE(token, nullptr);
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    EXPECT_NE(application, nullptr);
    auto contextImpl = std::make_shared<ContextImpl>();
    auto applicationContext = ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    EXPECT_NE(handler, nullptr);
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record = nullptr;
    
    auto context = autoFillExtension->CreateAndInitContext(record, application, handler, token);
    EXPECT_NE(context, nullptr);

    EXPECT_EQ(context->GetToken(), token);
    auto appContext = Context::GetApplicationContext();
    auto appInfo = appContext->GetApplicationInfo();
    EXPECT_EQ(context->GetApplicationInfo(), appInfo);

    auto resourceManager = appContext->GetResourceManager();
    EXPECT_EQ(context->GetResourceManager(), resourceManager);
    EXPECT_EQ(context->parentContext_, appContext);
    
    EXPECT_EQ(context->GetAbilityInfo(), nullptr);
    EXPECT_EQ(context->GetConfiguration(), nullptr);

    GTEST_LOG_(INFO) << "CreateAndInitContext_0200 end";
}
} // namespace AbilityRuntime
} // namespace OHOS