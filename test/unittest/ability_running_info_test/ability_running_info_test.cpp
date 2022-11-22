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
#include "parcel.h"

#define private public
#include "ability_running_info.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
const int32_t USER_ID_U100  = 100;
class AbilityRunningInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityRunningInfoTest::SetUpTestCase(void)
{}

void AbilityRunningInfoTest::TearDownTestCase(void)
{}

void AbilityRunningInfoTest::SetUp()
{}

void AbilityRunningInfoTest::TearDown()
{}

/*
 * Feature: AbilityRunningInfo
 * Function: ReadFromParcel
 * SubFunction: NA
 * FunctionPoints: AbilityRunningInfo ReadFromParcel
 */
HWTEST_F(AbilityRunningInfoTest, ReadFromParcel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "AbilityRunningInfoTest, AbilityRunningInfo_0100, TestSize.Level1";
    AbilityRunningInfo info;
    MessageParcel parcel;
    EXPECT_FALSE(info.ReadFromParcel(parcel));
}

/*
 * Feature: AbilityRunningInfo
 * Function: ReadFromParcel
 * SubFunction: NA
 * FunctionPoints: AbilityRunningInfo ReadFromParcel
 */
HWTEST_F(AbilityRunningInfoTest, ReadFromParcel_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "AbilityRunningInfoTest, AbilityRunningInfo_0200, TestSize.Level1";
    AbilityRunningInfo info;
    MessageParcel parcel;

    auto uid = USER_ID_U100;
    parcel.WriteInt32(uid);
    EXPECT_TRUE(info.ReadFromParcel(parcel));
}

/*
 * Feature: AbilityRunningInfo
 * Function: Marshalling
 * SubFunction: NA
 * FunctionPoints: AbilityRunningInfo Marshalling
 */
HWTEST_F(AbilityRunningInfoTest, Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "AbilityRunningInfoTest, Marshalling_0100, TestSize.Level1";
    AbilityRunningInfo info;
    MessageParcel parcel;
    EXPECT_TRUE(info.Marshalling(parcel));
}

/*
 * Feature: AbilityRunningInfo
 * Function: Unmarshalling
 * SubFunction: NA
 * FunctionPoints: AbilityRunningInfo Unmarshalling
 */
HWTEST_F(AbilityRunningInfoTest, Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "AbilityRunningInfoTest, Unmarshalling_0100, TestSize.Level1";
    AbilityRunningInfo info;
    MessageParcel parcel;
    EXPECT_EQ(info.Unmarshalling(parcel), nullptr);
}

/*
 * Feature: AbilityRunningInfo
 * Function: Unmarshalling
 * SubFunction: NA
 * FunctionPoints: AbilityRunningInfo Unmarshalling
 */
HWTEST_F(AbilityRunningInfoTest, Unmarshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "AbilityRunningInfoTest, Unmarshalling_0200, TestSize.Level1";
    AbilityRunningInfo info;
    MessageParcel parcel;

    auto uid = USER_ID_U100;
    parcel.WriteInt32(uid);
    EXPECT_NE(info.Unmarshalling(parcel), nullptr);
}
} // namespace AppExecFwk
} // namespace OHOS