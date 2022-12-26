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
#include "ability_handler.h"
#include "ohos_application.h"
#undef private

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;

class AbilityHandlerTest : public testing::Test {
public:
    AbilityHandlerTest() : abilityhandler_(nullptr)
    {}
    ~AbilityHandlerTest()
    {}
    std::shared_ptr<AbilityHandler> abilityhandler_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityHandlerTest::SetUpTestCase(void)
{}

void AbilityHandlerTest::TearDownTestCase(void)
{}

void AbilityHandlerTest::SetUp(void)
{}

void AbilityHandlerTest::TearDown(void)
{}

/**
 * @tc.number: Ability_Handler_ProcessEvent_0100
 * @tc.name: ProcessEvent
 * @tc.desc: call ProcessEvent and test constructor
 */
HWTEST_F(AbilityHandlerTest, Ability_Handler_ProcessEvent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Handler_ProcessEvent_0100 start";
    std::shared_ptr<EventRunner> runner = EventRunner::Create(nullptr);
    abilityhandler_ = std::make_shared<AbilityHandler>(runner);
    EXPECT_TRUE(abilityhandler_->eventRunner_ != nullptr);
    InnerEvent::Pointer event = InnerEvent::Get();
    abilityhandler_->ProcessEvent(event);
    GTEST_LOG_(INFO) << "Ability_Handler_ProcessEvent_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
