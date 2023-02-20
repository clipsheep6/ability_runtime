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
#include "ability_manager_service.h"
#include "ability_event_handler.h"
#include "free_install_manager.h"
#undef private
#include "mock_bundle_manager.h"
#include "mock_app_thread.h"
#include "ability_record.h"
#include "token.h"
#include "sa_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
}
class FreeInstallTest : public testing::Test {
public:
    FreeInstallTest()
    {}
    ~FreeInstallTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<Token> MockToken();
    void WaitUntilTaskFinished();
    std::shared_ptr<AbilityManagerService> abilityMs_ = DelayedSingleton<AbilityManagerService>::GetInstance();
    std::shared_ptr<FreeInstallManager> freeInstallManager_ = nullptr;
};

void FreeInstallTest::SetUpTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void FreeInstallTest::TearDownTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void FreeInstallTest::SetUp(void)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    // runner_ = EventRunner::Create("AppkitNativeModuleTestMockHandlerFirst");
    abilityMs_->eventLoop_ = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    abilityMs_->handler_ = std::make_shared<AbilityEventHandler>(abilityMs_->eventLoop_, abilityMs_);
    // abilityMs_->handler_ = std::make_shared<MockHandler>(runner_);
}

void FreeInstallTest::TearDown(void)
{}

sptr<Token> FreeInstallTest::MockToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

void FreeInstallTest::WaitUntilTaskFinished()
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    auto handler = abilityMs_->handler_;
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    if (handler->PostTask(f)) {
        while (!taskCalled.load()) {
            ++count;
            if (count >= maxRetryCount) {
                break;
            }
            usleep(sleepTime);
        }
    }
}

/**
 * @tc.number: FreeInstall_StartFreeInstall_001
 * @tc.name: StartFreeInstall
 * @tc.desc: Test StartFreeInstall when callback is success.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartFreeInstall_001, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    // mock callerToken
    const sptr<IRemoteObject> callerToken = MockToken();
    int res = 0;
    auto task = [manager = freeInstallManager_, want, userId, requestCode, callerToken, &res]() {
        res = manager->StartFreeInstall(want, userId, requestCode, callerToken);
    };
    abilityMs_->handler_->PostTask(task);

    usleep(100000);
    // from freeInstallManager_->freeInstallList_ find startInstallTime
    for (auto it = freeInstallManager_->freeInstallList_.begin(); it != freeInstallManager_->freeInstallList_.end();) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        if (want.GetElement().GetBundleName().compare(bundleName) != 0 ||
            want.GetElement().GetAbilityName().compare(abilityName) != 0) {
            want.SetParam(Want::PARAM_RESV_START_TIME, (*it).want.GetStringParam(Want::PARAM_RESV_START_TIME));
            break;
        }
    }
    freeInstallManager_->OnInstallFinished(0, want, userId, false);
    WaitUntilTaskFinished();

    EXPECT_NE(res, 0);
}

/**
 * @tc.number: FreeInstall_StartFreeInstall_002
 * @tc.name: StartFreeInstall
 * @tc.desc: Test StartFreeInstall when token is nullptr.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartFreeInstall_002, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    const int requestCode = 0;
    // token is nullptr, IsTopAbility failed
    const sptr<IRemoteObject> callerToken = nullptr;
    // NotTopAbility
    int res = freeInstallManager_->StartFreeInstall(want, userId, requestCode, callerToken);
    EXPECT_EQ(res, 0x500001);
}

/**
 * @tc.number: FreeInstall_StartFreeInstall_003
 * @tc.name: StartFreeInstall
 * @tc.desc: Test StartFreeInstall when callback is failed.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartFreeInstall_003, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    // mock callerToken
    const sptr<IRemoteObject> callerToken = MockToken();
    int res = 0;
    auto task = [manager = freeInstallManager_, want, userId, requestCode, callerToken, &res]() {
        res = manager->StartFreeInstall(want, userId, requestCode, callerToken);
    };
    abilityMs_->handler_->PostTask(task);

    usleep(100000);
    // from freeInstallManager_->freeInstallList_ find startInstallTime
    for (auto it = freeInstallManager_->freeInstallList_.begin(); it != freeInstallManager_->freeInstallList_.end();) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        if (want.GetElement().GetBundleName().compare(bundleName) != 0 ||
            want.GetElement().GetAbilityName().compare(abilityName) != 0) {
            want.SetParam(Want::PARAM_RESV_START_TIME, (*it).want.GetStringParam(Want::PARAM_RESV_START_TIME));
            break;
        }
    }
    freeInstallManager_->OnInstallFinished(1, want, userId, false);
    WaitUntilTaskFinished();

    EXPECT_EQ(res, 5242881);
}

/**
 * @tc.number: FreeInstall_OnInstallFinished_001
 * @tc.name: OnInstallFinished
 * @tc.desc: Test OnInstallFinished succeed.
 */
