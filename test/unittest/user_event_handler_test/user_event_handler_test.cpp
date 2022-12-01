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
#include "user_event_handler.h"
#undef private

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class UserEventHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UserEventHandlerTest::SetUpTestCase(void)
{}
void UserEventHandlerTest::TearDownTestCase(void)
{}
void UserEventHandlerTest::SetUp(void)
{}
void UserEventHandlerTest::TearDown(void)
{}

/*
 * Feature: UserEventHandler
 * Function: ProcessEvent
 * SubFunction: NA
 * FunctionPoints: UserEventHandler ProcessEvent
 * EnvConditions: NA
 * CaseDescription: Verify ProcessEvent
 */
HWTEST_F(UserEventHandlerTest, ProcessEvent_001, TestSize.Level1)
{
    std::shared_ptr<EventRunner> runner;
    std::weak_ptr<UserController> owner;
    std::shared_ptr<UserEventHandler> handler = std::make_shared<UserEventHandler>(runner, owner);
    InnerEvent::Pointer event = InnerEvent::Get();
    event.reset();
    handler->ProcessEvent(event);
}

/*
 * Feature: UserEventHandler
 * Function: ProcessEvent
 * SubFunction: NA
 * FunctionPoints: UserEventHandler ProcessEvent
 * EnvConditions: NA
 * CaseDescription: Verify ProcessEvent
 */
HWTEST_F(UserEventHandlerTest, ProcessEvent_002, TestSize.Level1)
{
    std::shared_ptr<EventRunner> runner;
    std::weak_ptr<UserController> owner;
    std::shared_ptr<UserEventHandler> handler = std::make_shared<UserEventHandler>(runner, owner);
    InnerEvent::Pointer event = InnerEvent::Get();
    handler->ProcessEvent(event);
}
}  // namespace AAFwk
}  // namespace OHOS
