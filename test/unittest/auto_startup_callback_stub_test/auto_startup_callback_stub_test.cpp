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

#include "ability_manager_ipc_interface_code.h"
#include "auto_startup_info.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"
#define private public
#include "mock_auto_startup_callback_stub.h"
#undef private

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace AbilityRuntime {

class AutoStartupCallBackStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void WriteInterfaceToken(MessageParcel &parcel);
};

void AutoStartupCallBackStubTest::SetUpTestCase(void) {}

void AutoStartupCallBackStubTest::TearDownTestCase(void) {}

void AutoStartupCallBackStubTest::SetUp() {}

void AutoStartupCallBackStubTest::TearDown() {}

void AutoStartupCallBackStubTest::WriteInterfaceToken(MessageParcel &parcel)
{
    parcel.WriteInterfaceToken(AutoStartupCallBackStub::GetDescriptor());
}

/*
 * Feature: AutoStartupCallBackStubTest
 * Function: OnAutoStartupOnInner
 * SubFunction: AutoStartupCallBackStubTest
 * FunctionPoints: AutoStartupCallbackStubTest OnAutoStartupOnInner
 */
HWTEST_F(AutoStartupCallBackStubTest, OnAutoStartupOnInner_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AutoStartupCallBackStubTest OnAutoStartupOnInner_001 start";
    auto stub = new (std::nothrow) MockAutoStartupCallbackStub();
    EXPECT_TRUE(stub);

    AutoStartupInfo info;
    info.bundleName = "bundleName";
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    data.WriteParcelable(&info);

    auto result = stub->OnRemoteRequest(
        static_cast<uint32_t>(AAFwk::AbilityManagerInterfaceCode::ON_AUTO_STARTUP_ON), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
    GTEST_LOG_(INFO) << "AutoStartupCallBackStubTest OnAutoStartupOnInner_001 end";
}

/*
 * Feature: AutoStartupCallBackStubTest
 * Function: OnAutoStartupOffInner
 * SubFunction: NA
 * FunctionPoints: AutoStartupCallBackStubTest OnAutoStartupOffInner
 */
HWTEST_F(AutoStartupCallBackStubTest, OnAutoStartupOffInner_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AutoStartupCallBackStubTest OnAutoStartupOffInner_001 start";
    auto stub = new (std::nothrow) MockAutoStartupCallbackStub();
    EXPECT_TRUE(stub);

    AutoStartupInfo info;
    info.bundleName = "bundleName";
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    data.WriteParcelable(&info);

    auto result = stub->OnRemoteRequest(
        static_cast<uint32_t>(AAFwk::AbilityManagerInterfaceCode::ON_AUTO_STARTUP_OFF), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
    GTEST_LOG_(INFO) << "AutoStartupCallBackStubTest OnAutoStartupOffInner_001 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
