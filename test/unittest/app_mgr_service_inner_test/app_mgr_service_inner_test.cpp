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
#undef private
#include "hilog_wrapper.h"
#include "mock_ability_token.h"
#include "mock_app_scheduler.h"
#include "mock_native_token.h"
#include "parameters.h"

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

    void InitAppInfo(const std::string &deviceName, const std::string &abilityName,
        const std::string &appName, const std::string &bundleName, const std::string &moduleName);

public:
    std::shared_ptr<AbilityInfo> abilityInfo_;
    std::shared_ptr<ApplicationInfo> applicationInfo_;
};

void AppMgrServiceInnerTest::InitAppInfo(const std::string &deviceName,
    const std::string &abilityName, const std::string &appName, const std::string &bundleName,
    const std::string &moduleName)
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

    int callingPid = 1;
    int ret = appMgrServiceInner->PreStartNWebSpawnProcess(callingPid);
    EXPECT_EQ(ret, ERR_OK);
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

    appMgrServiceInner->KillApplicationSelf();

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->KillApplicationSelf();

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
} // namespace AppExecFwk
} // namespace OHOS
