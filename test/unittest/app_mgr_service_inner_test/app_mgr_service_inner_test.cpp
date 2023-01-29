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
static int recordId_ = 0;
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
 * @tc.name: PointerDeviceCallback_0100
 * @tc.desc: set parameter, expect config update
 * @tc.type: FUNC
 * @tc.require: I581UL
 */
HWTEST_F(AppMgrServiceInnerTest, PointerDeviceCallback_0100, TestSize.Level1)
{
    HILOG_INFO("PointerDeviceCallback_0100 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string key = AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE;
    auto context = new (std::nothrow) std::weak_ptr<AppMgrServiceInner>(appMgrServiceInner);
    std::shared_ptr<AppExecFwk::Configuration> config = nullptr;
    std::string value;

    // invalid parameter value
    appMgrServiceInner->PointerDeviceEventCallback(key.c_str(), "false", nullptr);
    appMgrServiceInner->PointerDeviceEventCallback("invalid_key", "false", context);
    appMgrServiceInner->PointerDeviceEventCallback(key.c_str(), "invalid", context);

    // set "input.pointer.device" to false
    appMgrServiceInner->PointerDeviceEventCallback(key.c_str(), "false", context);
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);
    value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    EXPECT_EQ(value, "false");

    // set "input.pointer.device" to true
    appMgrServiceInner->PointerDeviceEventCallback(key.c_str(), "true", context);
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);
    value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    EXPECT_EQ(value, "true");

    HILOG_INFO("PointerDeviceCallback_0100 end");
}

/**
 * @tc.name: PointerDeviceWatchParameter_0100
 * @tc.desc: set parameter, expect config update
 * @tc.type: FUNC
 * @tc.require: I581UL
 */
HWTEST_F(AppMgrServiceInnerTest, PointerDeviceWatchParameter_0100, TestSize.Level1)
{
    HILOG_INFO("PointerDeviceWatchParameter_0100 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string key = AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE;
    std::shared_ptr<AppExecFwk::Configuration> config = nullptr;
    std::string value;

    appMgrServiceInner->AddWatchParameter();
    sleep(1);

    // invalid parameter value
    system::SetParameter(key.c_str(), "invalid");
    sleep(1);

    // set "input.pointer.device" to false
    system::SetParameter(key.c_str(), "false");
    sleep(2); // sleep 2s, wait until UpdateConfiguration finished.
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);
    value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    EXPECT_EQ(value, "false");

    // set "input.pointer.device" to true
    system::SetParameter(key.c_str(), "true");
    sleep(2); // sleep 2s, wait until UpdateConfiguration finished.
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);
    value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    EXPECT_EQ(value, "true");

    HILOG_INFO("PointerDeviceWatchParameter_0100 end");
}

/**
 * @tc.name: PointerDeviceUpdateConfig_0100
 * @tc.desc: set parameter, expect config update
 * @tc.type: FUNC
 * @tc.require: I581UL
 */
HWTEST_F(AppMgrServiceInnerTest, PointerDeviceUpdateConfig_0100, TestSize.Level1)
{
    HILOG_INFO("PointerDeviceUpdateConfig_0100 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::shared_ptr<AppExecFwk::Configuration> config;
    std::string value;
    int32_t result;

    appMgrServiceInner->InitGlobalConfiguration();
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);
    value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    EXPECT_TRUE((value == "true") || (value == "false"));

    // config didn't change
    result = appMgrServiceInner->UpdateConfiguration(*config);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    Configuration changeConfig;
    if (value == "true") {
        changeConfig.AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "false");
        result = appMgrServiceInner->UpdateConfiguration(changeConfig);
        EXPECT_EQ(result, 0);
        config = appMgrServiceInner->GetConfiguration();
        EXPECT_NE(config, nullptr);
        value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
        EXPECT_EQ(value, "false");
    } else {
        changeConfig.AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "true");
        result = appMgrServiceInner->UpdateConfiguration(changeConfig);
        EXPECT_EQ(result, 0);
        config = appMgrServiceInner->GetConfiguration();
        EXPECT_NE(config, nullptr);
        value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
        EXPECT_EQ(value, "true");
    }

    HILOG_INFO("PointerDeviceUpdateConfig_0100 end");
}

