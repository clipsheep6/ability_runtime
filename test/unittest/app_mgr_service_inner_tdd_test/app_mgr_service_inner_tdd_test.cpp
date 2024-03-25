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
#include "hilog_tag_wrapper.h"
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
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_FALSE(appMgrServiceInner->SendProcessStartEvent(nullptr));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_001 end");
}

/**
 * @tc.name: SendProcessStartEvent_002
 * @tc.desc: Verify that the SendProcessStartEvent interface calls just like a service called
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_002 end");
}

/**
 * @tc.name: SendProcessStartEvent_003
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a service called
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_003 start");
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
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_003 end");
}

/**
 * @tc.name: SendProcessStartEvent_004
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a service called
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_004 start");
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
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_004 end");
}

/**
 * @tc.name: SendProcessStartEvent_005
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a service called
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_005 start");
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
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_005 end");
}

/**
 * @tc.name: SendProcessStartEvent_006
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a service called
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_006 start");
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
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_006 end");
}

/**
 * @tc.name: SendProcessStartEvent_007
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a application called
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_007 start");
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
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
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
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_007 end");
}

/**
 * @tc.name: SendProcessStartEvent_008
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a application called
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_008 start");
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
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
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
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_008 end");
}

/**
 * @tc.name: SendProcessExitEvent_001
 * @tc.desc: Verify that the SendProcessExitEvent interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessExitEvent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessExitEvent_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    pid_t pid = -1;
    appMgrServiceInner->SendProcessExitEvent(pid);
    TAG_LOGI(AAFwkTag::TEST, "SendProcessExitEvent_001 end");
}

/**
 * @tc.name: CheckIsolationMode_001
 * @tc.desc: CheckIsolationMode
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, CheckIsolationMode_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckIsolationMode_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    HapModuleInfo hapModuleInfo;
    string supportIsolationMode = OHOS::system::GetParameter("persist.bms.supportIsolationMode", "false");
    if (supportIsolationMode.compare("true") == 0) {
        hapModuleInfo.isolationMode = IsolationMode::ISOLATION_FIRST;
        EXPECT_TRUE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
        hapModuleInfo.isolationMode = IsolationMode::ISOLATION_ONLY;
        EXPECT_TRUE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
        hapModuleInfo.isolationMode = IsolationMode::NONISOLATION_FIRST;
        EXPECT_FALSE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
        hapModuleInfo.isolationMode = IsolationMode::NONISOLATION_ONLY;
        EXPECT_FALSE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
    } else {
        hapModuleInfo.isolationMode = IsolationMode::ISOLATION_FIRST;
        EXPECT_FALSE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
        hapModuleInfo.isolationMode = IsolationMode::ISOLATION_ONLY;
        EXPECT_FALSE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
        hapModuleInfo.isolationMode = IsolationMode::NONISOLATION_FIRST;
        EXPECT_FALSE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
        hapModuleInfo.isolationMode = IsolationMode::NONISOLATION_ONLY;
        EXPECT_FALSE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
    }
    TAG_LOGI(AAFwkTag::TEST, "CheckIsolationMode_001 end");
}

/**
 * @tc.name: GenerateRenderUid_001
 * @tc.desc: Generate RenderUid
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GenerateRenderUid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateRenderUid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int32_t renderUid = Constants::INVALID_UID;
    EXPECT_TRUE(appMgrServiceInner->GenerateRenderUid(renderUid));
    int32_t renderUid1 = Constants::INVALID_UID;
    EXPECT_TRUE(appMgrServiceInner->GenerateRenderUid(renderUid1));
    TAG_LOGI(AAFwkTag::TEST, "GenerateRenderUid_001 end");
}

/**
 * @tc.name: StartRenderProcessImpl_001
 * @tc.desc: start render process.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcessImpl_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcessImpl_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo bundleInfo;
    std::string appName = "test_appName";
    std::string processName = "test_processName";
    std::string bundleName = "test_bundleName";
    sptr<IRemoteObject> token = new MockAbilityToken();
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    pid_t hostPid = 1;
    std::string renderParam = "test_render_param";
    int32_t ipcFd = 1;
    int32_t sharedFd = 1;
    int32_t crashFd = 1;
    std::shared_ptr<RenderRecord> renderRecord =
        RenderRecord::CreateRenderRecord(hostPid, renderParam, ipcFd, sharedFd, crashFd, appRecord);
    EXPECT_NE(renderRecord, nullptr);
    pid_t renderPid = 1;
    appMgrServiceInner->StartRenderProcessImpl(nullptr, nullptr, renderPid);
    appMgrServiceInner->StartRenderProcessImpl(renderRecord, appRecord, renderPid);
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcessImpl_001 end");
}

/**
 * @tc.name: UpDateStartupType_001
 * @tc.desc: Verify that the UpDateStartupType interface calls abnormal parameter
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UpDateStartupType_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpDateStartupType_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int32_t abilityType = -1;
    int32_t extensionType = -1;
    appMgrServiceInner->UpDateStartupType(nullptr, abilityType, extensionType);
    TAG_LOGI(AAFwkTag::TEST, "UpDateStartupType_001 end");
}

/**
 * @tc.name: UpDateStartupType_002
 * @tc.desc: Verify that the UpDateStartupType interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UpDateStartupType_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpDateStartupType_002 start");
    constexpr int32_t expectedVal = 3;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    auto info = std::make_shared<AbilityInfo>();
    info->type = static_cast<AbilityType>(expectedVal);
    int32_t abilityType = -1;
    int32_t extensionType = -1;
    appMgrServiceInner->UpDateStartupType(info, abilityType, extensionType);
    EXPECT_EQ(expectedVal, abilityType);
    TAG_LOGI(AAFwkTag::TEST, "UpDateStartupType_002 end");
}

/**
 * @tc.name: UpDateStartupType_003
 * @tc.desc: Verify that the UpDateStartupType interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UpDateStartupType_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpDateStartupType_003 start");
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
    TAG_LOGI(AAFwkTag::TEST, "UpDateStartupType_003 end");
}

/**
 * @tc.name: NotifyAppFault_001
 * @tc.desc: Verify that the NotifyAppFault interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyAppFault_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFault_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    FaultData faultData;
    EXPECT_EQ(ERR_INVALID_VALUE, appMgrServiceInner->NotifyAppFault(faultData));
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFault_001 end");
}

/**
 * @tc.name: NotifyAppFaultBySA_001
 * @tc.desc: Verify that the NotifyAppFaultBySA interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyAppFaultBySA_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFaultBySA_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    AppFaultDataBySA faultData;
    appMgrServiceInner->appRunningManager_ = nullptr;
    EXPECT_EQ(ERR_INVALID_VALUE, appMgrServiceInner->NotifyAppFaultBySA(faultData));
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFaultBySA_001 end");
}

/**
 * @tc.name: FaultTypeToString_001
 * @tc.desc: Verify that the FaultTypeToString interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, FaultTypeToString_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FaultTypeToString_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_EQ("CPP_CRASH", appMgrServiceInner->FaultTypeToString(AppExecFwk::FaultDataType::CPP_CRASH));
    EXPECT_EQ("JS_ERROR", appMgrServiceInner->FaultTypeToString(AppExecFwk::FaultDataType::JS_ERROR));
    EXPECT_EQ("APP_FREEZE", appMgrServiceInner->FaultTypeToString(AppExecFwk::FaultDataType::APP_FREEZE));
    EXPECT_EQ("PERFORMANCE_CONTROL",
        appMgrServiceInner->FaultTypeToString(AppExecFwk::FaultDataType::PERFORMANCE_CONTROL));
    EXPECT_EQ("RESOURCE_CONTROL", appMgrServiceInner->FaultTypeToString(AppExecFwk::FaultDataType::RESOURCE_CONTROL));
    TAG_LOGI(AAFwkTag::TEST, "FaultTypeToString_001 end");
}

/**
 * @tc.name: ChangeAppGcState_001
 * @tc.desc: Change app Gc state
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, ChangeAppGcState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ChangeAppGcState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int32_t pid = 0;
    int32_t state = 0;
    int32_t ret = appMgrServiceInner->ChangeAppGcState(pid, state);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "ChangeAppGcState_001 end");
}
} // namespace AppExecFwk
} // namespace OHOS