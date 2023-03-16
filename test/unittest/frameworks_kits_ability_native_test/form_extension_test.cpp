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
#include "form_extension.h"
#undef private
#undef protected
#include "ability_handler.h"
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "runtime.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class FormExtensionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FormExtensionTest::SetUpTestCase(void)
{
}

void FormExtensionTest::TearDownTestCase(void)
{
}

void FormExtensionTest::SetUp(void)
{
}

void FormExtensionTest::TearDown(void)
{
}

/**
 * @tc.number: AaFwk_Form_Extension_0100
 * @tc.name: Create
 * @tc.desc: The runtime is nullptr, and the verification of create succeeds.
 */
HWTEST_F(FormExtensionTest, AaFwk_Form_Extension_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Extension_0100 start";
    std::unique_ptr<AbilityRuntime::Runtime> runtime;
    auto formExtension = AbilityRuntime::FormExtension::Create(runtime);
    EXPECT_TRUE(formExtension != nullptr);
    GTEST_LOG_(INFO) << "AaFwk_Extension_0100 end";
}

/**
 * @tc.number: AaFwk_Form_Extension_0200
 * @tc.name: Create
 * @tc.desc: The language is js, and the verification of create is successful.
 */
HWTEST_F(FormExtensionTest, AaFwk_Form_Extension_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0200 start";
    AbilityRuntime::Runtime::Options options;
    std::unique_ptr<AbilityRuntime::Runtime> runtime = AbilityRuntime::Runtime::Create(options);
    auto formExtension = AbilityRuntime::FormExtension::Create(runtime);
    EXPECT_TRUE(formExtension != nullptr);
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0200 end";
}

/**
 * @tc.number: AaFwk_Form_Extension_0300
 * @tc.name: Init
 * @tc.desc: Validation initialization succeeded.
 */
HWTEST_F(FormExtensionTest, AaFwk_Form_Extension_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0300 start";
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    AbilityRuntime::Runtime::Options options;
    std::unique_ptr<AbilityRuntime::Runtime> runtime = AbilityRuntime::Runtime::Create(options);
    auto formExtension = AbilityRuntime::FormExtension::Create(runtime);
    formExtension->Init(record, application, handler, token);
    EXPECT_TRUE(formExtension != nullptr);
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0300 end";
}

/**
 * @tc.number: AaFwk_Form_Extension_0400
 * @tc.name: CreateAndInitContext
 * @tc.desc: record is nullptr, Validation CreateAndInitContext succeeded.
 */
HWTEST_F(FormExtensionTest, AaFwk_Form_Extension_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0400 start";
    std::shared_ptr<AbilityLocalRecord> record;
    std::shared_ptr<OHOSApplication> application;
    std::shared_ptr<AbilityHandler> handler;
    sptr<IRemoteObject> token = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::unique_ptr<AbilityRuntime::Runtime> runtime = AbilityRuntime::Runtime::Create(options);
    auto formExtension = AbilityRuntime::FormExtension::Create(runtime);
    auto context = formExtension->CreateAndInitContext(record, application, handler, token);
    EXPECT_TRUE(context != nullptr);
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0400 end";
}

/**
 * @tc.number: AaFwk_Form_Extension_0500
 * @tc.name: CreateAndInitContext
 * @tc.desc: record is not nullptr, Validation CreateAndInitContext succeeded.
 */
HWTEST_F(FormExtensionTest, AaFwk_Form_Extension_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0500 start";
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AbilityLocalRecord>(info, token);
    std::shared_ptr<OHOSApplication> application;
    std::shared_ptr<AbilityHandler> handler;
    AbilityRuntime::Runtime::Options options;
    std::unique_ptr<AbilityRuntime::Runtime> runtime = AbilityRuntime::Runtime::Create(options);
    auto formExtension = AbilityRuntime::FormExtension::Create(runtime);
    auto context = formExtension->CreateAndInitContext(record, application, handler, token);
    EXPECT_TRUE(context != nullptr);
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0500 end";
}

/**
 * @tc.number: AaFwk_Form_Extension_0600
 * @tc.name: OnCreate
 * @tc.desc: Validation OnCreate succeeded.
 */
HWTEST_F(FormExtensionTest, AaFwk_Form_Extension_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0600 start";
    AbilityRuntime::Runtime::Options options;
    std::unique_ptr<AbilityRuntime::Runtime> runtime = AbilityRuntime::Runtime::Create(options);
    auto formExtension = AbilityRuntime::FormExtension::Create(runtime);
    Want want;
    auto info = formExtension->OnCreate(want);
    EXPECT_FALSE(info.GetUpgradeFlg());
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0600 end";
}

