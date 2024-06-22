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
#include "mock_ipc_skeleton.h"
#include "mock_permission_verification.h"
#include "mock_my_flag.h"
#include "mock_permission_verification.h"
#include "ability_manager_service.h"
#include "ability_start_setting.h"
#include "recovery_param.h"
#undef private
#undef protected

#include "ability_bundle_event_callback.h"
#include "ability_manager_errors.h"
#include "ability_manager_stub_mock_test.h"
#include "connection_observer_errors.h"
#include "hilog_tag_wrapper.h"
#include "mock_sa_call.h"
#include "session/host/include/session.h"
#include "scene_board_judgement.h"
#include "system_ability_definition.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;
namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID_U100 = 100;
constexpr int32_t FOUNDATION_UID = 5523;
const int32_t APP_MEMORY_SIZE = 512;
const std::string EMPTY_DEVICE_ID = "";
}  // namespace
class AbilityManagerServiceThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

public:
    Want want_{};
};


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
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest HandleActiveTimeOut_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->HandleActiveTimeOut(100);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest HandleActiveTimeOut_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: HandleInactiveTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleInactiveTimeOut
 */
HWTEST_F(AbilityManagerServiceThirdTest, HandleInactiveTimeOut_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest HandleInactiveTimeOut_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->HandleInactiveTimeOut(100);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest HandleInactiveTimeOut_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: VerificationToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService VerificationToken
 */
HWTEST_F(AbilityManagerServiceThirdTest, VerificationToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest VerificationToken_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_FALSE(abilityMs_->VerificationToken(nullptr));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest VerificationToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: VerificationAllToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService VerificationAllToken
 */
HWTEST_F(AbilityManagerServiceThirdTest, VerificationAllToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest VerificationAllToken_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_FALSE(abilityMs_->VerificationAllToken(nullptr));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest VerificationAllToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityManager
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityManager
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetDataAbilityManager_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityManager_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetDataAbilityManager(nullptr), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityManager_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionListManagerByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionListManagerByUserId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetListManagerByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetListManagerByUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_EQ(abilityMs_->GetMissionListManagerByUserId(100), nullptr);

    auto temp = abilityMs_->subManagersHelper_->missionListManagers_;
    abilityMs_->subManagersHelper_->missionListManagers_.clear();
    EXPECT_EQ(abilityMs_->GetMissionListManagerByUserId(100).get(), nullptr);
    abilityMs_->subManagersHelper_->missionListManagers_ = temp;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetListManagerByUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityManagerByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityManagerByUserId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetDataAbilityManagerByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityManagerByUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_EQ(abilityMs_->GetDataAbilityManagerByUserId(100), nullptr);

    auto temp = abilityMs_->subManagersHelper_->dataAbilityManagers_;
    abilityMs_->subManagersHelper_->dataAbilityManagers_.clear();
    EXPECT_EQ(abilityMs_->GetDataAbilityManagerByUserId(100).get(), nullptr);
    abilityMs_->subManagersHelper_->dataAbilityManagers_ = temp;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityManagerByUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetConnectManagerByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectManagerByToken
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetConnectManagerByToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetConnectManagerByToken_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetConnectManagerByToken(nullptr), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetConnectManagerByToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityManagerByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityManagerByToken
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetDataAbilityManagerByToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityManagerByToken_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetDataAbilityManagerByToken(nullptr), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityManagerByToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectBmsService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectBmsService
 */
HWTEST_F(AbilityManagerServiceThirdTest, ConnectBmsService_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ConnectBmsService_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->ConnectBmsService();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ConnectBmsService_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetWantSenderInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSenderInfo
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetWantSenderInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetWantSenderInfo_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::shared_ptr<WantSenderInfo> info;
    EXPECT_EQ(abilityMs_->GetWantSenderInfo(nullptr, info), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetWantSenderInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAppMemorySize
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAppMemorySize
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetAppMemorySize_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetAppMemorySize_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetAppMemorySize(), APP_MEMORY_SIZE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetAppMemorySize_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsRamConstrainedDevice
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRamConstrainedDevice
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsRamConstrainedDevice_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsRamConstrainedDevice_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_FALSE(abilityMs_->IsRamConstrainedDevice());
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsRamConstrainedDevice_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionSaveTime
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionSaveTime
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetMissionSaveTime_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetMissionSaveTime_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(AmsConfigurationParameter::GetInstance().GetMissionSaveTime(), 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetMissionSaveTime_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityTokenByMissionId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityTokenByMissionId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetAbilityTokenByMissionId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetAbilityTokenByMissionId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_EQ(abilityMs_->GetAbilityTokenByMissionId(100), nullptr);

    auto temp = abilityMs_->subManagersHelper_->currentMissionListManager_;
    abilityMs_->subManagersHelper_->currentMissionListManager_.reset();
    EXPECT_EQ(abilityMs_->GetAbilityTokenByMissionId(100), nullptr);
    abilityMs_->subManagersHelper_->currentMissionListManager_ = temp;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetAbilityTokenByMissionId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartRemoteAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartRemoteAbilityByCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartRemoteAbilityByCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StartRemoteAbilityByCall_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    Want want;
    EXPECT_EQ(abilityMs_->StartRemoteAbilityByCall(want, nullptr, nullptr), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StartRemoteAbilityByCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseRemoteAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, ReleaseRemoteAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ReleaseRemoteAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AppExecFwk::ElementName element;
    EXPECT_EQ(abilityMs_->ReleaseRemoteAbility(nullptr, element), ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ReleaseRemoteAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, ReleaseCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ReleaseCall_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    AppExecFwk::ElementName element;
    EXPECT_EQ(abilityMs_->ReleaseCall(nullptr, element), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ReleaseCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: JudgeAbilityVisibleControl
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService JudgeAbilityVisibleControl
 */
HWTEST_F(AbilityManagerServiceThirdTest, JudgeAbilityVisibleControl_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest JudgeAbilityVisibleControl_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    EXPECT_EQ(abilityMs_->JudgeAbilityVisibleControl(abilityInfo), ERR_OK);

    abilityInfo.visible = false;
    EXPECT_EQ(abilityMs_->JudgeAbilityVisibleControl(abilityInfo), ERR_OK);

    abilityInfo.applicationInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    EXPECT_EQ(abilityMs_->JudgeAbilityVisibleControl(abilityInfo), ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest JudgeAbilityVisibleControl_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnAcceptWantResponse
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAcceptWantResponse
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnAcceptWantResponse_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest OnAcceptWantResponse_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    AAFwk::Want want;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->OnAcceptWantResponse(want, "test");

    auto temp = abilityMs_->subManagersHelper_->currentMissionListManager_;
    abilityMs_->subManagersHelper_->currentMissionListManager_.reset();
    abilityMs_->OnAcceptWantResponse(want, "test");
    abilityMs_->subManagersHelper_->currentMissionListManager_ = temp;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest OnAcceptWantResponse_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnStartSpecifiedAbilityTimeoutResponse
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnStartSpecifiedAbilityTimeoutResponse
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnStartSpecifiedAbilityTimeoutResponse_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest OnStartSpecifiedAbilityTimeoutResponse_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    AAFwk::Want want;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->OnStartSpecifiedAbilityTimeoutResponse(want);

    auto temp = abilityMs_->subManagersHelper_->currentMissionListManager_;
    abilityMs_->subManagersHelper_->currentMissionListManager_.reset();
    abilityMs_->OnStartSpecifiedAbilityTimeoutResponse(want);
    abilityMs_->subManagersHelper_->currentMissionListManager_ = temp;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest OnStartSpecifiedAbilityTimeoutResponse_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityRunningInfos
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetAbilityRunningInfos_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetAbilityRunningInfos_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    std::vector<AbilityRunningInfo> info;
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(abilityMs_->GetAbilityRunningInfos(info), ERR_OK);

        auto temp1 = abilityMs_->subManagersHelper_->currentMissionListManager_;
        abilityMs_->subManagersHelper_->currentMissionListManager_.reset();
        EXPECT_EQ(abilityMs_->GetAbilityRunningInfos(info), ERR_INVALID_VALUE);
        abilityMs_->subManagersHelper_->currentMissionListManager_ = temp1;

        auto temp2 = abilityMs_->subManagersHelper_->currentConnectManager_;
        abilityMs_->subManagersHelper_->currentConnectManager_.reset();
        EXPECT_NE(abilityMs_->GetAbilityRunningInfos(info), ERR_OK);
        abilityMs_->subManagersHelper_->currentConnectManager_ = temp2;

        auto temp3 = abilityMs_->subManagersHelper_->currentDataAbilityManager_;
        abilityMs_->subManagersHelper_->currentDataAbilityManager_.reset();
        EXPECT_NE(abilityMs_->GetAbilityRunningInfos(info), ERR_OK);
        abilityMs_->subManagersHelper_->currentDataAbilityManager_ = temp3;
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetAbilityRunningInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetExtensionRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetExtensionRunningInfos
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetExtensionRunningInfos_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetExtensionRunningInfos_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    std::vector<AAFwk::ExtensionRunningInfo> extensionRunningInfo;
    EXPECT_NE(abilityMs_->GetExtensionRunningInfos(10, extensionRunningInfo), ERR_OK);

    auto temp = abilityMs_->subManagersHelper_->currentConnectManager_;
    abilityMs_->subManagersHelper_->currentConnectManager_.reset();
    EXPECT_EQ(abilityMs_->GetExtensionRunningInfos(10, extensionRunningInfo), ERR_INVALID_VALUE);
    abilityMs_->subManagersHelper_->currentConnectManager_ = temp;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetExtensionRunningInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetProcessRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetProcessRunningInfos
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetProcessRunningInfos_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetProcessRunningInfos_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<AppExecFwk::RunningProcessInfo> info;
    EXPECT_EQ(abilityMs_->GetProcessRunningInfos(info), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetProcessRunningInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetProcessRunningInfosByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetProcessRunningInfosByUserId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetProcessRunningInfosByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetProcessRunningInfosByUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<AppExecFwk::RunningProcessInfo> info;
    EXPECT_NE(abilityMs_->GetProcessRunningInfosByUserId(info, 100), INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetProcessRunningInfosByUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ClearUserData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ClearUserData
 */
HWTEST_F(AbilityManagerServiceThirdTest, ClearUserData_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearUserData_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->ClearUserData(100);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearUserData_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CallRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CallRequestDone
 */
HWTEST_F(AbilityManagerServiceThirdTest, CallRequestDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CallRequestDone_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> callStub = nullptr;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->CallRequestDone(token, callStub);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CallRequestDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateMissionSnapShot
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateMissionSnapShot
 */
HWTEST_F(AbilityManagerServiceThirdTest, UpdateMissionSnapShot_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest UpdateMissionSnapShot_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto pixelMap = std::shared_ptr<Media::PixelMap>();
    MissionSnapshot missionSnapshot;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->UpdateMissionSnapShot(nullptr, pixelMap);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest UpdateMissionSnapShot_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetRemoteMissionSnapshotInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetRemoteMissionSnapshotInfo
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetRemoteMissionSnapshotInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetRemoteMissionSnapshotInfo_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    MissionSnapshot missionSnapshot;
    EXPECT_EQ(abilityMs_->GetRemoteMissionSnapshotInfo("", 1, missionSnapshot), ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetRemoteMissionSnapshotInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetValidUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetValidUserId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetValidUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetValidUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    MissionSnapshot missionSnapshot;
    EXPECT_EQ(abilityMs_->GetValidUserId(100), 100);
    EXPECT_EQ(abilityMs_->GetValidUserId(0), 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetValidUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsRunningInStabilityTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRunningInStabilityTest
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsRunningInStabilityTest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsRunningInStabilityTest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->controllerIsAStabilityTest_ = false;
    EXPECT_FALSE(abilityMs_->IsRunningInStabilityTest());
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsRunningInStabilityTest_001 end");
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
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ForceTimeoutForTest_001 start");
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
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ForceTimeoutForTest_001 end");
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
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest JudgeMultiUserConcurrency_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_->JudgeMultiUserConcurrency(0));

    auto temp = abilityMs_->userController_;
    abilityMs_->userController_ = nullptr;
    EXPECT_FALSE(abilityMs_->JudgeMultiUserConcurrency(100));
    abilityMs_->userController_ = temp;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest JudgeMultiUserConcurrency_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckWindowMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckWindowMode
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckWindowMode_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CheckWindowMode_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto windowMode = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED;
    std::vector<AppExecFwk::SupportWindowMode> windowModes;
    EXPECT_TRUE(abilityMs_->CheckWindowMode(windowMode, windowModes));

    windowMode = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN;
    EXPECT_FALSE(abilityMs_->CheckWindowMode(windowMode, windowModes));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CheckWindowMode_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsNeedTimeoutForTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsNeedTimeoutForTest
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsNeedTimeoutForTest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsNeedTimeoutForTest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_FALSE(abilityMs_->IsNeedTimeoutForTest("", ""));
    abilityMs_->timeoutMap_.insert({"state", "abilityName"});
    EXPECT_TRUE(abilityMs_->IsNeedTimeoutForTest("abilityName", "state"));
    abilityMs_->timeoutMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsNeedTimeoutForTest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetValidDataAbilityUri
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetValidDataAbilityUri
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetValidDataAbilityUri_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetValidDataAbilityUri_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string adjustUri;
    EXPECT_FALSE(abilityMs_->GetValidDataAbilityUri("test", adjustUri));

    EXPECT_TRUE(abilityMs_->GetValidDataAbilityUri("//test", adjustUri));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetValidDataAbilityUri_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityUri
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityUri
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetDataAbilityUri_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityUri_001 start");
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
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityUri_001 end");
}

/**
 * @tc.number: ReportDrawnCompleted_001
 * @tc.name: ReportDrawnCompleted
 * @tc.desc: After passing in a callerToken with parameter nullptr, INNER_ERR is returned
 */
HWTEST_F(AbilityManagerServiceThirdTest, ReportDrawnCompleted_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ReportDrawnCompleted_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<IRemoteObject> callerToken = nullptr;
    EXPECT_EQ(abilityMs_->ReportDrawnCompleted(callerToken), INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ReportDrawnCompleted_001 end");
}

/**
 * @tc.number: ReportDrawnCompleted_002
 * @tc.name: ReportDrawnCompleted
 * @tc.desc: After passing in a callerToken with parameter nullptr, INNER_ERR is returned
 */
HWTEST_F(AbilityManagerServiceThirdTest, ReportDrawnCompleted_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ReportDrawnCompleted_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    EXPECT_EQ(abilityMs_->ReportDrawnCompleted(callerToken), INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ReportDrawnCompleted_002 end");
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
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest BlockAmsService_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto temp = abilityMs_->taskHandler_;
    abilityMs_->taskHandler_ = nullptr;
    EXPECT_EQ(abilityMs_->BlockAmsService(), ERR_NO_INIT);

    abilityMs_->taskHandler_ = temp;
    EXPECT_EQ(abilityMs_->BlockAmsService(), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest BlockAmsService_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: BlockAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, BlockAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest BlockAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    auto temp = abilityMs_->subManagersHelper_->currentMissionListManager_;
    abilityMs_->subManagersHelper_->currentMissionListManager_ = nullptr;
    EXPECT_EQ(abilityMs_->BlockAbility(1), ERR_OK);

    abilityMs_->subManagersHelper_->currentMissionListManager_ = temp;
    EXPECT_EQ(abilityMs_->BlockAbility(1), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest BlockAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: BlockAppService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAppService
 */
HWTEST_F(AbilityManagerServiceThirdTest, BlockAppService_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest BlockAppService_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->BlockAppService(1), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest BlockAppService_001 end");
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
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CreateVerificationInfo_001 start");
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
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CreateVerificationInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUser
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUser
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartUser_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StartUser_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->StartUser(USER_ID_U100, nullptr), CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StartUser_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopUser
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopUser
 */
HWTEST_F(AbilityManagerServiceThirdTest, StopUser_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StopUser_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->StopUser(USER_ID_U100, nullptr), CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StopUser_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetStartUpNewRuleFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetStartUpNewRuleFlag
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetStartUpNewRuleFlag_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetStartUpNewRuleFlag_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetStartUpNewRuleFlag(), abilityMs_->startUpNewRule_);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetStartUpNewRuleFlag_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCrossUserCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCrossUserCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsCrossUserCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsCrossUserCall_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = -1;
    EXPECT_EQ(abilityMs_->IsCrossUserCall(userId), false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsCrossUserCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCrossUserCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCrossUserCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsCrossUserCall_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsCrossUserCall_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = 0;
    EXPECT_EQ(abilityMs_->IsCrossUserCall(userId), false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsCrossUserCall_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCrossUserCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCrossUserCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsCrossUserCall_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsCrossUserCall_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = 10;
    EXPECT_EQ(abilityMs_->IsCrossUserCall(userId), true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsCrossUserCall_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIds
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIds
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsValidMissionIds_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsValidMissionIds_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;
    EXPECT_EQ(abilityMs_->IsValidMissionIds(missionIds, results), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsValidMissionIds_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckUIExtensionIsFocused
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckUIExtensionIsFocused
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckUIExtensionIsFocused_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CheckUIExtensionIsFocused_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    bool isFocused = false;
    MyFlag::flag_ = 0;
    EXPECT_EQ(abilityMs_->CheckUIExtensionIsFocused(0, isFocused), CHECK_PERMISSION_FAILED);
    MyFlag::flag_ = 1;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CheckUIExtensionIsFocused_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AddFreeInstallObserver
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AddFreeInstallObserver
 */
HWTEST_F(AbilityManagerServiceThirdTest, AddFreeInstallObserver_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest AddFreeInstallObserver_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<AbilityRuntime::IFreeInstallObserver> observer;
    EXPECT_EQ(abilityMs_->AddFreeInstallObserver(observer), ERR_INVALID_VALUE);

    abilityMs_->freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    EXPECT_EQ(abilityMs_->AddFreeInstallObserver(observer), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest AddFreeInstallObserver_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: VerifyPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService VerifyPermission
 */
HWTEST_F(AbilityManagerServiceThirdTest, VerifyPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest VerifyPermission_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();

    std::string permission = "test_permission";
    int pid = 0;
    int uid = 0;
    EXPECT_EQ(abilityMs_->VerifyPermission(permission, pid, uid), CHECK_PERMISSION_FAILED);

    std::string permission2 = "";
    EXPECT_EQ(abilityMs_->VerifyPermission(permission2, pid, uid), CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest VerifyPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireShareData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireShareData
 */
HWTEST_F(AbilityManagerServiceThirdTest, AcquireShareData_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest AcquireShareData_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();

    int32_t missionId = 1;
    sptr<IAcquireShareDataCallback> shareData = nullptr;
    EXPECT_EQ(abilityMs_->AcquireShareData(missionId, shareData), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest AcquireShareData_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ShareDataDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ShareDataDone
 */
HWTEST_F(AbilityManagerServiceThirdTest, ShareDataDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ShareDataDone_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();

    sptr<IRemoteObject> token = nullptr;
    int32_t resultCode = 1;
    int32_t uniqueId = 1;
    WantParams wantParam;
    EXPECT_EQ(abilityMs_->ShareDataDone(token, resultCode, uniqueId, wantParam), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ShareDataDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: NotifySaveAsResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifySaveAsResult
 */
HWTEST_F(AbilityManagerServiceThirdTest, NotifySaveAsResult_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest NotifySaveAsResult_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();

    Want want;
    auto result = abilityMs_->NotifySaveAsResult(want, 0, 0);
    EXPECT_EQ(result, ERR_INVALID_CALLER);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest NotifySaveAsResult_001 end");
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
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RegisterSessionHandler_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->RegisterSessionHandler(nullptr), ERR_WRONG_INTERFACE_CALL);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RegisterSessionHandler_001 end");
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
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RegisterSessionHandler_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(abilityMs_->RegisterSessionHandler(nullptr), ERR_WRONG_INTERFACE_CALL);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RegisterSessionHandler_002 end");
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
 * Function: RestartApp
 * FunctionPoints: AbilityManagerService RestartApp
 */
HWTEST_F(AbilityManagerServiceThirdTest, RestartApp_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    AAFwk::Want want;
    int32_t res = abilityMs->RestartApp(want);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: UnloadUIExtensionAbility
 * FunctionPoints: AbilityManagerService UnloadUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, UnloadUIExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    AbilityRequest abilityRequest;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    abilityRequest.want.SetElement(providerElement);
    abilityRequest.abilityInfo.type = AbilityType::EXTENSION;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    std::string hostBundleName = "com.ohos.uiextensionuser";
    auto result = abilityMs->UnloadUIExtensionAbility(abilityRecord, hostBundleName);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/*
 * Feature: AbilityManagerService
 * Function: PreloadUIExtensionAbility
 * FunctionPoints: AbilityManagerService PreloadUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, PreloadUIExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want providerWant;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    providerWant.SetElement(providerElement);
    std::string hostBundleName = "com.ohos.uiextensionuser";
    auto result = abilityMs->PreloadUIExtensionAbility(providerWant, hostBundleName, DEFAULT_INVAL_VALUE);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/*
 * Feature: AbilityManagerService
 * Function: PreloadUIExtensionAbilityInner
 * FunctionPoints: AbilityManagerService PreloadUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, PreloadUIExtensionAbilityInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want providerWant;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    providerWant.SetElement(providerElement);
    std::string hostBundleName = "com.ohos.uiextensionuser";
    auto result = abilityMs->PreloadUIExtensionAbilityInner(providerWant, hostBundleName, DEFAULT_INVAL_VALUE);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckRestartAppWant
 * FunctionPoints: AbilityManagerService CheckRestartAppWant
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckRestartAppWant_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    AAFwk::Want want;
    int32_t res = abilityMs->CheckRestartAppWant(want);
    EXPECT_EQ(res, AAFwk::ERR_RESTART_APP_INCORRECT_ABILITY);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityStateByPersistentId
 * FunctionPoints: AbilityManagerService GetAbilityStateByPersistentId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetAbilityStateByPersistentId_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    int32_t persistentId = 100;
    bool state;
    int32_t res = abilityMs->GetAbilityStateByPersistentId(persistentId, state);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/*
 * Feature: AbilityManagerService
 * Function: InitDeepLinkReserve
 * FunctionPoints: AbilityManagerService InitDeepLinkReserve
 */
HWTEST_F(AbilityManagerServiceThirdTest, InitDeepLinkReserve_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->InitDeepLinkReserve();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: InitInterceptor
 * FunctionPoints: AbilityManagerService InitInterceptor
 */
HWTEST_F(AbilityManagerServiceThirdTest, InitInterceptor_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->InitInterceptor();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: InitDefaultRecoveryList
 * FunctionPoints: AbilityManagerService InitDefaultRecoveryList
 */
HWTEST_F(AbilityManagerServiceThirdTest, InitDefaultRecoveryList_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->InitDefaultRecoveryList();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: InitPushTask
 * FunctionPoints: AbilityManagerService InitPushTask
 */
HWTEST_F(AbilityManagerServiceThirdTest, InitPushTask_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->InitPushTask();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: InitPushTask
 * FunctionPoints: AbilityManagerService InitPushTask
 */
HWTEST_F(AbilityManagerServiceThirdTest, InitPushTask_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->taskHandler_ = nullptr;
    abilityMs->InitPushTask();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: InitStartupFlag
 * FunctionPoints: AbilityManagerService InitStartupFlag
 */
HWTEST_F(AbilityManagerServiceThirdTest, InitStartupFlag_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->InitStartupFlag();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: InitStartAbilityChain
 * FunctionPoints: AbilityManagerService InitStartAbilityChain
 */
HWTEST_F(AbilityManagerServiceThirdTest, InitStartAbilityChain_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->InitStartAbilityChain();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnStop
 * FunctionPoints: AbilityManagerService OnStop
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnStop_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->abilityBundleEventCallback_ = new (std::nothrow) AbilityBundleEventCallback(nullptr, nullptr);
    abilityMs->OnStop();
    EXPECT_NE(abilityMs->abilityBundleEventCallback_, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnStop
 * FunctionPoints: AbilityManagerService OnStop
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnStop_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->OnStop();
    EXPECT_EQ(abilityMs->abilityBundleEventCallback_, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: QueryServiceState
 * FunctionPoints: AbilityManagerService QueryServiceState
 */
HWTEST_F(AbilityManagerServiceThirdTest, QueryServiceState_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->QueryServiceState();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByFreeInstall
 * FunctionPoints: AbilityManagerService StartAbilityByFreeInstall
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByFreeInstall_001, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int32_t requestCode = 0;
    auto result = abilityMs->StartAbilityByFreeInstall(want, callerToken, userId, requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByFreeInstall
 * FunctionPoints: AbilityManagerService StartAbilityByFreeInstall
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByFreeInstall_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int32_t requestCode = 0;
    auto result = abilityMs->StartAbilityByFreeInstall(want, callerToken, userId, requestCode);
    EXPECT_EQ(result, ERR_INVALID_CONTINUATION_FLAG);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWithSpecifyTokenId
 * FunctionPoints: AbilityManagerService StartAbilityWithSpecifyTokenId
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityWithSpecifyTokenId_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    uint32_t specifyTokenId = 0;
    int32_t userId = 0;
    int32_t requestCode = 0;
    auto result = abilityMs->StartAbilityWithSpecifyTokenId(want, callerToken, specifyTokenId, userId, requestCode);
    EXPECT_EQ(result, ERR_INVALID_CONTINUATION_FLAG);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWithSpecifyTokenId
 * FunctionPoints: AbilityManagerService StartAbilityWithSpecifyTokenId
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityWithSpecifyTokenId_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    uint32_t specifyTokenId = 0;
    int32_t userId = 0;
    int32_t requestCode = 0;
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    auto result = abilityMs->StartAbilityWithSpecifyTokenId(want, callerToken, specifyTokenId, userId, requestCode);
    EXPECT_NE(result, ERR_INVALID_CONTINUATION_FLAG);
}
/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByInsightIntent
 * FunctionPoints: AbilityManagerService StartAbilityByInsightIntent
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByInsightIntent_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    uint64_t intentId = 0;
    int32_t userId = 0;
    auto result = abilityMs->StartAbilityByInsightIntent(want, callerToken, intentId, userId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWithSpecifyTokenIdInner
 * FunctionPoints: AbilityManagerService StartAbilityWithSpecifyTokenIdInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityWithSpecifyTokenIdInner_001, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 0;
    EXPECT_EQ(abilityMs_->StartAbilityWithSpecifyTokenIdInner(want, callerToken, USER_ID_U100, requestCode),
        ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWithSpecifyTokenIdInner
 * FunctionPoints: AbilityManagerService StartAbilityWithSpecifyTokenIdInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityWithSpecifyTokenIdInner_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 0;
    EXPECT_EQ(abilityMs_->StartAbilityWithSpecifyTokenIdInner(want, callerToken, USER_ID_U100, requestCode),
        ERR_INVALID_CONTINUATION_FLAG);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    sptr<SessionInfo> sessionInfo;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    sptr<SessionInfo> sessionInfo;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_003, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_004, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_005, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken;
    sptr<SessionInfo> sessionInfo;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, startOptions, callerToken, sessionInfo, userId,
        requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_006, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    sptr<SessionInfo> sessionInfo;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, startOptions, callerToken, sessionInfo, userId,
        requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_007, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken;
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, startOptions, callerToken, sessionInfo, userId,
        requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_008, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, startOptions, callerToken, sessionInfo, userId,
        requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ImplicitStartAbilityAsCaller
 * FunctionPoints: AbilityManagerService ImplicitStartAbilityAsCaller
 */
HWTEST_F(AbilityManagerServiceThirdTest, ImplicitStartAbilityAsCaller_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    sptr<IRemoteObject> asCallerSourceToken;
    int32_t userId = 0;
    int requestCode = 0;
    bool isSendDialogResult = true;
    auto result = abilityMs->ImplicitStartAbilityAsCaller(want, callerToken, asCallerSourceToken, userId,
        requestCode, isSendDialogResult);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCallerDetails
 * FunctionPoints: AbilityManagerService StartAbilityAsCallerDetails
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityAsCallerDetails_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    sptr<IRemoteObject> asCallerSourceToken;
    int32_t userId = 0;
    int requestCode = 0;
    bool isSendDialogResult = true;
    bool isImplicit = true;
    auto result = abilityMs->StartAbilityAsCallerDetails(want, callerToken, asCallerSourceToken, userId,
        requestCode, isSendDialogResult, isImplicit);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCallerDetails
 * FunctionPoints: AbilityManagerService StartAbilityAsCallerDetails
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityAsCallerDetails_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    sptr<IRemoteObject> callerToken;
    sptr<IRemoteObject> asCallerSourceToken;
    int32_t userId = 0;
    int requestCode = 0;
    bool isSendDialogResult = true;
    bool isImplicit = true;
    auto result = abilityMs->StartAbilityAsCallerDetails(want, callerToken, asCallerSourceToken, userId,
        requestCode, isSendDialogResult, isImplicit);
    EXPECT_EQ(result, ERR_INVALID_CONTINUATION_FLAG);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCallerDetails
 * FunctionPoints: AbilityManagerService StartAbilityAsCallerDetails
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityAsCallerDetails_003, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    sptr<IRemoteObject> asCallerSourceToken = new AbilityManagerStubTestMock();
    int32_t userId = 0;
    int requestCode = 0;
    bool isSendDialogResult = true;
    bool isImplicit = true;
    auto result = abilityMs->StartAbilityAsCallerDetails(want, callerToken, asCallerSourceToken, userId,
        requestCode, isSendDialogResult, isImplicit);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityPublicPrechainCheck
 * FunctionPoints: AbilityManagerService StartAbilityPublicPrechainCheck
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityPublicPrechainCheck_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartAbilityParams startParams(const_cast<Want &>(want));
    auto result = abilityMs->StartAbilityPublicPrechainCheck(startParams);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityPrechainInterceptor
 * FunctionPoints: AbilityManagerService StartAbilityPrechainInterceptor
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityPrechainInterceptor_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartAbilityParams startParams(const_cast<Want &>(want));
    auto result = abilityMs->StartAbilityPrechainInterceptor(startParams);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInChain
 * FunctionPoints: AbilityManagerService StartAbilityInChain
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityInChain_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartAbilityParams startParams(const_cast<Want &>(want));
    int result = 0;
    auto ret = abilityMs->StartAbilityInChain(startParams, result);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWrap
 * FunctionPoints: AbilityManagerService StartAbilityWrap
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityWrap_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    int requestCode = 0;
    int32_t userId = 0;
    bool isStartAsCaller = true;
    bool isSendDialogResult = true;
    uint32_t specifyToken = 0;
    bool isForegroundToRestartApp = true;
    bool isImplicit = true;
    auto result = abilityMs->StartAbilityWrap(want, callerToken, requestCode, userId, isStartAsCaller,
        isSendDialogResult, specifyToken, isForegroundToRestartApp, isImplicit);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: SetReserveInfo
 * FunctionPoints: AbilityManagerService SetReserveInfo
 */
HWTEST_F(AbilityManagerServiceThirdTest, SetReserveInfo_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string linkString = "";
    abilityMs->SetReserveInfo(linkString);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: ImplicitStartAbility
 * FunctionPoints: AbilityManagerService ImplicitStartAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, ImplicitStartAbility_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    AbilityStartSetting abilityStartSetting;
    const sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->ImplicitStartAbility(want, abilityStartSetting, callerToken, userId, requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ImplicitStartAbility
 * FunctionPoints: AbilityManagerService ImplicitStartAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, ImplicitStartAbility_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    const sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->ImplicitStartAbility(want, startOptions, callerToken, userId, requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityForOptionWrap
 * FunctionPoints: AbilityManagerService StartUIAbilityForOptionWrap
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartUIAbilityForOptionWrap_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    const sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int requestCode = 0;
    uint32_t callerTokenId = 0;
    bool isImplicit = true;
    auto result = abilityMs->StartUIAbilityForOptionWrap(want, startOptions, callerToken, userId, requestCode,
        callerTokenId, isImplicit);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForOptionWrap
 * FunctionPoints: AbilityManagerService StartAbilityForOptionWrap
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityForOptionWrap_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    const sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int requestCode = 0;
    bool isStartAsCaller = true;
    uint32_t callerTokenId = 0;
    bool isImplicit = true;
    auto result = abilityMs->StartAbilityForOptionWrap(want, startOptions, callerToken, userId, requestCode,
        isStartAsCaller, callerTokenId, isImplicit);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForOptionInner
 * FunctionPoints: AbilityManagerService StartAbilityForOptionInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityForOptionInner_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    const sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int requestCode = 0;
    bool isStartAsCaller = true;
    uint32_t specifyTokenId = 0;
    bool isImplicit = true;
    auto result = abilityMs->StartAbilityForOptionInner(want, startOptions, callerToken, userId, requestCode,
        isStartAsCaller, specifyTokenId, isImplicit);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    result = abilityMs->StartAbilityForOptionInner(want, startOptions, callerToken, userId, requestCode,
        isStartAsCaller, specifyTokenId, isImplicit);
    EXPECT_NE(result, ERR_INVALID_VALUE);

    abilityMs-> implicitStartProcessor_ = std::make_shared<ImplicitStartProcessor>();
    result = abilityMs->StartAbilityForOptionInner(want, startOptions, callerToken, userId, requestCode,
        isStartAsCaller, specifyTokenId, isImplicit);
    EXPECT_NE(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RequestDialogService
 * FunctionPoints: AbilityManagerService RequestDialogService
 */
HWTEST_F(AbilityManagerServiceThirdTest, RequestDialogService_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    const sptr<IRemoteObject> callerToken;
    auto result = abilityMs->RequestDialogService(want, callerToken);
    EXPECT_EQ(result, ERR_INVALID_CALLER);
}

/*
 * Feature: AbilityManagerService
 * Function: RequestDialogService
 * FunctionPoints: AbilityManagerService RequestDialogService
 */
HWTEST_F(AbilityManagerServiceThirdTest, RequestDialogService_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    const sptr<IRemoteObject> callerToken;
    auto result = abilityMs->RequestDialogService(want, callerToken);
    EXPECT_EQ(result, ERR_INVALID_CONTINUATION_FLAG);
}

/*
 * Feature: AbilityManagerService
 * Function: RequestDialogServiceInner
 * FunctionPoints: AbilityManagerService RequestDialogServiceInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, RequestDialogServiceInner_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    const sptr<IRemoteObject> callerToken;
    int requestCode = 0;
    int32_t userId = 0;
    auto result = abilityMs->RequestDialogServiceInner(want, callerToken, requestCode, userId);
    EXPECT_EQ(result, ERR_INVALID_CALLER);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityBySCB
 * FunctionPoints: AbilityManagerService StartUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartUIAbilityBySCB_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> sessionInfo = nullptr;
    bool isColdStart = true;
    auto result = abilityMs->StartUIAbilityBySCB(sessionInfo, isColdStart);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityBySCB
 * FunctionPoints: AbilityManagerService StartUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartUIAbilityBySCB_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    bool isColdStart = true;
    auto result = abilityMs->StartUIAbilityBySCB(sessionInfo, isColdStart);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityBySCB
 * FunctionPoints: AbilityManagerService StartUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartUIAbilityBySCB_003, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    bool isColdStart = true;
    auto result = abilityMs->StartUIAbilityBySCB(sessionInfo, isColdStart);
    EXPECT_EQ(result, ERR_WRONG_INTERFACE_CALL);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    result = abilityMs->StartUIAbilityBySCB(sessionInfo, isColdStart);
    EXPECT_EQ(result, ERR_WRONG_INTERFACE_CALL);
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallingTokenId
 * FunctionPoints: AbilityManagerService CheckCallingTokenId
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckCallingTokenId_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string bundleName = "test";
    int32_t userId = 0;
    auto result = abilityMs->CheckCallingTokenId(bundleName, userId);
    EXPECT_EQ(result, true);
}

/*
 * Feature: AbilityManagerService
 * Function: IsCallerSceneBoard
 * FunctionPoints: AbilityManagerService IsCallerSceneBoard
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsCallerSceneBoard_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto result = abilityMs->IsCallerSceneBoard();
    EXPECT_EQ(result, false);
}

/*
 * Feature: AbilityManagerService
 * Function: IsDmsAlive
 * FunctionPoints: AbilityManagerService IsDmsAlive
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsDmsAlive_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto result = abilityMs->IsDmsAlive();
    EXPECT_EQ(result, false);
}

/*
 * Feature: AbilityManagerService
 * Function: AppUpgradeCompleted
 * FunctionPoints: AbilityManagerService AppUpgradeCompleted
 */
HWTEST_F(AbilityManagerServiceThirdTest, AppUpgradeCompleted_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string bundleName = "test";
    int32_t uid = 0;
    abilityMs->AppUpgradeCompleted(bundleName, uid);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: AppUpgradeCompleted
 * FunctionPoints: AbilityManagerService AppUpgradeCompleted
 */
HWTEST_F(AbilityManagerServiceThirdTest, AppUpgradeCompleted_002, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string bundleName = "test";
    int32_t uid = 1;
    abilityMs->AppUpgradeCompleted(bundleName, uid);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAppMgr
 * FunctionPoints: AbilityManagerService GetAppMgr
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetAppMgr_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto result = abilityMs->GetAppMgr();
    EXPECT_NE(result, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnAddSystemAbility
 * FunctionPoints: AbilityManagerService OnAddSystemAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnAddSystemAbility_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t systemAbilityId = BACKGROUND_TASK_MANAGER_SERVICE_ID;
    std::string deviceId = "";
    abilityMs->OnAddSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnAddSystemAbility
 * FunctionPoints: AbilityManagerService OnAddSystemAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnAddSystemAbility_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t systemAbilityId = DISTRIBUTED_SCHED_SA_ID;
    std::string deviceId = "";
    abilityMs->OnAddSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnAddSystemAbility
 * FunctionPoints: AbilityManagerService OnAddSystemAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnAddSystemAbility_003, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t systemAbilityId = BUNDLE_MGR_SERVICE_SYS_ABILITY_ID;
    std::string deviceId = "";
    abilityMs->OnAddSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoveSystemAbility
 * FunctionPoints: AbilityManagerService OnRemoveSystemAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnRemoveSystemAbility_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t systemAbilityId = BACKGROUND_TASK_MANAGER_SERVICE_ID;
    std::string deviceId = "";
    abilityMs->OnRemoveSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoveSystemAbility
 * FunctionPoints: AbilityManagerService OnRemoveSystemAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnRemoveSystemAbility_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t systemAbilityId = DISTRIBUTED_SCHED_SA_ID;
    std::string deviceId = "";
    abilityMs->OnRemoveSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoveSystemAbility
 * FunctionPoints: AbilityManagerService OnRemoveSystemAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnRemoveSystemAbility_003, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t systemAbilityId = BUNDLE_MGR_SERVICE_SYS_ABILITY_ID;
    std::string deviceId = "";
    abilityMs->OnRemoveSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: SubscribeBundleEventCallback
 * FunctionPoints: AbilityManagerService SubscribeBundleEventCallback
 */
HWTEST_F(AbilityManagerServiceThirdTest, SubscribeBundleEventCallback_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->abilityBundleEventCallback_ = nullptr;
    abilityMs->SubscribeBundleEventCallback();
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: SubscribeBundleEventCallback
 * FunctionPoints: AbilityManagerService SubscribeBundleEventCallback
 */
HWTEST_F(AbilityManagerServiceThirdTest, SubscribeBundleEventCallback_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->abilityBundleEventCallback_ = new (std::nothrow) AbilityBundleEventCallback(nullptr, nullptr);
    abilityMs->SubscribeBundleEventCallback();
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: UnsubscribeBundleEventCallback
 * FunctionPoints: AbilityManagerService UnsubscribeBundleEventCallback
 */
HWTEST_F(AbilityManagerServiceThirdTest, UnsubscribeBundleEventCallback_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->abilityBundleEventCallback_ = nullptr;
    abilityMs->UnsubscribeBundleEventCallback();
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: UnsubscribeBundleEventCallback
 * FunctionPoints: AbilityManagerService UnsubscribeBundleEventCallback
 */
HWTEST_F(AbilityManagerServiceThirdTest, UnsubscribeBundleEventCallback_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->abilityBundleEventCallback_ = new (std::nothrow) AbilityBundleEventCallback(nullptr, nullptr);
    abilityMs->UnsubscribeBundleEventCallback();
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: ImplicitStartExtensionAbility
 * FunctionPoints: AbilityManagerService ImplicitStartExtensionAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, ImplicitStartExtensionAbility_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UI;
    auto result = abilityMs->ImplicitStartExtensionAbility(want, callerToken, userId, extensionType);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ImplicitStartExtensionAbility
 * FunctionPoints: AbilityManagerService ImplicitStartExtensionAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, ImplicitStartExtensionAbility_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::VPN;
    auto result = abilityMs->ImplicitStartExtensionAbility(want, callerToken, userId, extensionType);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtension
 * FunctionPoints: AbilityManagerService RequestModalUIExtension
 */
HWTEST_F(AbilityManagerServiceThirdTest, RequestModalUIExtension_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    auto result = abilityMs->RequestModalUIExtension(want);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionInner
 * FunctionPoints: AbilityManagerService RequestModalUIExtensionInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, RequestModalUIExtensionInner_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    auto result = abilityMs->RequestModalUIExtensionInner(want);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ChangeAbilityVisibility
 * FunctionPoints: AbilityManagerService ChangeAbilityVisibility
 */
HWTEST_F(AbilityManagerServiceThirdTest, ChangeAbilityVisibility_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<IRemoteObject> token;
    bool isShow = true;
    auto result = abilityMs->ChangeAbilityVisibility(token, isShow);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
}

/*
 * Feature: AbilityManagerService
 * Function: ChangeUIAbilityVisibilityBySCB
 * FunctionPoints: AbilityManagerService ChangeUIAbilityVisibilityBySCB
 */
HWTEST_F(AbilityManagerServiceThirdTest, ChangeUIAbilityVisibilityBySCB_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> sessionInfo;
    bool isShow = true;
    auto result = abilityMs->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow);
    EXPECT_EQ(result, ERR_WRONG_INTERFACE_CALL);
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbilityInner
 * FunctionPoints: AbilityManagerService StartExtensionAbilityInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartExtensionAbilityInner_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t userId = 0;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::VPN;
    bool checkSystemCaller = true;
    bool isImplicit = true;
    bool isDlp = true;
    auto result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbilityInner
 * FunctionPoints: AbilityManagerService StartExtensionAbilityInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartExtensionAbilityInner_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    int32_t userId = 0;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::VPN;
    bool checkSystemCaller = true;
    bool isImplicit = true;
    bool isDlp = true;
    auto result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_INVALID_CALLER);
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbilityInner
 * FunctionPoints: AbilityManagerService StartExtensionAbilityInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartExtensionAbilityInner_003, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    int32_t userId = 0;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::VPN;
    bool checkSystemCaller = true;
    bool isImplicit = true;
    bool isDlp = false;
    auto result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_INVALID_CALLER);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveAbilityToBackground
 * FunctionPoints: AbilityManagerService MoveAbilityToBackground
 */
HWTEST_F(AbilityManagerServiceThirdTest, MoveAbilityToBackground_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<IRemoteObject> token = nullptr;
    auto result = abilityMs->MoveAbilityToBackground(token);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetLocalDeviceId
 * FunctionPoints: AbilityManagerService GetLocalDeviceId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetLocalDeviceId_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string localDeviceId = "device";
    auto result = abilityMs->GetLocalDeviceId(localDeviceId);
    EXPECT_EQ(result, false);
}

/*
 * Feature: AbilityManagerService
 * Function: AnonymizeDeviceId
 * FunctionPoints: AbilityManagerService AnonymizeDeviceId
 */
HWTEST_F(AbilityManagerServiceThirdTest, AnonymizeDeviceId_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string localDeviceId = "device";
    auto result = abilityMs->AnonymizeDeviceId(localDeviceId);
    EXPECT_NE(result, EMPTY_DEVICE_ID);
}

/*
 * Feature: AbilityManagerService
 * Function: AnonymizeDeviceId
 * FunctionPoints: AbilityManagerService AnonymizeDeviceId
 */
HWTEST_F(AbilityManagerServiceThirdTest, AnonymizeDeviceId_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string localDeviceId = "";
    auto result = abilityMs->AnonymizeDeviceId(localDeviceId);
    EXPECT_EQ(result, EMPTY_DEVICE_ID);
}

/*
 * Feature: AbilityManagerService
 * Function: TransferAbilityResultForExtension
 * FunctionPoints: AbilityManagerService TransferAbilityResultForExtension
 */
HWTEST_F(AbilityManagerServiceThirdTest, TransferAbilityResultForExtension_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    sptr<IRemoteObject> token = nullptr;
    int32_t resultCode = 0;
    AAFwk::Want want;
    int32_t res = abilityMs->TransferAbilityResultForExtension(token, resultCode, want);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}
}  // namespace AAFwk
}  // namespace OHOS