/**
 * @tc.name: PreStartNWebSpawnProcess_001
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, PreStartNWebSpawnProcess_001, TestSize.Level0)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int callingPid = IPCSkeleton::GetCallingPid();
    int ret = appMgrServiceInner->PreStartNWebSpawnProcess(callingPid);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: PreStartNWebSpawnProcess_002
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, PreStartNWebSpawnProcess_002, TestSize.Level0)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int callingPid = 0;
    int ret = appMgrServiceInner->PreStartNWebSpawnProcess(callingPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadAbility_001
 * @tc.desc: load ability.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, LoadAbility_001, TestSize.Level0)
{
    HILOG_INFO("LoadAbility_001 start");
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->LoadAbility(token, nullptr, abilityInfo_, applicationInfo_, nullptr);

    auto appMgrServiceInner1 = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner1, nullptr);

    appMgrServiceInner1->remoteClientManager_->SetBundleManager(nullptr);
    appMgrServiceInner1->LoadAbility(token, nullptr, abilityInfo_, applicationInfo_, nullptr);

    auto appMgrServiceInner2 = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner2, nullptr);

    appMgrServiceInner2->LoadAbility(token, nullptr, abilityInfo_, applicationInfo_, nullptr);
    HILOG_INFO("LoadAbility_001 end");
}

/**
 * @tc.name: CheckLoadAbilityConditions_001
 * @tc.desc: check load ability conditions.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, CheckLoadAbilityConditions_001, TestSize.Level0)
{
    HILOG_INFO("CheckLoadAbilityConditions_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, nullptr, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, abilityInfo_, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, nullptr, applicationInfo_);

    appMgrServiceInner->CheckLoadAbilityConditions(token, nullptr, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, abilityInfo_, applicationInfo_);

    appMgrServiceInner->CheckLoadAbilityConditions(token, nullptr, applicationInfo_);

    appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, applicationInfo_);

    EXPECT_NE(appMgrServiceInner, nullptr);
    HILOG_INFO("CheckLoadAbilityConditions_001 end");
}

/**
 * @tc.name: MakeProcessName_001
 * @tc.desc: make process name.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, MakeProcessName_001, TestSize.Level0)
{
    HILOG_INFO("MakeProcessName_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    std::string processName = "test_processName";
    appMgrServiceInner->MakeProcessName(nullptr, nullptr, hapModuleInfo, 1, processName);
    appMgrServiceInner->MakeProcessName(nullptr, applicationInfo_, hapModuleInfo, 1, processName);
    appMgrServiceInner->MakeProcessName(abilityInfo_, nullptr, hapModuleInfo, 1, processName);
    appMgrServiceInner->MakeProcessName(abilityInfo_, applicationInfo_, hapModuleInfo, 1, processName);

    EXPECT_NE(appMgrServiceInner, nullptr);
    HILOG_INFO("MakeProcessName_001 end");
}

/**
 * @tc.name: MakeProcessName_002
 * @tc.desc: make process name.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, MakeProcessName_002, TestSize.Level0)
{
    HILOG_INFO("MakeProcessName_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    std::string processName = "test_processName";
    appMgrServiceInner->MakeProcessName(nullptr, hapModuleInfo, processName);
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    hapModuleInfo.isStageBasedModel = false;
    hapModuleInfo.process = "";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);

    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = "";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);

    hapModuleInfo.isStageBasedModel = false;
    hapModuleInfo.process = "test_process";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);

    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = "test_process";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);

    hapModuleInfo.isStageBasedModel = false;
    applicationInfo_->process = "";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);

    hapModuleInfo.isStageBasedModel = false;
    applicationInfo_->process = "test_process";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);

    EXPECT_NE(appMgrServiceInner, nullptr);
    HILOG_INFO("MakeProcessName_002 end");
}

/**
 * @tc.name: GetBundleAndHapInfo_001
 * @tc.desc: get bundle and hapInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetBundleAndHapInfo_001, TestSize.Level0)
{
    HILOG_INFO("GetBundleAndHapInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    appMgrServiceInner->GetBundleAndHapInfo(*abilityInfo_, applicationInfo_, bundleInfo, hapModuleInfo, 1);

    appMgrServiceInner->remoteClientManager_->SetBundleManager(nullptr);
    appMgrServiceInner->GetBundleAndHapInfo(*abilityInfo_, applicationInfo_, bundleInfo, hapModuleInfo, 1);
    HILOG_INFO("GetBundleAndHapInfo_001 end");
}

/**
 * @tc.name: AttachApplication_001
 * @tc.desc: attach application.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, AttachApplication_001, TestSize.Level0)
{
    HILOG_INFO("AttachApplication_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->AttachApplication(0, nullptr);

    appMgrServiceInner->AttachApplication(1, nullptr);

    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    sptr<IAppScheduler> client = iface_cast<IAppScheduler>(mockAppScheduler.GetRefPtr());
    appMgrServiceInner->AttachApplication(1, client);
    HILOG_INFO("AttachApplication_001 end");
}

/**
 * @tc.name: LaunchApplication_001
 * @tc.desc: launch application.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, LaunchApplication_001, TestSize.Level0)
{
    HILOG_INFO("LaunchApplication_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->LaunchApplication(nullptr);

    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info);
    recordId_ += 1;
    appMgrServiceInner->LaunchApplication(appRecord);

    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appMgrServiceInner->LaunchApplication(appRecord);

    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    appMgrServiceInner->LaunchApplication(appRecord);

    appRecord->SetKeepAliveAppState(false, true);
    appMgrServiceInner->LaunchApplication(appRecord);

    appRecord->SetKeepAliveAppState(true, false);
    appMgrServiceInner->LaunchApplication(appRecord);

    appRecord->SetKeepAliveAppState(true, true);
    appMgrServiceInner->LaunchApplication(appRecord);

    appRecord->SetKeepAliveAppState(false, false);
    appMgrServiceInner->LaunchApplication(appRecord);

    Want want;
    appRecord->SetSpecifiedAbilityFlagAndWant(false, want, "");
    appMgrServiceInner->LaunchApplication(appRecord);

    appRecord->SetSpecifiedAbilityFlagAndWant(true, want, "");
    appMgrServiceInner->LaunchApplication(appRecord);

    appMgrServiceInner->configuration_ = nullptr;
    appMgrServiceInner->LaunchApplication(appRecord);
    HILOG_INFO("LaunchApplication_001 end");
}

/**
 * @tc.name: AddAbilityStageDone_001
 * @tc.desc: add ability stage done.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, AddAbilityStageDone_001, TestSize.Level0)
{
    HILOG_INFO("AddAbilityStageDone_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->AddAbilityStageDone(99);

    BundleInfo info;
    std::string processName = "test_processName";
    appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info);
    recordId_ += 1;

    appMgrServiceInner->AddAbilityStageDone(recordId_);
    HILOG_INFO("AddAbilityStageDone_001 end");
}

/**
 * @tc.name: AddAbilityStageDone_001
 * @tc.desc: application foregrounded.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ApplicationForegrounded_001, TestSize.Level0)
{
    HILOG_INFO("ApplicationForegrounded_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->ApplicationForegrounded(99);

    BundleInfo info;
    std::string processName = "test_processName";
    appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info);
    recordId_ += 1;

    appMgrServiceInner->ApplicationForegrounded(recordId_);
    HILOG_INFO("ApplicationForegrounded_001 end");
}

/**
 * @tc.name: ApplicationBackgrounded_001
 * @tc.desc: application backgrounded.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ApplicationBackgrounded_001, TestSize.Level0)
{
    HILOG_INFO("ApplicationBackgrounded_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->Init();

    appMgrServiceInner->ApplicationBackgrounded(99);

    BundleInfo info;
    std::string processName = "test_processName";
    auto appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info);
    EXPECT_NE(appRecord, nullptr);
    recordId_ += 1;

    appMgrServiceInner->ApplicationBackgrounded(recordId_);

    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appMgrServiceInner->ApplicationBackgrounded(recordId_);

    HILOG_INFO("ApplicationBackgrounded_001 end");
}

/**
 * @tc.name: ApplicationTerminated_001
 * @tc.desc: application terminated.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ApplicationTerminated_001, TestSize.Level0)
{
    HILOG_INFO("ApplicationTerminated_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->ApplicationTerminated(99);

    BundleInfo info;
    std::string processName = "test_processName";
    auto appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info);
    EXPECT_NE(appRecord, nullptr);
    recordId_ += 1;

    appMgrServiceInner->ApplicationTerminated(recordId_);

    appRecord->SetKeepAliveAppState(false, true);
    appMgrServiceInner->ApplicationTerminated(recordId_);

    appRecord->SetKeepAliveAppState(true, false);
    appMgrServiceInner->ApplicationTerminated(recordId_);

    appRecord->SetKeepAliveAppState(true, true);
    appMgrServiceInner->ApplicationTerminated(recordId_);

    appRecord->SetKeepAliveAppState(false, false);
    appMgrServiceInner->ApplicationTerminated(recordId_);

    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appMgrServiceInner->ApplicationTerminated(recordId_);

    appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    appMgrServiceInner->ApplicationTerminated(recordId_);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->ApplicationTerminated(recordId_);

    HILOG_INFO("ApplicationTerminated_001 end");
}

/**
 * @tc.name: KillApplication_001
 * @tc.desc: kill application.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillApplication_001, TestSize.Level0)
{
    HILOG_INFO("KillApplication_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    appMgrServiceInner->KillApplication(bundleName);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->KillApplication(bundleName);

    HILOG_INFO("KillApplication_001 end");
}

/**
 * @tc.name: KillApplicationByUid_001
 * @tc.desc: kill application by uid.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillApplicationByUid_001, TestSize.Level0)
{
    HILOG_INFO("KillApplicationByUid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    appMgrServiceInner->KillApplicationByUid(bundleName, 0);

    appMgrServiceInner->remoteClientManager_->SetBundleManager(nullptr);
    appMgrServiceInner->KillApplicationByUid(bundleName, 0);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->KillApplicationByUid(bundleName, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->KillApplicationByUid(bundleName, 0);

    HILOG_INFO("KillApplicationByUid_001 end");
}

/**
 * @tc.name: KillApplicationSelf_001
 * @tc.desc: kill application self.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillApplicationSelf_001, TestSize.Level0)
{
    HILOG_INFO("KillApplicationSelf_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    EXPECT_EQ(appMgrServiceInner->KillApplicationSelf(), ERR_INVALID_VALUE);

    appMgrServiceInner->appRunningManager_ = nullptr;
    EXPECT_EQ(appMgrServiceInner->KillApplicationSelf(), ERR_NO_INIT);

    HILOG_INFO("KillApplicationSelf_001 end");
}

/**
 * @tc.name: KillApplicationByUserId_001
 * @tc.desc: kill application by user id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillApplicationByUserId_001, TestSize.Level0)
{
    HILOG_INFO("KillApplicationByUserId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    int result = appMgrServiceInner->KillApplicationByUserId(bundleName, 0);
    EXPECT_EQ(result, 0);

    appMgrServiceInner->remoteClientManager_->SetBundleManager(nullptr);
    appMgrServiceInner->KillApplicationByUserId(bundleName, 0);
    EXPECT_EQ(result, 0);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->KillApplicationByUserId(bundleName, 0);
    EXPECT_EQ(result, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->KillApplicationByUserId(bundleName, 0);
    EXPECT_EQ(result, 0);

    HILOG_INFO("KillApplicationByUserId_001 end");
}

/**
 * @tc.name: KillApplicationByUserIdLocked_001
 * @tc.desc: kill application by user id locked.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillApplicationByUserIdLocked_001, TestSize.Level0)
{
    HILOG_INFO("KillApplicationByUserIdLocked_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    int result = appMgrServiceInner->KillApplicationByUserIdLocked(bundleName, 0);
    EXPECT_EQ(result, 0);

    appMgrServiceInner->remoteClientManager_->SetBundleManager(nullptr);
    appMgrServiceInner->KillApplicationByUserIdLocked(bundleName, 0);
    EXPECT_EQ(result, 0);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->KillApplicationByUserIdLocked(bundleName, 0);
    EXPECT_EQ(result, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->KillApplicationByUserIdLocked(bundleName, 0);
    EXPECT_EQ(result, 0);

    HILOG_INFO("KillApplicationByUserIdLocked_001 end");
}

/**
 * @tc.name: ClearUpApplicationData_001
 * @tc.desc: clear up application data.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ClearUpApplicationData_001, TestSize.Level0)
{
    HILOG_INFO("ClearUpApplicationData_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    appMgrServiceInner->ClearUpApplicationData(bundleName, 0, 0);

    HILOG_INFO("ClearUpApplicationData_001 end");
}

/**
 * @tc.name: ClearUpApplicationDataByUserId_001
 * @tc.desc: clear up application data by user id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ClearUpApplicationDataByUserId_001, TestSize.Level0)
{
    HILOG_INFO("ClearUpApplicationDataByUserId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, 0, 0, 0);
    appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, 1, 0, 0);
    appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, 1, 1, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, 1, 1, 0);

    appMgrServiceInner->remoteClientManager_->SetBundleManager(nullptr);
    appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, 1, 1, 0);

    HILOG_INFO("ClearUpApplicationDataByUserId_001 end");
}

/**
 * @tc.name: GetAllRunningProcesses_001
 * @tc.desc: get all running processes.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetAllRunningProcesses_001, TestSize.Level0)
{
    HILOG_INFO("GetAllRunningProcesses_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<RunningProcessInfo> info;
    appMgrServiceInner->GetAllRunningProcesses(info);

    HILOG_INFO("GetAllRunningProcesses_001 end");
}

/**
 * @tc.name: GetProcessRunningInfosByUserId_001
 * @tc.desc: get process running infos by user id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetProcessRunningInfosByUserId_001, TestSize.Level0)
{
    HILOG_INFO("GetProcessRunningInfosByUserId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<RunningProcessInfo> info;
    appMgrServiceInner->GetProcessRunningInfosByUserId(info, 0);

    HILOG_INFO("GetProcessRunningInfosByUserId_001 end");
}

/**
 * @tc.name: NotifyMemoryLevel_001
 * @tc.desc: notify memory level.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyMemoryLevel_001, TestSize.Level0)
{
    HILOG_INFO("NotifyMemoryLevel_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<RunningProcessInfo> info;
    int result = appMgrServiceInner->NotifyMemoryLevel(0);
    EXPECT_EQ(result, 0);

    result = appMgrServiceInner->NotifyMemoryLevel(1);
    EXPECT_EQ(result, 0);

    result = appMgrServiceInner->NotifyMemoryLevel(2);
    EXPECT_EQ(result, 0);

    result = appMgrServiceInner->NotifyMemoryLevel(3);
    EXPECT_EQ(result, 22);

    appMgrServiceInner->appRunningManager_ = nullptr;
    result = appMgrServiceInner->NotifyMemoryLevel(3);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    HILOG_INFO("NotifyMemoryLevel_001 end");
}

/**
 * @tc.name: KillProcessByPid_001
 * @tc.desc: kill process by pid.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillProcessByPid_001, TestSize.Level0)
{
    HILOG_INFO("KillProcessByPid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int result = appMgrServiceInner->KillProcessByPid(0);
    EXPECT_EQ(result, -1);

    result = appMgrServiceInner->KillProcessByPid(1);
    EXPECT_EQ(result, 0);

    HILOG_INFO("KillProcessByPid_001 end");
}

/**
 * @tc.name: GetAllPids_001
 * @tc.desc: get all pids.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetAllPids_001, TestSize.Level0)
{
    HILOG_INFO("GetAllPids_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::list<pid_t> pids;
    bool result = appMgrServiceInner->GetAllPids(pids);
    EXPECT_FALSE(result);

    pids.push_back(1);
    result = appMgrServiceInner->GetAllPids(pids);
    EXPECT_TRUE(result);

    std::string appName = "test_appName";
    std::string processName = "test_processName";
    appMgrServiceInner->appProcessManager_->AddAppToRecentList(appName, processName, 0, 0);
    result = appMgrServiceInner->GetAllPids(pids);
    EXPECT_TRUE(result);

    HILOG_INFO("GetAllPids_001 end");
}

/**
 * @tc.name: ProcessExist_001
 * @tc.desc: process exist.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ProcessExist_001, TestSize.Level0)
{
    HILOG_INFO("ProcessExist_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t pid = 0;
    bool result = appMgrServiceInner->ProcessExist(pid);
    EXPECT_FALSE(result);

    HILOG_INFO("ProcessExist_001 end");
}

/**
 * @tc.name: CreateAppRunningRecord_001
 * @tc.desc: create app running record.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, CreateAppRunningRecord_001, TestSize.Level0)
{
    HILOG_INFO("CreateAppRunningRecord_001 start");
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";

    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(nullptr, nullptr,
        nullptr, nullptr, "", bundleInfo, hapModuleInfo, nullptr);
    EXPECT_EQ(appRecord, nullptr);

    appRecord = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        nullptr, nullptr, "", bundleInfo, hapModuleInfo, nullptr);
    EXPECT_EQ(appRecord, nullptr);

    appRecord = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, nullptr, "", bundleInfo, hapModuleInfo, nullptr);
    EXPECT_EQ(appRecord, nullptr);

    appRecord = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, abilityInfo_, "", bundleInfo, hapModuleInfo, nullptr);
    EXPECT_EQ(appRecord, nullptr);

    appRecord = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, nullptr);
    EXPECT_NE(appRecord, nullptr);

    appRecord = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, nullptr);
    EXPECT_NE(appRecord, nullptr);

    appRecord = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, nullptr);
    EXPECT_NE(appRecord, nullptr);

    std::shared_ptr<AppRunningRecord> appRecord1 = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        nullptr, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_EQ(appRecord1, nullptr);

    std::shared_ptr<AppRunningRecord> appRecord2 = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord2, nullptr);

    want = std::make_shared<Want>();
    const std::string COLD_START = "coldStart";
    want->SetParam(COLD_START, true);
    std::shared_ptr<AppRunningRecord> appRecord3 = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord3, nullptr);

    want->SetParam(COLD_START, false);
    std::shared_ptr<AppRunningRecord> appRecord4 = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord4, nullptr);

    appMgrServiceInner->appRunningManager_ = nullptr;
    std::shared_ptr<AppRunningRecord> appRecord5 = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_EQ(appRecord5, nullptr);

    HILOG_INFO("CreateAppRunningRecord_001 end");
}

/**
 * @tc.name: TerminateAbility_001
 * @tc.desc: terminate ability.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, TerminateAbility_001, TestSize.Level0)
{
    HILOG_INFO("TerminateAbility_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->TerminateAbility(nullptr, true);
    appMgrServiceInner->TerminateAbility(nullptr, false);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->TerminateAbility(token, true);
    appMgrServiceInner->TerminateAbility(token, false);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->TerminateAbility(token, true);

    HILOG_INFO("TerminateAbility_001 end");
}

/**
 * @tc.name: UpdateAbilityState_001
 * @tc.desc: update ability state.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, UpdateAbilityState_001, TestSize.Level0)
{
    HILOG_INFO("UpdateAbilityState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->UpdateAbilityState(nullptr, AbilityState::ABILITY_STATE_CREATE);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->UpdateAbilityState(token, AbilityState::ABILITY_STATE_CREATE);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, nullptr, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->UpdateAbilityState(token, AbilityState::ABILITY_STATE_CREATE);

    OHOS::sptr<IRemoteObject> token1 = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AppRunningRecord> appRecord1 = appMgrServiceInner->CreateAppRunningRecord(token1, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord1, nullptr);

    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_READY);
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_CREATE);

    auto abilityRecord1 =
        appMgrServiceInner->GetAppRunningRecordByAbilityToken(token1)->GetAbilityRunningRecordByToken(token1);
    abilityRecord1->SetState(AbilityState::ABILITY_STATE_TERMINATED);
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_TERMINATED);

    abilityRecord1->SetState(AbilityState::ABILITY_STATE_CONNECTED);
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_CONNECTED);

    abilityRecord1->SetState(AbilityState::ABILITY_STATE_DISCONNECTED);
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_DISCONNECTED);

    abilityRecord1->SetState(AbilityState::ABILITY_STATE_END);
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_END);

    abilityRecord1->SetState(AbilityState::ABILITY_STATE_BACKGROUND);
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_BACKGROUND);

    OHOS::sptr<IRemoteObject> token2 = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    abilityInfo_->type = AbilityType::SERVICE;
    std::shared_ptr<AppRunningRecord> appRecord2 = appMgrServiceInner->CreateAppRunningRecord(token2, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord2, nullptr);
    appMgrServiceInner->UpdateAbilityState(token2, AbilityState::ABILITY_STATE_CREATE);

    auto abilityRecord2 =
        appMgrServiceInner->GetAppRunningRecordByAbilityToken(token2)->GetAbilityRunningRecordByToken(token2);
    abilityRecord2->SetState(AbilityState::ABILITY_STATE_TERMINATED);
    appMgrServiceInner->UpdateAbilityState(token2, AbilityState::ABILITY_STATE_TERMINATED);

    abilityRecord2->SetState(AbilityState::ABILITY_STATE_CONNECTED);
    appMgrServiceInner->UpdateAbilityState(token2, AbilityState::ABILITY_STATE_CONNECTED);

    abilityRecord2->SetState(AbilityState::ABILITY_STATE_DISCONNECTED);
    appMgrServiceInner->UpdateAbilityState(token2, AbilityState::ABILITY_STATE_DISCONNECTED);

    abilityRecord2->SetState(AbilityState::ABILITY_STATE_END);
    appMgrServiceInner->UpdateAbilityState(token2, AbilityState::ABILITY_STATE_END);

    abilityRecord2->SetState(AbilityState::ABILITY_STATE_BACKGROUND);
    appMgrServiceInner->UpdateAbilityState(token2, AbilityState::ABILITY_STATE_BACKGROUND);

    HILOG_INFO("UpdateAbilityState_001 end");
}

/**
 * @tc.name: UpdateExtensionState_001
 * @tc.desc: update extension state.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, UpdateExtensionState_001, TestSize.Level0)
{
    HILOG_INFO("UpdateExtensionState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->UpdateExtensionState(nullptr, ExtensionState::EXTENSION_STATE_CREATE);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->UpdateExtensionState(token, ExtensionState::EXTENSION_STATE_CREATE);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->UpdateExtensionState(token, ExtensionState::EXTENSION_STATE_CREATE);

    HILOG_INFO("UpdateExtensionState_001 end");
}

/**
 * @tc.name: OpenAppSpawnConnection_001
 * @tc.desc: open app spawn connection.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, OpenAppSpawnConnection_001, TestSize.Level0)
{
    HILOG_INFO("OpenAppSpawnConnection_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->remoteClientManager_->SetSpawnClient(nullptr);
    auto errorCode = appMgrServiceInner->OpenAppSpawnConnection();
    EXPECT_EQ(errorCode, ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    auto errorCode1 = appMgrServiceInner->OpenAppSpawnConnection();
    EXPECT_EQ(errorCode1, ERR_INVALID_VALUE);

    HILOG_INFO("OpenAppSpawnConnection_001 end");
}

/**
 * @tc.name: CloseAppSpawnConnection_001
 * @tc.desc: close app spawn connection.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, CloseAppSpawnConnection_001, TestSize.Level0)
{
    HILOG_INFO("CloseAppSpawnConnection_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->CloseAppSpawnConnection();

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->CloseAppSpawnConnection();

    HILOG_INFO("CloseAppSpawnConnection_001 end");
}

/**
 * @tc.name: QueryAppSpawnConnectionState_001
 * @tc.desc: query app spawn connection.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, QueryAppSpawnConnectionState_001, TestSize.Level0)
{
    HILOG_INFO("QueryAppSpawnConnectionState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    auto connectionState = appMgrServiceInner->QueryAppSpawnConnectionState();
    EXPECT_EQ(connectionState, SpawnConnectionState::STATE_NOT_CONNECT);

    appMgrServiceInner->remoteClientManager_->SetSpawnClient(nullptr);
    connectionState = appMgrServiceInner->QueryAppSpawnConnectionState();
    EXPECT_EQ(connectionState, SpawnConnectionState::STATE_NOT_CONNECT);


    appMgrServiceInner->remoteClientManager_ = nullptr;
    connectionState = appMgrServiceInner->QueryAppSpawnConnectionState();
    EXPECT_EQ(connectionState, SpawnConnectionState::STATE_NOT_CONNECT);

    HILOG_INFO("QueryAppSpawnConnectionState_001 end");
}

/**
 * @tc.name: GetRecordMap_001
 * @tc.desc: get record map.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetRecordMap_001, TestSize.Level0)
{
    HILOG_INFO("GetRecordMap_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::map<const int32_t, const std::shared_ptr<AppRunningRecord>> appRunningRecordMap;

    auto appRunningRecordMap1 = appMgrServiceInner->GetRecordMap();
    EXPECT_EQ(appRunningRecordMap1, appRunningRecordMap);

    appMgrServiceInner->appRunningManager_ = nullptr;
    auto appRunningRecordMap2 = appMgrServiceInner->GetRecordMap();
    EXPECT_EQ(appRunningRecordMap2, appRunningRecordMap);

    HILOG_INFO("GetRecordMap_001 end");
}

/**
 * @tc.name: SetAppSpawnClient_001
 * @tc.desc: set app spawn client.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SetAppSpawnClient_001, TestSize.Level0)
{
    HILOG_INFO("SetAppSpawnClient_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::shared_ptr<AppSpawnClient> spawnClient;
    appMgrServiceInner->SetAppSpawnClient(spawnClient);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->SetAppSpawnClient(spawnClient);

    HILOG_INFO("SetAppSpawnClient_001 end");
}

/**
 * @tc.name: SetBundleManager_001
 * @tc.desc: set bundle manager.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SetBundleManager_001, TestSize.Level0)
{
    HILOG_INFO("SetBundleManager_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<IBundleMgr> bundleManager;
    appMgrServiceInner->SetBundleManager(bundleManager);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->SetBundleManager(bundleManager);

    HILOG_INFO("SetBundleManager_001 end");
}

/**
 * @tc.name: RegisterAppStateCallback_001
 * @tc.desc: register app state call back.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterAppStateCallback_001, TestSize.Level0)
{
    HILOG_INFO("RegisterAppStateCallback_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->RegisterAppStateCallback(nullptr);

    sptr<IAppStateCallback> callback;
    appMgrServiceInner->RegisterAppStateCallback(callback);

    HILOG_INFO("RegisterAppStateCallback_001 end");
}

/**
 * @tc.name: AbilityBehaviorAnalysis_001
 * @tc.desc: ability behavior analysis.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, AbilityBehaviorAnalysis_001, TestSize.Level0)
{
    HILOG_INFO("AbilityBehaviorAnalysis_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->AbilityBehaviorAnalysis(nullptr, nullptr, 0, 0, 0);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->AbilityBehaviorAnalysis(token, nullptr, 0, 0, 0);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->AbilityBehaviorAnalysis(token, nullptr, 0, 0, 0);

    OHOS::sptr<IRemoteObject> preToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->AbilityBehaviorAnalysis(token, preToken, 0, 0, 0);

    HILOG_INFO("AbilityBehaviorAnalysis_001 end");
}

/**
 * @tc.name: KillProcessByAbilityToken_001
 * @tc.desc: kill process by ability token.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillProcessByAbilityToken_001, TestSize.Level0)
{
    HILOG_INFO("KillProcessByAbilityToken_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->KillProcessByAbilityToken(nullptr);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->KillProcessByAbilityToken(token);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->KillProcessByAbilityToken(token);

    appRecord->SetKeepAliveAppState(true, true);
    appMgrServiceInner->KillProcessByAbilityToken(token);

    HILOG_INFO("KillProcessByAbilityToken_001 end");
}

/**
 * @tc.name: KillProcessesByUserId_001
 * @tc.desc: kill process by user id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillProcessesByUserId_001, TestSize.Level0)
{
    HILOG_INFO("KillProcessesByUserId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->KillProcessesByUserId(0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->KillProcessesByUserId(0);

    HILOG_INFO("KillProcessesByUserId_001 end");
}

/**
 * @tc.name: StartAbility_001
 * @tc.desc: start ability.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartAbility_001, TestSize.Level0)
{
    HILOG_INFO("StartAbility_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::shared_ptr<AppRunningRecord> appRecord;
    appMgrServiceInner->StartAbility(nullptr, nullptr, abilityInfo_, nullptr, hapModuleInfo, nullptr);
    appMgrServiceInner->StartAbility(nullptr, nullptr, abilityInfo_, appRecord, hapModuleInfo, nullptr);
    appMgrServiceInner->StartAbility(nullptr, nullptr, abilityInfo_, appRecord, hapModuleInfo, want);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    OHOS::sptr<IRemoteObject> preToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->StartAbility(token, nullptr, abilityInfo_, appRecord, hapModuleInfo, want);
    appMgrServiceInner->StartAbility(nullptr, preToken, abilityInfo_, appRecord, hapModuleInfo, want);
    appMgrServiceInner->StartAbility(token, preToken, abilityInfo_, appRecord, hapModuleInfo, want);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    appRecord = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->StartAbility(token, nullptr, abilityInfo_, appRecord, hapModuleInfo, want);
    appMgrServiceInner->StartAbility(token, preToken, abilityInfo_, appRecord, hapModuleInfo, want);

    abilityInfo_->applicationInfo.name = "hiservcie";
    abilityInfo_->applicationInfo.bundleName = "com.ix.hiservcie";
    appMgrServiceInner->StartAbility(token, preToken, abilityInfo_, appRecord, hapModuleInfo, want);

    HILOG_INFO("StartAbility_001 end");
}

/**
 * @tc.name: GetAppRunningRecordByAbilityToken_001
 * @tc.desc: get app running record by ability token.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetAppRunningRecordByAbilityToken_001, TestSize.Level0)
{
    HILOG_INFO("GetAppRunningRecordByAbilityToken_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->GetAppRunningRecordByAbilityToken(token);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->GetAppRunningRecordByAbilityToken(token);

    HILOG_INFO("GetAppRunningRecordByAbilityToken_001 end");
}

/**
 * @tc.name: AbilityTerminated_001
 * @tc.desc: ability terminated.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, AbilityTerminated_001, TestSize.Level0)
{
    HILOG_INFO("AbilityTerminated_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->AbilityTerminated(nullptr);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->AbilityTerminated(token);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->AbilityTerminated(token);

    HILOG_INFO("AbilityTerminated_001 end");
}

/**
 * @tc.name: GetAppRunningRecordByAppRecordId_001
 * @tc.desc: get app running record by app record id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetAppRunningRecordByAppRecordId_001, TestSize.Level0)
{
    HILOG_INFO("GetAppRunningRecordByAppRecordId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->GetAppRunningRecordByAppRecordId(0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->GetAppRunningRecordByAppRecordId(0);

    HILOG_INFO("GetAppRunningRecordByAppRecordId_001 end");
}

/**
 * @tc.name: OnAppStateChanged_001
 * @tc.desc: on app state changed.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, OnAppStateChanged_001, TestSize.Level0)
{
    HILOG_INFO("OnAppStateChanged_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->OnAppStateChanged(nullptr, ApplicationState::APP_STATE_CREATE, true);
    appMgrServiceInner->OnAppStateChanged(nullptr, ApplicationState::APP_STATE_CREATE, false);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->OnAppStateChanged(appRecord, ApplicationState::APP_STATE_CREATE, true);

    sptr<MockAppStateCallback> mockCallback(new MockAppStateCallback());
    EXPECT_CALL(*mockCallback, OnAppStateChanged(_)).Times(2);
    sptr<IAppStateCallback> callback1 = iface_cast<IAppStateCallback>(mockCallback);
    appMgrServiceInner->appStateCallbacks_.push_back(callback1);
    appMgrServiceInner->OnAppStateChanged(appRecord, ApplicationState::APP_STATE_CREATE, true);

    sptr<IAppStateCallback> callback;
    appMgrServiceInner->appStateCallbacks_.push_back(callback);
    appMgrServiceInner->OnAppStateChanged(appRecord, ApplicationState::APP_STATE_CREATE, true);

    HILOG_INFO("OnAppStateChanged_001 end");
}

/**
 * @tc.name: OnAbilityStateChanged_001
 * @tc.desc: on ability state changed.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, OnAbilityStateChanged_001, TestSize.Level0)
{
    HILOG_INFO("OnAbilityStateChanged_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->OnAbilityStateChanged(nullptr, AbilityState::ABILITY_STATE_CREATE);

    sptr<IRemoteObject> token = new MockAbilityToken();
    std::shared_ptr<AbilityRunningRecord> abilityRunningRecord =
        std::make_shared<AbilityRunningRecord>(abilityInfo_, token);
    appMgrServiceInner->OnAbilityStateChanged(abilityRunningRecord, AbilityState::ABILITY_STATE_CREATE);

    sptr<MockAppStateCallback> mockCallback(new MockAppStateCallback());
    EXPECT_CALL(*mockCallback, OnAbilityRequestDone(_, _)).Times(2);
    sptr<IAppStateCallback> callback1 = iface_cast<IAppStateCallback>(mockCallback);
    appMgrServiceInner->appStateCallbacks_.push_back(callback1);
    appMgrServiceInner->OnAbilityStateChanged(abilityRunningRecord, AbilityState::ABILITY_STATE_CREATE);

    sptr<IAppStateCallback> callback;
    appMgrServiceInner->appStateCallbacks_.push_back(callback);
    appMgrServiceInner->OnAbilityStateChanged(abilityRunningRecord, AbilityState::ABILITY_STATE_CREATE);

    HILOG_INFO("OnAbilityStateChanged_001 end");
}

/**
 * @tc.name: StartProcess_001
 * @tc.desc: start process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartProcess_001, TestSize.Level0)
{
    HILOG_INFO("StartProcess_001 start");
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
    appMgrServiceInner->StartProcess(appName, processName, 0, nullptr, 0, bundleName, 0);
    appMgrServiceInner->StartProcess(appName, processName, 0, appRecord, 0, bundleName, 0);
    appMgrServiceInner->StartProcess(appName, processName, 0, appRecord, 0, bundleName, 1);

    appMgrServiceInner->SetBundleManager(nullptr);
    appMgrServiceInner->StartProcess(appName, processName, 0, appRecord, 0, bundleName, 0);

    appMgrServiceInner->SetAppSpawnClient(nullptr);
    appMgrServiceInner->StartProcess(appName, processName, 0, nullptr, 0, bundleName, 0);
    appMgrServiceInner->StartProcess(appName, processName, 0, appRecord, 0, bundleName, 0);

    HILOG_INFO("StartProcess_001 end");
}

/**
 * @tc.name: RemoveAppFromRecentList_001
 * @tc.desc: remove app from recent list.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, RemoveAppFromRecentList_001, TestSize.Level0)
{
    HILOG_INFO("RemoveAppFromRecentList_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string appName = "test_appName";
    std::string processName = "test_processName";
    appMgrServiceInner->RemoveAppFromRecentList(appName, processName);

    appMgrServiceInner->AddAppToRecentList(appName, processName, 0, 0);
    appMgrServiceInner->RemoveAppFromRecentList(appName, processName);

    appMgrServiceInner->ClearRecentAppList();
    BundleInfo bundleInfo;
    std::string appName1 = "hiservcie";
    std::string processName1 = "hiservcie_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName1, bundleInfo);
    HILOG_INFO("RemoveAppFromRecentList_001 start 22");

    pid_t pid = 123;
    std::string renderParam = "test_renderParam";
    std::shared_ptr<RenderRecord> renderRecord = RenderRecord::CreateRenderRecord(pid, renderParam, 1, 1, appRecord);
    appRecord->SetRenderRecord(renderRecord);
    appMgrServiceInner->AddAppToRecentList(appName1, processName1, pid, 0);
    appRecord->SetKeepAliveAppState(true, true);
    appMgrServiceInner->RemoveAppFromRecentList(appName1, processName1);
    appRecord->SetKeepAliveAppState(false, false);
    appMgrServiceInner->RemoveAppFromRecentList(appName1, processName1);

    HILOG_INFO("RemoveAppFromRecentList_001 end");
}

/**
 * @tc.name: ClearRecentAppList_001
 * @tc.desc: clear recent list.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ClearRecentAppList_001, TestSize.Level0)
{
    HILOG_INFO("ClearRecentAppList_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->ClearRecentAppList();
    std::list<const std::shared_ptr<AppTaskInfo>> list = appMgrServiceInner->GetRecentAppList();
    EXPECT_EQ(list.size(), 0);

    HILOG_INFO("ClearRecentAppList_001 end");
}

/**
 * @tc.name: OnRemoteDied_001
 * @tc.desc: on remote died.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, OnRemoteDied_001, TestSize.Level0)
{
    HILOG_INFO("OnRemoteDied_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<IRemoteObject> remoteObject;
    appMgrServiceInner->OnRemoteDied(remoteObject, true);
    appMgrServiceInner->OnRemoteDied(remoteObject, false);

    HILOG_INFO("OnRemoteDied_001 end");
}

/**
 * @tc.name: ClearAppRunningData_001
 * @tc.desc: clear app running data.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ClearAppRunningData_001, TestSize.Level0)
{
    HILOG_INFO("ClearAppRunningData_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->ClearAppRunningData(nullptr, true);

    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info);
    appMgrServiceInner->ClearAppRunningData(appRecord, true);
    appMgrServiceInner->ClearAppRunningData(appRecord, false);

    std::shared_ptr<RenderRecord> renderRecord;
    appRecord->SetRenderRecord(renderRecord);
    appMgrServiceInner->ClearAppRunningData(appRecord, false);

    pid_t pid = 123;
    std::string renderParam = "test_renderParam";
    std::shared_ptr<RenderRecord> renderRecord1 = RenderRecord::CreateRenderRecord(pid, renderParam, 1, 1, appRecord);
    appRecord->SetRenderRecord(renderRecord1);
    appMgrServiceInner->ClearAppRunningData(appRecord, false);

    appRecord->SetKeepAliveAppState(true, true);
    appMgrServiceInner->ClearAppRunningData(appRecord, false);

    appMgrServiceInner->eventHandler_ = nullptr;
    appMgrServiceInner->ClearAppRunningData(appRecord, false);

    appRecord->restartResidentProcCount_ = 0;
    appMgrServiceInner->ClearAppRunningData(appRecord, false);

    appRecord->appInfo_ = nullptr;
    appMgrServiceInner->ClearAppRunningData(appRecord, false);

    HILOG_INFO("ClearAppRunningData_001 end");
}

/**
 * @tc.name: AddAppDeathRecipient_001
 * @tc.desc: add app death recipient.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, AddAppDeathRecipient_001, TestSize.Level0)
{
    HILOG_INFO("AddAppDeathRecipient_001 start");
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
    sptr<AppDeathRecipient> appDeathRecipient;
    pid_t pid = 999;
    appMgrServiceInner->AddAppDeathRecipient(pid, appDeathRecipient);

    pid_t pid1 = 123;
    appMgrServiceInner->AddAppDeathRecipient(pid1, appDeathRecipient);

    HILOG_INFO("AddAppDeathRecipient_001 end");
}

/**
 * @tc.name: HandleTimeOut_001
 * @tc.desc: handle time out.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleTimeOut_001, TestSize.Level0)
{
    HILOG_INFO("HandleTimeOut_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    InnerEvent::Pointer innerEvent = InnerEvent::Pointer(nullptr, nullptr);
    appMgrServiceInner->HandleTimeOut(innerEvent);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->HandleTimeOut(innerEvent);

    HILOG_INFO("HandleTimeOut_001 end");
}

/**
 * @tc.name: HandleAbilityAttachTimeOut_001
 * @tc.desc: handle ability attach time out.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleAbilityAttachTimeOut_001, TestSize.Level0)
{
    HILOG_INFO("HandleAbilityAttachTimeOut_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->HandleAbilityAttachTimeOut(nullptr);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->HandleAbilityAttachTimeOut(nullptr);

    HILOG_INFO("HandleAbilityAttachTimeOut_001 end");
}

/**
 * @tc.name: PrepareTerminate_001
 * @tc.desc: prepare terminate.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, PrepareTerminate_001, TestSize.Level0)
{
    HILOG_INFO("PrepareTerminate_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->PrepareTerminate(nullptr);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->PrepareTerminate(nullptr);

    HILOG_INFO("PrepareTerminate_001 end");
}

/**
 * @tc.name: HandleTerminateApplicationTimeOut_001
 * @tc.desc: handle terminate application time out.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleTerminateApplicationTimeOut_001, TestSize.Level0)
{
    HILOG_INFO("HandleTerminateApplicationTimeOut_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->HandleTerminateApplicationTimeOut(0);

    BundleInfo bundleInfo;
    std::string appName = "test_appName";
    std::string processName = "test_processName";
    std::string bundleName = "test_bundleName";
    sptr<IRemoteObject> token = new MockAbilityToken();
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    appRecord->eventId_ = 0;
    appMgrServiceInner->HandleTerminateApplicationTimeOut(0);

    pid_t pid = 1;
    appRecord->GetPriorityObject()->SetPid(pid);
    appMgrServiceInner->HandleTerminateApplicationTimeOut(0);

    appMgrServiceInner->eventHandler_ = nullptr;
    appMgrServiceInner->HandleTerminateApplicationTimeOut(0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->HandleTerminateApplicationTimeOut(0);

    HILOG_INFO("HandleTerminateApplicationTimeOut_001 end");
}

/**
 * @tc.name: HandleAddAbilityStageTimeOut_001
 * @tc.desc: handle add ability stage time out.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleAddAbilityStageTimeOut_001, TestSize.Level0)
{
    HILOG_INFO("HandleAddAbilityStageTimeOut_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->HandleAddAbilityStageTimeOut(0);

    BundleInfo bundleInfo;
    std::string appName = "test_appName";
    std::string processName = "test_processName";
    std::string bundleName = "test_bundleName";
    sptr<IRemoteObject> token = new MockAbilityToken();
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    appRecord->eventId_ = 0;
    appMgrServiceInner->HandleAddAbilityStageTimeOut(0);

    appRecord->isSpecifiedAbility_ = true;
    appMgrServiceInner->HandleAddAbilityStageTimeOut(0);

    sptr<IStartSpecifiedAbilityResponse> response;
    appMgrServiceInner->startSpecifiedAbilityResponse_ = response;
    appMgrServiceInner->HandleAddAbilityStageTimeOut(0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->HandleAddAbilityStageTimeOut(0);

    HILOG_INFO("HandleAddAbilityStageTimeOut_001 end");
}

/**
 * @tc.name: GetRunningProcessInfoByToken_001
 * @tc.desc: get running process info by token.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetRunningProcessInfoByToken_001, TestSize.Level0)
{
    HILOG_INFO("GetRunningProcessInfoByToken_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AppExecFwk::RunningProcessInfo info;
    appMgrServiceInner->GetRunningProcessInfoByToken(nullptr, info);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->GetRunningProcessInfoByToken(nullptr, info);

    HILOG_INFO("GetRunningProcessInfoByToken_001 end");
}

/**
 * @tc.name: GetRunningProcessInfoByPid_001
 * @tc.desc: get running process info by pid.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetRunningProcessInfoByPid_001, TestSize.Level0)
{
    HILOG_INFO("GetRunningProcessInfoByPid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AppExecFwk::RunningProcessInfo info;
    appMgrServiceInner->GetRunningProcessInfoByPid(0, info);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->GetRunningProcessInfoByPid(0, info);

    HILOG_INFO("GetRunningProcessInfoByPid_001 end");
}

/**
 * @tc.name: CheckGetRunningInfoPermission_001
 * @tc.desc: check get running info permission.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, CheckGetRunningInfoPermission_001, TestSize.Level0)
{
    HILOG_INFO("CheckGetRunningInfoPermission_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->CheckGetRunningInfoPermission();

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->CheckGetRunningInfoPermission();

    HILOG_INFO("CheckGetRunningInfoPermission_001 end");
}

/**
 * @tc.name: LoadResidentProcess_001
 * @tc.desc: load resident process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, LoadResidentProcess_001, TestSize.Level0)
{
    HILOG_INFO("LoadResidentProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<BundleInfo> infos;
    appMgrServiceInner->LoadResidentProcess(infos);

    HILOG_INFO("LoadResidentProcess_001 end");
}

/**
 * @tc.name: StartResidentProcess_001
 * @tc.desc: start resident process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartResidentProcess_001, TestSize.Level0)
{
    HILOG_INFO("StartResidentProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<BundleInfo> infos;
    appMgrServiceInner->StartResidentProcess(infos, 0, true);

    BundleInfo info;
    infos.push_back(info);

    BundleInfo info1;
    info1.applicationInfo.process = "";
    infos.push_back(info1);

    BundleInfo info2;
    info2.applicationInfo.process = "test_process";
    infos.push_back(info2);
    appMgrServiceInner->StartResidentProcess(infos, 0, true);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->StartResidentProcess(infos, 0, true);

    HILOG_INFO("StartResidentProcess_001 end");
}

/**
 * @tc.name: StartEmptyResidentProcess_001
 * @tc.desc: start empty resident process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartEmptyResidentProcess_001, TestSize.Level0)
{
    HILOG_INFO("StartEmptyResidentProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo info;
    info.applicationInfo = *applicationInfo_;
    std::string processName = "test_process";
    appMgrServiceInner->StartEmptyResidentProcess(info, processName, 0, true);

    appMgrServiceInner->StartEmptyResidentProcess(info, processName, 1, true);

    appMgrServiceInner->StartEmptyResidentProcess(info, "", 0, true);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->StartEmptyResidentProcess(info, processName, 0, true);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->StartEmptyResidentProcess(info, processName, 0, true);

    HILOG_INFO("StartEmptyResidentProcess_001 end");
}

/**
 * @tc.name: CheckRemoteClient_001
 * @tc.desc: check remote client.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, CheckRemoteClient_001, TestSize.Level0)
{
    HILOG_INFO("CheckRemoteClient_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->CheckRemoteClient();

    appMgrServiceInner->remoteClientManager_->SetSpawnClient(nullptr);
    appMgrServiceInner->CheckRemoteClient();

    appMgrServiceInner->remoteClientManager_->SetBundleManager(nullptr);
    appMgrServiceInner->CheckRemoteClient();

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->CheckRemoteClient();

    HILOG_INFO("CheckRemoteClient_001 end");
}

/**
 * @tc.name: RestartResidentProcess_001
 * @tc.desc: restart resident process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, RestartResidentProcess_001, TestSize.Level0)
{
    HILOG_INFO("RestartResidentProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->RestartResidentProcess(nullptr);

    std::shared_ptr<AppRunningRecord> appRecord;
    appMgrServiceInner->RestartResidentProcess(appRecord);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    appRecord->mainBundleName_ = "com.ohos.settings";
    appMgrServiceInner->RestartResidentProcess(appRecord);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->RestartResidentProcess(appRecord);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->RestartResidentProcess(appRecord);

    HILOG_INFO("RestartResidentProcess_001 end");
}

/**
 * @tc.name: NotifyAppStatusByCallerUid_001
 * @tc.desc: notify app status by caller uid.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyAppStatusByCallerUid_001, TestSize.Level0)
{
    HILOG_INFO("NotifyAppStatusByCallerUid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundle_name";
    std::string eventData = "test_event_data";
    appMgrServiceInner->NotifyAppStatusByCallerUid(bundleName, 0, 0, eventData);

    HILOG_INFO("NotifyAppStatusByCallerUid_001 end");
}

/**
 * @tc.name: RegisterApplicationStateObserver_001
 * @tc.desc: register application state observer.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterApplicationStateObserver_001, TestSize.Level0)
{
    HILOG_INFO("RegisterApplicationStateObserver_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<IApplicationStateObserver> observer;
    std::vector<std::string> bundleNameList;
    appMgrServiceInner->RegisterApplicationStateObserver(observer, bundleNameList);

    HILOG_INFO("RegisterApplicationStateObserver_001 end");
}

/**
 * @tc.name: UnregisterApplicationStateObserver_001
 * @tc.desc: unregister application state observer.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, UnregisterApplicationStateObserver_001, TestSize.Level0)
{
    HILOG_INFO("UnregisterApplicationStateObserver_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<IApplicationStateObserver> observer;
    appMgrServiceInner->UnregisterApplicationStateObserver(observer);

    HILOG_INFO("UnregisterApplicationStateObserver_001 end");
}

/**
 * @tc.name: GetForegroundApplications_001
 * @tc.desc: get foreground applications.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetForegroundApplications_001, TestSize.Level0)
{
    HILOG_INFO("GetForegroundApplications_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<AppStateData> list;
    appMgrServiceInner->GetForegroundApplications(list);

    HILOG_INFO("GetForegroundApplications_001 end");
}

/**
 * @tc.name: StartUserTestProcess_001
 * @tc.desc: start user test process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartUserTestProcess_001, TestSize.Level0)
{
    HILOG_INFO("StartUserTestProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AAFwk::Want want;
    sptr<IRemoteObject> observer;
    BundleInfo bundleInfo;
    appMgrServiceInner->StartUserTestProcess(want, nullptr, bundleInfo, 0);

    appMgrServiceInner->StartUserTestProcess(want, observer, bundleInfo, 0);

    std::string bundle_name = "test_bundle_name";
    want.SetParam("-b", bundle_name);
    appMgrServiceInner->StartUserTestProcess(want, observer, bundleInfo, 0);

    std::string moduleName = "test_module_name";
    want.SetParam("-m", moduleName);
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = moduleName;
    bundleInfo.hapModuleInfos.push_back(hapModuleInfo);
    appMgrServiceInner->StartUserTestProcess(want, observer, bundleInfo, 0);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->StartUserTestProcess(want, observer, bundleInfo, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->StartUserTestProcess(want, observer, bundleInfo, 0);

    HILOG_INFO("StartUserTestProcess_001 end");
}

/**
 * @tc.name: GetHapModuleInfoForTestRunner_001
 * @tc.desc: get hap module info for test runner.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetHapModuleInfoForTestRunner_001, TestSize.Level0)
{
    HILOG_INFO("GetHapModuleInfoForTestRunner_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AAFwk::Want want;
    sptr<IRemoteObject> observer;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    appMgrServiceInner->GetHapModuleInfoForTestRunner(want, nullptr, bundleInfo, hapModuleInfo);

    appMgrServiceInner->GetHapModuleInfoForTestRunner(want, observer, bundleInfo, hapModuleInfo);

    hapModuleInfo.moduleName = "test_module_name";
    bundleInfo.hapModuleInfos.push_back(hapModuleInfo);
    appMgrServiceInner->GetHapModuleInfoForTestRunner(want, observer, bundleInfo, hapModuleInfo);

    bundleInfo.hapModuleInfos.back().isModuleJson = true;
    appMgrServiceInner->GetHapModuleInfoForTestRunner(want, observer, bundleInfo, hapModuleInfo);

    std::string testmoduleName = "test_XXX";
    want.SetParam("-m", testmoduleName);
    appMgrServiceInner->GetHapModuleInfoForTestRunner(want, observer, bundleInfo, hapModuleInfo);

    std::string moduleName = "test_module_name";
    want.SetParam("-m", moduleName);
    appMgrServiceInner->GetHapModuleInfoForTestRunner(want, observer, bundleInfo, hapModuleInfo);

    HILOG_INFO("GetHapModuleInfoForTestRunner_001 end");
}

/**
 * @tc.name: UserTestAbnormalFinish_001
 * @tc.desc: user test abnormal finish.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, UserTestAbnormalFinish_001, TestSize.Level0)
{
    HILOG_INFO("UserTestAbnormalFinish_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<IRemoteObject> observer;
    std::string msg = "testmsg";
    appMgrServiceInner->UserTestAbnormalFinish(nullptr, "");
    appMgrServiceInner->UserTestAbnormalFinish(nullptr, msg);
    appMgrServiceInner->UserTestAbnormalFinish(observer, "");
    appMgrServiceInner->UserTestAbnormalFinish(observer, msg);

    HILOG_INFO("UserTestAbnormalFinish_001 end");
}

/**
 * @tc.name: StartEmptyProcess_001
 * @tc.desc: start empty process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartEmptyProcess_001, TestSize.Level0)
{
    HILOG_INFO("StartEmptyProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AAFwk::Want want;
    sptr<IRemoteObject> observer;
    BundleInfo info;
    HapModuleInfo hapModuleInfo;
    std::string processName = "test_processName";
    appMgrServiceInner->StartEmptyProcess(want, nullptr, info, "", 0);
    appMgrServiceInner->StartEmptyProcess(want, observer, info, "", 0);
    appMgrServiceInner->StartEmptyProcess(want, observer, info, processName, 0);

    info.applicationInfo = *applicationInfo_;
    appMgrServiceInner->StartEmptyProcess(want, observer, info, processName, 0);

    want.SetParam("coldStart", true);
    appMgrServiceInner->StartEmptyProcess(want, observer, info, processName, 0);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->StartEmptyProcess(want, observer, info, processName, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->StartEmptyProcess(want, observer, info, processName, 0);

    HILOG_INFO("StartEmptyProcess_001 end");
}

/**
 * @tc.name: FinishUserTest_001
 * @tc.desc: finish user test.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, FinishUserTest_001, TestSize.Level0)
{
    HILOG_INFO("FinishUserTest_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t pid = 0;
    appMgrServiceInner->FinishUserTest("", 0, "", pid);

    std::string msg = "testmsg";
    std::string bundleName = "test_bundle_name";
    appMgrServiceInner->FinishUserTest("", 0, bundleName, pid);
    appMgrServiceInner->FinishUserTest(msg, 0, "", pid);
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    sptr<IRemoteObject> token = new MockAbilityToken();
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(token, nullptr,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    pid = appRecord->GetPriorityObject()->GetPid();
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);

    std::shared_ptr<UserTestRecord> record = std::make_shared<UserTestRecord>();
    appRecord->SetUserTestInfo(record);
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);

    HILOG_INFO("FinishUserTest_001 end");
}

/**
 * @tc.name: FinishUserTestLocked_001
 * @tc.desc: finish user test locked.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, FinishUserTestLocked_001, TestSize.Level0)
{
    HILOG_INFO("FinishUserTestLocked_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->FinishUserTestLocked("", 0, nullptr);

    std::shared_ptr<AppRunningRecord> appRecord;
    appMgrServiceInner->FinishUserTestLocked("", 0, appRecord);

    std::string msg = "testmsg";
    appMgrServiceInner->FinishUserTestLocked(msg, 0, nullptr);
    appMgrServiceInner->FinishUserTestLocked(msg, 0, appRecord);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<UserTestRecord> record = std::make_shared<UserTestRecord>();
    appRecord->SetUserTestInfo(record);
    appMgrServiceInner->FinishUserTestLocked(msg, 0, appRecord);

    record->isFinished = true;
    appRecord->SetUserTestInfo(record);
    appMgrServiceInner->FinishUserTestLocked(msg, 0, appRecord);

    record->observer = nullptr;
    appRecord->SetUserTestInfo(record);
    appMgrServiceInner->FinishUserTestLocked(msg, 0, appRecord);

    HILOG_INFO("FinishUserTestLocked_001 end");
}

/**
 * @tc.name: StartSpecifiedAbility_001
 * @tc.desc: start specified ability.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartSpecifiedAbility_001, TestSize.Level0)
{
    HILOG_INFO("StartSpecifiedAbility_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AAFwk::Want want;
    AbilityInfo abilityInfo;
    appMgrServiceInner->StartSpecifiedAbility(want, abilityInfo);

    appMgrServiceInner->StartSpecifiedAbility(want, *abilityInfo_);

    abilityInfo_->applicationInfo = *applicationInfo_;
    appMgrServiceInner->StartSpecifiedAbility(want, *abilityInfo_);

    appMgrServiceInner->remoteClientManager_->SetBundleManager(nullptr);
    appMgrServiceInner->StartSpecifiedAbility(want, *abilityInfo_);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->StartSpecifiedAbility(want, *abilityInfo_);

    HILOG_INFO("StartSpecifiedAbility_001 end");
}

/**
 * @tc.name: RegisterStartSpecifiedAbilityResponse_001
 * @tc.desc: register start specified ability response.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterStartSpecifiedAbilityResponse_001, TestSize.Level0)
{
    HILOG_INFO("RegisterStartSpecifiedAbilityResponse_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->RegisterStartSpecifiedAbilityResponse(nullptr);

    sptr<IStartSpecifiedAbilityResponse> response;
    appMgrServiceInner->RegisterStartSpecifiedAbilityResponse(response);

    HILOG_INFO("RegisterStartSpecifiedAbilityResponse_001 end");
}

/**
 * @tc.name: ScheduleAcceptWantDone_001
 * @tc.desc: schedule accept want done.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ScheduleAcceptWantDone_001, TestSize.Level0)
{
    HILOG_INFO("ScheduleAcceptWantDone_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AAFwk::Want want;
    std::string flag = "test_flag";
    appMgrServiceInner->ScheduleAcceptWantDone(0, want, flag);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    appMgrServiceInner->ScheduleAcceptWantDone(appRecord->GetRecordId(), want, flag);

    sptr<IStartSpecifiedAbilityResponse> response;
    appMgrServiceInner->RegisterStartSpecifiedAbilityResponse(response);
    appMgrServiceInner->ScheduleAcceptWantDone(appRecord->GetRecordId(), want, flag);

    HILOG_INFO("ScheduleAcceptWantDone_001 end");
}

/**
 * @tc.name: HandleStartSpecifiedAbilityTimeOut_001
 * @tc.desc: handle start specified ability time out.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleStartSpecifiedAbilityTimeOut_001, TestSize.Level0)
{
    HILOG_INFO("HandleStartSpecifiedAbilityTimeOut_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->HandleStartSpecifiedAbilityTimeOut(0);

    BundleInfo bundleInfo;
    std::string appName = "test_appName";
    std::string processName = "test_processName";
    std::string bundleName = "test_bundleName";
    sptr<IRemoteObject> token = new MockAbilityToken();
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    appRecord->eventId_ = 0;
    appMgrServiceInner->HandleStartSpecifiedAbilityTimeOut(0);

    appRecord->isSpecifiedAbility_ = true;
    appMgrServiceInner->HandleStartSpecifiedAbilityTimeOut(0);

    sptr<IStartSpecifiedAbilityResponse> response;
    appMgrServiceInner->startSpecifiedAbilityResponse_ = response;
    appMgrServiceInner->HandleStartSpecifiedAbilityTimeOut(0);

    appRecord->isSpecifiedAbility_ = false;
    appMgrServiceInner->HandleStartSpecifiedAbilityTimeOut(0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->HandleStartSpecifiedAbilityTimeOut(0);

    HILOG_INFO("HandleStartSpecifiedAbilityTimeOut_001 end");
}

/**
 * @tc.name: UpdateConfiguration_001
 * @tc.desc: update configuration.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, UpdateConfiguration_001, TestSize.Level0)
{
    HILOG_INFO("UpdateConfiguration_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    Configuration config;
    appMgrServiceInner->UpdateConfiguration(config);

    auto testLanguge = "ch-zh";
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, testLanguge);
    appMgrServiceInner->UpdateConfiguration(config);

    auto appRunningRecordMap = appMgrServiceInner->appRunningManager_->appRunningRecordMap_;
    for (const auto& item : appRunningRecordMap) {
        const auto& appRecord = item.second;
        if (appRecord) {
            appRecord->appLifeCycleDeal_ = nullptr;
        }
    }
    appMgrServiceInner->UpdateConfiguration(config);

    sptr<MockConfigurationObserver> observer(new (std::nothrow) MockConfigurationObserver());
    appMgrServiceInner->configurationObservers_.push_back(observer);
    sptr<IConfigurationObserver> observer1;
    appMgrServiceInner->configurationObservers_.push_back(observer1);
    appMgrServiceInner->configurationObservers_.push_back(nullptr);
    appMgrServiceInner->UpdateConfiguration(config);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->UpdateConfiguration(config);

    HILOG_INFO("UpdateConfiguration_001 end");
}

/**
 * @tc.name: RegisterConfigurationObserver_001
 * @tc.desc: register configuration observer.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterConfigurationObserver_001, TestSize.Level0)
{
    HILOG_INFO("RegisterConfigurationObserver_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->configurationObservers_.clear();

    appMgrServiceInner->RegisterConfigurationObserver(nullptr);

    sptr<MockConfigurationObserver> observer(new (std::nothrow) MockConfigurationObserver());
    appMgrServiceInner->RegisterConfigurationObserver(observer);
    appMgrServiceInner->RegisterConfigurationObserver(observer);

    HILOG_INFO("RegisterConfigurationObserver_001 end");
}

/**
 * @tc.name: UnregisterConfigurationObserver_001
 * @tc.desc: unregister configuration observer.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, UnregisterConfigurationObserver_001, TestSize.Level0)
{
    HILOG_INFO("UnregisterConfigurationObserver_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->configurationObservers_.clear();

    appMgrServiceInner->UnregisterConfigurationObserver(nullptr);

    sptr<MockConfigurationObserver> observer(new (std::nothrow) MockConfigurationObserver());
    appMgrServiceInner->UnregisterConfigurationObserver(observer);

    appMgrServiceInner->RegisterConfigurationObserver(observer);
    appMgrServiceInner->UnregisterConfigurationObserver(observer);

    HILOG_INFO("UnregisterConfigurationObserver_001 end");
}

/**
 * @tc.name: InitGlobalConfiguration_001
 * @tc.desc: init global configuration.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, InitGlobalConfiguration_001, TestSize.Level0)
{
    HILOG_INFO("InitGlobalConfiguration_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->InitGlobalConfiguration();

    appMgrServiceInner->configuration_ = nullptr;
    appMgrServiceInner->InitGlobalConfiguration();

    HILOG_INFO("InitGlobalConfiguration_001 end");
}

/**
 * @tc.name: KillApplicationByRecord_001
 * @tc.desc: kill application by record.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillApplicationByRecord_001, TestSize.Level0)
{
    HILOG_INFO("KillApplicationByRecord_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord1 =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord1, nullptr);
    appMgrServiceInner->KillApplicationByRecord(appRecord);
    appMgrServiceInner->KillApplicationByRecord(appRecord1);

    appMgrServiceInner->eventHandler_ = nullptr;
    appMgrServiceInner->KillApplicationByRecord(appRecord);
    appMgrServiceInner->KillApplicationByRecord(appRecord1);

    HILOG_INFO("KillApplicationByRecord_001 end");
}

/**
 * @tc.name: SendHiSysEvent_001
 * @tc.desc: send hi sys event.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SendHiSysEvent_001, TestSize.Level0)
{
    HILOG_INFO("SendHiSysEvent_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->SendHiSysEvent(0, 0);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    appRecord->eventId_ = 0;
    appMgrServiceInner->SendHiSysEvent(0, 0);
    appMgrServiceInner->SendHiSysEvent(1, 0);
    appMgrServiceInner->SendHiSysEvent(2, 0);
    appMgrServiceInner->SendHiSysEvent(3, 0);
    appMgrServiceInner->SendHiSysEvent(4, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->SendHiSysEvent(0, 0);

    HILOG_INFO("SendHiSysEvent_001 end");
}

/**
 * @tc.name: GetAbilityRecordsByProcessID_001
 * @tc.desc: get ability records by process id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetAbilityRecordsByProcessID_001, TestSize.Level0)
{
    HILOG_INFO("GetAbilityRecordsByProcessID_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<sptr<IRemoteObject>> tokens;
    appMgrServiceInner->GetAbilityRecordsByProcessID(0, tokens);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    int pid = appRecord->GetPriorityObject()->GetPid();
    appMgrServiceInner->GetAbilityRecordsByProcessID(pid, tokens);

    HILOG_INFO("GetAbilityRecordsByProcessID_001 end");
}

/**
 * @tc.name: GetApplicationInfoByProcessID_001
 * @tc.desc: get applicationInfo by process id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetApplicationInfoByProcessID_001, TestSize.Level0)
{
    HILOG_INFO("GetApplicationInfoByProcessID_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    ApplicationInfo application;
    bool debug = false;
    appMgrServiceInner->GetApplicationInfoByProcessID(0, application, debug);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    int pid = appRecord->GetPriorityObject()->GetPid();
    appMgrServiceInner->GetApplicationInfoByProcessID(pid, application, debug);

    appRecord->appInfo_ = nullptr;
    appMgrServiceInner->GetApplicationInfoByProcessID(pid, application, debug);

    HILOG_INFO("GetApplicationInfoByProcessID_001 end");
}

/**
 * @tc.name: VerifyProcessPermission_001
 * @tc.desc: verify process permission.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, VerifyProcessPermission_001, TestSize.Level0)
{
    HILOG_INFO("VerifyProcessPermission_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->VerifyProcessPermission();

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->VerifyProcessPermission();

    HILOG_INFO("VerifyProcessPermission_001 end");
}

/**
 * @tc.name: VerifyAPL_001
 * @tc.desc: verify APL.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, VerifyAPL_001, TestSize.Level0)
{
    HILOG_INFO("VerifyAPL_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->VerifyAPL();

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->VerifyAPL();

    HILOG_INFO("VerifyAPL_001 end");
}

/**
 * @tc.name: VerifyAccountPermission_001
 * @tc.desc: verify account permission.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, VerifyAccountPermission_001, TestSize.Level0)
{
    HILOG_INFO("VerifyAccountPermission_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string permissionName = "test_permissionName";
    appMgrServiceInner->VerifyAccountPermission(permissionName, 0);

    HILOG_INFO("VerifyAccountPermission_001 end");
}

/**
 * @tc.name: PreStartNWebSpawnProcess_003
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, PreStartNWebSpawnProcess_003, TestSize.Level0)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int callingPid = 1;
    appMgrServiceInner->remoteClientManager_->nwebSpawnClient_ = nullptr;
    int ret = appMgrServiceInner->PreStartNWebSpawnProcess(callingPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: StartRenderProcess_001
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_001, TestSize.Level0)
{
    HILOG_INFO("StartRenderProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    pid_t hostPid1 = 1;
    std::string renderParam = "test_renderParam";
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, "", 0, 0, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = appMgrServiceInner->StartRenderProcess(hostPid, "", 0, 1, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = appMgrServiceInner->StartRenderProcess(hostPid, "", 1, 0, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = appMgrServiceInner->StartRenderProcess(hostPid, "", 1, 1, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, 0, 0, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, 0, 1, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, 1, 0, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, 1, 1, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = appMgrServiceInner->StartRenderProcess(hostPid1, "", 0, 0, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = appMgrServiceInner->StartRenderProcess(hostPid1, "", 0, 1, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = appMgrServiceInner->StartRenderProcess(hostPid1, "", 1, 0, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = appMgrServiceInner->StartRenderProcess(hostPid1, "", 1, 1, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = appMgrServiceInner->StartRenderProcess(hostPid1, renderParam, 0, 0, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = appMgrServiceInner->StartRenderProcess(hostPid1, renderParam, 0, 1, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = appMgrServiceInner->StartRenderProcess(hostPid1, renderParam, 1, 0, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    ret = appMgrServiceInner->StartRenderProcess(hostPid1, renderParam, 1, 1, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    HILOG_INFO("StartRenderProcess_001 end");
}

/**
 * @tc.name: StartRenderProcess_002
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_002, TestSize.Level0)
{
    HILOG_INFO("StartRenderProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid1 = 1;
    std::string renderParam = "test_renderParam";
    pid_t renderPid = 0;

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    appRecord->GetPriorityObject()->SetPid(hostPid1);
    int ret = appMgrServiceInner->StartRenderProcess(hostPid1, renderParam, 1, 1, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    std::shared_ptr<RenderRecord> renderRecord =
        RenderRecord::CreateRenderRecord(hostPid1, renderParam, 1, 1, appRecord);
    appRecord->SetRenderRecord(renderRecord);
    ret = appMgrServiceInner->StartRenderProcess(hostPid1, renderParam, 1, 1, renderPid);
    EXPECT_EQ(ret, 8454244);

    appMgrServiceInner->appRunningManager_ = nullptr;
    ret = appMgrServiceInner->StartRenderProcess(hostPid1, renderParam, 1, 1, renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    HILOG_INFO("StartRenderProcess_002 end");
}

/**
 * @tc.name: AttachRenderProcess_001
 * @tc.desc: attach render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, AttachRenderProcess_001, TestSize.Level0)
{
    HILOG_INFO("AttachRenderProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t pid = 0;
    sptr<IRenderScheduler> scheduler;
    appMgrServiceInner->AttachRenderProcess(pid, scheduler);

    pid = 1;
    appMgrServiceInner->AttachRenderProcess(pid, scheduler);

    sptr<MockRenderScheduler> mockRenderScheduler = new (std::nothrow) MockRenderScheduler();
    EXPECT_CALL(*mockRenderScheduler, AsObject()).Times(1);
    EXPECT_CALL(*mockRenderScheduler, NotifyBrowserFd(1, 1)).Times(1);
    appMgrServiceInner->AttachRenderProcess(pid, mockRenderScheduler);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    appRecord->GetPriorityObject()->SetPid(pid);
    std::string renderParam = "test_renderParam";
    std::shared_ptr<RenderRecord> renderRecord = RenderRecord::CreateRenderRecord(pid, renderParam, 1, 1, appRecord);
    EXPECT_NE(renderRecord, nullptr);
    renderRecord->SetPid(pid);
    appRecord->SetRenderRecord(renderRecord);
    appMgrServiceInner->AttachRenderProcess(pid, mockRenderScheduler);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->AttachRenderProcess(pid, mockRenderScheduler);

    HILOG_INFO("AttachRenderProcess_001 end");
}

/**
 * @tc.name: BuildStartFlags_001
 * @tc.desc: build start flags.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, BuildStartFlags_001, TestSize.Level0)
{
    HILOG_INFO("BuildStartFlags_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AAFwk::Want want;
    AbilityInfo abilityInfo;
    appMgrServiceInner->BuildStartFlags(want, abilityInfo);

    want.SetParam("coldStart", true);
    want.SetParam("ohos.dlp.params.index", 1);
    abilityInfo.extensionAbilityType = ExtensionAbilityType::BACKUP;
    appMgrServiceInner->BuildStartFlags(want, abilityInfo);

    HILOG_INFO("BuildStartFlags_001 end");
}

/**
 * @tc.name: RegisterFocusListener_001
 * @tc.desc: register focus listener.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterFocusListener_001, TestSize.Level0)
{
    HILOG_INFO("RegisterFocusListener_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->RegisterFocusListener();

    appMgrServiceInner->focusListener_ = nullptr;
    appMgrServiceInner->RegisterFocusListener();

    HILOG_INFO("RegisterFocusListener_001 end");
}

/**
 * @tc.name: HandleFocused_001
 * @tc.desc: handle focused.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleFocused_001, TestSize.Level0)
{
    HILOG_INFO("HandleFocused_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<Rosen::FocusChangeInfo> focusChangeInfo;
    appMgrServiceInner->HandleFocused(focusChangeInfo);

    pid_t pid = 1;
    focusChangeInfo = new Rosen::FocusChangeInfo();
    appMgrServiceInner->HandleFocused(focusChangeInfo);

    focusChangeInfo->pid_ = pid;
    appMgrServiceInner->HandleFocused(focusChangeInfo);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    appRecord->GetPriorityObject()->SetPid(pid);
    appMgrServiceInner->HandleFocused(focusChangeInfo);

    HILOG_INFO("HandleFocused_001 end");
}

/**
 * @tc.name: HandleUnfocused_001
 * @tc.desc: handle unfocused.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleUnfocused_001, TestSize.Level0)
{
    HILOG_INFO("HandleUnfocused_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<Rosen::FocusChangeInfo> focusChangeInfo;
    appMgrServiceInner->HandleUnfocused(focusChangeInfo);

    pid_t pid = 1;
    focusChangeInfo = new Rosen::FocusChangeInfo();
    appMgrServiceInner->HandleUnfocused(focusChangeInfo);

    focusChangeInfo->pid_ = pid;
    appMgrServiceInner->HandleUnfocused(focusChangeInfo);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    appRecord->GetPriorityObject()->SetPid(pid);
    appMgrServiceInner->HandleUnfocused(focusChangeInfo);

    HILOG_INFO("HandleUnfocused_001 end");
}

/**
 * @tc.name: GetAppRunningStateByBundleName_001
 * @tc.desc: get app running state by bundle name.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetAppRunningStateByBundleName_001, TestSize.Level0)
{
    HILOG_INFO("GetAppRunningStateByBundleName_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    appMgrServiceInner->GetAppRunningStateByBundleName(bundleName);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->GetAppRunningStateByBundleName(bundleName);

    HILOG_INFO("GetAppRunningStateByBundleName_001 end");
}

/**
 * @tc.name: NotifyLoadRepairPatch_001
 * @tc.desc: notify load repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyLoadRepairPatch_001, TestSize.Level0)
{
    HILOG_INFO("NotifyLoadRepairPatch_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    sptr<IQuickFixCallback> callback;
    appMgrServiceInner->NotifyLoadRepairPatch(bundleName, callback);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->NotifyLoadRepairPatch(bundleName, callback);

    HILOG_INFO("NotifyLoadRepairPatch_001 end");
}

/**
 * @tc.name: NotifyHotReloadPage_001
 * @tc.desc: notify hot reload page.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyHotReloadPage_001, TestSize.Level0)
{
    HILOG_INFO("NotifyHotReloadPage_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    sptr<IQuickFixCallback> callback;
    appMgrServiceInner->NotifyHotReloadPage(bundleName, callback);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->NotifyHotReloadPage(bundleName, callback);

    HILOG_INFO("NotifyHotReloadPage_001 end");
}

/**
 * @tc.name: SetContinuousTaskProcess_001
 * @tc.desc: set continuous task process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
HWTEST_F(AppMgrServiceInnerTest, SetContinuousTaskProcess_001, TestSize.Level0)
{
    HILOG_INFO("SetContinuousTaskProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int32_t ret = appMgrServiceInner->SetContinuousTaskProcess(0, true);
    EXPECT_EQ(ret, 0);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo);
    EXPECT_NE(appRecord, nullptr);
    appRecord->GetPriorityObject()->SetPid(0);
    ret = appMgrServiceInner->SetContinuousTaskProcess(0, true);
    EXPECT_EQ(ret, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    ret = appMgrServiceInner->SetContinuousTaskProcess(0, true);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    HILOG_INFO("SetContinuousTaskProcess_001 end");
}
#endif

/**
 * @tc.name: NotifyUnLoadRepairPatch_001
 * @tc.desc: notify unload repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyUnLoadRepairPatch_001, TestSize.Level0)
{
    HILOG_INFO("NotifyUnLoadRepairPatch_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    sptr<IQuickFixCallback> callback;
    appMgrServiceInner->NotifyUnLoadRepairPatch(bundleName, callback);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->NotifyUnLoadRepairPatch(bundleName, callback);

    HILOG_INFO("NotifyUnLoadRepairPatch_001 end");
}
} // namespace AppExecFwk
} // namespace OHOS
