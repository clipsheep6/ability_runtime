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

#include "unlock_screen_manager.h"
#include "ability_manager_service.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

class UnlockScreenTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UnlockScreenTest::SetUpTestCase(void)
{}
void UnlockScreenTest::TearDownTestCase(void)
{}
void UnlockScreenTest::SetUp()
{}
void UnlockScreenTest::TearDown()
{}

/*
 * Feature: UnlockScreenTest
 * Function: UnlockScreen
 * SubFunction: NA
 * FunctionPoints:UnlockScreenTest UnlockScreen
 * EnvConditions: NA
 * CaseDescription: Verify UnlockScreen
 */
HWTEST_F(UnlockScreenTest, UnlockScreen_001, TestSize.Level1)
{
    bool ret = UnlockScreenManager::GetInstance().UnlockScreen();
    EXPECT_EQ(ret, true);
}
}  // namespace AbilityRuntime
}  // namespace OHOS