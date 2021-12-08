/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <algorithm>
#include <string>

#include "ability_parameter_container.h"
#include "iremote_proxy.h"
#include "ability_scheduler.h"
#include "mock_ability_connect_callback.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {

class AbilityParameterContaierTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityParameterContaierTest::SetUpTestCase(void)
{}
void AbilityParameterContaierTest::TearDownTestCase(void)
{}
void AbilityParameterContaierTest::SetUp()
{}
void AbilityParameterContaierTest::TearDown()
{}

/*
 * Feature: AbilityParameterContaier
 * Function: AddParameter
 * SubFunction: NA
 * FunctionPoints: AbilityParameterContaier AddParameter
 * EnvConditions: NA
 * CaseDescription: Verify that AddParameter is normal and abnormal
 */
HWTEST_F(AbilityParameterContaierTest, AbilityParameterContaier_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityParameterContaier_001 start";

    std::unique_ptr<AbilityParameterContaier> abilityParameterContaier = std::make_unique<AbilityParameterContaier>();
    int abilityRecordId = 1;
    EXPECT_EQ(false, abilityParameterContaier->IsExist(abilityRecordId));    

    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.restart = true;
    abilityRequest.requestCode = 10;

    abilityParameterContaier->AddParameter(abilityRecordId, abilityRequest);

    EXPECT_EQ(true, abilityParameterContaier->IsExist(abilityRecordId));
    GTEST_LOG_(INFO) << "AbilityParameterContaier_001 end";
}

/*
 * Feature: AbilityParameterContaier
 * Function: RemoveParameterByID
 * SubFunction: NA
 * FunctionPoints: AbilityParameterContaier RemoveParameterByID
 * EnvConditions: NA
 * CaseDescription: Verify that RemoveParameterByID is normal and abnormal
 */
HWTEST_F(AbilityParameterContaierTest, AbilityParameterContaier_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityParameterContaier_002 start";

    std::unique_ptr<AbilityParameterContaier> abilityParameterContaier = std::make_unique<AbilityParameterContaier>(); 
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.restart = true;
    abilityRequest.requestCode = 10;
    int abilityRecordId = 1;
    abilityParameterContaier->AddParameter(abilityRecordId, abilityRequest);

    EXPECT_EQ(true, abilityParameterContaier->IsExist(abilityRecordId));

    abilityParameterContaier->RemoveParameterByID(abilityRecordId);

    EXPECT_EQ(false, abilityParameterContaier->IsExist(abilityRecordId));
    GTEST_LOG_(INFO) << "AbilityParameterContaier_002 end";
}

/*
 * Feature: AbilityParameterContaier
 * Function: GetAbilityRequestFromContaier
 * SubFunction: NA
 * FunctionPoints: AbilityParameterContaier GetAbilityRequestFromContaier
 * EnvConditions: NA
 * CaseDescription: Verify that GetAbilityRequestFromContaier is normal and abnormal
 */
HWTEST_F(AbilityParameterContaierTest, AbilityParameterContaier_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityParameterContaier_003 start";

    std::unique_ptr<AbilityParameterContaier> abilityParameterContaier = std::make_unique<AbilityParameterContaier>();
    AbilityRequest abilityRequest;
    AbilityRequest abilityResult;
    abilityRequest.callerUid = 1;
    abilityRequest.restart = true;
    abilityRequest.requestCode = 10;
    int abilityRecordId = 1;
    abilityParameterContaier->AddParameter(abilityRecordId, abilityRequest);

    EXPECT_EQ(true, abilityParameterContaier->IsExist(abilityRecordId));

    abilityResult = abilityParameterContaier->GetAbilityRequestFromContaier(abilityRecordId);

    EXPECT_EQ(abilityResult.callerUid, abilityRequest.callerUid);
    GTEST_LOG_(INFO) << "AbilityParameterContaier_003 end";
}

/*
 * Feature: AbilityParameterContaier
 * Function: IsExist
 * SubFunction: NA
 * FunctionPoints: AbilityParameterContaier IsExist
 * EnvConditions: NA
 * CaseDescription: Verify that IsExist is normal and abnormal
 */
HWTEST_F(AbilityParameterContaierTest, AbilityParameterContaier_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityParameterContaier_004 start";

    std::unique_ptr<AbilityParameterContaier> abilityParameterContaier = std::make_unique<AbilityParameterContaier>();
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.restart = true;
    abilityRequest.requestCode = 10;
    int abilityRecordId = 1;

    EXPECT_EQ(false, abilityParameterContaier->IsExist(abilityRecordId));

    abilityParameterContaier->AddParameter(abilityRecordId, abilityRequest);

    EXPECT_EQ(true, abilityParameterContaier->IsExist(abilityRecordId));
    GTEST_LOG_(INFO) << "AbilityParameterContaier_004 end";
}


}  // namespace AAFwk
}  // namespace OHOS


