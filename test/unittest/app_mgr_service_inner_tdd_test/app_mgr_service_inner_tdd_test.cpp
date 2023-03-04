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
#include <thread>

#define private public
#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "remote_client_manager.h"
#undef private
#include "app_scheduler.h"
#include "event_handler.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "mock_ability_token.h"
#include "mock_app_scheduler.h"
#include "mock_bundle_manager.h"
#include "mock_configuration_observer.h"
#include "mock_iapp_state_callback.h"
#include "mock_native_token.h"
#include "mock_render_scheduler.h"
#include "parameters.h"
#include "window_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
// static int recordId_ = 0;
class AppMgrServiceInnerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void InitAppInfo(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

public:
    std::shared_ptr<AbilityInfo> abilityInfo_;
    std::shared_ptr<ApplicationInfo> applicationInfo_;
};

void AppMgrServiceInnerTest::InitAppInfo(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ApplicationInfo applicationInfo;
    applicationInfo.name = appName;
    applicationInfo.bundleName = bundleName;
    applicationInfo_ = std::make_shared<ApplicationInfo>(applicationInfo);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    abilityInfo_ = std::make_shared<AbilityInfo>(abilityInfo);
}

void AppMgrServiceInnerTest::SetUpTestCase(void)
{
    MockNativeToken::SetNativeToken();
}

void AppMgrServiceInnerTest::TearDownTestCase(void)
{}

void AppMgrServiceInnerTest::SetUp()
{
    // init test app info
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    InitAppInfo(deviceName, abilityName, appName, bundleName, moduleName);
}

void AppMgrServiceInnerTest::TearDown()
{}

