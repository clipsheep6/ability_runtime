/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "app_debug_info.h"
#include "parcel.h"
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
    const std::string STRING_BUNDLE_NAME = "bundleName";
    constexpr int INT_PID = 10;
    constexpr int INT_UID = 12345;
}
class AppDebugInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<AppDebugInfo> appDebugInfo_;
};

void AppDebugInfoTest::SetUpTestCase(void)
{}

void AppDebugInfoTest::TearDownTestCase(void)
{}

void AppDebugInfoTest::SetUp()
{
    appDebugInfo_ = new AppDebugInfo();
}

void AppDebugInfoTest::TearDown()
{}

/**
 * @tc.name: ReadFromParcel_0100
 * @tc.desc: Verify read data from parcel normally.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugInfoTest, ReadFromParcel_0100, TestSize.Level1)
{
    EXPECT_NE(appDebugInfo_, nullptr);
    Parcel data;
    data.WriteString(STRING_BUNDLE_NAME);
    data.WriteInt32(INT_PID);
    data.WriteInt32(INT_UID);

    EXPECT_TRUE(appDebugInfo_->ReadFromParcel(data));
    EXPECT_EQ(appDebugInfo_->bundleName, STRING_BUNDLE_NAME);
}

/**
 * @tc.name: UnMarshalling_0100
 * @tc.desc: Verify unmarshall parcel normally.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugInfoTest, UnMarshalling_0100, TestSize.Level1)
{
    EXPECT_NE(appDebugInfo_, nullptr);
    Parcel data;
    data.WriteString(STRING_BUNDLE_NAME);
    data.WriteInt32(INT_PID);
    data.WriteInt32(INT_UID);

    auto info = appDebugInfo_->Unmarshalling(data);
    EXPECT_EQ(info->bundleName, STRING_BUNDLE_NAME);
    EXPECT_EQ(info->pid, INT_PID);
    EXPECT_EQ(info->uid, INT_UID);
}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: Verify marshall into parcel normally.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugInfoTest, Marshalling_0100, TestSize.Level1)
{
    EXPECT_NE(appDebugInfo_, nullptr);
    Parcel data;
    appDebugInfo_->bundleName = STRING_BUNDLE_NAME;
    appDebugInfo_->uid = INT_UID;
    appDebugInfo_->pid = INT_PID;

    EXPECT_TRUE(appDebugInfo_->Marshalling(data));
    EXPECT_EQ(data.ReadString(), STRING_BUNDLE_NAME);
    EXPECT_EQ(data.ReadInt32(), INT_PID);
    EXPECT_EQ(data.ReadInt32(), INT_UID);
}
} // namespace AppExecFwk
} // namespace OHOS
