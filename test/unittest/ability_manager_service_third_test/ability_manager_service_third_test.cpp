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

#define private public
#define protected public
#include "ability_manager_service.h"
#include "ability_connect_manager.h"
#include "ability_connection.h"
#include "ability_start_setting.h"
#include "recovery_param.h"
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "connection_observer_errors.h"
#include "session/host/include/session.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;
namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID_U100 = 100;
const int32_t APP_MEMORY_SIZE = 512;
}  // namespace
class AbilityManagerServiceThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

public:
    AbilityRequest abilityRequest_{};
    Want want_{};
};

AbilityRequest AbilityManagerServiceThirdTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ElementName element(deviceName, bundleName, abilityName, moduleName);
    want_.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    ApplicationInfo appinfo;
    appinfo.name = appName;
    appinfo.bundleName = bundleName;
    abilityInfo.applicationInfo = appinfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want_;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    return abilityRequest;
}

void AbilityManagerServiceThirdTest::SetUpTestCase() {}

void AbilityManagerServiceThirdTest::TearDownTestCase() {}

void AbilityManagerServiceThirdTest::SetUp() {}

void AbilityManagerServiceThirdTest::TearDown() {}

/*
 * Feature: AbilityManagerService
 * Function: HandleActiveTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleActiveTimeOut
 */
HWTEST_F(AbilityManagerServiceThirdTest, HandleActiveTimeOut_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest HandleActiveTimeOut_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->HandleActiveTimeOut(100);
    HILOG_INFO("AbilityManagerServiceThirdTest HandleActiveTimeOut_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: HandleInactiveTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleInactiveTimeOut
 */
HWTEST_F(AbilityManagerServiceThirdTest, HandleInactiveTimeOut_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest HandleInactiveTimeOut_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->HandleInactiveTimeOut(100);
    HILOG_INFO("AbilityManagerServiceThirdTest HandleInactiveTimeOut_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: VerificationToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService VerificationToken
 */
HWTEST_F(AbilityManagerServiceThirdTest, VerificationToken_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest VerificationToken_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_FALSE(abilityMs_->VerificationToken(nullptr));
    HILOG_INFO("AbilityManagerServiceThirdTest VerificationToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: VerificationAllToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService VerificationAllToken
 */
HWTEST_F(AbilityManagerServiceThirdTest, VerificationAllToken_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest VerificationAllToken_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_FALSE(abilityMs_->VerificationAllToken(nullptr));
    HILOG_INFO("AbilityManagerServiceThirdTest VerificationAllToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityManager
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityManager
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetDataAbilityManager_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetDataAbilityManager_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetDataAbilityManager(nullptr), nullptr);
    HILOG_INFO("AbilityManagerServiceThirdTest GetDataAbilityManager_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetListManagerByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetListManagerByUserId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetListManagerByUserId_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetListManagerByUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetListManagerByUserId(100), nullptr);

    auto temp = abilityMs_->missionListManagers_;
    abilityMs_->missionListManagers_.clear();
    EXPECT_EQ(abilityMs_->GetListManagerByUserId(100).get(), nullptr);
    abilityMs_->missionListManagers_ = temp;
    HILOG_INFO("AbilityManagerServiceThirdTest GetListManagerByUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityManagerByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityManagerByUserId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetDataAbilityManagerByUserId_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetDataAbilityManagerByUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetDataAbilityManagerByUserId(100), nullptr);

    auto temp = abilityMs_->dataAbilityManagers_;
    abilityMs_->dataAbilityManagers_.clear();
    EXPECT_EQ(abilityMs_->GetDataAbilityManagerByUserId(100).get(), nullptr);
    abilityMs_->dataAbilityManagers_ = temp;
    HILOG_INFO("AbilityManagerServiceThirdTest GetDataAbilityManagerByUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetConnectManagerByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectManagerByToken
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetConnectManagerByToken_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetConnectManagerByToken_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetConnectManagerByToken(nullptr), nullptr);
    HILOG_INFO("AbilityManagerServiceThirdTest GetConnectManagerByToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityManagerByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityManagerByToken
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetDataAbilityManagerByToken_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetDataAbilityManagerByToken_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetDataAbilityManagerByToken(nullptr), nullptr);
    HILOG_INFO("AbilityManagerServiceThirdTest GetDataAbilityManagerByToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectBmsService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectBmsService
 */
HWTEST_F(AbilityManagerServiceThirdTest, ConnectBmsService_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest ConnectBmsService_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->ConnectBmsService();
    HILOG_INFO("AbilityManagerServiceThirdTest ConnectBmsService_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetWantSenderInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSenderInfo
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetWantSenderInfo_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetWantSenderInfo_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::shared_ptr<WantSenderInfo> info;
    EXPECT_EQ(abilityMs_->GetWantSenderInfo(nullptr, info), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceThirdTest GetWantSenderInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAppMemorySize
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAppMemorySize
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetAppMemorySize_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetAppMemorySize_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetAppMemorySize(), APP_MEMORY_SIZE);
    HILOG_INFO("AbilityManagerServiceThirdTest GetAppMemorySize_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsRamConstrainedDevice
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRamConstrainedDevice
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsRamConstrainedDevice_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest IsRamConstrainedDevice_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_FALSE(abilityMs_->IsRamConstrainedDevice());
    HILOG_INFO("AbilityManagerServiceThirdTest IsRamConstrainedDevice_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionSaveTime
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionSaveTime
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetMissionSaveTime_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetMissionSaveTime_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(AmsConfigurationParameter::GetInstance().GetMissionSaveTime(), 0);
    HILOG_INFO("AbilityManagerServiceThirdTest GetMissionSaveTime_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityTokenByMissionId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityTokenByMissionId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetAbilityTokenByMissionId_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetAbilityTokenByMissionId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetAbilityTokenByMissionId(100), nullptr);

    auto temp = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_.reset();
    EXPECT_EQ(abilityMs_->GetAbilityTokenByMissionId(100), nullptr);
    abilityMs_->currentMissionListManager_ = temp;
    HILOG_INFO("AbilityManagerServiceThirdTest GetAbilityTokenByMissionId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartRemoteAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartRemoteAbilityByCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartRemoteAbilityByCall_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest StartRemoteAbilityByCall_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    Want want;
    EXPECT_EQ(abilityMs_->StartRemoteAbilityByCall(want, nullptr, nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceThirdTest StartRemoteAbilityByCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseRemoteAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, ReleaseRemoteAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest ReleaseRemoteAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AppExecFwk::ElementName element;
    EXPECT_EQ(abilityMs_->ReleaseRemoteAbility(nullptr, element), ERR_NULL_OBJECT);
    HILOG_INFO("AbilityManagerServiceThirdTest ReleaseRemoteAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, ReleaseCall_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest ReleaseCall_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    AppExecFwk::ElementName element;
    EXPECT_EQ(abilityMs_->ReleaseCall(nullptr, element), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceThirdTest ReleaseCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: JudgeAbilityVisibleControl
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService JudgeAbilityVisibleControl
 */
HWTEST_F(AbilityManagerServiceThirdTest, JudgeAbilityVisibleControl_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest JudgeAbilityVisibleControl_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    EXPECT_EQ(abilityMs_->JudgeAbilityVisibleControl(abilityInfo), ERR_OK);

    abilityInfo.visible = false;
    EXPECT_EQ(abilityMs_->JudgeAbilityVisibleControl(abilityInfo), ERR_OK);

    abilityInfo.applicationInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    EXPECT_EQ(abilityMs_->JudgeAbilityVisibleControl(abilityInfo), ERR_OK);

    HILOG_INFO("AbilityManagerServiceThirdTest JudgeAbilityVisibleControl_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnAcceptWantResponse
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAcceptWantResponse
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnAcceptWantResponse_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest OnAcceptWantResponse_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    AAFwk::Want want;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->OnAcceptWantResponse(want, "test");

    auto temp = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_.reset();
    abilityMs_->OnAcceptWantResponse(want, "test");
    abilityMs_->currentMissionListManager_ = temp;
    HILOG_INFO("AbilityManagerServiceThirdTest OnAcceptWantResponse_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnStartSpecifiedAbilityTimeoutResponse
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnStartSpecifiedAbilityTimeoutResponse
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnStartSpecifiedAbilityTimeoutResponse_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest OnStartSpecifiedAbilityTimeoutResponse_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AAFwk::Want want;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->OnStartSpecifiedAbilityTimeoutResponse(want);

    auto temp = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_.reset();
    abilityMs_->OnStartSpecifiedAbilityTimeoutResponse(want);
    abilityMs_->currentMissionListManager_ = temp;
    HILOG_INFO("AbilityManagerServiceThirdTest OnStartSpecifiedAbilityTimeoutResponse_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityRunningInfos
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetAbilityRunningInfos_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetAbilityRunningInfos_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    std::vector<AbilityRunningInfo> info;
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(abilityMs_->GetAbilityRunningInfos(info), ERR_OK);

        auto temp1 = abilityMs_->currentMissionListManager_;
        abilityMs_->currentMissionListManager_.reset();
        EXPECT_EQ(abilityMs_->GetAbilityRunningInfos(info), ERR_INVALID_VALUE);
        abilityMs_->currentMissionListManager_ = temp1;

        auto temp2 = abilityMs_->connectManager_;
        abilityMs_->connectManager_.reset();
        EXPECT_NE(abilityMs_->GetAbilityRunningInfos(info), ERR_OK);
        abilityMs_->connectManager_ = temp2;

        auto temp3 = abilityMs_->dataAbilityManager_;
        abilityMs_->dataAbilityManager_.reset();
        EXPECT_NE(abilityMs_->GetAbilityRunningInfos(info), ERR_OK);
        abilityMs_->dataAbilityManager_ = temp3;
    }
    HILOG_INFO("AbilityManagerServiceThirdTest GetAbilityRunningInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetExtensionRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetExtensionRunningInfos
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetExtensionRunningInfos_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetExtensionRunningInfos_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<AAFwk::ExtensionRunningInfo> extensionRunningInfo;
    EXPECT_NE(abilityMs_->GetExtensionRunningInfos(10, extensionRunningInfo), ERR_OK);

    auto temp = abilityMs_->connectManager_;
    abilityMs_->connectManager_.reset();
    EXPECT_EQ(abilityMs_->GetExtensionRunningInfos(10, extensionRunningInfo), ERR_INVALID_VALUE);
    abilityMs_->connectManager_ = temp;
    HILOG_INFO("AbilityManagerServiceThirdTest GetExtensionRunningInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetProcessRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetProcessRunningInfos
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetProcessRunningInfos_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetProcessRunningInfos_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<AppExecFwk::RunningProcessInfo> info;
    EXPECT_EQ(abilityMs_->GetProcessRunningInfos(info), ERR_OK);
    HILOG_INFO("AbilityManagerServiceThirdTest GetProcessRunningInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetProcessRunningInfosByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetProcessRunningInfosByUserId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetProcessRunningInfosByUserId_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetProcessRunningInfosByUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<AppExecFwk::RunningProcessInfo> info;
    EXPECT_NE(abilityMs_->GetProcessRunningInfosByUserId(info, 100), INNER_ERR);
    HILOG_INFO("AbilityManagerServiceThirdTest GetProcessRunningInfosByUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ClearUserData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ClearUserData
 */
HWTEST_F(AbilityManagerServiceThirdTest, ClearUserData_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest ClearUserData_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->ClearUserData(100);
    HILOG_INFO("AbilityManagerServiceThirdTest ClearUserData_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CallRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CallRequestDone
 */
HWTEST_F(AbilityManagerServiceThirdTest, CallRequestDone_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest CallRequestDone_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> callStub = nullptr;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->CallRequestDone(token, callStub);
    HILOG_INFO("AbilityManagerServiceThirdTest CallRequestDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateMissionSnapShot
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateMissionSnapShot
 */
HWTEST_F(AbilityManagerServiceThirdTest, UpdateMissionSnapShot_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest UpdateMissionSnapShot_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto pixelMap = std::shared_ptr<Media::PixelMap>();
    MissionSnapshot missionSnapshot;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->UpdateMissionSnapShot(nullptr, pixelMap);
    HILOG_INFO("AbilityManagerServiceThirdTest UpdateMissionSnapShot_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetRemoteMissionSnapshotInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetRemoteMissionSnapshotInfo
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetRemoteMissionSnapshotInfo_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetRemoteMissionSnapshotInfo_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    MissionSnapshot missionSnapshot;
    EXPECT_EQ(abilityMs_->GetRemoteMissionSnapshotInfo("", 1, missionSnapshot), ERR_NULL_OBJECT);
    HILOG_INFO("AbilityManagerServiceThirdTest GetRemoteMissionSnapshotInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetValidUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetValidUserId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetValidUserId_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetValidUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    MissionSnapshot missionSnapshot;
    EXPECT_EQ(abilityMs_->GetValidUserId(100), 100);
    EXPECT_EQ(abilityMs_->GetValidUserId(0), 0);
    HILOG_INFO("AbilityManagerServiceThirdTest GetValidUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsRunningInStabilityTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRunningInStabilityTest
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsRunningInStabilityTest_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest IsRunningInStabilityTest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->controllerIsAStabilityTest_ = false;
    EXPECT_FALSE(abilityMs_->IsRunningInStabilityTest());
    HILOG_INFO("AbilityManagerServiceThirdTest IsRunningInStabilityTest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: InitAbilityInfoFromExtension
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService InitAbilityInfoFromExtension
 */
HWTEST_F(AbilityManagerServiceThirdTest, InitAbilityInfoFromExtension_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest InitAbilityInfoFromExtension_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ExtensionAbilityInfo extensionInfo;
    AbilityInfo abilityInfo;
    EXPECT_EQ(abilityMs_->InitAbilityInfoFromExtension(extensionInfo, abilityInfo), 0);
    HILOG_INFO("AbilityManagerServiceThirdTest InitAbilityInfoFromExtension_001 end");
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: ForceTimeoutForTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ForceTimeoutForTest
 */
HWTEST_F(AbilityManagerServiceThirdTest, ForceTimeoutForTest_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest ForceTimeoutForTest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->ForceTimeoutForTest("", ""), INVALID_DATA);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("clean", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("INITIAL", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("INACTIVE", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("FOREGROUND", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("BACKGROUND", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("TERMINATING", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("COMMAND", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("test", ""), INVALID_DATA);
    HILOG_INFO("AbilityManagerServiceThirdTest ForceTimeoutForTest_001 end");
}
#endif

/*
 * Feature: AbilityManagerService
 * Function: JudgeMultiUserConcurrency
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService JudgeMultiUserConcurrency
 */
HWTEST_F(AbilityManagerServiceThirdTest, JudgeMultiUserConcurrency_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest JudgeMultiUserConcurrency_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_->JudgeMultiUserConcurrency(0));

    auto temp = abilityMs_->userController_;
    abilityMs_->userController_ = nullptr;
    EXPECT_FALSE(abilityMs_->JudgeMultiUserConcurrency(100));
    abilityMs_->userController_ = temp;
    HILOG_INFO("AbilityManagerServiceThirdTest JudgeMultiUserConcurrency_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckWindowMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckWindowMode
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckWindowMode_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest CheckWindowMode_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto windowMode = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED;
    std::vector<AppExecFwk::SupportWindowMode> windowModes;
    EXPECT_TRUE(abilityMs_->CheckWindowMode(windowMode, windowModes));

    windowMode = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN;
    EXPECT_FALSE(abilityMs_->CheckWindowMode(windowMode, windowModes));
    HILOG_INFO("AbilityManagerServiceThirdTest CheckWindowMode_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsNeedTimeoutForTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsNeedTimeoutForTest
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsNeedTimeoutForTest_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest IsNeedTimeoutForTest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_FALSE(abilityMs_->IsNeedTimeoutForTest("", ""));
    abilityMs_->timeoutMap_.insert({"state", "abilityName"});
    EXPECT_TRUE(abilityMs_->IsNeedTimeoutForTest("abilityName", "state"));
    abilityMs_->timeoutMap_.clear();
    HILOG_INFO("AbilityManagerServiceThirdTest IsNeedTimeoutForTest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetValidDataAbilityUri
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetValidDataAbilityUri
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetValidDataAbilityUri_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetValidDataAbilityUri_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string adjustUri;
    EXPECT_FALSE(abilityMs_->GetValidDataAbilityUri("test", adjustUri));

    EXPECT_TRUE(abilityMs_->GetValidDataAbilityUri("//test", adjustUri));
    HILOG_INFO("AbilityManagerServiceThirdTest GetValidDataAbilityUri_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityUri
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityUri
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetDataAbilityUri_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetDataAbilityUri_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::string uri;
    EXPECT_FALSE(abilityMs_->GetDataAbilityUri(abilityInfos, "", uri));
    EXPECT_FALSE(abilityMs_->GetDataAbilityUri(abilityInfos, "test", uri));

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfos.push_back(abilityInfo);
    EXPECT_FALSE(abilityMs_->GetDataAbilityUri(abilityInfos, "", uri));

    abilityInfo.type = AbilityType::DATA;
    abilityInfo.name = "test";
    EXPECT_FALSE(abilityMs_->GetDataAbilityUri(abilityInfos, "test", uri));
    HILOG_INFO("AbilityManagerServiceThirdTest GetDataAbilityUri_001 end");
}

/**
 * @tc.number: ReportDrawnCompleted_002
 * @tc.name: ReportDrawnCompleted
 * @tc.desc: After passing in a callerToken with parameter nullptr, INNER_ERR is returned
 */
HWTEST_F(AbilityManagerServiceThirdTest, ReportDrawnCompleted_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest ReportDrawnCompleted_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<IRemoteObject> callerToken = nullptr;
    EXPECT_EQ(abilityMs_->ReportDrawnCompleted(callerToken), INNER_ERR);
    HILOG_INFO("AbilityManagerServiceThirdTest ReportDrawnCompleted_002 end");
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: BlockAmsService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAmsService
 */
HWTEST_F(AbilityManagerServiceThirdTest, BlockAmsService_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest BlockAmsService_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto temp = abilityMs_->taskHandler_;
    abilityMs_->taskHandler_ = nullptr;
    EXPECT_EQ(abilityMs_->BlockAmsService(), ERR_NO_INIT);

    abilityMs_->taskHandler_ = temp;
    EXPECT_EQ(abilityMs_->BlockAmsService(), ERR_OK);
    HILOG_INFO("AbilityManagerServiceThirdTest BlockAmsService_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: BlockAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, BlockAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest BlockAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    auto temp = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_ = nullptr;
    EXPECT_EQ(abilityMs_->BlockAbility(1), ERR_OK);

    abilityMs_->currentMissionListManager_ = temp;
    EXPECT_EQ(abilityMs_->BlockAbility(1), ERR_OK);
    HILOG_INFO("AbilityManagerServiceThirdTest BlockAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: BlockAppService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAppService
 */
HWTEST_F(AbilityManagerServiceThirdTest, BlockAppService_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest BlockAppService_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->BlockAppService(1), ERR_OK);
    HILOG_INFO("AbilityManagerServiceThirdTest BlockAppService_001 end");
}
#endif

/*
 * Feature: AbilityManagerService
 * Function: CreateVerificationInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CreateVerificationInfo
 */
HWTEST_F(AbilityManagerServiceThirdTest, CreateVerificationInfo_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest CreateVerificationInfo_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AbilityRequest abilityRequest;
    abilityMs_->whiteListassociatedWakeUpFlag_ = false;
    EXPECT_FALSE(abilityMs_->CreateVerificationInfo(abilityRequest).associatedWakeUp);

    abilityMs_->whiteListassociatedWakeUpFlag_ = true;
    abilityRequest.appInfo.bundleName = "com.ohos.settingsdata";
    EXPECT_TRUE(abilityMs_->CreateVerificationInfo(abilityRequest).associatedWakeUp);

    abilityRequest.appInfo.bundleName = "test";
    abilityRequest.appInfo.associatedWakeUp = false;
    EXPECT_FALSE(abilityMs_->CreateVerificationInfo(abilityRequest).associatedWakeUp);
    HILOG_INFO("AbilityManagerServiceThirdTest CreateVerificationInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUser
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUser
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartUser_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest StartUser_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->StartUser(USER_ID_U100, nullptr), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceThirdTest StartUser_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopUser
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopUser
 */
HWTEST_F(AbilityManagerServiceThirdTest, StopUser_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest StopUser_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->StopUser(USER_ID_U100, nullptr), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceThirdTest StopUser_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetStartUpNewRuleFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetStartUpNewRuleFlag
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetStartUpNewRuleFlag_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest GetStartUpNewRuleFlag_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetStartUpNewRuleFlag(), abilityMs_->startUpNewRule_);
    HILOG_INFO("AbilityManagerServiceThirdTest GetStartUpNewRuleFlag_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCrossUserCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCrossUserCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsCrossUserCall_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest IsCrossUserCall_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = -1;
    EXPECT_EQ(abilityMs_->IsCrossUserCall(userId), false);
    HILOG_INFO("AbilityManagerServiceThirdTest IsCrossUserCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCrossUserCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCrossUserCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsCrossUserCall_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest IsCrossUserCall_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = 0;
    EXPECT_EQ(abilityMs_->IsCrossUserCall(userId), false);
    HILOG_INFO("AbilityManagerServiceThirdTest IsCrossUserCall_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCrossUserCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCrossUserCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsCrossUserCall_003, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest IsCrossUserCall_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = 10;
    EXPECT_EQ(abilityMs_->IsCrossUserCall(userId), true);
    HILOG_INFO("AbilityManagerServiceThirdTest IsCrossUserCall_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIds
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIds
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsValidMissionIds_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest IsValidMissionIds_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;
    EXPECT_EQ(abilityMs_->IsValidMissionIds(missionIds, results), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceThirdTest IsValidMissionIds_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckUIExtensionIsFocused
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckUIExtensionIsFocused
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckUIExtensionIsFocused_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest CheckUIExtensionIsFocused_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    bool isFocused = false;
    EXPECT_EQ(abilityMs_->CheckUIExtensionIsFocused(0, isFocused), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceThirdTest CheckUIExtensionIsFocused_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCollaboratorType
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCollaboratorType
 * @tc.require: issueI7LF4X
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckCollaboratorType_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    bool res = abilityMs_->CheckCollaboratorType(CollaboratorType::RESERVE_TYPE);
    EXPECT_EQ(res, true);

    res = abilityMs_->CheckCollaboratorType(CollaboratorType::DEFAULT_TYPE);
    EXPECT_EQ(res, false);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSessionHandler
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSessionHandler
 */
HWTEST_F(AbilityManagerServiceThirdTest, RegisterSessionHandler_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest RegisterSessionHandler_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->RegisterSessionHandler(nullptr), ERR_NO_INIT);
    HILOG_INFO("AbilityManagerServiceThirdTest RegisterSessionHandler_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckUserIdActive
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckUserIdActive
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckUserIdActive_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->CheckUserIdActive(USER_ID_U100);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSessionHandler
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSessionHandler
 */
HWTEST_F(AbilityManagerServiceThirdTest, RegisterSessionHandler_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceThirdTest RegisterSessionHandler_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(abilityMs_->RegisterSessionHandler(nullptr), ERR_WRONG_INTERFACE_CALL);
    HILOG_INFO("AbilityManagerServiceThirdTest RegisterSessionHandler_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsAbilityControllerStart
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsAbilityControllerStart
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsAbilityControllerStart_003, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_TRUE(abilityMs_->IsAbilityControllerStart(want));
}

/*
 * Feature: AbilityManagerService
 * Function: SetPickerElementName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetPickerElementName
 */
HWTEST_F(AbilityManagerServiceThirdTest, SetPickerElementName_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->SetPickerElementName(nullptr, USER_ID_U100);
}

/*
 * Feature: AbilityManagerService
 * Function: SetPickerElementName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetPickerElementName
 */
HWTEST_F(AbilityManagerServiceThirdTest, SetPickerElementName_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    const sptr<SessionInfo> extensionSessionInfo = sessionInfo;
    abilityMs_->SetPickerElementName(extensionSessionInfo, USER_ID_U100);
}

/*
 * Feature: AbilityManagerService
 * Function: SetPickerElementName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetPickerElementName
 */
HWTEST_F(AbilityManagerServiceThirdTest, SetPickerElementName_003, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    Want want;
    const std::string type = "share";
    want.SetParam("ability.want.params.uiExtensionTargetType", type);
    sessionInfo->want = want;
    const sptr<SessionInfo> extensionSessionInfo = sessionInfo;
    abilityMs_->SetPickerElementName(extensionSessionInfo, USER_ID_U100);
}

/*
 * Feature: AbilityManagerService
 * Function: SetPickerElementName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetPickerElementName
 */
HWTEST_F(AbilityManagerServiceThirdTest, SetPickerElementName_004, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    Want want;
    want.SetElementName("com.example.share", "ShareUIExtensionAbility");
    sessionInfo->want = want;
    const sptr<SessionInfo> extensionSessionInfo = sessionInfo;
    abilityMs_->SetPickerElementName(extensionSessionInfo, USER_ID_U100);
}

/*
 * Feature: AbilityManagerService
 * Function: GetElementNameByAppId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetElementNameByAppId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetElementNameByAppId_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    std::string appId = "";
    auto element = abilityMs->GetElementNameByAppId(appId);
    EXPECT_EQ(element.GetBundleName(), "");
}

/*
 * Feature: AbilityManagerService
 * Function: GetElementNameByAppId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetElementNameByAppId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetElementNameByAppId_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    std::string appId = "com.ohos.mms_BCGe7sedrxc1rYrKpF/n6UElJwTjGp/z03SDQ66oBvat7ycay9aTDbq4N6R+cFiJx34bcLJ2prbMUjBX";
    auto element = abilityMs->GetElementNameByAppId(appId);
    EXPECT_EQ(element.GetBundleName(), "");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenAtomicService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OpenAtomicService
 */
HWTEST_F(AbilityManagerServiceThirdTest, OpenAtomicService_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    Want want;
    StartOptions startOptions;
    int32_t userId = 100;
    auto openRet = abilityMs->OpenAtomicService(want, startOptions, nullptr, 1, userId);
    EXPECT_EQ(openRet, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: IsEmbeddedOpenAllowedInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsEmbeddedOpenAllowedInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsEmbeddedOpenAllowedInner_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    std::string appId = "";
    auto isAllowed = abilityMs->IsEmbeddedOpenAllowedInner(nullptr, appId, nullptr);
    EXPECT_FALSE(isAllowed);
}
}  // namespace AAFwk
}  // namespace OHOS
