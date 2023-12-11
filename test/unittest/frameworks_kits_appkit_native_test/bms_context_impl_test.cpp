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
#include <singleton.h>

#define private public
#include "context_impl.h"
#include "iservice_registry.h"
#undef private

#include "ability_local_record.h"
#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "mock_bundle_installer_service.h"
#include "mock_bundle_manager_service.h"
#include "mock_system_ability_manager.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
sptr<MockBundleInstallerService> mockBundleInstaller = new (std::nothrow) MockBundleInstallerService();
sptr<MockBundleManagerService> mockBundleMgr = new (std::nothrow) MockBundleManagerService();
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class BmsContextImplTest : public testing::Test {
public:
    BmsContextImplTest() : contextImpl_(nullptr) {}
    ~BmsContextImplTest() {}
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void MockBundleInstaller();
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;
};

void BmsContextImplTest::SetUpTestCase(void) {}

void BmsContextImplTest::TearDownTestCase(void) {}

void BmsContextImplTest::SetUp(void)
{
    contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    DelayedSingleton<SysMrgClient>::GetInstance()->RegisterSystemAbility(
        BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, mockBundleMgr);
    mockSystemAbility_ = new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
}

void BmsContextImplTest::TearDown(void)
{
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
}

void BmsContextImplTest::MockBundleInstaller()
{
    auto mockGetBundleInstaller = []() { return mockBundleInstaller; };
    auto mockGetSystemAbility = [bms = mockBundleMgr, saMgr = iSystemAbilityMgr_](int32_t systemAbilityId) {
        if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
            return bms->AsObject();
        } else {
            return saMgr->GetSystemAbility(systemAbilityId);
        }
    };
    EXPECT_CALL(*mockBundleMgr, GetBundleInstaller()).WillOnce(testing::Invoke(mockGetBundleInstaller));
    EXPECT_CALL(*mockSystemAbility_, GetSystemAbility(testing::_))
        .WillOnce(testing::Invoke(mockGetSystemAbility))
        .WillRepeatedly(testing::Invoke(mockGetSystemAbility));
}

/**
 * @tc.name: CreateBundleContext_0100
 * @tc.desc: Create bundle context test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(BmsContextImplTest, CreateBundleContext_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    MockBundleInstaller();
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // bundle name is empty
    auto context = contextImpl->CreateBundleContext("");
    EXPECT_EQ(context, nullptr);

    // bundle name is invalid
    context = contextImpl->CreateBundleContext("invalid_bundleName");
    EXPECT_EQ(context, nullptr);

    context = contextImpl->CreateBundleContext("test_contextImpl");
    EXPECT_NE(context, nullptr);

    // parent context is not nullptr
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    contextImpl->SetParentContext(parentContext);
    context = contextImpl->CreateBundleContext("");
    EXPECT_EQ(context, nullptr);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: CreateModuleContext_002
 * @tc.desc: Create module context test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(BmsContextImplTest, CreateModuleContext_002, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // bundleName is valid, but module name is empty
    auto moduleContext = contextImpl->CreateModuleContext("test_contextImpl", "");
    EXPECT_EQ(moduleContext, nullptr);

    // bundle name is invalid
    moduleContext = contextImpl->CreateModuleContext("invalid_bundleName", "invalid_moduleName");
    EXPECT_EQ(moduleContext, nullptr);

    // module didn't exist
    moduleContext = contextImpl->CreateModuleContext("test_contextImpl", "invalid_moduleName");
    EXPECT_EQ(moduleContext, nullptr);

    moduleContext = contextImpl->CreateModuleContext("test_contextImpl", "test_moduleName");
    EXPECT_NE(moduleContext, nullptr);

    HILOG_INFO("%{public}s end.", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS