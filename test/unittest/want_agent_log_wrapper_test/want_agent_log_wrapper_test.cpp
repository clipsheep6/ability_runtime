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
#define protected public
#include "want_agent_log_wrapper.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AbilityRuntime::WantAgent;

namespace OHOS::AbilityRuntime::WantAgent {
class WantAgentLogWrapperTest : public testing::Test {
public:
    WantAgentLogWrapperTest()
    {}
    ~WantAgentLogWrapperTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void WantAgentLogWrapperTest::SetUpTestCase(void)
{}

void WantAgentLogWrapperTest::TearDownTestCase(void)
{}

void WantAgentLogWrapperTest::SetUp(void)
{}

void WantAgentLogWrapperTest::TearDown(void)
{}

/*
 * @tc.number    : SetLogLevel_0100
 * @tc.name      : SetLogLevel
 * @tc.desc      : Test SetLogLevel
 */
HWTEST_F(WantAgentLogWrapperTest, SetLogLevel_0100, Function | MediumTest | Level1)
{
    WantAgentLogLevel level = WantAgentLogLevel::ERROR;
    WantAgentLogWrapper::SetLogLevel(level);
    EXPECT_EQ(WantAgentLogWrapper::GetLogLevel(), level);
}

}  // namespace OHOS::AbilityRuntime::WantAgent
