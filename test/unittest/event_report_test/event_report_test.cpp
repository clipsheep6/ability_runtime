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
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAppEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAppEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAppEvent_0200
 * @tc.desc: Check SendAppEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI6HXXS
 */
HWTEST_F(EventReportTest, SendAppEvent_0200, TestSize.Level0)
{
    EventName eventName = EventName::APP_STARTUP_TYPE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "APP_STARTUP_TYPE");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAppEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAppEvent_0300
 * @tc.desc: Check SendAppEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI6HXXS
 */
HWTEST_F(EventReportTest, SendAppEvent_0300, TestSize.Level0)
{
    EventName eventName = EventName::DRAWN_COMPLETED;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "DRAWN_COMPLETED");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    eventInfo.extensionType = 0;
    EventReport::SendAppEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAppEvent_0400
 * @tc.desc: Check SendAppEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI6HXXS
 */
HWTEST_F(EventReportTest, SendAppEvent_0400, TestSize.Level0)
{
    EventName eventName = EventName::APP_ATTACH;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "APP_ATTACH");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAppEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0100
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0200
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0200, TestSize.Level0)
{
    EventName eventName = EventName::START_ABILITY_ERROR;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "START_ABILITY_ERROR");
    HiSysEventType type = HiSysEventType::FAULT;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0300
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0300, TestSize.Level0)
{
    EventName eventName = EventName::TERMINATE_ABILITY_ERROR;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "TERMINATE_ABILITY_ERROR");
    HiSysEventType type = HiSysEventType::FAULT;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0400
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0400, TestSize.Level0)
{
    EventName eventName = EventName::START_ABILITY;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "START_ABILITY");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0500
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0500, TestSize.Level0)
{
    EventName eventName = EventName::TERMINATE_ABILITY;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "TERMINATE_ABILITY");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0600
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0600, TestSize.Level0)
{
    EventName eventName = EventName::CLOSE_ABILITY;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "CLOSE_ABILITY");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0700
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0700, TestSize.Level0)
{
    EventName eventName = EventName::ABILITY_ONFOREGROUND;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "ABILITY_ONFOREGROUND");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0800
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0800, TestSize.Level0)
{
    EventName eventName = EventName::ABILITY_ONBACKGROUND;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "ABILITY_ONBACKGROUND");
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
    std::string name  = "ABILITY_ONACTIVE";
    EXPECT_EQ(EventReport::ConvertEventName(eventName), name);
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_1000
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI6A12D
 */
