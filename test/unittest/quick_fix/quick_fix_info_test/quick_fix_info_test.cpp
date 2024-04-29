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
#include "quick_fix_info.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class QuickFixInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void QuickFixInfoTest::SetUpTestCase(void)
{}

void QuickFixInfoTest::TearDownTestCase(void)
{}

void QuickFixInfoTest::SetUp()
{}

void QuickFixInfoTest::TearDown()
{}

/**
 * @tc.name: ReadFromParcel_0100
 * @tc.desc: ReadFromParcel
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixInfoTest, ReadFromParcel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "QuickFixInfoTest, QuickFixInfo_0100, TestSize.Level1";
    ApplicationQuickFixInfo info;
    Parcel parcel;
    MockReadParcelable(true);
    EXPECT_TRUE(info.ReadFromParcel(parcel));
    ResetParcelState();
}

/**
 * @tc.name: ReadFromParcel_0200
 * @tc.desc: ReadFromParcel
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixInfoTest, ReadFromParcel_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "QuickFixInfoTest, QuickFixInfo_0200, TestSize.Level1";
    ApplicationQuickFixInfo info;
    Parcel parcel;
    MockReadParcelable(false);
    EXPECT_FALSE(info.ReadFromParcel(parcel));
    ResetParcelState();
}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: Marshalling
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixInfoTest, Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "QuickFixInfoTest, Marshalling_0100, TestSize.Level1";
    ApplicationQuickFixInfo info;
    Parcel parcel;
    MockWriteString(true, 0);
    MockWriteUint32(true);
    MockWriteParcelable(true);
    EXPECT_TRUE(info.Marshalling(parcel));
    ResetParcelState();
}

/**
 * @tc.name: Marshalling_0200
 * @tc.desc: Marshalling
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixInfoTest, Marshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "QuickFixInfoTest, Marshalling_0200, TestSize.Level1";
    ApplicationQuickFixInfo info;
    Parcel parcel;
    MockWriteString(true, 1);
    MockWriteUint32(true);
    MockWriteParcelable(true);
    EXPECT_FALSE(info.Marshalling(parcel));
    ResetParcelState();
}

/**
 * @tc.name: Marshalling_0300
 * @tc.desc: Marshalling
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixInfoTest, Marshalling_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "QuickFixInfoTest, Marshalling_0300, TestSize.Level1";
    ApplicationQuickFixInfo info;
    Parcel parcel;
    MockWriteString(true, 0);
    MockWriteUint32(false);
    MockWriteParcelable(true);
    EXPECT_FALSE(info.Marshalling(parcel));
    ResetParcelState();
}

/**
 * @tc.name: Marshalling_0400
 * @tc.desc: Marshalling
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixInfoTest, Marshalling_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "QuickFixInfoTest, Marshalling_0400, TestSize.Level1";
    ApplicationQuickFixInfo info;
    Parcel parcel;
    MockWriteString(true, 0);
    MockWriteUint32(true);
    MockWriteParcelable(false);
    EXPECT_FALSE(info.Marshalling(parcel));
    ResetParcelState();
}

/**
 * @tc.name: Marshalling_0500
 * @tc.desc: Marshalling
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixInfoTest, Marshalling_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "QuickFixInfoTest, Marshalling_0500, TestSize.Level1";
    ApplicationQuickFixInfo info;
    Parcel parcel;
    MockWriteString(false, 1);
    MockWriteUint32(true);
    MockWriteParcelable(true);
    EXPECT_FALSE(info.Marshalling(parcel));
    ResetParcelState();
}

/**
 * @tc.name: Unmarshalling_0100
 * @tc.desc: Unmarshalling
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixInfoTest, Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "QuickFixInfoTest, Unmarshalling_0100, TestSize.Level1";
    ApplicationQuickFixInfo info;
    Parcel parcel;
    MockReadParcelable(true);
    EXPECT_NE(info.Unmarshalling(parcel), nullptr);
    ResetParcelState();
}

/**
 * @tc.name: Unmarshalling_0200
 * @tc.desc: Unmarshalling
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixInfoTest, Unmarshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "QuickFixInfoTest, Unmarshalling_0200, TestSize.Level1";
    ApplicationQuickFixInfo info;
    Parcel parcel;
    MockReadParcelable(false);
    EXPECT_EQ(info.Unmarshalling(parcel), nullptr);
    ResetParcelState();
}
} // namespace AppExecFwk
} // namespace OHOS