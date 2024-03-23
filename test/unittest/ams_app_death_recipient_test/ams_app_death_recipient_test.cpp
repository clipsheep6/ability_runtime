/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "app_death_recipient.h"
#include "app_mgr_service_inner.h"
#include "iservice_registry.h"
#undef private

#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "mock_ability_token.h"
#include "mock_app_scheduler.h"
#include "mock_app_spawn_client.h"
#include "mock_bundle_installer_service.h"
#include "mock_bundle_manager_service.h"
#include "mock_system_ability_manager.h"
#include "singleton.h"

using namespace testing::ext;
using testing::_;
using testing::Return;
using testing::SetArgReferee;
using ::testing::DoAll;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
sptr<MockBundleInstallerService> mockBundleInstaller = new (std::nothrow) MockBundleInstallerService();
sptr<MockBundleManagerService> mockBundleMgr = new (std::nothrow) MockBundleManagerService();
} // namespace
class AppDeathRecipientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void MockBundleInstallerAndSA() const;
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;

public:
    const std::shared_ptr<AbilityInfo> GetAbilityInfoByIndex(const int32_t index) const;
    const std::shared_ptr<ApplicationInfo> GetApplicationByIndex(const int32_t index) const;
    const std::shared_ptr<AppRunningRecord> GetAppRunningRecordByIndex(const int32_t index) const;
    sptr<IRemoteObject> GetApp(int32_t pid, int size);

public:
    std::shared_ptr<AAFwk::TaskHandlerWrap> handler_;
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner_;
    sptr<AppDeathRecipient> appDeathRecipientObject_;
    OHOS::sptr<MockAbilityToken> mockToken_;
};

static void WaitUntilTaskFinished(std::shared_ptr<AAFwk::TaskHandlerWrap> handler)
{
    if (!handler) {
        return;
    }

    const uint32_t MAX_RETRY_COUNT = 1000;
    const uint32_t SLEEP_TIME = 1000;
    uint32_t count = 0;
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    if (handler->SubmitTask(f)) {
        while (!taskCalled.load()) {
            ++count;
            // if delay more than 1 second, break
            if (count >= MAX_RETRY_COUNT) {
                break;
            }

            usleep(SLEEP_TIME);
        }
    }
}

void AppDeathRecipientTest::SetUpTestCase()
{}

void AppDeathRecipientTest::TearDownTestCase()
{}

void AppDeathRecipientTest::SetUp()
{
    appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner_->Init();

    handler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("AppDeathRecipientTest");

    appDeathRecipientObject_ = new (std::nothrow) AppDeathRecipient();
    mockSystemAbility_ = new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
}

void AppDeathRecipientTest::MockBundleInstallerAndSA() const
{
    auto mockGetBundleInstaller = []() { return mockBundleInstaller; };
    auto mockGetSystemAbility = [&](int32_t systemAbilityId) {
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

void AppDeathRecipientTest::TearDown()
{
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
}

const std::shared_ptr<AbilityInfo> AppDeathRecipientTest::GetAbilityInfoByIndex(const int32_t index) const
{
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "AppDeathRecipientTest_ability" + std::to_string(index);
    abilityInfo->applicationName = "com.ohos.test.helloworld" + std::to_string(index);
    abilityInfo->applicationInfo.bundleName = "com.ohos.test.helloworld" + std::to_string(index);
    return abilityInfo;
}

const std::shared_ptr<ApplicationInfo> AppDeathRecipientTest::GetApplicationByIndex(const int32_t index) const
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = "com.ohos.test.helloworld" + std::to_string(index);
    appInfo->bundleName = "com.ohos.test.helloworld" + std::to_string(index);
    return appInfo;
}

const std::shared_ptr<AppRunningRecord> AppDeathRecipientTest::GetAppRunningRecordByIndex(const int32_t index) const
{
    auto appInfo = GetApplicationByIndex(index);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto appRecord = appMgrServiceInner_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, appInfo->name, appInfo->uid, bundleInfo);

    EXPECT_NE(nullptr, appRecord);
    return appRecord;
}

sptr<IRemoteObject> AppDeathRecipientTest::GetApp(int32_t pid, int size)
{
    EXPECT_CALL(*mockBundleMgr, GetHapModuleInfo(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(true))
        .WillRepeatedly(testing::Return(true));
    auto abilityInfo = GetAbilityInfoByIndex(pid);
    auto appInfo = GetApplicationByIndex(pid);
    sptr<IRemoteObject> token = new MockAbilityToken();

    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _)).Times(1).WillOnce(DoAll(SetArgReferee<1>(pid), Return(ERR_OK)));
    std::shared_ptr<MockAppSpawnClient> mockClientstr(mockClientPtr);
    appMgrServiceInner_->SetAppSpawnClient(mockClientstr);

    appMgrServiceInner_->LoadAbility(token, nullptr, abilityInfo, appInfo, nullptr, 0);

    auto appRecord = GetAppRunningRecordByIndex(pid);

    EXPECT_EQ(size, static_cast<int>(appMgrServiceInner_->GetRecentAppList().size()));

    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    sptr<IAppScheduler> client = iface_cast<IAppScheduler>(mockAppScheduler.GetRefPtr());
    appRecord->SetApplicationClient(client);

    return client->AsObject();
}

/*
 * Feature: Ams
 * Function: SetTaskHandler ande SetAppMgrServiceInner.
 * SubFunction: AppDeathRecipient
 * FunctionPoints: initialization
 * EnvConditions: have to an application
 * CaseDescription: How to set parameters
 */

HWTEST_F(AppDeathRecipientTest, AppDeathRecipient_001, TestSize.Level1)
{
    HILOG_INFO("AppDeathRecipient_001 start");
    appDeathRecipientObject_->SetTaskHandler(handler_);
    EXPECT_TRUE(appDeathRecipientObject_->handler_.lock() != nullptr);

    appDeathRecipientObject_->SetAppMgrServiceInner(appMgrServiceInner_);
    EXPECT_TRUE(appDeathRecipientObject_->appMgrServiceInner_.lock() != nullptr);
    HILOG_INFO("AppDeathRecipient_001 end");
}

/*
 * Feature: Ams
 * Function: OnRemoteDied
 * SubFunction: AppDeathRecipient
 * FunctionPoints: Applied death notification
 * EnvConditions: have to an application
 * CaseDescription: Call back the death notification of the notification application
 */
HWTEST_F(AppDeathRecipientTest, AppDeathRecipient_002, TestSize.Level1)
{
    HILOG_INFO("AppDeathRecipient_002 start");
    MockBundleInstallerAndSA();
    pid_t pid1 = 24;
    pid_t pid2 = 25;
    sptr<IRemoteObject> appOne = GetApp(pid1, 1);
    sptr<IRemoteObject> appTwo = GetApp(pid2, 2);

    appDeathRecipientObject_->SetTaskHandler(handler_);
    appDeathRecipientObject_->SetAppMgrServiceInner(appMgrServiceInner_);
    appDeathRecipientObject_->OnRemoteDied(appOne);

    WaitUntilTaskFinished(handler_);
    EXPECT_EQ(1, static_cast<int>(appDeathRecipientObject_->appMgrServiceInner_.lock()->GetRecentAppList().size()));
    HILOG_INFO("AppDeathRecipient_002 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