/**
 * @tc.number: AaFwk_Form_Extension_0700
 * @tc.name: OnDestroy
 * @tc.desc: Validation OnDestroy succeeded.
 */
HWTEST_F(FormExtensionTest, AaFwk_Form_Extension_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0700 start";
    AbilityRuntime::Runtime::Options options;
    std::unique_ptr<AbilityRuntime::Runtime> runtime = AbilityRuntime::Runtime::Create(options);
    auto formExtension = AbilityRuntime::FormExtension::Create(runtime);
    int64_t formId = 0;
    formExtension->OnDestroy(formId);
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0700 end";
}

/**
 * @tc.number: AaFwk_Form_Extension_0800
 * @tc.name: OnEvent
 * @tc.desc: Validation OnEvent succeeded.
 */
HWTEST_F(FormExtensionTest, AaFwk_Form_Extension_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0800 start";
    AbilityRuntime::Runtime::Options options;
    std::unique_ptr<AbilityRuntime::Runtime> runtime = AbilityRuntime::Runtime::Create(options);
    auto formExtension = AbilityRuntime::FormExtension::Create(runtime);
    int64_t formId = 0;
    std::string message;
    formExtension->OnEvent(formId, message);
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0800 end";
}

/**
 * @tc.number: AaFwk_Form_Extension_0900
 * @tc.name: OnUpdate
 * @tc.desc: Validation OnUpdate succeeded.
 */
HWTEST_F(FormExtensionTest, AaFwk_Form_Extension_0900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0900 start";
    AbilityRuntime::Runtime::Options options;
    std::unique_ptr<AbilityRuntime::Runtime> runtime = AbilityRuntime::Runtime::Create(options);
    auto formExtension = AbilityRuntime::FormExtension::Create(runtime);
    int64_t formId = 0;
    formExtension->OnUpdate(formId);
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_0900 end";
}

/**
 * @tc.number: AaFwk_Form_Extension_1000
 * @tc.name: OnUpdate
 * @tc.desc: Validation OnUpdate succeeded.
 */
HWTEST_F(FormExtensionTest, AaFwk_Form_Extension_1000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_1000 start";
    AbilityRuntime::Runtime::Options options;
    std::unique_ptr<AbilityRuntime::Runtime> runtime = AbilityRuntime::Runtime::Create(options);
    auto formExtension = AbilityRuntime::FormExtension::Create(runtime);
    int64_t formId = 0;
    formExtension->OnCastToNormal(formId);
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_1000 end";
}

/**
 * @tc.number: AaFwk_Form_Extension_1100
 * @tc.name: OnVisibilityChange
 * @tc.desc: Validation OnVisibilityChange succeeded.
 */
HWTEST_F(FormExtensionTest, AaFwk_Form_Extension_1100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_1100 start";
    AbilityRuntime::Runtime::Options options;
    std::unique_ptr<AbilityRuntime::Runtime> runtime = AbilityRuntime::Runtime::Create(options);
    auto formExtension = AbilityRuntime::FormExtension::Create(runtime);
    std::map<int64_t, int32_t> formEventsMap;
    formExtension->OnVisibilityChange(formEventsMap);
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_1100 end";
}

/**
 * @tc.number: AaFwk_Form_Extension_1200
 * @tc.name: OnAcquireFormState
 * @tc.desc: Validation OnAcquireFormState succeeded.
 */
HWTEST_F(FormExtensionTest, AaFwk_Form_Extension_1200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_1200 start";
    AbilityRuntime::Runtime::Options options;
    std::unique_ptr<AbilityRuntime::Runtime> runtime = AbilityRuntime::Runtime::Create(options);
    auto formExtension = AbilityRuntime::FormExtension::Create(runtime);
    Want want;
    auto state = formExtension->OnAcquireFormState(want);
    EXPECT_EQ(state, FormState::DEFAULT);
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_1200 end";
}

/**
 * @tc.number: AaFwk_Form_Extension_1300
 * @tc.name: OnShare
 * @tc.desc: Validation OnShare succeeded.
 */
HWTEST_F(FormExtensionTest, AaFwk_Form_Extension_1300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_1300 start";
    AbilityRuntime::Runtime::Options options;
    std::unique_ptr<AbilityRuntime::Runtime> runtime = AbilityRuntime::Runtime::Create(options);
    auto formExtension = AbilityRuntime::FormExtension::Create(runtime);
    int64_t formId = 0;
    AAFwk::WantParams params;
    auto result = formExtension->OnShare(formId, params);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AaFwk_Form_Extension_1300 end";
}
} // namespace AppExecFwk
} // namespace OHOS