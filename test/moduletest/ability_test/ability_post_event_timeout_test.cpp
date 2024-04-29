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
#include "ability_post_event_timeout.h"
#undef protected
#undef private
#include "ability_handler.h"
#include "event_handler.h"
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AppExecFwk {
class AbilityPostEventTimeoutTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityPostEventTimeout> abilityPostEventTimeout_;
};
void AbilityPostEventTimeoutTest::SetUpTestCase(void) {}
void AbilityPostEventTimeoutTest::TearDownTestCase(void) {}
void AbilityPostEventTimeoutTest::SetUp(void) {}
void AbilityPostEventTimeoutTest::TearDown(void) {}

/**
 * @tc.number: AaFwk_Ability_Context_TimingBegin_001
 * @tc.name: TimingBegin
 * @tc.desc: Ability Post Event Timeout and handler_ is nullptr.
 */
HWTEST_F(AbilityPostEventTimeoutTest, AaFwk_Ability_Context_TimingBegin_001, Function | MediumTest | Level1)
{
    std::string str = "";
    std::shared_ptr<AbilityHandler> eventHandler = nullptr;
    abilityPostEventTimeout_ = std::make_shared<AbilityPostEventTimeout>(str, eventHandler);
    int64_t delaytime = 1;
    abilityPostEventTimeout_->TimingBegin(delaytime);
    EXPECT_TRUE(abilityPostEventTimeout_->handler_ == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_TimingBegin_002
 * @tc.name: TimingBegin
 * @tc.desc: Ability Post Event Timeout and handler_ Not empty.
 */
HWTEST_F(AbilityPostEventTimeoutTest, AaFwk_Ability_Context_TimingBegin_002, Function | MediumTest | Level1)
{
    std::string str = "";
    std::shared_ptr<EventRunner> runner = nullptr;
    std::shared_ptr<AbilityHandler> eventHandler = std::make_shared<AbilityHandler>(runner);
    abilityPostEventTimeout_ = std::make_shared<AbilityPostEventTimeout>(str, eventHandler);
    int64_t delaytime = 1;
    abilityPostEventTimeout_->TimingBegin(delaytime);
    EXPECT_TRUE(abilityPostEventTimeout_->handler_ != nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_TimeEnd_001
 * @tc.name: TimeEnd
 * @tc.desc: Verification function TimeEnd and the result is handler_ is nullptr.
 */
HWTEST_F(AbilityPostEventTimeoutTest, AaFwk_Ability_Context_TimeEnd_001, Function | MediumTest | Level1)
{
    std::string str = "";
    std::shared_ptr<AbilityHandler> eventHandler = nullptr;
    abilityPostEventTimeout_ = std::make_shared<AbilityPostEventTimeout>(str, eventHandler);
    abilityPostEventTimeout_->TimeEnd();
    EXPECT_TRUE(abilityPostEventTimeout_->handler_ == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_TimeEnd_002
 * @tc.name: TimeEnd
 * @tc.desc: Verification function TimeEnd and the result is handler_ isn't nullptr.
 */
HWTEST_F(AbilityPostEventTimeoutTest, AaFwk_Ability_Context_TimeEnd_002, Function | MediumTest | Level1)
{
    std::string str = "";
    std::shared_ptr<EventRunner> runner = nullptr;
    std::shared_ptr<AbilityHandler> eventHandler = std::make_shared<AbilityHandler>(runner);
    abilityPostEventTimeout_ = std::make_shared<AbilityPostEventTimeout>(str, eventHandler);
    abilityPostEventTimeout_->TimeEnd();
    EXPECT_TRUE(abilityPostEventTimeout_->handler_ != nullptr);
    EXPECT_TRUE(abilityPostEventTimeout_->taskExec_);
}

/**
 * @tc.number: AaFwk_Ability_Context_TimeEnd_003
 * @tc.name: TimeEnd
 * @tc.desc: Verification function TimeEnd and the result is handler_ isn't nullptr.
 */
HWTEST_F(AbilityPostEventTimeoutTest, AaFwk_Ability_Context_TimeEnd_003, Function | MediumTest | Level1)
{
    std::string str = "";
    std::shared_ptr<EventRunner> runner = nullptr;
    std::shared_ptr<AbilityHandler> eventHandler = std::make_shared<AbilityHandler>(runner);
    abilityPostEventTimeout_ = std::make_shared<AbilityPostEventTimeout>(str, eventHandler);
    abilityPostEventTimeout_->taskExec_ = true;
    abilityPostEventTimeout_->TimeEnd();
    EXPECT_TRUE(abilityPostEventTimeout_->handler_ != nullptr);
    EXPECT_TRUE(abilityPostEventTimeout_->taskExec_);
}

/**
 * @tc.number: AaFwk_Ability_Context_TimeOutProc_001
 * @tc.name: TimeOutProc
 * @tc.desc: Verification function TimeOutProc and the result is handler_ is nullptr.
 */
HWTEST_F(AbilityPostEventTimeoutTest, AaFwk_Ability_Context_TimeOutProc_001, Function | MediumTest | Level1)
{
    std::string str = "";
    std::shared_ptr<AbilityHandler> eventHandler = nullptr;
    abilityPostEventTimeout_ = std::make_shared<AbilityPostEventTimeout>(str, eventHandler);
    abilityPostEventTimeout_->TimeOutProc();
    EXPECT_TRUE(abilityPostEventTimeout_->handler_ == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_TimeOutProc_002
 * @tc.name: TimeOutProc
 * @tc.desc: Verification function TimeOutProc and the result is handler_ isn't nullptr.
 */
HWTEST_F(AbilityPostEventTimeoutTest, AaFwk_Ability_Context_TimeOutProc_002, Function | MediumTest | Level1)
{
    std::string str = "";
    std::shared_ptr<EventRunner> runner = nullptr;
    std::shared_ptr<AbilityHandler> eventHandler = std::make_shared<AbilityHandler>(runner);
    abilityPostEventTimeout_ = std::make_shared<AbilityPostEventTimeout>(str, eventHandler);
    abilityPostEventTimeout_->TimeOutProc();
    EXPECT_TRUE(abilityPostEventTimeout_->handler_ != nullptr);
    EXPECT_TRUE(abilityPostEventTimeout_->taskExec_);
}

/**
 * @tc.number: AaFwk_Ability_Context_TimeOutProc_003
 * @tc.name: TimeOutProc
 * @tc.desc: Verification function TimeOutProc and the result is handler_ isn't nullptr.
 */
HWTEST_F(AbilityPostEventTimeoutTest, AaFwk_Ability_Context_TimeOutProc_003, Function | MediumTest | Level3)
{
    std::string str = "";
    std::shared_ptr<EventRunner> runner = nullptr;
    std::shared_ptr<AbilityHandler> eventHandler = std::make_shared<AbilityHandler>(runner);
    abilityPostEventTimeout_ = std::make_shared<AbilityPostEventTimeout>(str, eventHandler);
    abilityPostEventTimeout_->taskExec_ = true;
    abilityPostEventTimeout_->TimeOutProc();
    EXPECT_TRUE(abilityPostEventTimeout_->handler_ != nullptr);
    EXPECT_TRUE(abilityPostEventTimeout_->taskExec_);
}
} // namespace AppExecFwk
} // namespace OHOS