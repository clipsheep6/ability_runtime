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
#include "system_time.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AAFwk {
namespace {
} // namespace

class SystemTimeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SystemTimeTest::SetUpTestCase()
{}

void SystemTimeTest::TearDownTestCase()
{}

void SystemTimeTest::SetUp()
{}

void SystemTimeTest::TearDown()
{}

/**
 * @tc.name: System_Time_Get_Now_System_0100
 * @tc.desc: "Get System Time" test.
 * @tc.type: FUNC
 */
HWTEST_F(SystemTimeTest, System_Time_Get_Now_System_0100, TestSize.Level1)
{
    SystemTime systemTime;
    EXPECT_GT(systemTime.GetNowSysTime(), 0);
}
} // namespace AAFwk
} // namespace OHOS