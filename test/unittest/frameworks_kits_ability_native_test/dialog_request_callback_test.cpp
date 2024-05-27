/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "dialog_request_callback_proxy.h"
#include "dialog_request_callback_stub.h"
#ifdef SUPPORT_GRAPHICS
#undef SUPPORT_GRAPHICS
#include "dialog_request_callback_impl.h"
#endif
#include "idialog_request_callback.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
class DialogRequestCallbackTest : public testing::Test {
public:
    DialogRequestCallbackTest()
    {}
    ~DialogRequestCallbackTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class MockDialogRequestCallbackStub : public AbilityRuntime::DialogRequestCallbackStub {
public:
    MockDialogRequestCallbackStub() = default;
    virtual ~MockDialogRequestCallbackStub() = default;
    MOCK_METHOD4(OnRemoteRequest, int(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));
    MOCK_METHOD2(SendResult, void(int32_t resultCode, const AAFwk::Want &want));
};

void DialogRequestCallbackTest::SetUpTestCase(void)
{}

void DialogRequestCallbackTest::TearDownTestCase(void)
{}

void DialogRequestCallbackTest::SetUp(void)
{}

void DialogRequestCallbackTest::TearDown(void)
{}

/**
 * @tc.number:
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(DialogRequestCallbackTest, AppExecFwk_DialogRequestCallbackTest0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DialogRequestCallbackTest0100 start";
    auto mockDialogRequestCallbackStub = new MockDialogRequestCallbackStub();
    AbilityRuntime::DialogRequestCallbackProxy dialogRequestCallbackProxy(mockDialogRequestCallbackStub);
    EXPECT_CALL(*mockDialogRequestCallbackStub, OnRemoteRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1);
    AAFwk::Want want;
    int32_t resultCode = 0;
    dialogRequestCallbackProxy.SendResult(resultCode, want);
    GTEST_LOG_(INFO) << "AppExecFwk_DialogRequestCallbackTest0100 end";
}

/**
 * @tc.number:
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(DialogRequestCallbackTest, AppExecFwk_DialogRequestCallbackTest0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DialogRequestCallbackTest0200 start";
    AbilityRuntime::RequestDialogResultTask task =
        [](int32_t resultCode, const AAFwk::Want &resultWant) {
            GTEST_LOG_(INFO) << "AppExecFwk_DialogRequestCallbackTest0200 fuction is called.";
        };
    AbilityRuntime::DialogRequestCallbackImpl dialogRequestCallbackImpl(std::move(task));
    int32_t resultCode = AbilityRuntime::IDialogRequestCallback::CODE_SEND_RESULT;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    data.WriteInterfaceToken(AbilityRuntime::DialogRequestCallbackStub::GetDescriptor());
    data.WriteInt32(resultCode);
    AAFwk::Want want = {};
    want.SetParam("debugApp", false);
    data.WriteParcelable(&want);
    dialogRequestCallbackImpl.OnRemoteRequest(resultCode, data, reply, option);
    GTEST_LOG_(INFO) << "AppExecFwk_DialogRequestCallbackTest0200 end";
}


}  // namespace AppExecFwk
}  // namespace OHOS
