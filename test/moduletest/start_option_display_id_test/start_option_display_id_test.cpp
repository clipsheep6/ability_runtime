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
#define private public
#define protected public
#include "ability_manager_service.h"
#undef private
#undef protected
#include "ability_manager_errors.h"
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID_U100 = 100;
constexpr int32_t DISPLAY_ID = 256;
}  // namespace

class StartOptionDisplayIdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void StartOptionDisplayIdTest::SetUpTestCase() {}

void StartOptionDisplayIdTest::TearDownTestCase() {}

void StartOptionDisplayIdTest::SetUp() {}

void StartOptionDisplayIdTest::TearDown() {}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Set displayId, enable ability, get displayId value consistent with the setting
 */
HWTEST_F(StartOptionDisplayIdTest, start_option_001, TestSize.Level1)
{
    auto abilityMgrServ_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    StartOptions option;
    option.SetDisplayID(DISPLAY_ID);
    auto result = abilityMgrServ_->StartAbility(want, option, nullptr, USER_ID_U100, 0);
    if (result == OHOS::ERR_OK) {
        auto topAbility = abilityMgrServ_->GetMissionListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
        if (topAbility) {
            auto defualtDisplayId = 0;
            auto displayId = topAbility->GetWant().GetIntParam(Want::PARAM_RESV_DISPLAY_ID, defualtDisplayId);
            EXPECT_EQ(displayId, DISPLAY_ID);
        }
    }
}
}  // namespace AAFwk
}  // namespace OHOS
