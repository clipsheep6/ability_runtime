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
#include "stop_user_callback_proxy.h"
#undef private
#include "stop_user_callback_stub_mock.h"
#include "ipc_types.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class StopUserCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<StopUserCallbackProxy> proxy_ {nullptr};
    sptr<StopUserCallbackStubMock> mock_ {nullptr};
};

void StopUserCallbackProxyTest::SetUpTestCase(void)
{}
void StopUserCallbackProxyTest::TearDownTestCase(void)
{}
void StopUserCallbackProxyTest::TearDown(void)
{}
void StopUserCallbackProxyTest::SetUp()
{
    mock_ = new StopUserCallbackStubMock();
    proxy_ = std::make_shared<StopUserCallbackProxy>(mock_);
}

/*
 * Feature: StopUserCallbackProxy
 * Function: OnStopUserDone
 * SubFunction: NA
 * FunctionPoints: StopUserCallbackProxy OnStopUserDone
 * EnvConditions: NA
 * CaseDescription: Verify OnStopUserDone
 */
HWTEST_F(StopUserCallbackProxyTest, OnStopUserDone_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &StopUserCallbackStubMock::InvokeSendRequest));
    int userId = 0;
    int errcode = 0;
    proxy_->OnStopUserDone(userId, errcode);
    EXPECT_EQ(IStopUserCallback::StopUserCallbackCmd::ON_STOP_USER_DONE, mock_->code_);
}

/*
 * Feature: StopUserCallbackProxy
 * Function: OnStopUserDone
 * SubFunction: NA
 * FunctionPoints: StopUserCallbackProxy OnStopUserDone
 * EnvConditions: NA
 * CaseDescription: Verify OnStopUserDone
 */
HWTEST_F(StopUserCallbackProxyTest, OnStopUserDone_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &StopUserCallbackStubMock::InvokeErrorSendRequest));
    int userId = 0;
    int errcode = 0;
    proxy_->OnStopUserDone(userId, errcode);
    EXPECT_EQ(IStopUserCallback::StopUserCallbackCmd::ON_STOP_USER_DONE, mock_->code_);
}
}  // namespace AAFwk
}  // namespace OHOS
