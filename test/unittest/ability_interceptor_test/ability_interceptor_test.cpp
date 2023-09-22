/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "iservice_registry.h"
#define protected public
#include "ability_manager_service.h"
#undef private
#undef protected

#include "ability_interceptor.h"
#include "ability_interceptor_executer.h"
#include "bundle_mgr_interface.h"
#include "mock_bundle_installer_service.h"
#include "mock_bundle_manager_service.h"
#include "mock_ecological_rule_manager.h"
#include "mock_system_ability_manager.h"
#include "permission_verification.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
#ifndef SUPPORT_ERMS
using ExperienceRule = OHOS::AppExecFwk::ErmsParams::ExperienceRule;
#endif

namespace {
const std::string BUNDLE_NAME = "testBundle";
const int32_t  ECOLOGICAL_RULE_SA_ID = 9999;
const std::string ATOMIC_SERVICE_BUNDLE_NAME = "com.test.atomicservice";
const std::string PASS_ABILITY_NAME = "com.test.pass";
const std::string DENY_ABILITY_NAME = "com.test.deny";
const std::string JUMP_ABILITY_NAME = "com.test.jump";
}

namespace OHOS {
namespace AAFwk {
sptr<MockBundleInstallerService> mockBundleInstaller = new (std::nothrow) MockBundleInstallerService();
sptr<MockBundleManagerService> mockBundleMgr = new (std::nothrow) MockBundleManagerService();
class AbilityInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void MockBundleInstaller();
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;

public:
};

void AbilityInterceptorTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "AbilityInterceptorTest SetUpTestCase called";
    AbilityManagerClient::GetInstance()->CleanAllMissions();
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void AbilityInterceptorTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "AbilityInterceptorTest TearDownTestCase called";
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void AbilityInterceptorTest::SetUp()
{
    mockSystemAbility_ = new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
}

void AbilityInterceptorTest::TearDown()
{
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
}

void AbilityInterceptorTest::MockBundleInstaller()
{
    auto mockGetBundleInstaller = [] () {
        return mockBundleInstaller;
    };
    auto mockGetSystemAbility = [&] (int32_t systemAbilityId) {
        if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
            return mockBundleMgr->AsObject();
        } else {
            return iSystemAbilityMgr_->GetSystemAbility(systemAbilityId);
        }
    };
    EXPECT_CALL(*mockBundleMgr, GetBundleInstaller()).WillOnce(testing::Invoke(mockGetBundleInstaller));
    EXPECT_CALL(*mockSystemAbility_, GetSystemAbility(testing::_))
        .WillOnce(testing::Invoke(mockGetSystemAbility))
        .WillRepeatedly(testing::Invoke(mockGetSystemAbility));
}

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
    MockBundleInstaller();
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
    EXPECT_EQ(result, ERR_OK);
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
    EXPECT_EQ(result, ERR_OK);
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
    EXPECT_EQ(result, ERR_OK);
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

#ifndef SUPPORT_ERMS
/**
 * @tc.name: AbilityInterceptorTest_EcologicalRuleInterceptor_001
 * @tc.desc: EcologicalRuleInterceptor DoProcess ERR_OK when erms invalid
 * @tc.type: FUNC
 * @tc.require: issueI6HT6C
 */
HWTEST_F(AbilityInterceptorTest, EcologicalRuleInterceptor_001, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    int requestCode = 0;
    int userId = 100;
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        ECOLOGICAL_RULE_SA_ID, nullptr);
    executer->AddInterceptor(std::make_shared<EcologicalRuleInterceptor>());
    int result = executer->DoProcess(want, requestCode, userId, true);
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
            ECOLOGICAL_RULE_SA_ID, new MockEcologicalRuleMgrService());
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_EcologicalRuleInterceptor_002
 * @tc.desc: EcologicalRuleInterceptor DoProcess ERR_OK when erms return error
 * @tc.type: FUNC
 * @tc.require: issueI6HT6C
 */
HWTEST_F(AbilityInterceptorTest, EcologicalRuleInterceptor_002, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName elementName;
    elementName.SetBundleName(ATOMIC_SERVICE_BUNDLE_NAME);
    want.SetElement(elementName);
    int requestCode = 0;
    int userId = 100;

    executer->AddInterceptor(std::make_shared<EcologicalRuleInterceptor>());
    int result = executer->DoProcess(want, requestCode, userId, true);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_EcologicalRuleInterceptor_003
 * @tc.desc: EcologicalRuleInterceptor DoProcess ERR_OK when erms return pass
 * @tc.type: FUNC
 * @tc.require: issueI6HT6C
 */
HWTEST_F(AbilityInterceptorTest, EcologicalRuleInterceptor_003, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName elementName;
    elementName.SetBundleName(ATOMIC_SERVICE_BUNDLE_NAME);
    elementName.SetAbilityName(PASS_ABILITY_NAME);
    want.SetElement(elementName);
    int requestCode = 0;
    int userId = 100;

    executer->AddInterceptor(std::make_shared<EcologicalRuleInterceptor>());
    int result = executer->DoProcess(want, requestCode, userId, true);
    EXPECT_EQ(result, ERR_OK);
    }

/**
 * @tc.name: AbilityInterceptorTest_EcologicalRuleInterceptor_004
 * @tc.desc: EcologicalRuleInterceptor DoProcess ERR_ECOLOGICAL_CONTROL_STATUS when erms return DENY
 * @tc.type: FUNC
 * @tc.require: issueI6HT6C
 */
HWTEST_F(AbilityInterceptorTest, EcologicalRuleInterceptor_004, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName elementName;
    elementName.SetBundleName(ATOMIC_SERVICE_BUNDLE_NAME);
    elementName.SetAbilityName(DENY_ABILITY_NAME);
    want.SetElement(elementName);
    int requestCode = 0;
    int userId = 100;

    executer->AddInterceptor(std::make_shared<EcologicalRuleInterceptor>());
    int result = executer->DoProcess(want, requestCode, userId, true);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_EcologicalRuleInterceptor_005
 * @tc.desc: EcologicalRuleInterceptor DoProcess ERR_ECOLOGICAL_CONTROL_STATUS when erms return JUMP
 * @tc.type: FUNC
 * @tc.require: issueI6HT6C
 */
HWTEST_F(AbilityInterceptorTest, EcologicalRuleInterceptor_005, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName elementName;
    elementName.SetBundleName(ATOMIC_SERVICE_BUNDLE_NAME);
    elementName.SetAbilityName(JUMP_ABILITY_NAME);
    want.SetElement(elementName);
    int requestCode = 0;
    int userId = 100;

    executer->AddInterceptor(std::make_shared<EcologicalRuleInterceptor>());
    int result = executer->DoProcess(want, requestCode, userId, false);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_EcologicalRuleInterceptor_006
 * @tc.desc: EcologicalRuleInterceptor DoProcess ERR_OK when not ATOMIC_SERVICE
 * @tc.type: FUNC
 * @tc.require: issueI6HT6C
 */
HWTEST_F(AbilityInterceptorTest, EcologicalRuleInterceptor_006, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    int requestCode = 0;
    int userId = 100;
    executer->AddInterceptor(std::make_shared<EcologicalRuleInterceptor>());
    int result = executer->DoProcess(want, requestCode, userId, true);
    EXPECT_EQ(result, ERR_OK);
}
#endif
} // namespace AAFwk
} // namespace OHOS
