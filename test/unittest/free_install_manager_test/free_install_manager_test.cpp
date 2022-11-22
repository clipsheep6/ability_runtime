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
#include "free_install_manager.h"
#undef private
#include "ability_manager_service.h"
#include "mock_bundle_manager.h"
#include "mock_app_thread.h"
#include "ability_record.h"
#include "token.h"
#include "sa_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {
#define SLEEP(milli) std::this_thread::sleep_for(std::chrono::seconds(milli))

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
    std::shared_ptr<AbilityManagerService> service_ {nullptr};
    std::shared_ptr<FreeInstallManager> freeInstallManager_ = nullptr;
    std::shared_ptr<MockHandler> mockHandler_ = nullptr;
    std::shared_ptr<EventRunner> runner_ = nullptr;
};

void FreeInstallTest::SetUpTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void FreeInstallTest::TearDownTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void FreeInstallTest::SetUp(void)
{
    // service_ = std::make_shared<MockAbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(service_);
    runner_ = EventRunner::Create("AppkitNativeModuleTestMockHandlerFirst");
    mockHandler_ = std::make_shared<MockHandler>(runner_);
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
    auto handler = mockHandler_;
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

// success
HWTEST_F(FreeInstallTest, FreeInstall_StartFreeInstall_001, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    const int requestCode = 0;
    int64_t startInstallTime = 0;
    // mock callerToken
    const sptr<IRemoteObject> callerToken = MockToken();
    // 传到bms的callback完成处理，返回0
    int res = 0;
    auto task = [manager = freeInstallManager_, want, userId, requestCode, callerToken, &res](){
        res = manager->StartFreeInstall(want, userId, requestCode, callerToken);
    };
    mockHandler_->PostTask(task);

    usleep(100000);
    // from freeInstallManager_->freeInstallList_ find startInstallTime
    for (auto it = freeInstallManager_->freeInstallList_.begin(); it != freeInstallManager_->freeInstallList_.end();) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        if (want.GetElement().GetBundleName().compare(bundleName) != 0 ||
            want.GetElement().GetAbilityName().compare(abilityName) != 0) {
                startInstallTime = (*it).startInstallTime;
                break;
            }
    }
    freeInstallManager_->OnInstallFinished(0, want, userId, startInstallTime);
    WaitUntilTaskFinished();

    EXPECT_EQ(res, 0);
}

// failed
HWTEST_F(FreeInstallTest, FreeInstall_StartFreeInstall_002, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    const int requestCode = 0;
    // token is nullptr, IsTopAbility failed
    const sptr<IRemoteObject> callerToken = nullptr;
    // ams-mock返回NotTopAbility
    int res = freeInstallManager_->StartFreeInstall(want, userId, requestCode, callerToken);
    EXPECT_EQ(res, 0x500001);
}

// failed
HWTEST_F(FreeInstallTest, FreeInstall_StartFreeInstall_003, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    // mock token
    const sptr<IRemoteObject> callerToken = MockToken();
    // 传到bms的callback未处理，超时40s
    int res = freeInstallManager_->StartFreeInstall(want, userId, requestCode, callerToken);
    EXPECT_EQ(res, 29360300);
}

// failed
HWTEST_F(FreeInstallTest, FreeInstall_StartFreeInstall_004, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    int64_t startInstallTime = 0;
    // mock callerToken
    const sptr<IRemoteObject> callerToken = MockToken();
    // 传到bms的callback处理，返回非0
    // 通过handler进行StartFreeInstall，sleep几秒后再调OnInstallFinished触发回调
    int res = 0;
    auto task = [manager = freeInstallManager_, want, userId, requestCode, callerToken, &res](){
        res = manager->StartFreeInstall(want, userId, requestCode, callerToken);
    };
    mockHandler_->PostTask(task);

    usleep(100000);
    // from freeInstallManager_->freeInstallList_ find startInstallTime
    for (auto it = freeInstallManager_->freeInstallList_.begin(); it != freeInstallManager_->freeInstallList_.end();) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        if (want.GetElement().GetBundleName().compare(bundleName) != 0 ||
            want.GetElement().GetAbilityName().compare(abilityName) != 0) {
                startInstallTime = (*it).startInstallTime;
                break;
            }
    }
    freeInstallManager_->OnInstallFinished(1, want, userId, startInstallTime);
    WaitUntilTaskFinished();

    EXPECT_EQ(res, 1);
}

