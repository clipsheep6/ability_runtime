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
#include "ability_manager_service.h"
#undef private
#undef protected

#include "ability_interceptor.h"
#include "ability_interceptor_executer.h"
#include "bundlemgr/mock_bundle_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace {
    const std::string BUNDLE_NAME = "testBundle";
}

namespace OHOS {
namespace AAFwk {
class AbilityInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

public:
};

void AbilityInterceptorTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "AbilityInterceptorTest SetUpTestCase called";
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void AbilityInterceptorTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "AbilityInterceptorTest TearDownTestCase called";
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void AbilityInterceptorTest::SetUp()
{}

void AbilityInterceptorTest::TearDown()
{}

HWTEST_F(AbilityInterceptorTest, CreateExecuter_001, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    EXPECT_NE(executer, nullptr);
}

/**
 * @tc.name: AbilityInterceptorTest_CrowdTestInterceptor_001
 * @tc.desc: CrowdTestInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI5I0DY
 */
HWTEST_F(AbilityInterceptorTest, CrowdTestInterceptor_001, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.crowdtest", "CrowdtestExpired");
    want.SetElement(element);
    int requestCode = 0;
    int userId = 100;
    executer->AddInterceptor(std::make_shared<CrowdTestInterceptor>());
    int result = executer->DoProcess(want, requestCode, userId, true);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_CrowdTestInterceptor_002
 * @tc.desc: CrowdTestInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI5I0DY
 */
HWTEST_F(AbilityInterceptorTest, CrowdTestInterceptor_002, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.crowdtest", "CrowdtestExpired");
    want.SetElement(element);
    int userId = 100;
    executer->AddInterceptor(std::make_shared<CrowdTestInterceptor>());
    int result = executer->DoProcess(want, 0, userId, false);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_ControlInterceptor_001
 * @tc.desc: ControlInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI5QT7P
 */
HWTEST_F(AbilityInterceptorTest, ControlInterceptor_001, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.control", "MainAbility");
    want.SetElement(element);
    int requestCode = 0;
    int userId = 100;
    executer->AddInterceptor(std::make_shared<ControlInterceptor>());
    int result = executer->DoProcess(want, requestCode, userId, true);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_ControlInterceptor_002
 * @tc.desc: ControlInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI5QT7P
 */
HWTEST_F(AbilityInterceptorTest, ControlInterceptor_002, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.control", "MainAbility");
    want.SetElement(element);
    int userId = 100;
    executer->AddInterceptor(std::make_shared<ControlInterceptor>());
    int result = executer->DoProcess(want, 0, userId, false);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_ControlInterceptor_003
 * @tc.desc: ControlInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI5QT7P
 */
HWTEST_F(AbilityInterceptorTest, ControlInterceptor_003, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.control2", "MainAbility");
    want.SetElement(element);
    int userId = 100;
    executer->AddInterceptor(std::make_shared<ControlInterceptor>());
    int result = executer->DoProcess(want, 0, userId, false);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_ControlInterceptor_004
 * @tc.desc: ControlInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI5QT7P
 */
HWTEST_F(AbilityInterceptorTest, ControlInterceptor_004, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.control3", "MainAbility");
    want.SetElement(element);
    int userId = 100;
    executer->AddInterceptor(std::make_shared<ControlInterceptor>());
    int result = executer->DoProcess(want, 0, userId, false);
    EXPECT_EQ(result, ERR_DISPOSED_STATUS);
}

/**
 * @tc.name: AbilityInterceptorTest_ControlInterceptor_005
 * @tc.desc: ControlInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI5QT7P
 */
HWTEST_F(AbilityInterceptorTest, ControlInterceptor_005, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.control", "MainAbility");
    want.SetElement(element);
    int userId = 100;
    auto bms = AbilityUtil::GetBundleManager();
    // make appControlRule become nullptr by crowdtest interceptor
    executer->AddInterceptor(std::make_shared<CrowdTestInterceptor>());
    executer->AddInterceptor(std::make_shared<ControlInterceptor>());
    int result = executer->DoProcess(want, 0, userId, false);
    EXPECT_EQ(result, ERR_OK);
}
} // namespace AAFwk
} // namespace OHOS
