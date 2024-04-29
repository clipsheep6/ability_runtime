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
#include "page_state_data.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class PageStateDataTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void PageStateDataTest::SetUpTestCase(void)
{}

void PageStateDataTest::TearDownTestCase(void)
{}

void PageStateDataTest::SetUp()
{}

void PageStateDataTest::TearDown()
{}

/**
 * @tc.name: ReadFromParcel_0100
 * @tc.desc: ReadFromParcel
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(PageStateDataTest, ReadFromParcel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PageStateDataTest, ReadFromParcel_0100, TestSize.Level1";
    OHOS::AppExecFwk::PageStateData info;
    Parcel parcel;
    auto result = info.ReadFromParcel(parcel);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: Marshalling
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(PageStateDataTest, Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PageStateDataTest, Marshalling_0100, TestSize.Level1";
    OHOS::AppExecFwk::PageStateData info;
    Parcel parcel;
    auto result = info.Marshalling(parcel);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: Unmarshalling_0100
 * @tc.desc: Unmarshalling
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(PageStateDataTest, Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PageStateDataTest, Unmarshalling_0100, TestSize.Level1";
    OHOS::AppExecFwk::PageStateData info;
    Parcel parcel;
    auto result = info.Unmarshalling(parcel);
    EXPECT_TRUE(result);
}
} // namespace AppExecFwk
} // namespace OHOS