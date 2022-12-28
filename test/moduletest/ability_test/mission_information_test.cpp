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
#include "mission_information.h"
#undef protected
#undef private
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AppExecFwk {
class MissionInformationTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<MissionInformation> missionInformation_;
};
void MissionInformationTest::SetUpTestCase(void) {}
void MissionInformationTest::TearDownTestCase(void) {}
void MissionInformationTest::SetUp(void)
{
    missionInformation_ = std::make_shared<MissionInformation>();
}
void MissionInformationTest::TearDown(void)
{
    missionInformation_ = nullptr;
}

/**
 * @tc.number: AaFwk_Ability_Context_ReadFromParcel_001
 * @tc.name: ReadFromParcel
 * @tc.desc: Verification function ReadFromParcel and the result is true.
 */
HWTEST_F(MissionInformationTest, AaFwk_Ability_Context_ReadFromParcel_001, Function | MediumTest | Level1)
{
    Parcel parcel;
    auto result = missionInformation_->ReadFromParcel(parcel);
    EXPECT_TRUE(result);
}

/**
 * @tc.number: AaFwk_Ability_Context_Marshalling_001
 * @tc.name: Marshalling
 * @tc.desc: Verification function Marshalling and the result is true.
 */
HWTEST_F(MissionInformationTest, AaFwk_Ability_Context_Marshalling_001, Function | MediumTest | Level1)
{
    Parcel parcel;
    auto result = missionInformation_->Marshalling(parcel);
    EXPECT_TRUE(result);
}

/**
 * @tc.number: AaFwk_Ability_Context_Unmarshalling_001
 * @tc.name: Unmarshalling
 * @tc.desc: Verification function Unmarshalling and the result isn't nullptr.
 */
HWTEST_F(MissionInformationTest, AaFwk_Ability_Context_Unmarshalling_001, Function | MediumTest | Level1)
{
    Parcel parcel;
    auto result = missionInformation_->Unmarshalling(parcel);
    EXPECT_TRUE(result != nullptr);
}
} // namespace AppExecFwk
} // namespace OHOS