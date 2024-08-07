/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "runtime.h"
#include "live_form_extension.h"
 
namespace OHOS {
namespace AbilityRuntime {
using namespace testing::ext;
 
class LiveFormExtensionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
 
void LiveFormExtensionTest::SetUpTestCase(void)
{}
 
void LiveFormExtensionTest::TearDownTestCase(void)
{}
 
void LiveFormExtensionTest::SetUp(void)
{}
 
void LiveFormExtensionTest::TearDown(void)
{}
 
/**
 * @tc.number: Live_Form_Extension_0100
 * @tc.name: Create
 * @tc.desc: The runtime is nullptr, and the verification of create succeeds.
 */
HWTEST_F(LiveFormExtensionTest, Live_Form_Extension_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Live_Form_Extension_0100 start";
    std::unique_ptr<Runtime> runtime;
    auto liveFormExtension = LiveFormExtension::Create(runtime);
    EXPECT_TRUE(liveFormExtension != nullptr);
    GTEST_LOG_(INFO) << "Live_Form_Extension_0100 end";
}
 
/**
 * @tc.number: Live_Form_Extension_0200
 * @tc.name: Create
 * @tc.desc: The language is js, and the verification of create is successful.
 */
HWTEST_F(LiveFormExtensionTest, Live_Form_Extension_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Live_Form_Extension_0200 start";
    Runtime::Options options;
    auto runtime = Runtime::Create(options);
    auto liveFormExtension = LiveFormExtension::Create(runtime);
    EXPECT_TRUE(liveFormExtension != nullptr);
    GTEST_LOG_(INFO) << "Live_Form_Extension_0200 end";
}
 
/**
 * @tc.number: Live_Form_Extension_0300
 * @tc.name: Init
 * @tc.desc: Validation initialization succeeded.
 */
HWTEST_F(LiveFormExtensionTest, Live_Form_Extension_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Live_Form_Extension_0300 start";
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(abilityInfo, token);
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    auto contextImpl = std::make_shared<ContextImpl>();
    auto applicationContext = ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    Runtime::Options options;
    std::unique_ptr<Runtime> runtime = Runtime::Create(options);
    auto liveFormExtension = LiveFormExtension::Create(runtime);
    liveFormExtension->Init(record, application, handler, token);
    EXPECT_TRUE(liveFormExtension != nullptr);
    GTEST_LOG_(INFO) << "Live_Form_Extension_0300 end";
}
} // namespace AbilityRuntime
} // namespace OHOS