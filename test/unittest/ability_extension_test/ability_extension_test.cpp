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

/**
 * @tc.name: Init_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityExtensionTest, Init_0100, TestSize.Level1)
{
    HILOG_INFO("Init start");

    Extension extension;
    std::shared_ptr<AppExecFwk::OHOSApplication> application = nullptr;
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record = nullptr;

    extension.Init(record, application, handler, token);
    EXPECT_EQ(extension.application_, nullptr);
    EXPECT_EQ(extension.handler_, nullptr);
    EXPECT_EQ(extension.abilityInfo_, nullptr);

    HILOG_INFO("Init end");
}

/**
 * @tc.name: SetLaunchWant_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityExtensionTest, SetLaunchWant_0100, TestSize.Level1)
{
    HILOG_INFO("SetLaunchWant start");

    Extension extension;
    Want want;
    extension.SetLaunchWant(want);
    EXPECT_NE(extension.launchWant_, nullptr);

    HILOG_INFO("SetLaunchWant end");
}

/**
 * @tc.name: SetLastRequestWant_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityExtensionTest, SetLastRequestWant_0100, TestSize.Level1)
{
    HILOG_INFO("SetLastRequestWant start");

    Extension extension;
    Want want;
    extension.SetLastRequestWant(want);
    EXPECT_NE(extension.lastRequestWant_, nullptr);

    HILOG_INFO("SetLastRequestWant end");
}

/**
 * @tc.name: Dump_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityExtensionTest, Dump_0100, TestSize.Level1)
{
    HILOG_INFO("SetLastRequestWant start");

    Extension extension;
    const std::vector<std::string> params;
    std::vector<std::string> info;
    extension.Dump(params, info);

    HILOG_INFO("SetLastRequestWant end");
}
}  // namespace AbilityRuntime
}  // namespace OHOS