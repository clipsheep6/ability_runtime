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
#include "extension.h"
#undef private
#undef protected

#include "hilog_wrapper.h"
#include "iremote_object.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class AbilityExtensionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityExtensionTest::SetUpTestCase(void)
{}

void AbilityExtensionTest::TearDownTestCase(void)
{}

void AbilityExtensionTest::SetUp()
{}

void AbilityExtensionTest::TearDown()
{}

/**
 * @tc.name: SetCallingInfo_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, SetCallingInfo_0100, TestSize.Level1)
{
    HILOG_INFO("SetCallingInfo start");

    Extension extension;
    CallingInfo callingInfo;
    extension.SetCallingInfo(callingInfo);
    EXPECT_NE(extension.callingInfo_, nullptr);

    HILOG_INFO("SetCallingInfo end");
}

/**
 * @tc.name: GetCallingInfo_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, GetCallingInfo_0100, TestSize.Level1)
{
    HILOG_INFO("GetCallingInfo start");

    Extension extension;
    CallingInfo callingInfo;
    extension.SetCallingInfo(callingInfo);
    auto result = extension.GetCallingInfo();
    EXPECT_NE(result, nullptr);

    HILOG_INFO("GetCallingInfo end");
}
} // namespace AbilityRuntime
} // namespace OHOS