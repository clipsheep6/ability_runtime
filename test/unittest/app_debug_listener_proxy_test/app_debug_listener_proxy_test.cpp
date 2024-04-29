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

#include<gtest/gtest.h>

#define private public
#include "app_debug_info.h"
#include "app_debug_listener_proxy.h"
#include "app_debug_listener_stub_mock.h"
#undef private

#include "parcel.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace AppExecFwk {

class AppDebugListenerProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AppDebugListenerProxy> proxy_;
    sptr<AppDebugListenerStubMock> mock_;
};

void AppDebugListenerProxyTest::SetUpTestCase(void)
{}

void AppDebugListenerProxyTest::TearDownTestCase(void)
{}

void AppDebugListenerProxyTest::SetUp()
{
    mock_ = new AppDebugListenerStubMock();
    proxy_ = std::make_shared<AppDebugListenerProxy>(mock_);
}

void AppDebugListenerProxyTest::TearDown()
{}

/**
 * @tc.name: WriteInterfaceToken_0100
 * @tc.desc: write token into parcel data
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugListenerProxyTest, WriteInterfaceToken_0100, TestSize.Level1)
{
    MessageParcel data;
    EXPECT_TRUE(proxy_->WriteInterfaceToken(data));
}

/**
 * @tc.name: OnAppDebugStarted_0100
 * @tc.desc: Callback of app debug started, verify that AppDebugListener interface calls normally.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugListenerProxyTest, OnAppDebugStarted_0100, TestSize.Level1)
{
    EXPECT_NE(proxy_, nullptr);
    EXPECT_NE(mock_, nullptr);

    std::vector<AppDebugInfo> appDebugInfos;
    AppDebugInfo debugInfo;
    appDebugInfos.push_back(debugInfo);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AppDebugListenerStubMock::InvokeSendRequest));
    proxy_->OnAppDebugStarted(appDebugInfos);
    auto code = static_cast<uint32_t>(IAppDebugListener::Message::ON_APP_DEBUG_STARTED);
    EXPECT_EQ(code, static_cast<uint32_t>(mock_->code_));
}

/**
 * @tc.name: OnAppDebugStoped_0100
 * @tc.desc: Callback of app debug started, verify that AppDebugListener interface calls normally.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugListenerProxyTest, OnAppDebugStoped_0100, TestSize.Level1)
{
    EXPECT_NE(proxy_, nullptr);
    EXPECT_NE(mock_, nullptr);
    
    std::vector<AppDebugInfo> appDebugInfos;
    AppDebugInfo debugInfo;
    appDebugInfos.push_back(debugInfo);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AppDebugListenerStubMock::InvokeSendRequest));
    proxy_->OnAppDebugStoped(appDebugInfos);
    auto code = static_cast<uint32_t>(IAppDebugListener::Message::ON_APP_DEBUG_STOPED);
    EXPECT_EQ(code, static_cast<uint32_t>(mock_->code_));
}
} // namespace AppExecFwk
} // namespace OHOS
