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

#include "startup_util.h"
#include "want.h"

using namespace testing::ext;
namespace OHOS {
namespace AbilityRuntime {

class StartupUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void StartupUtilTest::SetUpTestCase()
{}

void StartupUtilTest::TearDownTestCase()
{}

void StartupUtilTest::SetUp()
{}

void StartupUtilTest::TearDown()
{}

/**
 * @tc.name: startup_util_test_001
 * @tc.desc: test class StartupUtil number function
 * @tc.type: FUNC
 */
HWTEST_F(StartupUtilTest, startup_util_test_001, TestSize.Level1)
{
    AAFwk::Want want;
    auto appTwinIndex = StartupUtil::GetAppTwinIndex(want);
    EXPECT_EQ(appTwinIndex, 0);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
