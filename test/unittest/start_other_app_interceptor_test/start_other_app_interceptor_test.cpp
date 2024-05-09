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

#include <gtest/gtest.h>
#include <gtest/gtest.h>

#define private public
#include "interceptor/start_other_app_interceptor.h"
#undef private
#include "start_ability_utils.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
thread_local bool StartAbilityUtils::skipStartOther = false;

class StartOtherAppInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void StartOtherAppInterceptorTest::SetUpTestCase()
{}

void StartOtherAppInterceptorTest::TearDownTestCase()
{}

void StartOtherAppInterceptorTest::SetUp()
{}

void StartOtherAppInterceptorTest::TearDown()
{}

/**
 * @tc.name: CheckNativeCall_001
 * @tc.desc: test function CheckNaticeCall when call from shell
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, CheckNativeCall_001, TestSize.Level1)
{
    auto interceptor = std::make_shared<StartOtherAppInterceptor>();
    bool res = interceptor->CheckNativeCall();
    EXPECT_TRUE(res);
}

/**
 * @tc.name: CheckCallerIsSystemApp_001
 * @tc.desc: test function CheckCallerIsSystemApp when call from shell
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, CheckCallerIsSystemApp_001, TestSize.Level1)
{
    auto interceptor = std::make_shared<StartOtherAppInterceptor>();
    bool res = interceptor->CheckCallerIsSystemApp();
    EXPECT_FALSE(res);
}

/**
 * @tc.name: GetApplicationInfo_001
 * @tc.desc: test function GetApplicationInfo when callerToken is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, GetApplicationInfo_001, TestSize.Level1)
{
    auto interceptor = std::make_shared<StartOtherAppInterceptor>();
    AppExecFwk::ApplicationInfo applicationInfo;
    bool res = interceptor->GetApplicationInfo(nullptr, applicationInfo);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: GetApplicationInfo_002
 * @tc.desc: test function GetApplicationInfo when callerToken is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, GetApplicationInfo_002, TestSize.Level1)
{
    auto interceptor = std::make_shared<StartOtherAppInterceptor>();
    AppExecFwk::ApplicationInfo applicationInfo;
    sptr<IRemoteObject> callerToken;
    bool res = interceptor->GetApplicationInfo(callerToken, applicationInfo);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: CheckAncoShellCall_001
 * @tc.desc: test function CheckAncoShellCall when caller is anco shell
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, CheckAncoShellCall_001, TestSize.Level1)
{
    auto interceptor = std::make_shared<StartOtherAppInterceptor>();
    AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    ElementName element("", "com.huawei.shell_assistant", "MainAbility");
    want.SetElement(element);
    bool res = interceptor->CheckAncoShellCall(applicationInfo, want);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: CheckAncoShellCall_002
 * @tc.desc: test function CheckAncoShellCall when caller is not anco shell
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, CheckAncoShellCall_002, TestSize.Level1)
{
    auto interceptor = std::make_shared<StartOtherAppInterceptor>();
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.codePath = "1";
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    bool res = interceptor->CheckAncoShellCall(applicationInfo, want);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: CheckStartOtherApp_001
 * @tc.desc: test function CheckStartOtherApp when start the same app
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, CheckStartOtherApp_001, TestSize.Level1)
{
    auto interceptor = std::make_shared<StartOtherAppInterceptor>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    std::string bundleName = "com.test.demo";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, bundleName);
    bool res = interceptor->CheckStartOtherApp(want);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: CheckStartOtherApp_002
 * @tc.desc: test function CheckStartOtherApp when start other app
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, CheckStartOtherApp_002, TestSize.Level1)
{
    auto interceptor = std::make_shared<StartOtherAppInterceptor>();
    Want want;
    ElementName element("", "com.test.demo2", "MainAbility");
    want.SetElement(element);std::string bundleName = "com.test.demo";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, bundleName);
    bool res = interceptor->CheckStartOtherApp(want);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: CheckCallerApiBelow12_001
 * @tc.desc: test function CheckCallerApiBelow12 when api is 12
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, CheckCallerApiBelow12_001, TestSize.Level1)
{
    auto interceptor = std::make_shared<StartOtherAppInterceptor>();
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.apiTargetVersion = 12;
    bool res = interceptor->CheckCallerApiBelow12(applicationInfo);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: CheckCallerApiBelow12_002
 * @tc.desc: test function CheckCallerApiBelow12 when api is 11
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, CheckCallerApiBelow12_002, TestSize.Level1)
{
    auto interceptor = std::make_shared<StartOtherAppInterceptor>();
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.apiCompatibleVersion = 11;
    bool res = interceptor->CheckCallerApiBelow12(applicationInfo);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: DoProcess_001
 * @tc.desc: test function DoProcess when background call
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, DoProcess_001, TestSize.Level1)
{
    auto interceptor = std::make_shared<StartOtherAppInterceptor>();
    Want want;
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, 0, false, nullptr);
    int32_t res = interceptor->DoProcess(param);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: DoProcess_002
 * @tc.desc: test function DoProcess when shell call
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, DoProcess_002, TestSize.Level1)
{
    auto interceptor = std::make_shared<StartOtherAppInterceptor>();
    Want want;
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, 0, false, nullptr);
    int32_t res = interceptor->DoProcess(param);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: CheckTargetIsSystemApp_001
 * @tc.desc: test function CheckTargetIsSystemApp when applicationInfo is true
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, CheckTargetIsSystemApp_001, TestSize.Level1)
{
    std::shared_ptr<StartOtherAppInterceptor> interceptor = std::make_shared<StartOtherAppInterceptor>();
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.isSystemApp = true;
    bool res = interceptor->CheckTargetIsSystemApp(applicationInfo);
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: CheckTargetIsSystemApp_002
 * @tc.desc: test function CheckTargetIsSystemApp when applicationInfo is false
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, CheckTargetIsSystemApp_002, TestSize.Level1)
{
    std::shared_ptr<StartOtherAppInterceptor> interceptor = std::make_shared<StartOtherAppInterceptor>();
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.isSystemApp = false;
    bool res = interceptor->CheckTargetIsSystemApp(applicationInfo);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: IsDelegatorCall_001
 * @tc.desc: test function IsDelegatorCall when applicationInfo is true
 * @tc.type: FUNC
 */
HWTEST_F(StartOtherAppInterceptorTest, IsDelegatorCall_001, TestSize.Level1)
{
    std::shared_ptr<StartOtherAppInterceptor> interceptor = std::make_shared<StartOtherAppInterceptor>();
    Want want;
    bool res = interceptor->IsDelegatorCall(want);
    EXPECT_EQ(res, false);
}
} // namespace AAFwk
} // namespace OHOS