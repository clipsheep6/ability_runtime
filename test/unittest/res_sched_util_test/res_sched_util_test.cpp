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

#include "ability_info.h"
#include "hilog_tag_wrapper.h"
#define private public
#include "res_sched_util.h"
#undef private
#include "parameters.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
using AbilityInfo = AppExecFwk::AbilityInfo;
class ResSchedUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ResSchedUtilTest::SetUpTestCase(void)
{}

void ResSchedUtilTest::TearDownTestCase(void)
{}

void ResSchedUtilTest::SetUp()
{}

void ResSchedUtilTest::TearDown()
{}

/**
 * @tc.number: ResSchedUtilTest_0100
 * @tc.desc: Test ReportAbilitStartInfoToRSS works
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedUtilTest, ResSchedUtilTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "ResSchedUtilTest_0100 called.");
    AbilityInfo abilityInfo;
    int64_t resSchedType = -1;
    AAFwk::ResSchedUtil::GetInstance().ReportAbilitStartInfoToRSS(abilityInfo, -1, false);
    AAFwk::ResSchedUtil::GetInstance().ReportAbilitAssociatedStartInfoToRSS(abilityInfo, resSchedType, 0, 0);
    int64_t ret = AAFwk::ResSchedUtil::GetInstance().convertType(resSchedType);
    EXPECT_EQ(resSchedType, ret);
}

/**
 * @tc.number: ResSchedUtilTest_0200
 * @tc.desc: Test ReportAbilitStartInfoToRSS works
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedUtilTest, ResSchedUtilTest_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "ResSchedUtilTest_0200 called.");
    std::string testName = "ResSchedUtilTest";
    AAFwk::ResSchedUtil::GetInstance().ReportEventToRSS(0, testName, testName);
    std::unordered_set<int32_t> frozenPids;
    AAFwk::ResSchedUtil::GetInstance().GetAllFrozenPidsFromRSS(frozenPids);
    int64_t resSchedType = AAFwk::RES_TYPE_SCB_START_ABILITY;
    int64_t ret = AAFwk::ResSchedUtil::GetInstance().convertType(resSchedType);
    EXPECT_EQ(resSchedType, ret);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