// success
// HWTEST_F(FreeInstallTest, FreeInstall_RemoteFreeInstall_001, TestSize.Level1)
// {
//     Want want;
//     const int32_t userId = 100;
//     const int requestCode = 0;
//     // mock callerToken
//     const sptr<IRemoteObject> callerToken = nullptr;
//     int res = freeInstallList_.RemoteFreeInstall(want, userId, requestCode, callerToken);
    
// }

// // failed
// HWTEST_F(FreeInstallTest, FreeInstall_RemoteFreeInstall_001, TestSize.Level1)
// {
//     Want want;
//     const int32_t userId = 100;
//     const int requestCode = 0;
//     // mock callerToken
//     const sptr<IRemoteObject> callerToken = nullptr;
//     // Not top ability
//     int res = freeInstallList_.RemoteFreeInstall(want, userId, requestCode, callerToken);
//     EXPECT_EQ(res, 0x500001);
// }

// // failed
// HWTEST_F(FreeInstallTest, FreeInstall_RemoteFreeInstall_001, TestSize.Level1)
// {
//     Want want;
//     const int32_t userId = 100;
//     const int requestCode = 0;
//     // mock callerToken
//     const sptr<IRemoteObject> callerToken = nullptr;
//     // dms failed
//     int res = freeInstallList_.RemoteFreeInstall(want, userId, requestCode, callerToken);
//     EXPECT_EQ(res, 0x500001);
// }

// HWTEST_F(FreeInstallTest, FreeInstall_OnInstallFinished_001, TestSize.Level1)
// {
//     Want want;
//     const int resultCode = 0;
//     const int32_t userId = 100;
//     FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, 0, nullptr);
//     const int32_t startInstallTime = info.startInstallTime;
//     int size = freeInstallManager_->freeInstallList_.size();
//     freeInstallManager_->freeInstallList_.emplace_back(info);
//     freeInstallManager_->OnInstallFinished(resultCode, want, userId, startInstallTime);
//     EXPECT_EQ(freeInstallManager_->freeInstallList_.size(), size);
// }

// HWTEST_F(FreeInstallTest, FreeInstall_OnRemoteInstallFinished_001, TestSize.Level1)
// {
//     Want want;
//     const int resultCode = 0;
//     const int32_t userId = 100;
//     FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, 0, nullptr);
//     const int32_t startInstallTime = info.startInstallTime;
//     int size = freeInstallManager_->freeInstallList_.size();
//     freeInstallManager_->freeInstallList_.emplace_back(info);
//     freeInstallManager_->OnRemoteInstallFinished(resultCode, want, userId, startInstallTime);
//     EXPECT_EQ(freeInstallManager_->freeInstallList_.size(), size);
// }

// HWTEST_F(FreeInstallTest, FreeInstall_FreeInstallAbilityFromRemote_001, TestSize.Level1)
// {
    
// }

// HWTEST_F(FreeInstallTest, FreeInstall_ConnectFreeInstall_001, TestSize.Level1)
// {
    
// }

// HWTEST_F(FreeInstallTest, AtomicServiceStatusCallback_OnInstallFinished_001, TestSize.Level1)
// {
    
// }

// HWTEST_F(FreeInstallTest, AtomicServiceStatusCallback_OnRemoteInstallFinished_001, TestSize.Level1)
// {
    
// }

// HWTEST_F(FreeInstallTest, AtomicServiceStatusCallbackStub_OnRemoteRequest_001, TestSize.Level1)
// {
    
// }

// HWTEST_F(FreeInstallTest, AtomicServiceStatusCallbackProxy_OnInstallFinished_001, TestSize.Level1)
// {
    
// }

// HWTEST_F(FreeInstallTest, AtomicServiceStatusCallbackProxy_OnRemoteInstallFinished_001, TestSize.Level1)
// {
    
// }
}  // namespace AppExecFwk
}  // namespace OHOS