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
#include "dummy_values_bucket.h"
#include "parcel.h"
#undef private
#undef protected
#include "hilog_wrapper.h"
using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
class DummyValuesBucketTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/*
 * @tc.number    : DummyValuesBucketTest_0100
 * @tc.name      : DummyValuesBucketTest
 * @tc.desc      : Test Function ValuesBucket::ValuesBucket
 */
HWTEST_F(DummyValuesBucketTest, DummyValuesBucketTest_0100, TestSize.Level1)
{
    HILOG_INFO("DummyValuesBucketTest_0100 is start");
    const std::string testInf = "test";
    ValuesBucket valuesBucket(testInf);
    EXPECT_FALSE(valuesBucket.IsEmpty());
    HILOG_INFO("DummyValuesBucketTest_0100 is end");
}

/*
 * @tc.number    : DummyValuesBucketTest_0200
 * @tc.name      : DummyValuesBucketTest
 * @tc.desc      : Test Function ValuesBucket::ReadFromParcel
 */
HWTEST_F(DummyValuesBucketTest, DummyValuesBucketTest_0200, TestSize.Level1)
{
    HILOG_INFO("DummyValuesBucketTest_0200 is start");
    const std::string testInf = "test";
    ValuesBucket valuesBucket(testInf);
    Parcel parcel;
    auto res = valuesBucket.ReadFromParcel(parcel);
    EXPECT_TRUE(res);
    HILOG_INFO("DummyValuesBucketTest_0200 is end");
}

/*
 * @tc.number    : DummyValuesBucketTest_0300
 * @tc.name      : DummyValuesBucketTest
 * @tc.desc      : Test Function ValuesBucket::Unmarshalling
 */
HWTEST_F(DummyValuesBucketTest, DummyValuesBucketTest_0300, TestSize.Level1)
{
    HILOG_INFO("DummyValuesBucketTest_0300 is start");
    const std::string testInf = "test";
    ValuesBucket valuesBucket(testInf);
    Parcel parcel;
    auto res = valuesBucket.Unmarshalling(parcel);
    EXPECT_TRUE(res);
    HILOG_INFO("DummyValuesBucketTest_0300 is end");
}

/*
 * @tc.number    : DummyValuesBucketTest_0400
 * @tc.name      : DummyValuesBucketTest
 * @tc.desc      : Test Function ValuesBucket::Marshalling
 */
HWTEST_F(DummyValuesBucketTest, DummyValuesBucketTest_0400, TestSize.Level1)
{
    HILOG_INFO("DummyValuesBucketTest_0400 is start");
    const std::string testInf = "test";
    ValuesBucket valuesBucket(testInf);
    Parcel parcel;
    auto res = valuesBucket.Marshalling(parcel);
    EXPECT_TRUE(res);
    HILOG_INFO("DummyValuesBucketTest_0400 is end");
}
} // namespace AAFwk
} // namespace OHOS