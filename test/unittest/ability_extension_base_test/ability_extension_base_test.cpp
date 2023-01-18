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
#define protected public
#include "extension_base.h"
#undef private
#undef protected

#include "ability_handler.h"
#include "ability_transaction_callback_info.h"
#include "configuration.h"
#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "extension_context.h"
#include "js_runtime.h"
#include "js_extension_common.h"
#include "want.h"

using namespace testing::ext;
using OHOS::AppExecFwk::ElementName;

namespace OHOS {
namespace AbilityRuntime {
class AbilityExtensionBaseTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityExtensionBaseTest::SetUpTestCase(void)
{}

void AbilityExtensionBaseTest::TearDownTestCase(void)
{}

void AbilityExtensionBaseTest::SetUp()
{}

void AbilityExtensionBaseTest::TearDown()
{}

/**
 * @tc.name: Init_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityExtensionBaseTest, Init_0100, TestSize.Level1)
{
    HILOG_INFO("Init start");

    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record = nullptr;
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = nullptr;

    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);

    ExtensionBase<ExtensionContext> extensionBase;
    extensionBase.Init(record, application, handler, token);
    EXPECT_TRUE(true);

    HILOG_INFO("Init end");
}

/**
 * @tc.name: CreateAndInitContext_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityExtensionBaseTest, CreateAndInitContext_0100, TestSize.Level1)
{
    HILOG_INFO("CreateAndInitContext start");

    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "ExtensionBaseTest";
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();

    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);

    ExtensionBase<ExtensionContext> extensionBase;
    extensionBase.Init(record, application, handler, token);
    std::shared_ptr<ExtensionContext> context = extensionBase.CreateAndInitContext(record, application, handler, token);
    EXPECT_STREQ(context->GetAbilityInfo()->name.c_str(), "ExtensionBaseTest");
    EXPECT_TRUE(true);

    HILOG_INFO("CreateAndInitContext end");
}

/**
 * @tc.name: GetContext_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityExtensionBaseTest, GetContext_0100, TestSize.Level1)
{
    HILOG_INFO("GetContext start");

    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "ExtensionBaseTest";
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();

    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);

    ExtensionBase<ExtensionContext> extensionBase;
    extensionBase.Init(record, application, handler, token);
    std::shared_ptr<ExtensionContext> context = extensionBase.GetContext();
    EXPECT_STREQ(context->GetAbilityInfo()->name.c_str(), "ExtensionBaseTest");
    EXPECT_TRUE(true);

    HILOG_INFO("GetContext end");
}

/**
 * @tc.name: OnConfigurationUpdated_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityExtensionBaseTest, OnConfigurationUpdated_0100, TestSize.Level1)
{
    HILOG_INFO("OnConfigurationUpdated start");

    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();

    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);

    AppExecFwk::Configuration configuration;
    application->SetConfiguration(configuration);

    ExtensionBase<ExtensionContext> extensionBase;
    extensionBase.Init(record, application, handler, token);
    extensionBase.OnConfigurationUpdated(configuration);
    EXPECT_TRUE(true);

    HILOG_INFO("OnConfigurationUpdated end");
}

/**
 * @tc.name: OnMemoryLevel_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityExtensionBaseTest, OnMemoryLevel_0100, TestSize.Level1)
{
    HILOG_INFO("OnMemoryLevel start");

    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();

    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);

    ExtensionBase<ExtensionContext> extensionBase;
    extensionBase.Init(record, application, handler, token);

    int level = 0;
    extensionBase.OnMemoryLevel(level);
    EXPECT_TRUE(true);

    HILOG_INFO("OnMemoryLevel end");
}

/**
 * @tc.name: SetExtensionCommon_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityExtensionBaseTest, SetExtensionCommon_0100, TestSize.Level1)
{
    HILOG_INFO("SetExtensionCommon start");

    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();

    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);

    ExtensionBase<ExtensionContext> extensionBase;
    extensionBase.Init(record, application, handler, token);

    Runtime::Options options;
    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options);
    std::unique_ptr<NativeReference> jsObj;
    extensionBase.SetExtensionCommon(JsExtensionCommon::Create(
        static_cast<JsRuntime&>(*jsRuntime), static_cast<NativeReference&>(*jsObj), nullptr));
    EXPECT_NE(extensionBase.extensionCommon_, nullptr);
    EXPECT_TRUE(true);

    HILOG_INFO("SetExtensionCommon end");
}

}  // namespace AbilityRuntime
}  // namespace OHOS
