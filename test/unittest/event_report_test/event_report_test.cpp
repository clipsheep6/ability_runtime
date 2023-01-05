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
#include "event_report.h"

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
 * @tc.name: SendAbilityEvent_0100
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0100, TestSize.Level0)
{
    EventName eventName = EventName::ABILITY_ONACTIVE;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0100
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0200, TestSize.Level0)
{
    EventName eventName = EventName::ABILITY_ONINACTIVE;
    HiSysEventType type = HiSysEventType::BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}
}  // namespace AAFwk
}  // namespace OHOS