/**
 * @tc.name: SendProcessStartEvent_001
 * @tc.desc: Verify that the SendProcessStartEvent interface calls abnormal parameter
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_001, TestSize.Level1)
{
    HILOG_INFO("SendProcessStartEvent_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_FALSE(appMgrServiceInner->SendProcessStartEvent(nullptr));
    HILOG_INFO("SendProcessStartEvent_001 end");
}

/**
 * @tc.name: SendProcessStartEvent_002
 * @tc.desc: Verify that the SendProcessStartEvent interface calls just like a service called
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_002, TestSize.Level1)
{
    HILOG_INFO("SendProcessStartEvent_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord));
    HILOG_INFO("SendProcessStartEvent_002 end");
}

/**
 * @tc.name: SendProcessStartEvent_003
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a service called
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_003, TestSize.Level1)
{
    HILOG_INFO("SendProcessStartEvent_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    std::shared_ptr<AbilityRunningRecord> abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord));
    HILOG_INFO("SendProcessStartEvent_003 end");
}

/**
 * @tc.name: SendProcessStartEvent_004
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a service called
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_004, TestSize.Level1)
{
    HILOG_INFO("SendProcessStartEvent_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    std::shared_ptr<AbilityRunningRecord> abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityRunningRecord> abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord));
    HILOG_INFO("SendProcessStartEvent_004 end");
}

/**
 * @tc.name: SendProcessStartEvent_005
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a service called
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_005, TestSize.Level1)
{
    HILOG_INFO("SendProcessStartEvent_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    std::shared_ptr<AbilityRunningRecord> abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityRunningRecord> abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord));
    HILOG_INFO("SendProcessStartEvent_005 end");
}

/**
 * @tc.name: SendProcessStartEvent_006
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a service called
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_006, TestSize.Level1)
{
    HILOG_INFO("SendProcessStartEvent_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    std::shared_ptr<AbilityRunningRecord> abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityRunningRecord> abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord));
    HILOG_INFO("SendProcessStartEvent_006 end");
}

/**
 * @tc.name: SendProcessStartEvent_007
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a application called
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_007, TestSize.Level1)
{
    HILOG_INFO("SendProcessStartEvent_007 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    std::shared_ptr<AbilityRunningRecord> abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityRunningRecord> abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    auto &recordMap = appMgrServiceInner->appRunningManager_->appRunningRecordMap_;
    auto iter = recordMap.find(IPCSkeleton::GetCallingPid());
    if (iter == recordMap.end()) {
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    } else {
        recordMap.erase(iter);
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    }
    appRecord->GetPriorityObject()->pid_ = IPCSkeleton::GetCallingPid();
    appRecord->SetCallerPid(IPCSkeleton::GetCallingPid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord));
    HILOG_INFO("SendProcessStartEvent_007 end");
}

/**
 * @tc.name: SendProcessStartEvent_008
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a application called
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_008, TestSize.Level1)
{
    HILOG_INFO("SendProcessStartEvent_008 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    applicationInfo_->bundleName = "";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    std::shared_ptr<AbilityRunningRecord> abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityRunningRecord> abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    auto &recordMap = appMgrServiceInner->appRunningManager_->appRunningRecordMap_;
    auto iter = recordMap.find(IPCSkeleton::GetCallingPid());
    if (iter == recordMap.end()) {
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    } else {
        recordMap.erase(iter);
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    }
    appRecord->GetPriorityObject()->pid_ = IPCSkeleton::GetCallingPid();
    appRecord->SetCallerPid(IPCSkeleton::GetCallingPid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord));
    HILOG_INFO("SendProcessStartEvent_008 end");
}

/**
 * @tc.name: SendProcessExitEventTask_001
 * @tc.desc: Verify that the SendProcessExitEventTask interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessExitEventTask_001, TestSize.Level1)
{
    HILOG_INFO("SendProcessExitEventTask_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    auto runner = EventRunner::Create(Constants::APP_MGR_SERVICE_NAME);
    appMgrServiceInner->eventHandler_ = std::make_shared<AMSEventHandler>(runner, appMgrServiceInner);

    pid_t pid = -1;
    time_t exitTime = 0;
    int32_t count = 2;
    appMgrServiceInner->SendProcessExitEventTask(pid, exitTime, count);

    auto eventTask = [eventRunner = runner] () {
        eventRunner->Run();
    };
    std::thread testThread(eventTask);
    pid_t pid1 = IPCSkeleton::GetCallingPid();
    appMgrServiceInner->SendProcessExitEventTask(pid1, exitTime, count);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto stopEventTask = [eventRunner = runner] () {
        eventRunner->Stop();
    };
    appMgrServiceInner->eventHandler_->PostTask(stopEventTask, "stopEventTask");
    testThread.join();
    HILOG_INFO("SendProcessExitEventTask_001 end");
}

/**
 * @tc.name: SendProcessExitEvent_001
 * @tc.desc: Verify that the SendProcessExitEvent interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessExitEvent_001, TestSize.Level1)
{
    HILOG_INFO("SendProcessExitEvent_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    pid_t pid = -1;
    appMgrServiceInner->SendProcessExitEvent(pid);
    HILOG_INFO("SendProcessExitEvent_001 end");
}

/**
 * @tc.name: UpDateStartupType_001
 * @tc.desc: Verify that the UpDateStartupType interface calls abnormal parameter
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UpDateStartupType_001, TestSize.Level1)
{
    HILOG_INFO("UpDateStartupType_001 start");
    constexpr int32_t defaultVal = -1;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int32_t abilityType = -1;
    int32_t extensionType = -1;
    appMgrServiceInner->UpDateStartupType(nullptr, abilityType, extensionType);
    EXPECT_EQ(defaultVal, abilityType);
    EXPECT_EQ(defaultVal, extensionType);
    HILOG_INFO("UpDateStartupType_001 end");
}

/**
 * @tc.name: UpDateStartupType_002
 * @tc.desc: Verify that the UpDateStartupType interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UpDateStartupType_002, TestSize.Level1)
{
    HILOG_INFO("UpDateStartupType_002 start");
    constexpr int32_t expectedVal = 3;
    constexpr int32_t defaultVal = -1;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    auto info = std::make_shared<AbilityInfo>();
    info->type = static_cast<AbilityType>(expectedVal);
    int32_t abilityType = -1;
    int32_t extensionType = -1;
    appMgrServiceInner->UpDateStartupType(info, abilityType, extensionType);
    EXPECT_EQ(expectedVal, abilityType);
    EXPECT_EQ(defaultVal, extensionType);
    HILOG_INFO("UpDateStartupType_002 end");
}

/**
 * @tc.name: UpDateStartupType_003
 * @tc.desc: Verify that the UpDateStartupType interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UpDateStartupType_003, TestSize.Level1)
{
    HILOG_INFO("UpDateStartupType_003 start");
    constexpr int32_t expectedVal = 5;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    auto info = std::make_shared<AbilityInfo>();
    info->type = static_cast<AbilityType>(expectedVal);
    info->extensionAbilityType = static_cast<ExtensionAbilityType>(expectedVal);
    int32_t abilityType = -1;
    int32_t extensionType = -1;
    appMgrServiceInner->UpDateStartupType(info, abilityType, extensionType);
    EXPECT_EQ(expectedVal, abilityType);
    EXPECT_EQ(expectedVal, extensionType);
    HILOG_INFO("UpDateStartupType_003 end");
}
} // namespace AppExecFwk
} // namespace OHOS