HWTEST_F(FreeInstallTest, FreeInstall_OnInstallFinished_001, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));

    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr, false);
    freeInstallManager_->freeInstallList_.resize(0);
    freeInstallManager_->freeInstallList_.emplace_back(info);
    freeInstallManager_->OnInstallFinished(0, want, userId, false);

    for (auto it = freeInstallManager_->freeInstallList_.begin(); it != freeInstallManager_->freeInstallList_.end(); it++) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        std::string startTime = (*it).want.GetStringParam(Want::PARAM_RESV_START_TIME);
        if (want.GetElement().GetBundleName().compare(bundleName) == 0 &&
            want.GetElement().GetAbilityName().compare(abilityName) == 0 &&
            want.GetStringParam(Want::PARAM_RESV_START_TIME).compare(startTime) == 0) {
            EXPECT_EQ((*it).promise->get_future().get(), 0);
        }
    }
}

/**
 * @tc.number: FreeInstall_StartFreeInstall_004
 * @tc.name: OnInstallFinished
 * @tc.desc: Test OnInstallFinished failed.
 */
HWTEST_F(FreeInstallTest, FreeInstall_OnInstallFinished_002, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));

    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr, false);
    freeInstallManager_->freeInstallList_.resize(0);
    freeInstallManager_->freeInstallList_.emplace_back(info);
    freeInstallManager_->OnInstallFinished(1, want, userId, false);

    for (auto it = freeInstallManager_->freeInstallList_.begin(); it != freeInstallManager_->freeInstallList_.end(); it++) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        std::string startTime = (*it).want.GetStringParam(Want::PARAM_RESV_START_TIME);
        if (want.GetElement().GetBundleName().compare(bundleName) == 0 &&
            want.GetElement().GetAbilityName().compare(abilityName) == 0 &&
            want.GetStringParam(Want::PARAM_RESV_START_TIME).compare(startTime) == 0) {
            EXPECT_EQ((*it).promise->get_future().get(), 1);
        }
    }
}

/**
 * @tc.number: FreeInstall_OnInstallFinished_003
 * @tc.name: OnInstallFinished
 * @tc.desc: Test OnInstallFinished failed.
 */
HWTEST_F(FreeInstallTest, FreeInstall_OnInstallFinished_003, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    const int32_t userId = 1;
    const int requestCode = 0;

    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr, false);
    info.isInstalled = true;
    freeInstallManager_->freeInstallList_.resize(0);
    freeInstallManager_->freeInstallList_.emplace_back(info);
    freeInstallManager_->OnInstallFinished(0, want, userId, false);

    int size = freeInstallManager_->freeInstallList_.size();
    EXPECT_EQ(size, 1);
}

/**
 * @tc.number: FreeInstall_FreeInstallAbilityFromRemote_001
 * @tc.name: FreeInstallAbilityFromRemote
 * @tc.desc: Test FreeInstallAbilityFromRemote.
 */
HWTEST_F(FreeInstallTest, FreeInstall_FreeInstallAbilityFromRemote_001, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;

    int res = freeInstallManager_->FreeInstallAbilityFromRemote(want, nullptr, userId, requestCode);
    EXPECT_EQ(res, 22);
}

/**
 * @tc.number: FreeInstall_OnRemoteInstallFinished_001
 * @tc.name: OnRemoteInstallFinished
 * @tc.desc: Test OnRemoteInstallFinished.
 */
HWTEST_F(FreeInstallTest, FreeInstall_OnRemoteInstallFinished_001, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));

    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr, false);
    freeInstallManager_->freeInstallList_.resize(0);
    freeInstallManager_->freeInstallList_.emplace_back(info);
    freeInstallManager_->OnRemoteInstallFinished(0, want, userId);

    for (auto it = freeInstallManager_->freeInstallList_.begin(); it != freeInstallManager_->freeInstallList_.end(); it++) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        std::string startTime = (*it).want.GetStringParam(Want::PARAM_RESV_START_TIME);
        if (want.GetElement().GetBundleName().compare(bundleName) == 0 &&
            want.GetElement().GetAbilityName().compare(abilityName) == 0 &&
            want.GetStringParam(Want::PARAM_RESV_START_TIME).compare(startTime) == 0) {
            EXPECT_EQ((*it).promise->get_future().get(), 0);
        }
    }
}

/**
 * @tc.number: FreeInstall_ConnectFreeInstall_001
 * @tc.name: ConnectFreeInstall
 * @tc.desc: Test ConnectFreeInstall.
 */
HWTEST_F(FreeInstallTest, FreeInstall_ConnectFreeInstall_001, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;

    int res = freeInstallManager_->ConnectFreeInstall(want, userId, nullptr, "");
    EXPECT_NE(res, 0);
}
}  // namespace AppExecFwk
}  // namespace OHOS