/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "event_report.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class EventReportTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void EventReportTest::SetUpTestCase(void)
{}
void EventReportTest::TearDownTestCase(void)
{}
void EventReportTest::SetUp()
{}
void EventReportTest::TearDown()
{}

/**
 * @tc.name: SendAppEvent_0100
 * @tc.desc: Check SendAppEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendAppEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(32);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0100
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(32);
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0200
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0200, TestSize.Level0)
{
    EventName eventName = EventName::START_ABILITY_ERROR;
    HiSysEventType type = HiSysEventType::FAULT;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0300
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0300, TestSize.Level0)
{
    EventName eventName = EventName::TERMINATE_ABILITY_ERROR;
    HiSysEventType type = HiSysEventType::FAULT;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0400
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0400, TestSize.Level0)
{
    EventName eventName = EventName::START_ABILITY;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0500
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0500, TestSize.Level0)
{
    EventName eventName = EventName::TERMINATE_ABILITY;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0600
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0600, TestSize.Level0)
{
    EventName eventName = EventName::CLOSE_ABILITY;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0700
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0700, TestSize.Level0)
{
    EventName eventName = EventName::ABILITY_ONFOREGROUND;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0800
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0800, TestSize.Level0)
{
    EventName eventName = EventName::ABILITY_ONBACKGROUND;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0900
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0900, TestSize.Level0)
{
    EventName eventName = EventName::ABILITY_ONACTIVE;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_1000
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_1000, TestSize.Level0)
{
    EventName eventName = EventName::ABILITY_ONINACTIVE;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0100
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(32);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0200
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0200, TestSize.Level0)
{
    EventName eventName = EventName::DISCONNECT_SERVICE;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0300
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0300, TestSize.Level0)
{
    EventName eventName = EventName::CONNECT_SERVICE;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0400
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0400, TestSize.Level0)
{
    EventName eventName = EventName::START_SERVICE;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0500
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0500, TestSize.Level0)
{
    EventName eventName = EventName::STOP_SERVICE;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0600
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0600, TestSize.Level0)
{
    EventName eventName = EventName::START_EXTENSION_ERROR;
    HiSysEventType type = HiSysEventType::FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0700
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0700, TestSize.Level0)
{
    EventName eventName = EventName::STOP_EXTENSION_ERROR;
    HiSysEventType type = HiSysEventType::FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0800
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0800, TestSize.Level0)
{
    EventName eventName = EventName::CONNECT_SERVICE_ERROR;
    HiSysEventType type = HiSysEventType::FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0900
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0900, TestSize.Level0)
{
    EventName eventName = EventName::DISCONNECT_SERVICE_ERROR;
    HiSysEventType type = HiSysEventType::FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendFormEvent_0100
 * @tc.desc: Check SendFormEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendFormEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(32);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendFormEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendFormEvent_0200
 * @tc.desc: Check SendFormEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendFormEvent_0200, TestSize.Level0)
{
    EventName eventName = EventName::DELETE_INVALID_FORM;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendFormEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendFormEvent_0300
 * @tc.desc: Check SendFormEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendFormEvent_0300, TestSize.Level0)
{
    EventName eventName = EventName::ACQUIREFORMSTATE_FORM;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendFormEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendFormEvent_0400
 * @tc.desc: Check SendFormEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendFormEvent_0400, TestSize.Level0)
{
    EventName eventName = EventName::MESSAGE_EVENT_FORM;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendFormEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendFormEvent_0500
 * @tc.desc: Check SendFormEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendFormEvent_0500, TestSize.Level0)
{
    EventName eventName = EventName::ADD_FORM;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendFormEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendFormEvent_0600
 * @tc.desc: Check SendFormEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendFormEvent_0600, TestSize.Level0)
{
    EventName eventName = EventName::REQUEST_FORM;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendFormEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendFormEvent_0700
 * @tc.desc: Check SendFormEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendFormEvent_0700, TestSize.Level0)
{
    EventName eventName = EventName::ROUTE_EVENT_FORM;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendFormEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendFormEvent_0800
 * @tc.desc: Check SendFormEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendFormEvent_0800, TestSize.Level0)
{
    EventName eventName = EventName::DELETE_FORM;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendFormEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendFormEvent_0900
 * @tc.desc: Check SendFormEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendFormEvent_0900, TestSize.Level0)
{
    EventName eventName = EventName::CASTTEMP_FORM;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendFormEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendFormEvent_1000
 * @tc.desc: Check SendFormEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendFormEvent_1000, TestSize.Level0)
{
    EventName eventName = EventName::RELEASE_FORM;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendFormEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendFormEvent_1100
 * @tc.desc: Check SendFormEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI57ZSO
 */
HWTEST_F(EventReportTest, SendFormEvent_1100, TestSize.Level0)
{
    EventName eventName = EventName::SET_NEXT_REFRESH_TIME_FORM;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendFormEvent(eventName, type, eventInfo);
}
}  // namespace AAFwk
}  // namespace OHOS
