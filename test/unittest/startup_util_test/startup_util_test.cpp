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

#include "extension_ability_info.h"
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
 * @tc.name: GetAppIndex_001
 * @tc.desc: test class StartupUtil number function
 * @tc.type: FUNC
 */
HWTEST_F(StartupUtilTest, GetAppIndex_001, TestSize.Level1)
{
    AAFwk::Want want;
    int32_t appIndex = 0;
    auto queryRet = StartupUtil::GetAppIndex(want, appIndex);
    EXPECT_TRUE(queryRet);
}

/**
 * @tc.name: GetAppIndex_002
 * @tc.desc: test class StartupUtil number function
 * @tc.type: FUNC
 */
HWTEST_F(StartupUtilTest, GetAppIndex_002, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, -1);
    int32_t appIndex = 0;
    auto queryRet = StartupUtil::GetAppIndex(want, appIndex);
    EXPECT_FALSE(queryRet);
}

/**
 * @tc.name: GetAppIndex_003
 * @tc.desc: test class StartupUtil number function
 * @tc.type: FUNC
 */
HWTEST_F(StartupUtilTest, GetAppIndex_003, TestSize.Level1)
{
    AAFwk::Want want;
    int32_t appIndex = 1001;
    want.SetParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, appIndex);
    auto queryRet = StartupUtil::GetAppIndex(want, appIndex);
    EXPECT_FALSE(queryRet);
}

/**
 * @tc.name: IsSupportAppClone_001
 * @tc.desc: test class StartupUtil number function
 * @tc.type: FUNC
 */
HWTEST_F(StartupUtilTest, IsSupportAppClone_001, TestSize.Level1)
{
    AppExecFwk::ExtensionAbilityType type = AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER;
    auto queryRet = StartupUtil::IsSupportAppClone(type);
    EXPECT_TRUE(queryRet);
}

/**
 * @tc.name: IsSupportAppClone_002
 * @tc.desc: test class StartupUtil number function
 * @tc.type: FUNC
 */
HWTEST_F(StartupUtilTest, IsSupportAppClone_002, TestSize.Level1)
{
    AppExecFwk::ExtensionAbilityType type = AppExecFwk::ExtensionAbilityType::FORM;
    auto queryRet = StartupUtil::IsSupportAppClone(type);
    EXPECT_FALSE(queryRet);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
