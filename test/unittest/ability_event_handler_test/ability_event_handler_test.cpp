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

#include <parameter.h>
#include "ability_manager_service.h"
#include "ability_event_handler.h"
#include "hilog_wrapper.h"

using namespace testing::ext;
namespace OHOS {
namespace AAFwk {

class AbilityEventHandlerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityEventHandlerTest::SetUpTestCase()
{}

void AbilityEventHandlerTest::TearDownTestCase()
{}

void AbilityEventHandlerTest::SetUp()
{}

void AbilityEventHandlerTest::TearDown()
{}

/*
 * Feature: Ability Event Handler
 * Function: ProcessEvent
 * SubFunction: NA
 * FunctionPoints: Ability Event Handler ProcessEvent
 * EnvConditions: NA
 * CaseDescription: Verify ProcessEvent
 */
HWTEST_F(AbilityEventHandlerTest, ability_event_handler_001, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    std::weak_ptr<AbilityManagerService> server;
    std::shared_ptr<UserEvent> eventData = std::make_shared<UserEvent>();
    auto event = AppExecFwk::InnerEvent::Get(UserEventHandler::EVENT_SYSTEM_USER_START, eventData);
    auto handler = std::make_shared<AbilityEventHandler>(runner, server);
    SetParameter("libc.hook_mode", "startup:");
    handler->ProcessEvent(event);
    SetParameter("libc.hook_mode", "test_parameter");
    handler->ProcessEvent(event);
    auto event2 = event->Get(AbilityManagerService::LOAD_TIMEOUT_MSG, event->GetParam());
    std::string str = std::to_string(event->GetInnerEventId());
    handler->ProcessEvent(event2);
    event2 = event->Get(AbilityManagerService::ACTIVE_TIMEOUT_MSG, event->GetParam());
    handler->ProcessEvent(event2);
    event2 = event->Get(AbilityManagerService::INACTIVE_TIMEOUT_MSG, event->GetParam());
    handler->ProcessEvent(event2);
    event2 = event->Get(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, event->GetParam());
    handler->ProcessEvent(event2);
    event2 = event->Get(AbilityManagerService::BACKGROUND_TIMEOUT_MSG, event->GetParam());
    handler->ProcessEvent(event2);
}  // namespace AppExecFwk
}  // namespace OHOS
}