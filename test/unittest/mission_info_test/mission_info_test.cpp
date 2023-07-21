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
#include "ability_info.h"
#include "mission.h"
#include "mission_info.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class MissionInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MissionInfoTest::SetUpTestCase(void)
{}
void MissionInfoTest::TearDownTestCase(void)
{}
void MissionInfoTest::SetUp(void)
{}
void MissionInfoTest::TearDown(void)
{}

/*
 * Feature: Mission Info
 * Function: Marshalling and Unmarshalling
 * SubFunction: NA
 * FunctionPoints: Mission Marshalling
 * EnvConditions: NA
 * CaseDescription: Verify Marshalling
 */
HWTEST_F(MissionInfoTest, mission_info_marshalling_001, TestSize.Level1)
{
    Parcel parcel;
    MissionInfo* parcelable1 = new MissionInfo();
    parcelable1->id = 1;
    parcelable1->runningState = 100;
    parcelable1->lockedState = true;
    parcelable1->continuable = true;
    parcelable1->time = "time";
    parcelable1->label = "label";
    parcelable1->iconPath = "iconpath";
    parcelable1->continueState = AAFwk::ContinueState::CONTINUESTATE_ACTIVE;
    EXPECT_EQ(true, parcelable1->Marshalling(parcel));
    MissionInfo* parcelable2 = parcelable1->Unmarshalling(parcel);
    EXPECT_EQ(parcelable2->id, 1);
    EXPECT_EQ(parcelable2->runningState, 100);
    EXPECT_EQ(parcelable2->lockedState, true);
    EXPECT_EQ(parcelable2->continuable, true);
    EXPECT_EQ(parcelable2->time, "time");
    EXPECT_EQ(parcelable2->label, "label");
    EXPECT_EQ(parcelable2->iconPath, "iconpath");
    EXPECT_EQ(parcelable2->continueState, AAFwk::ContinueState::CONTINUESTATE_ACTIVE);
}
}
}