HWTEST_F(EventReportTest, SendAbilityEvent_1000, TestSize.Level0)
{
    EventName eventName = EventName::ABILITY_ONINACTIVE;
    std::string name  = "ABILITY_ONINACTIVE";
    EXPECT_EQ(EventReport::ConvertEventName(eventName), name);
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_1100
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_1100, TestSize.Level0)
{
    EventName eventName = EventName::DISCONNECT_SERVICE;
    std::string name  = "DISCONNECT_SERVICE";
    EXPECT_EQ(EventReport::ConvertEventName(eventName), name);
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0100
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0200
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0200, TestSize.Level0)
{
    EventName eventName = EventName::START_EXTENSION_ERROR;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "START_EXTENSION_ERROR");
    HiSysEventType type = HiSysEventType::FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0300
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0300, TestSize.Level0)
{
    EventName eventName = EventName::STOP_EXTENSION_ERROR;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "STOP_EXTENSION_ERROR");
    HiSysEventType type = HiSysEventType::FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0400
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0400, TestSize.Level0)
{
    EventName eventName = EventName::CONNECT_SERVICE_ERROR;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "CONNECT_SERVICE_ERROR");
    HiSysEventType type = HiSysEventType::FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0500
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0500, TestSize.Level0)
{
    EventName eventName = EventName::DISCONNECT_SERVICE_ERROR;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "DISCONNECT_SERVICE_ERROR");
    HiSysEventType type = HiSysEventType::FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0600
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0600, TestSize.Level0)
{
    EventName eventName = EventName::START_SERVICE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "START_SERVICE");
    HiSysEventType type = HiSysEventType::FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendKeyEvent_0100
 * @tc.desc: Check SendKeyEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendKeyEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    HiSysEventType type = HiSysEventType::FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendKeyEvent_0200
 * @tc.desc: Check SendKeyEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendKeyEvent_0200, TestSize.Level0)
{
    EventName eventName = EventName::GRANT_URI_PERMISSION;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "GRANT_URI_PERMISSION");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendKeyEvent_0300
 * @tc.desc: Check SendKeyEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendKeyEvent_0300, TestSize.Level0)
{
    EventName eventName = EventName::FA_SHOW_ON_LOCK;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "FA_SHOW_ON_LOCK");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendKeyEvent_0400
 * @tc.desc: Check SendKeyEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendKeyEvent_0400, TestSize.Level0)
{
    EventName eventName = EventName::START_PRIVATE_ABILITY;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "START_PRIVATE_ABILITY");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendKeyEvent_0500
 * @tc.desc: Check SendKeyEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendKeyEvent_0500, TestSize.Level0)
{
    EventName eventName = EventName::RESTART_PROCESS_BY_SAME_APP;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "RESTART_PROCESS_BY_SAME_APP");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendKeyEvent_0600
 * @tc.desc: Check SendKeyEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendKeyEvent_0600, TestSize.Level0)
{
    EventName eventName = EventName::START_SERVICE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "START_SERVICE");
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAppLaunchEvent_0100
 * @tc.desc: Check SendAppLaunchEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendAppLaunchEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendAppLaunchEvent(eventName, eventInfo);
    eventName = EventName::APP_LAUNCH;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "APP_LAUNCH");
    EventReport::SendAppLaunchEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendAppForegroundEvent_0100
 * @tc.desc: Check SendAppForegroundEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendAppForegroundEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendAppForegroundEvent(eventName, eventInfo);
    eventName = EventName::APP_FOREGROUND;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "APP_FOREGROUND");
    EventReport::SendAppForegroundEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendAppBackgroundEvent_0100
 * @tc.desc: Check SendAppBackgroundEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendAppBackgroundEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendAppBackgroundEvent(eventName, eventInfo);
    eventName = EventName::APP_BACKGROUND;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "APP_BACKGROUND");
    EventReport::SendAppBackgroundEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendProcessStartEvent_0100
 * @tc.desc: Check SendProcessStartEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendProcessStartEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendProcessStartEvent(eventName, eventInfo);
    eventName = EventName::PROCESS_START;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "PROCESS_START");
    EventReport::SendProcessStartEvent(eventName, eventInfo);
    eventInfo.extensionType = 0;
    EventReport::SendProcessStartEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendProcessExitEvent_0100
 * @tc.desc: Check SendProcessExitEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendProcessExitEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendProcessExitEvent(eventName, eventInfo);
    eventName = EventName::PROCESS_EXIT;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "PROCESS_EXIT");
    EventReport::SendProcessExitEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendStartServiceEvent_0100
 * @tc.desc: Check SendStartServiceEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendStartServiceEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendStartServiceEvent(eventName, eventInfo);
    eventName = EventName::START_SERVICE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "START_SERVICE");
    EventReport::SendStartServiceEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendStopServiceEvent_0100
 * @tc.desc: Check SendStopServiceEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendStopServiceEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendStopServiceEvent(eventName, eventInfo);
    eventName = EventName::STOP_SERVICE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "STOP_SERVICE");
    EventReport::SendStopServiceEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendConnectServiceEvent_0100
 * @tc.desc: Check SendConnectServiceEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendConnectServiceEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendConnectServiceEvent(eventName, eventInfo);
    eventName = EventName::CONNECT_SERVICE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "CONNECT_SERVICE");
    EventReport::SendConnectServiceEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendDisconnectServiceEvent_0100
 * @tc.desc: Check SendDisconnectServiceEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendDisconnectServiceEvent_0100, TestSize.Level0)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendDisconnectServiceEvent(eventName, eventInfo);
    eventName = EventName::DISCONNECT_SERVICE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "DISCONNECT_SERVICE");
    EventReport::SendDisconnectServiceEvent(eventName, eventInfo);
}
}  // namespace AAFwk
}  // namespace OHOS
