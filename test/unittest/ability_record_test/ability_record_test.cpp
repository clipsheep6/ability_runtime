/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "ability_record.h"
#undef private
#undef protected

#include "ability_connect_callback_stub.h"
#include "ability_scheduler.h"
#include "connection_record.h"
#include "constants.h"
#include "mock_ability_connect_callback.h"
#include "mock_bundle_manager.h"
#include "parameters.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#ifdef SUPPORT_GRAPHICS
#define private public
#define protected public
#include "mission_info_mgr.h"
#undef private
#undef protected
#endif

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityBase::Constants;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string DEBUG_APP = "debugApp";
const std::string DLP_BUNDLE_NAME = "com.ohos.dlpmanager";
const std::string SHOW_ON_LOCK_SCREEN = "ShowOnLockScreen";
}
class AbilityRecordTest : public testing::TestWithParam<OHOS::AAFwk::AbilityState> {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRecord> GetAbilityRecord();

    std::shared_ptr<AbilityRecord> abilityRecord_{ nullptr };
    std::shared_ptr<AbilityResult> abilityResult_{ nullptr };
    std::shared_ptr<AbilityRequest> abilityRequest_{ nullptr };
    static constexpr unsigned int CHANGE_CONFIG_LOCALE = 0x00000001;
};

void AbilityRecordTest::SetUpTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
    OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->OnStart();
}
void AbilityRecordTest::TearDownTestCase(void)
{
    OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->OnStop();
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void AbilityRecordTest::SetUp(void)
{
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityResult_ = std::make_shared<AbilityResult>(-1, -1, want);
    abilityRequest_ = std::make_shared<AbilityRequest>();
    abilityRecord_->Init();
}

void AbilityRecordTest::TearDown(void)
{
    abilityRecord_.reset();
}

std::shared_ptr<AbilityRecord> AbilityRecordTest::GetAbilityRecord()
{
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    return std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
}

bool IsTestAbilityExist(const std::string& data)
{
    return std::string::npos != data.find("previous ability app name [NULL]");
}

bool IsTestAbilityExist1(const std::string& data)
{
    return std::string::npos != data.find("test_pre_app");
}

bool IsTestAbilityExist2(const std::string& data)
{
    return std::string::npos != data.find("test_next_app");
}

class MockWMSHandler : public IWindowManagerServiceHandler {
public:
    virtual void NotifyWindowTransition(sptr<AbilityTransitionInfo> fromInfo, sptr<AbilityTransitionInfo> toInfo)
    {}

    virtual int32_t GetFocusWindow(sptr<IRemoteObject>& abilityToken)
    {
        return 0;
    }

    virtual void StartingWindow(sptr<AbilityTransitionInfo> info,
        std::shared_ptr<Media::PixelMap> pixelMap, uint32_t bgColor) {}

    virtual void StartingWindow(sptr<AbilityTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap) {}

    virtual void CancelStartingWindow(sptr<IRemoteObject> abilityToken)
    {}

    virtual sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }
};

/*
 * Feature: AbilityRecord
 * Function: GetRecordId
 * SubFunction: GetRecordId
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through GetRecordId 1
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetRecordId, TestSize.Level1)
{
    EXPECT_EQ(abilityRecord_->GetRecordId(), 0);
}

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: LoadAbility Activate Inactivate MoveToBackground
 * EnvConditions: NA
 * CaseDescription: LoadAbility Activate Inactivate MoveToBackground UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_UpdateLifeState, TestSize.Level1)
{
    abilityRecord_->LoadAbility();
    EXPECT_EQ(abilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::INITIAL);
    abilityRecord_->Activate();
    EXPECT_EQ(abilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::ACTIVATING);
    abilityRecord_->Inactivate();
    EXPECT_EQ(abilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::INACTIVATING);
}

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: SetAbilityInfo GetAbilityInfo
 * EnvConditions: NA
 * CaseDescription: SetAbilityInfo GetAbilityInfo UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetGetAbilityInfo, TestSize.Level1)
{
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.applicationName = std::string("TestApp");
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_EQ(abilityRecord->GetAbilityInfo().applicationName, std::string("TestApp"));
}

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: SetApplicationInfo GetApplicationInfo
 * EnvConditions: NA
 * CaseDescription: SetApplicationInfo GetApplicationInfo UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetGetApplicationInfo, TestSize.Level1)
{
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.name = "TestApp";
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_EQ(abilityRecord->GetApplicationInfo().name, "TestApp");
}

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: SetAbilityState GetAbilityState
 * EnvConditions: NA
 * CaseDescription: SetAbilityState GetAbilityState UT.
 */
HWTEST_P(AbilityRecordTest, AaFwk_AbilityMS_SetGetAbilityState, TestSize.Level1)
{
    OHOS::AAFwk::AbilityState state = GetParam();
    abilityRecord_->SetAbilityState(state);
    EXPECT_EQ(static_cast<int>(state), static_cast<int>(abilityRecord_->GetAbilityState()));
}
INSTANTIATE_TEST_SUITE_P(AbilityRecordTestCaseP, AbilityRecordTest,
    testing::Values(AbilityState::INITIAL, AbilityState::INACTIVE, AbilityState::ACTIVE, AbilityState::INACTIVATING,
        AbilityState::ACTIVATING, AbilityState::TERMINATING, AbilityState::FOREGROUND,
        AbilityState::BACKGROUND, AbilityState::FOREGROUNDING, AbilityState::BACKGROUNDING,
        AbilityState::FOREGROUND_FAILED, AbilityState::FOREGROUND_INVALID_MODE));

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: SetAbilityState GetAbilityState
 * EnvConditions: NA
 * CaseDescription: SetAbilityState GetAbilityState UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetGetToken, TestSize.Level1)
{
    EXPECT_EQ(Token::GetAbilityRecordByToken(abilityRecord_->GetToken()).get(), abilityRecord_.get());
}

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: SetPreAbilityRecord SetNextAbilityRecord GetPreAbilityRecord GetNextAbilityRecord
 * EnvConditions: NA
 * CaseDescription: SetPreAbilityRecord SetNextAbilityRecord GetPreAbilityRecord GetNextAbilityRecord UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetGetPreNextAbilityReocrd, TestSize.Level1)
{
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    std::shared_ptr<AbilityRecord> preAbilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<AbilityRecord> nextAbilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->SetPreAbilityRecord(preAbilityRecord);
    abilityRecord_->SetNextAbilityRecord(nextAbilityRecord);
    EXPECT_EQ(abilityRecord_->GetPreAbilityRecord().get(), preAbilityRecord.get());
    EXPECT_EQ(abilityRecord_->GetNextAbilityRecord().get(), nextAbilityRecord.get());
}

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: SetEventId GetEventId
 * EnvConditions: NA
 * CaseDescription: SetEventId GetEventId UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetGetEventId, TestSize.Level1)
{
    int64_t eventId = 1;
    abilityRecord_->SetEventId(eventId);
    EXPECT_EQ(eventId, abilityRecord_->GetEventId());
}

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: IsReady
 * EnvConditions: NA
 * CaseDescription: IsReady UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsReady, TestSize.Level1)
{
    EXPECT_EQ(false, abilityRecord_->IsReady());
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    abilityRecord_->SetScheduler(scheduler);
    EXPECT_EQ(true, abilityRecord_->IsReady());
}

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: IsLauncherAbility
 * EnvConditions: NA
 * CaseDescription: IsLauncherAbility UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsLauncherAbility, TestSize.Level1)
{
    EXPECT_EQ(false, abilityRecord_->IsLauncherAbility());
    Want launcherWant;
    launcherWant.AddEntity(Want::ENTITY_HOME);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    std::unique_ptr<AbilityRecord> launcherAbilityRecord =
        std::make_unique<AbilityRecord>(launcherWant, abilityInfo, applicationInfo);
    launcherAbilityRecord->Init();
    EXPECT_EQ(false, launcherAbilityRecord->IsLauncherAbility());
}

/*
 * Feature: AbilityRecord
 * Function: Add connection record to ability record' list
 * SubFunction: NA
 * FunctionPoints: AddConnectRecordToList
 * EnvConditions: NA
 * CaseDescription: AddConnectRecordToList UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_AddConnectRecordToList, TestSize.Level1)
{
    // test1 for input param is null
    abilityRecord_->AddConnectRecordToList(nullptr);
    auto connList = abilityRecord_->GetConnectRecordList();
    EXPECT_EQ(0, static_cast<int>(connList.size()));

    // test2 for adding new connection record to empty list
    OHOS::sptr<IAbilityConnection> callback1 = new AbilityConnectCallback();
    auto newConnRecord1 =
        ConnectionRecord::CreateConnectionRecord(abilityRecord_->GetToken(), abilityRecord_, callback1);
    abilityRecord_->AddConnectRecordToList(newConnRecord1);
    connList = abilityRecord_->GetConnectRecordList();
    EXPECT_EQ(1, static_cast<int>(connList.size()));

    // test3 for adding new connection record to non-empty list
    OHOS::sptr<IAbilityConnection> callback2 = new AbilityConnectCallback();
    auto newConnRecord2 =
        ConnectionRecord::CreateConnectionRecord(abilityRecord_->GetToken(), abilityRecord_, callback2);
    abilityRecord_->AddConnectRecordToList(newConnRecord2);
    connList = abilityRecord_->GetConnectRecordList();
    EXPECT_EQ(2, static_cast<int>(connList.size()));

    // test4 for adding old connection record to non-empty list
    abilityRecord_->AddConnectRecordToList(newConnRecord2);
    connList = abilityRecord_->GetConnectRecordList();
    EXPECT_EQ(2, static_cast<int>(connList.size()));

    // test5 for delete nullptr from list
    abilityRecord_->RemoveConnectRecordFromList(nullptr);
    connList = abilityRecord_->GetConnectRecordList();
    EXPECT_EQ(2, static_cast<int>(connList.size()));

    // test6 for delete no-match member from list
    auto newConnRecord3 =
        ConnectionRecord::CreateConnectionRecord(abilityRecord_->GetToken(), abilityRecord_, callback2);
    abilityRecord_->RemoveConnectRecordFromList(newConnRecord3);
    connList = abilityRecord_->GetConnectRecordList();
    EXPECT_EQ(2, static_cast<int>(connList.size()));

    // test7 for delete match member from list
    abilityRecord_->RemoveConnectRecordFromList(newConnRecord2);
    connList = abilityRecord_->GetConnectRecordList();
    EXPECT_EQ(1, static_cast<int>(connList.size()));

    // test8 for get ability unknown type
    EXPECT_EQ(OHOS::AppExecFwk::AbilityType::UNKNOWN, abilityRecord_->GetAbilityInfo().type);
}

/*
 * Feature: AbilityRecord
 * Function: ConvertAbilityState
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify ConvertAbilityState convert success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ConvertAbilityState, TestSize.Level1)
{
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::INITIAL);
    EXPECT_EQ(abilityRecord_->ConvertAbilityState(abilityRecord_->GetAbilityState()), "INITIAL");
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVE);
    EXPECT_EQ(abilityRecord_->ConvertAbilityState(abilityRecord_->GetAbilityState()), "INACTIVE");
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    EXPECT_EQ(abilityRecord_->ConvertAbilityState(abilityRecord_->GetAbilityState()), "ACTIVE");
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVATING);
    EXPECT_EQ(abilityRecord_->ConvertAbilityState(abilityRecord_->GetAbilityState()), "INACTIVATING");
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVATING);
    EXPECT_EQ(abilityRecord_->ConvertAbilityState(abilityRecord_->GetAbilityState()), "ACTIVATING");
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::TERMINATING);
    EXPECT_EQ(abilityRecord_->ConvertAbilityState(abilityRecord_->GetAbilityState()), "TERMINATING");
}

/*
 * Feature: AbilityRecord
 * Function: IsTerminating
 * SubFunction: IsTerminating SetTerminatingState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsTerminating SetTerminatingState success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsTerminating, TestSize.Level1)
{
    abilityRecord_->SetTerminatingState();
    EXPECT_EQ(abilityRecord_->IsTerminating(), true);
}

/*
 * Feature: AbilityRecord
 * Function: Activate
 * SubFunction: Activate
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify lifecycleDeal_ is nullptr cause Activate is not call
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Activate, TestSize.Level1)
{
    abilityRecord_->lifecycleDeal_ = nullptr;
    abilityRecord_->currentState_ = OHOS::AAFwk::AbilityState::INITIAL;
    abilityRecord_->Activate();
    EXPECT_EQ(abilityRecord_->currentState_, OHOS::AAFwk::AbilityState::INITIAL);
    abilityRecord_->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord_->Activate();
    EXPECT_EQ(abilityRecord_->currentState_, OHOS::AAFwk::AbilityState::ACTIVATING);
}

/*
 * Feature: AbilityRecord
 * Function: Inactivate
 * SubFunction: Inactivate
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify lifecycleDeal_ is nullptr cause Inactivate is not call
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Inactivate, TestSize.Level1)
{
    abilityRecord_->lifecycleDeal_ = nullptr;
    abilityRecord_->currentState_ = OHOS::AAFwk::AbilityState::INITIAL;
    abilityRecord_->Inactivate();
    EXPECT_EQ(abilityRecord_->currentState_, OHOS::AAFwk::AbilityState::INITIAL);
    abilityRecord_->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord_->Inactivate();
    EXPECT_EQ(abilityRecord_->currentState_, OHOS::AAFwk::AbilityState::INACTIVATING);
}

/*
 * Feature: AbilityRecord
 * Function: Terminate
 * SubFunction: Terminate
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify lifecycleDeal_ is nullptr cause Terminate is not call
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Terminate, TestSize.Level1)
{
    abilityRecord_->lifecycleDeal_ = nullptr;
    abilityRecord_->currentState_ = OHOS::AAFwk::AbilityState::INITIAL;
    abilityRecord_->Terminate([]() {

        });
    EXPECT_EQ(abilityRecord_->currentState_, OHOS::AAFwk::AbilityState::INITIAL);
    abilityRecord_->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord_->Terminate([]() {

        });
    EXPECT_EQ(abilityRecord_->currentState_, OHOS::AAFwk::AbilityState::TERMINATING);
}

/*
 * Feature: AbilityRecord
 * Function: SetScheduler
 * SubFunction: SetScheduler
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetScheduler success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetScheduler, TestSize.Level1)
{
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    abilityRecord_->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    EXPECT_EQ(false, abilityRecord_->IsReady());
    abilityRecord_->SetScheduler(scheduler);
    EXPECT_EQ(true, abilityRecord_->IsReady());
}

/*
 * Feature: Token
 * Function: GetAbilityRecordByToken
 * SubFunction: GetAbilityRecordByToken
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord token GetAbilityRecordByToken success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetAbilityRecordByToken, TestSize.Level1)
{
    EXPECT_EQ(Token::GetAbilityRecordByToken(abilityRecord_->GetToken()).get(), abilityRecord_.get());
    EXPECT_EQ(abilityRecord_->GetToken()->GetAbilityRecord(), abilityRecord_);
}

/*
 * Feature: AbilityRecord
 * Function: Dump
 * SubFunction: Dump
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify Dump success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Dump, TestSize.Level1)
{
    std::vector<std::string> info;
    info.push_back(std::string("0"));
    abilityRecord_->Dump(info);
    EXPECT_EQ(std::find_if(info.begin(), info.end(), IsTestAbilityExist) != info.end(), true);
    Want wantPre;
    std::string entity = Want::ENTITY_HOME;
    wantPre.AddEntity(entity);

    std::string testAppName = "test_pre_app";
    OHOS::AppExecFwk::AbilityInfo abilityInfoPre;
    abilityInfoPre.applicationName = testAppName;
    OHOS::AppExecFwk::ApplicationInfo appinfoPre;
    appinfoPre.name = testAppName;

    auto preAbilityRecord = std::make_shared<AbilityRecord>(wantPre, abilityInfoPre, appinfoPre);
    abilityRecord_->SetPreAbilityRecord(nullptr);
    abilityRecord_->Dump(info);
    abilityRecord_->SetPreAbilityRecord(preAbilityRecord);
    abilityRecord_->Dump(info);

    Want wantNext;
    std::string entityNext = Want::ENTITY_HOME;
    wantNext.AddEntity(entityNext);
    std::string testAppNameNext = "test_next_app";
    OHOS::AppExecFwk::AbilityInfo abilityInfoNext;
    abilityInfoNext.applicationName = testAppNameNext;
    OHOS::AppExecFwk::ApplicationInfo appinfoNext;
    appinfoNext.name = testAppNameNext;
    auto nextAbilityRecord = std::make_shared<AbilityRecord>(wantNext, abilityInfoNext, appinfoNext);
    abilityRecord_->SetNextAbilityRecord(nullptr);
    abilityRecord_->Dump(info);
    abilityRecord_->SetNextAbilityRecord(nextAbilityRecord);
    abilityRecord_->Dump(info);
}  // namespace AAFwk

/*
 * Feature: AbilityRecord
 * Function: SetWant GetWant
 * SubFunction: SetWant GetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SetWant GetWant can get,set success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Want, TestSize.Level1)
{
    Want want;
    want.SetFlags(100);
    abilityRecord_->SetWant(want);
    EXPECT_EQ(want.GetFlags(), abilityRecord_->GetWant().GetFlags());
}

/*
 * Feature: AbilityRecord
 * Function: GetRequestCode
 * SubFunction: GetRequestCode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetRequestCode success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetRequestCode, TestSize.Level1)
{
    EXPECT_EQ(abilityRecord_->GetRequestCode(), -1);
}

/*
 * Feature: AbilityRecord
 * Function: GetAbilityTypeString
 * SubFunction: GetAbilityTypeString
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityTypeString can get success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetAbilityTypeString, TestSize.Level1)
{
    std::string typeStr;
    std::shared_ptr<AbilityRecord> recordUn;
    OHOS::AppExecFwk::AbilityInfo ability;
    OHOS::AppExecFwk::ApplicationInfo appInfo;
    Want wantUn;
    recordUn = std::make_shared<AbilityRecord>(wantUn, ability, appInfo);
    recordUn->GetAbilityTypeString(typeStr);
    EXPECT_EQ(typeStr, "UNKNOWN");

    std::shared_ptr<AbilityRecord> recordService;
    OHOS::AppExecFwk::AbilityInfo abilityService;
    abilityService.type = OHOS::AppExecFwk::AbilityType::SERVICE;
    OHOS::AppExecFwk::ApplicationInfo appInfoService;
    Want wantService;
    recordService = std::make_shared<AbilityRecord>(wantService, abilityService, appInfoService);
    recordService->GetAbilityTypeString(typeStr);
    EXPECT_EQ(typeStr, "SERVICE");

    std::shared_ptr<AbilityRecord> recordPage;
    OHOS::AppExecFwk::AbilityInfo abilityPage;
    abilityPage.type = OHOS::AppExecFwk::AbilityType::PAGE;
    OHOS::AppExecFwk::ApplicationInfo appInfoPage;
    Want wantPage;
    recordPage = std::make_shared<AbilityRecord>(wantPage, abilityPage, appInfoPage);
    recordPage->GetAbilityTypeString(typeStr);
    EXPECT_EQ(typeStr, "PAGE");

    std::shared_ptr<AbilityRecord> recordData;
    OHOS::AppExecFwk::AbilityInfo abilityData;
    abilityData.type = OHOS::AppExecFwk::AbilityType::DATA;
    OHOS::AppExecFwk::ApplicationInfo appInfoData;
    Want wantData;
    recordData = std::make_shared<AbilityRecord>(wantData, abilityData, appInfoData);
    recordData->GetAbilityTypeString(typeStr);
    EXPECT_EQ(typeStr, "DATA");
}

/*
 * Feature: AbilityRecord
 * Function: SetResult GetResult
 * SubFunction: SetResult GetResult
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SetResult GetResult can get,set success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Result, TestSize.Level1)
{
    abilityResult_->requestCode_ = 10;
    abilityRecord_->SetResult(abilityResult_);
    EXPECT_EQ(10, abilityRecord_->GetResult()->requestCode_);
}

/*
 * Feature: AbilityRecord
 * Function: SendResult
 * SubFunction: SendResult
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SendResult scheduler is nullptr
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SendResult, TestSize.Level1)
{
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    abilityRecord_->SetScheduler(scheduler);
    abilityRecord_->SetResult(abilityResult_);
    abilityRecord_->SendResult();
    EXPECT_EQ(nullptr, abilityRecord_->GetResult());
}

/*
 * Feature: AbilityRecord
 * Function: SetConnRemoteObject GetConnRemoteObject
 * SubFunction: SetConnRemoteObject GetConnRemoteObject
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SetConnRemoteObject GetConnRemoteObject UT
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ConnRemoteObject, TestSize.Level1)
{
    OHOS::sptr<OHOS::IRemoteObject> remote;
    abilityRecord_->SetConnRemoteObject(remote);
    EXPECT_EQ(remote, abilityRecord_->GetConnRemoteObject());
}

/*
 * Feature: AbilityRecord
 * Function: IsCreateByConnect SetCreateByConnectMode
 * SubFunction: IsCreateByConnect SetCreateByConnectMode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsCreateByConnect SetCreateByConnectMode UT
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateByConnect, TestSize.Level1)
{
    abilityRecord_->SetCreateByConnectMode();
    EXPECT_EQ(true, abilityRecord_->IsCreateByConnect());
}

/*
 * Feature: AbilityRecord
 * Function: IsActiveState
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsActiveState_001, TestSize.Level1)
{
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::TERMINATING);
    EXPECT_EQ(false, abilityRecord_->IsActiveState());

    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    EXPECT_EQ(true, abilityRecord_->IsActiveState());
}

/*
 * Feature: AbilityRecord
 * Function: SetAbilityState
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetAbilityState_001, TestSize.Level1)
{
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::TERMINATING);
    auto state = abilityRecord_->GetAbilityState();
    EXPECT_EQ(state, OHOS::AAFwk::AbilityState::TERMINATING);

    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    state = abilityRecord_->GetAbilityState();
    EXPECT_EQ(state, OHOS::AAFwk::AbilityState::ACTIVE);
}

/*
 * Feature: AbilityRecord
 * Function: SetSpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetSpecifiedFlag_001, TestSize.Level1)
{
    abilityRecord_->SetSpecifiedFlag("flag");
    auto flag = abilityRecord_->GetSpecifiedFlag();
    EXPECT_EQ(flag, "flag");
}

/*
 * Feature: AbilityRecord
 * Function: GetAbilityRecordByToken
 * SubFunction: GetAbilityRecordByToken
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord token GetAbilityRecordByToken
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetAbilityRecordByToken_001, TestSize.Level1)
{
    EXPECT_EQ(Token::GetAbilityRecordByToken(nullptr), nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityRecord
 * SubFunction: CreateAbilityRecord
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityRecord
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityRecord_001, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityStartSetting> abilityStartSetting = std::make_shared<AbilityStartSetting>();
    abilityRequest.startSetting = abilityStartSetting;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    auto res = abilityRecord_->CreateAbilityRecord(abilityRequest);
    EXPECT_NE(res, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: LoadAbility
 * SubFunction: LoadAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord LoadAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_LoadAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->applicationInfo_.name = "app";
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->isRestarting_ = true;
    abilityRecord->isLauncherAbility_ = true;
    abilityRecord->restartCount_ = 0;
    int res = abilityRecord->LoadAbility();
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityRecord
 * Function: LoadAbility
 * SubFunction: LoadAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord LoadAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_LoadAbility_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, abilityRecord);
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->applicationInfo_.name = "app";
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->isRestarting_ = true;
    abilityRecord->isLauncherAbility_ = false;
    abilityRecord->callerList_.push_back(caller);
    int res = abilityRecord->LoadAbility();
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityRecord
 * Function: LoadAbility
 * SubFunction: LoadAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord LoadAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_LoadAbility_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, nullptr);
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->applicationInfo_.name = "app";
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->isRestarting_ = true;
    abilityRecord->isLauncherAbility_ = true;
    abilityRecord->restartCount_ = 1;
    abilityRecord->callerList_.push_back(caller);
    int res = abilityRecord->LoadAbility();
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityRecord
 * Function: LoadAbility
 * SubFunction: LoadAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord LoadAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_LoadAbility_004, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->applicationInfo_.name = "app";
    abilityRecord->isLauncherRoot_ = false;
    abilityRecord->callerList_.push_back(nullptr);
    int res = abilityRecord->LoadAbility();
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityRecord
 * Function: ForegroundAbility
 * SubFunction: ForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ForegroundAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t sceneFlag = 0;
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->SetIsNewWant(true);
    abilityRecord->SetPreAbilityRecord(abilityRecord_);
    abilityRecord->ForegroundAbility(sceneFlag);
}

/*
 * Feature: AbilityRecord
 * Function: ForegroundAbility
 * SubFunction: ForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ForegroundAbility_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t sceneFlag = 0;
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->SetIsNewWant(true);
    abilityRecord->SetPreAbilityRecord(nullptr);
    abilityRecord->ForegroundAbility(sceneFlag);
}

/*
 * Feature: AbilityRecord
 * Function: ForegroundAbility
 * SubFunction: ForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ForegroundAbility_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t sceneFlag = 0;
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->SetIsNewWant(false);
    abilityRecord->ForegroundAbility(sceneFlag);
}

/*
 * Feature: AbilityRecord
 * Function: ProcessForegroundAbility
 * SubFunction: ProcessForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ProcessForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ProcessForegroundAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t sceneFlag = 0;
    abilityRecord->isReady_ = true;
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    abilityRecord->ProcessForegroundAbility(sceneFlag);
}

/*
 * Feature: AbilityRecord
 * Function: ProcessForegroundAbility
 * SubFunction: ProcessForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ProcessForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ProcessForegroundAbility_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t sceneFlag = 0;
    abilityRecord->isReady_ = true;
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    abilityRecord->ProcessForegroundAbility(sceneFlag);
}

/*
 * Feature: AbilityRecord
 * Function: ProcessForegroundAbility
 * SubFunction: ProcessForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ProcessForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ProcessForegroundAbility_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t sceneFlag = 0;
    abilityRecord->isReady_ = false;
    abilityRecord->ProcessForegroundAbility(sceneFlag);
}

/*
 * Feature: AbilityRecord
 * Function: GetLabel
 * SubFunction: GetLabel
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetLabel
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetLabel_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->applicationInfo_.label = "label";
    abilityRecord->abilityInfo_.resourcePath = "resource";
    std::string res = abilityRecord->GetLabel();
    EXPECT_EQ(res, "label");
}

#ifdef SUPPORT_GRAPHICS
/*
 * Feature: AbilityRecord
 * Function: ProcessForegroundAbility
 * SubFunction: ProcessForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ProcessForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ProcessForegroundAbility_004, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<AbilityRecord> callerAbility = GetAbilityRecord();
    uint32_t sceneFlag = 0;
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    abilityRecord->ProcessForegroundAbility(callerAbility, sceneFlag);
}

/*
 * Feature: AbilityRecord
 * Function: ProcessForegroundAbility
 * SubFunction: ProcessForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ProcessForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ProcessForegroundAbility_005, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<AbilityRecord> callerAbility = GetAbilityRecord();
    sptr<IWindowManagerServiceHandler> wmsHandler = new MockWMSHandler();
    uint32_t sceneFlag = 0;
    DelayedSingleton<AbilityManagerService>::GetInstance()->wmsHandler_ = wmsHandler;
    EXPECT_EQ(DelayedSingleton<AbilityManagerService>::GetInstance()->GetWMSHandler(), wmsHandler);
    abilityRecord->want_.SetParam(DEBUG_APP, true);
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    abilityRecord->ProcessForegroundAbility(callerAbility, sceneFlag);
}

/*
 * Feature: AbilityRecord
 * Function: NotifyAnimationFromTerminatingAbility
 * SubFunction: NotifyAnimationFromTerminatingAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord NotifyAnimationFromTerminatingAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_NotifyAnimationFromTerminatingAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    bool flag = false;
    sptr<IWindowManagerServiceHandler> wmsHandler = new MockWMSHandler();
    DelayedSingleton<AbilityManagerService>::GetInstance()->wmsHandler_ = wmsHandler;
    abilityRecord->NotifyAnimationFromTerminatingAbility(nullptr, flag);
}

/*
 * Feature: AbilityRecord
 * Function: NotifyAnimationFromTerminatingAbility
 * SubFunction: NotifyAnimationFromTerminatingAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord NotifyAnimationFromTerminatingAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_NotifyAnimationFromTerminatingAbility_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<IWindowManagerServiceHandler> wmsHandler = new MockWMSHandler();
    DelayedSingleton<AbilityManagerService>::GetInstance()->wmsHandler_ = wmsHandler;
    abilityRecord->NotifyAnimationFromTerminatingAbility();
}

/*
 * Feature: AbilityRecord
 * Function: NotifyAnimationFromTerminatingAbility
 * SubFunction: NotifyAnimationFromTerminatingAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord NotifyAnimationFromTerminatingAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_NotifyAnimationFromTerminatingAbility_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    DelayedSingleton<AbilityManagerService>::GetInstance()->wmsHandler_ = nullptr;
    abilityRecord->NotifyAnimationFromTerminatingAbility();
}

/*
 * Feature: AbilityRecord
 * Function: ProcessForegroundAbility
 * SubFunction: ProcessForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ProcessForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ProcessForegroundAbility_006, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    bool isRecent = false;
    AbilityRequest abilityRequest;
    std::shared_ptr<StartOptions> startOptions = nullptr ;
    std::shared_ptr<AbilityRecord> callerAbility;
    uint32_t sceneFlag = 1;
    abilityRecord->isReady_ = true;
    DelayedSingleton<AbilityManagerService>::GetInstance()->handler_ = nullptr;
    abilityRecord->ProcessForegroundAbility(isRecent, abilityRequest, startOptions, callerAbility, sceneFlag);
}

/*
 * Feature: AbilityRecord
 * Function: ProcessForegroundAbility
 * SubFunction: ProcessForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ProcessForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ProcessForegroundAbility_007, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    bool isRecent = false;
    AbilityRequest abilityRequest;
    std::shared_ptr<StartOptions> startOptions = nullptr ;
    std::shared_ptr<AbilityRecord> callerAbility;
    uint32_t sceneFlag = 1;
    DelayedSingleton<AbilityManagerService>::GetInstance()->Init();
    abilityRecord->isReady_ = true;
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    abilityRecord->ProcessForegroundAbility(isRecent, abilityRequest, startOptions, callerAbility, sceneFlag);
    isRecent = true;
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    abilityRecord->ProcessForegroundAbility(isRecent, abilityRequest, startOptions, callerAbility, sceneFlag);
}

/*
 * Feature: AbilityRecord
 * Function: ProcessForegroundAbility
 * SubFunction: ProcessForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ProcessForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ProcessForegroundAbility_008, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    bool isRecent = false;
    AbilityRequest abilityRequest;
    std::shared_ptr<StartOptions> startOptions = nullptr ;
    std::shared_ptr<AbilityRecord> callerAbility;
    uint32_t sceneFlag = 1;
    abilityRecord->isReady_ = false;
    abilityRecord->ProcessForegroundAbility(isRecent, abilityRequest, startOptions, callerAbility, sceneFlag);
}

/*
 * Feature: AbilityRecord
 * Function: GetWantFromMission
 * SubFunction: GetWantFromMission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetWantFromMission
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetWantFromMission_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    auto res = abilityRecord->GetWantFromMission();
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: GetWantFromMission
 * SubFunction: GetWantFromMission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetWantFromMission
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetWantFromMission_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int32_t missionId = 1;
    InnerMissionInfo innerMissionInfo;
    Want want;
    innerMissionInfo.missionInfo.id = missionId;
    innerMissionInfo.missionInfo.want = want;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_[missionId] = true;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.push_back(innerMissionInfo);
    abilityRecord->missionId_ = 1;
    auto res = abilityRecord->GetWantFromMission();
    EXPECT_NE(res, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: AnimationTask
 * SubFunction: AnimationTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord AnimationTask
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_AnimationTask_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    bool isRecent = true;
    AbilityRequest abilityRequest;
    std::shared_ptr<StartOptions> startOptions = nullptr ;
    std::shared_ptr<AbilityRecord> callerAbility;
    abilityRecord->AnimationTask(isRecent, abilityRequest, startOptions, callerAbility);
}

/*
 * Feature: AbilityRecord
 * Function: SetShowWhenLocked
 * SubFunction: SetShowWhenLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetShowWhenLocked
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetShowWhenLocked_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    AbilityInfo abilityInfo;
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    CustomizeData data1;
    CustomizeData data2;
    data1.name = SHOW_ON_LOCK_SCREEN;
    data2.name = "";
    abilityInfo.metaData.customizeData.push_back(data1);
    abilityInfo.metaData.customizeData.push_back(data2);
    info->isShowWhenLocked_ = false;
    abilityRecord->SetShowWhenLocked(abilityInfo, info);
}

/*
 * Feature: AbilityRecord
 * Function: NotifyAnimationFromRecentTask
 * SubFunction: NotifyAnimationFromRecentTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord NotifyAnimationFromRecentTask
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_NotifyAnimationFromRecentTask_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<StartOptions> startOptions = std::make_shared<StartOptions>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    DelayedSingleton<AbilityManagerService>::GetInstance()->handler_ = nullptr;
    abilityRecord->NotifyAnimationFromRecentTask(startOptions, want);
}

/*
 * Feature: AbilityRecord
 * Function: NotifyAnimationFromStartingAbility
 * SubFunction: NotifyAnimationFromStartingAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord NotifyAnimationFromStartingAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_NotifyAnimationFromStartingAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<AbilityRecord> callerAbility = GetAbilityRecord();
    AbilityRequest abilityRequest;
    DelayedSingleton<AbilityManagerService>::GetInstance()->Init();
    abilityRecord->NotifyAnimationFromStartingAbility(callerAbility, abilityRequest);
}

/*
 * Feature: AbilityRecord
 * Function: NotifyAnimationFromStartingAbility
 * SubFunction: NotifyAnimationFromStartingAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord NotifyAnimationFromStartingAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_NotifyAnimationFromStartingAbility_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<AbilityRecord> callerAbility = nullptr;
    AbilityRequest abilityRequest;
    DelayedSingleton<AbilityManagerService>::GetInstance()->Init();
    abilityRecord->NotifyAnimationFromStartingAbility(callerAbility, abilityRequest);
}

/*
 * Feature: AbilityRecord
 * Function: StartingWindowTask
 * SubFunction: StartingWindowTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord StartingWindowTask
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_StartingWindowTask_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    bool isRecent = true;
    AbilityRequest abilityRequest;
    std::shared_ptr<StartOptions> startOptions = std::make_shared<StartOptions>();
    DelayedSingleton<AbilityManagerService>::GetInstance()->Init();
    abilityRecord->StartingWindowTask(isRecent, true, abilityRequest, startOptions);
    abilityRecord->StartingWindowTask(isRecent, false, abilityRequest, startOptions);
}

/*
 * Feature: AbilityRecord
 * Function: SetWindowModeAndDisplayId
 * SubFunction: SetWindowModeAndDisplayId
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWindowModeAndDisplayId
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetWindowModeAndDisplayId_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<AbilityTransitionInfo> info = nullptr;
    std::shared_ptr<Want> want = nullptr;
    abilityRecord->SetWindowModeAndDisplayId(info, want);
}

/*
 * Feature: AbilityRecord
 * Function: SetWindowModeAndDisplayId
 * SubFunction: SetWindowModeAndDisplayId
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWindowModeAndDisplayId
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetWindowModeAndDisplayId_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    want->SetParam(Want::PARAM_RESV_WINDOW_MODE, 1);
    want->SetParam(Want::PARAM_RESV_DISPLAY_ID, 1);
    abilityRecord->SetWindowModeAndDisplayId(info, want);
}

/*
 * Feature: AbilityRecord
 * Function: SetWindowModeAndDisplayId
 * SubFunction: SetWindowModeAndDisplayId
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWindowModeAndDisplayId
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetWindowModeAndDisplayId_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    want->SetParam(Want::PARAM_RESV_WINDOW_MODE, -1);
    want->SetParam(Want::PARAM_RESV_DISPLAY_ID, -1);
    abilityRecord->SetWindowModeAndDisplayId(info, want);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityTransitionInfo
 * SubFunction: CreateAbilityTransitionInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityTransitionInfo
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityTransitionInfo_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<StartOptions> startOptions = nullptr;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    abilityRecord->CreateAbilityTransitionInfo(startOptions, want);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityTransitionInfo
 * SubFunction: CreateAbilityTransitionInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityTransitionInfo
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityTransitionInfo_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<StartOptions> startOptions = std::make_shared<StartOptions>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    startOptions->SetWindowMode(1);
    startOptions->SetDisplayID(1);
    abilityRecord->CreateAbilityTransitionInfo(startOptions, want);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityTransitionInfo
 * SubFunction: CreateAbilityTransitionInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityTransitionInfo
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityTransitionInfo_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    AbilityRequest abilityRequest;
    abilityRequest.startSetting = nullptr;
    abilityRecord->CreateAbilityTransitionInfo(abilityRequest);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityTransitionInfo
 * SubFunction: CreateAbilityTransitionInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityTransitionInfo
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityTransitionInfo_004, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityStartSetting> startSetting = std::make_shared<AbilityStartSetting>();
    startSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY, "windowMode");
    startSetting->AddProperty(AbilityStartSetting::WINDOW_DISPLAY_ID_KEY, "displayId");
    abilityRequest.startSetting = startSetting;
    abilityRecord->CreateAbilityTransitionInfo(abilityRequest);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityTransitionInfo
 * SubFunction: CreateAbilityTransitionInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityTransitionInfo
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityTransitionInfo_005, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<StartOptions> startOptions = nullptr;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityStartSetting> startSetting = std::make_shared<AbilityStartSetting>();
    startSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY, "windowMode");
    startSetting->AddProperty(AbilityStartSetting::WINDOW_DISPLAY_ID_KEY, "displayId");
    abilityRequest.startSetting = startSetting;
    abilityRecord->CreateAbilityTransitionInfo(startOptions, want, abilityRequest);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityTransitionInfo
 * SubFunction: CreateAbilityTransitionInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityTransitionInfo
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityTransitionInfo_006, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<StartOptions> startOptions = std::make_shared<StartOptions>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    AbilityRequest abilityRequest;
    abilityRecord->CreateAbilityTransitionInfo(startOptions, want, abilityRequest);
}

/*
 * Feature: AbilityRecord
 * Function: CreateResourceManager
 * SubFunction: CreateResourceManager
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateResourceManager
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateResourceManager_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    system::SetParameter(COMPRESS_PROPERTY, "1");
    abilityRecord->abilityInfo_.hapPath = "path";
    auto res = abilityRecord->CreateResourceManager();
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: GetPixelMap
 * SubFunction: GetPixelMap
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetPixelMap
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetPixelMap_001, TestSize.Level1)
{
    EXPECT_EQ(abilityRecord_->GetPixelMap(1, nullptr), nullptr);
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr(Global::Resource::CreateResourceManager());
    EXPECT_EQ(abilityRecord_->GetPixelMap(1, resourceMgr), nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: GetPixelMap
 * SubFunction: GetPixelMap
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetPixelMap
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetPixelMap_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr(Global::Resource::CreateResourceManager());
    system::SetParameter(COMPRESS_PROPERTY, "1");
    abilityRecord->abilityInfo_.hapPath = "path";
    auto res = abilityRecord->GetPixelMap(1, resourceMgr);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: StartingWindowHot
 * SubFunction: StartingWindowHot
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord StartingWindowHot
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_StartingWindowHot_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<StartOptions> startOptions = std::make_shared<StartOptions>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    AbilityRequest abilityRequest;
    DelayedSingleton<AbilityManagerService>::GetInstance()->Init();
    abilityRecord->StartingWindowHot(startOptions, want, abilityRequest);
}

/*
 * Feature: AbilityRecord
 * Function: GetColdStartingWindowResource
 * SubFunction: GetColdStartingWindowResource
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetColdStartingWindowResource
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetColdStartingWindowResource_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<Media::PixelMap> bg;
    uint32_t bgColor = 0;
    abilityRecord->startingWindowBg_ = std::make_shared<Media::PixelMap>();
    abilityRecord->GetColdStartingWindowResource(bg, bgColor);
    abilityRecord->startingWindowBg_ = nullptr;
    abilityRecord->GetColdStartingWindowResource(bg, bgColor);
}

/*
 * Feature: AbilityRecord
 * Function: InitColdStartingWindowResource
 * SubFunction: InitColdStartingWindowResource
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord InitColdStartingWindowResource
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_InitColdStartingWindowResource_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr(Global::Resource::CreateResourceManager());
    abilityRecord->InitColdStartingWindowResource(nullptr);
    abilityRecord->InitColdStartingWindowResource(resourceMgr);
}
#endif

/*
 * Feature: AbilityRecord
 * Function: SetPendingState
 * SubFunction: SetPendingState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: set AbilityRecord pending state
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetPendingState_001, TestSize.Level1)
{
    abilityRecord_->SetPendingState(OHOS::AAFwk::AbilityState::FOREGROUND);
    EXPECT_EQ(abilityRecord_->GetPendingState(), OHOS::AAFwk::AbilityState::FOREGROUND);
}

#ifdef SUPPORT_GRAPHICS
/*
 * Feature: AbilityRecord
 * Function: PostCancelStartingWindowHotTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: PostCancelStartingWindowHotTask
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_PostCancelStartingWindowHotTask_001, TestSize.Level1)
{
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    AbilityRecord abilityRecord(want, abilityInfo, applicationInfo, 0);
    want.SetParam("debugApp", true);
    abilityRecord.SetWant(want);
    abilityRecord.PostCancelStartingWindowHotTask();
    EXPECT_TRUE(want.GetBoolParam("debugApp", false));

    want.SetParam("debugApp", false);
    abilityRecord.PostCancelStartingWindowHotTask();
    EXPECT_FALSE(want.GetBoolParam("debugApp", false));
}

/*
 * Feature: AbilityRecord
 * Function: PostCancelStartingWindowColdTask
 * SubFunction: PostCancelStartingWindowColdTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: PostCancelStartingWindowColdTask
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_PostCancelStartingWindowColdTask_001, TestSize.Level1)
{
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    AbilityRecord abilityRecord(want, abilityInfo, applicationInfo, 0);
    want.SetParam("debugApp", true);
    abilityRecord.SetWant(want);
    abilityRecord.PostCancelStartingWindowColdTask();
    EXPECT_TRUE(want.GetBoolParam("debugApp", false));

    want.SetParam("debugApp", false);
    abilityRecord.PostCancelStartingWindowColdTask();
    EXPECT_FALSE(want.GetBoolParam("debugApp", false));
}

/*
 * Feature: AbilityRecord
 * Function: CreateResourceManager
 * SubFunction: CreateResourceManager
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: CreateResourceManager
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CreateResourceManager_001, TestSize.Level1)
{
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    AbilityRecord abilityRecord(want, abilityInfo, applicationInfo, 0);
    EXPECT_TRUE(abilityRecord.CreateResourceManager() == nullptr);

    abilityInfo.hapPath = "abc";
    EXPECT_TRUE(abilityRecord.CreateResourceManager() == nullptr);

    abilityInfo.resourcePath = "abc";
    abilityInfo.hapPath = "";
    EXPECT_TRUE(abilityRecord.CreateResourceManager() == nullptr);

    abilityInfo.hapPath = "abc";
    EXPECT_TRUE(abilityRecord.CreateResourceManager() == nullptr);
}
#endif

/*
 * Feature: AbilityRecord
 * Function: BackgroundAbility
 * SubFunction: BackgroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord BackgroundAbility
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_BackgroundAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Closure task;
    EXPECT_FALSE(task);
    abilityRecord->lifecycleDeal_ = nullptr;
    abilityRecord->BackgroundAbility(task);
}

/*
 * Feature: AbilityRecord
 * Function: BackgroundAbility
 * SubFunction: BackgroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord BackgroundAbility
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_BackgroundAbility_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Closure task = []() {};
    DelayedSingleton<AbilityManagerService>::GetInstance()->Init();
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->want_.SetParam(DEBUG_APP, false);
    abilityRecord->SetTerminatingState();
    abilityRecord->SetRestarting(false, 0);
    abilityRecord->BackgroundAbility(task);
}

/*
 * Feature: AbilityRecord
 * Function: BackgroundAbility
 * SubFunction: BackgroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord BackgroundAbility
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_BackgroundAbility_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Closure task = []() {};
    DelayedSingleton<AbilityManagerService>::GetInstance()->Init();
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->want_.SetParam(DEBUG_APP, true);
    abilityRecord->SetTerminatingState();
    abilityRecord->SetRestarting(true, 0);
    abilityRecord->BackgroundAbility(task);
}

/*
 * Feature: AbilityRecord
 * Function: SetScheduler
 * SubFunction: SetScheduler
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetScheduler
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetScheduler_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    abilityRecord->scheduler_ = scheduler;
    abilityRecord->schedulerDeathRecipient_ =
        new AbilitySchedulerRecipient([abilityRecord](const wptr<IRemoteObject> &remote) {});
    abilityRecord->SetScheduler(scheduler);
}

/*
 * Feature: AbilityRecord
 * Function: SetScheduler
 * SubFunction: SetScheduler
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetScheduler
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetScheduler_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    abilityRecord->scheduler_ = scheduler;
    abilityRecord->schedulerDeathRecipient_ = nullptr;
    abilityRecord->SetScheduler(scheduler);
}

/*
 * Feature: AbilityRecord
 * Function: SetScheduler
 * SubFunction: SetScheduler
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetScheduler
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetScheduler_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<IAbilityScheduler> scheduler = nullptr;
    abilityRecord->scheduler_ = new AbilityScheduler();
    abilityRecord->schedulerDeathRecipient_ = nullptr;
    abilityRecord->SetScheduler(scheduler);
}

/*
 * Feature: AbilityRecord
 * Function: Activate
 * SubFunction: Activate
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord Activate
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_Activate_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->SetIsNewWant(true);
    abilityRecord->SetPreAbilityRecord(abilityRecord_);
    abilityRecord->Activate();
}

/*
 * Feature: AbilityRecord
 * Function: Activate
 * SubFunction: Activate
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord Activate
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_Activate_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->SetIsNewWant(true);
    abilityRecord->SetPreAbilityRecord(nullptr);
    abilityRecord->Activate();
}

/*
 * Feature: AbilityRecord
 * Function: Terminate
 * SubFunction: Terminate
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord Terminate
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_Terminate_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Closure task = []() {};
    abilityRecord->want_.SetParam(DEBUG_APP, true);
    abilityRecord->Terminate(task);
}

/*
 * Feature: AbilityRecord
 * Function: SendResultToCallers
 * SubFunction: SendResultToCallers
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SendResultToCallers
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SendResultToCallers_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, callerAbilityRecord);
    std::shared_ptr<AbilityResult> result = std::make_shared<AbilityResult>();
    callerAbilityRecord->SetResult(result);
    abilityRecord->callerList_.push_back(nullptr);
    abilityRecord->callerList_.push_back(caller);
    abilityRecord->SendResultToCallers();
}

/*
 * Feature: AbilityRecord
 * Function: SendResultToCallers
 * SubFunction: SendResultToCallers
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SendResultToCallers
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SendResultToCallers_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, callerAbilityRecord);
    std::shared_ptr<AbilityResult> result = std::make_shared<AbilityResult>();
    std::string srcAbilityId = "id";
    callerAbilityRecord->SetResult(nullptr);
    caller->saCaller_ = std::make_shared<SystemAbilityCallerRecord>(srcAbilityId, abilityRecord->GetToken());
    abilityRecord->callerList_.push_back(caller);
    abilityRecord->SendResultToCallers();
}

/*
 * Feature: AbilityRecord
 * Function: SendResultToCallers
 * SubFunction: SendResultToCallers
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SendResultToCallers
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SendResultToCallers_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, nullptr);
    caller->saCaller_ = nullptr;
    abilityRecord->callerList_.push_back(caller);
    abilityRecord->SendResultToCallers();
}

/*
 * Feature: AbilityRecord
 * Function: SaveResultToCallers
 * SubFunction: SaveResultToCallers
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SaveResultToCallers
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SaveResultToCallers_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int resultCode = 0;
    Want *resultWant;
    abilityRecord->callerList_.clear();
    abilityRecord->SaveResultToCallers(resultCode, resultWant);
}

/*
 * Feature: AbilityRecord
 * Function: SaveResultToCallers
 * SubFunction: SaveResultToCallers
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SaveResultToCallers
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SaveResultToCallers_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller1 = std::make_shared<CallerRecord>(0, callerAbilityRecord);
    std::shared_ptr<CallerRecord> caller2 = std::make_shared<CallerRecord>();
    int resultCode = 0;
    Want *resultWant = new Want();
    abilityRecord->callerList_.push_back(nullptr);
    abilityRecord->callerList_.push_back(caller1);
    abilityRecord->callerList_.push_back(caller2);
    abilityRecord->SaveResultToCallers(resultCode, resultWant);
}

/*
 * Feature: AbilityRecord
 * Function: SaveResult
 * SubFunction: SaveResult
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SaveResult
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SaveResult_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, callerAbilityRecord);
    int resultCode = 0;
    Want *resultWant = new Want();
    caller->saCaller_ = nullptr;
    abilityRecord->SaveResult(resultCode, resultWant, caller);
}

/*
 * Feature: AbilityRecord
 * Function: SaveResult
 * SubFunction: SaveResult
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SaveResult
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SaveResult_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, nullptr);
    std::string srcAbilityId = "id";
    int resultCode = 0;
    Want *resultWant = new Want();
    caller->saCaller_ = std::make_shared<SystemAbilityCallerRecord>(srcAbilityId, abilityRecord->GetToken());
    abilityRecord->SaveResult(resultCode, resultWant, caller);
}

/*
 * Feature: AbilityRecord
 * Function: SetResultToSystemAbility
 * SubFunction: SetResultToSystemAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SystemAbilityCallerRecord SetResultToSystemAbility
 */
HWTEST_F(AbilityRecordTest, SystemAbilityCallerRecord_SetResultToSystemAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::string srcAbilityId = "srcAbility_id";
    std::shared_ptr<SystemAbilityCallerRecord> systemAbilityRecord =
        std::make_shared<SystemAbilityCallerRecord>(srcAbilityId, abilityRecord->GetToken());
    Want resultWant;
    int resultCode = 1;
    systemAbilityRecord->SetResultToSystemAbility(systemAbilityRecord, resultWant, resultCode);
}

/*
 * Feature: AbilityRecord
 * Function: SendResultToSystemAbility
 * SubFunction: SendResultToSystemAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SystemAbilityCallerRecord SendResultToSystemAbility
 */
HWTEST_F(AbilityRecordTest, SystemAbilityCallerRecord_SendResultToSystemAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::string srcAbilityId = "srcAbility_id";
    std::shared_ptr<SystemAbilityCallerRecord> systemAbilityRecord =
        std::make_shared<SystemAbilityCallerRecord>(srcAbilityId, abilityRecord->GetToken());
    int requestCode = 0;
    int resultCode = 0;
    Want resultWant;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    systemAbilityRecord->SendResultToSystemAbility(requestCode, resultCode, resultWant, callerToken);
}

/*
 * Feature: AbilityRecord
 * Function: AddCallerRecord
 * SubFunction: AddCallerRecord
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord AddCallerRecord
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_AddCallerRecord_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    callerAbilityRecord->Init();
    sptr<IRemoteObject> callerToken = callerAbilityRecord->GetToken();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, callerAbilityRecord);
    abilityRecord->callerList_.push_back(caller);
    int requestCode = 0;
    std::string srcAbilityId = "srcAbility_id";
    abilityRecord->AddCallerRecord(callerToken, requestCode, srcAbilityId);
}

/*
 * Feature: AbilityRecord
 * Function: IsSystemAbilityCall
 * SubFunction: IsSystemAbilityCall
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord IsSystemAbilityCall
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_IsSystemAbilityCall_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    bool res1 = abilityRecord->IsSystemAbilityCall(nullptr);
    EXPECT_FALSE(res1);
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    sptr<IRemoteObject> callerToken = callerAbilityRecord->GetToken();
    bool res2 = abilityRecord->IsSystemAbilityCall(callerToken);
    EXPECT_FALSE(res2);
    callerAbilityRecord->Init();
    callerToken = callerAbilityRecord->GetToken();
    bool res3 = abilityRecord->IsSystemAbilityCall(callerToken);
    EXPECT_FALSE(res3);
}

/*
 * Feature: AbilityRecord
 * Function: AddSystemAbilityCallerRecord
 * SubFunction: AddSystemAbilityCallerRecord
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord AddSystemAbilityCallerRecord
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_AddSystemAbilityCallerRecord_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int requestCode = 0;
    std::string srcAbilityId = "srcAbility_id";
    abilityRecord->callerList_.clear();
    abilityRecord->AddSystemAbilityCallerRecord(callerToken, requestCode, srcAbilityId);
}

/*
 * Feature: AbilityRecord
 * Function: AddSystemAbilityCallerRecord
 * SubFunction: AddSystemAbilityCallerRecord
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord AddSystemAbilityCallerRecord
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_AddSystemAbilityCallerRecord_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int requestCode = 0;
    std::string srcAbilityId = "srcAbility_id";
    std::shared_ptr<SystemAbilityCallerRecord> saCaller =
        std::make_shared<SystemAbilityCallerRecord>(srcAbilityId, callerToken);
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(requestCode, saCaller);
    abilityRecord->callerList_.push_back(caller);
    abilityRecord->AddSystemAbilityCallerRecord(callerToken, requestCode, srcAbilityId);
}

/*
 * Feature: AbilityRecord
 * Function: GetConnectingRecordList
 * SubFunction: GetConnectingRecordList
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetConnectingRecordList
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetConnectingRecordList_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection1 =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback);
    std::shared_ptr<ConnectionRecord> connection2 =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback);
    connection1->SetConnectState(ConnectionState::CONNECTING);
    connection2->SetConnectState(ConnectionState::CONNECTED);
    abilityRecord->connRecordList_.push_back(connection1);
    abilityRecord->connRecordList_.push_back(connection2);
    abilityRecord->GetConnectingRecordList();
}

/*
 * Feature: AbilityRecord
 * Function: DumpAbilityState
 * SubFunction: DumpAbilityState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord DumpAbilityState
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_DumpAbilityState_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::vector<std::string> info;
    bool isClient = false;
    std::vector<std::string> params;
    abilityRecord->callContainer_ = std::make_shared<CallContainer>();
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->DumpAbilityState(info, isClient, params);
}

/*
 * Feature: AbilityRecord
 * Function: SetStartTime
 * SubFunction: SetStartTime
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetStartTime
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetStartTime_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->startTime_ = 1;
    abilityRecord->SetStartTime();
}

/*
 * Feature: AbilityRecord
 * Function: DumpService
 * SubFunction: DumpService
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord DumpService
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_DumpService_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::vector<std::string> info;
    std::vector<std::string> params;
    bool isClient = false;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback);
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->connRecordList_.push_back(nullptr);
    abilityRecord->connRecordList_.push_back(connection);
    abilityRecord->DumpService(info, params, isClient);
}

/*
 * Feature: AbilityRecord
 * Function: SendEvent
 * SubFunction: SendEvent
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SendEvent
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SendEvent_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t msg = 0;
    uint32_t timeOut = 0;
    abilityRecord->want_.SetParam(DEBUG_APP, true);
    EXPECT_TRUE(abilityRecord->want_.GetBoolParam(DEBUG_APP, false));
    abilityRecord->SendEvent(msg, timeOut);
}

/*
 * Feature: AbilityRecord
 * Function: SetRestarting
 * SubFunction: SetRestarting
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetRestarting
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetRestarting_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    bool isRestart = false;
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->isLauncherAbility_ = true;
    abilityRecord->SetRestarting(isRestart);
    abilityRecord->isLauncherAbility_ = false;
    abilityRecord->SetRestarting(isRestart);
    abilityRecord->isLauncherRoot_ = false;
    abilityRecord->SetRestarting(isRestart);
}

/*
 * Feature: AbilityRecord
 * Function: SetRestarting
 * SubFunction: SetRestarting
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetRestarting
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetRestarting_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    bool isRestart = false;
    int32_t canReStartCount = 1;
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->isLauncherAbility_ = true;
    abilityRecord->SetRestarting(isRestart, canReStartCount);
    abilityRecord->isLauncherAbility_ = false;
    abilityRecord->SetRestarting(isRestart, canReStartCount);
    abilityRecord->isLauncherRoot_ = false;
    abilityRecord->SetRestarting(isRestart, canReStartCount);
}

/*
 * Feature: AbilityRecord
 * Function: CallRequestDone
 * SubFunction: CallRequestDone
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CallRequestDone
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CallRequestDone_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    bool res1 = abilityRecord->CallRequestDone(nullptr);
    EXPECT_FALSE(res1);
    abilityRecord->callContainer_ = std::make_shared<CallContainer>();
    abilityRecord->Init();
    sptr<IRemoteObject> callStub = abilityRecord->GetToken();
    bool res2 = abilityRecord->CallRequestDone(callStub);
    EXPECT_TRUE(res2);
}

/*
 * Feature: AbilityRecord
 * Function: DumpClientInfo
 * SubFunction: DumpClientInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord DumpClientInfo
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_DumpClientInfo_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::vector<std::string> info;
    const std::vector<std::string> params;
    bool isClient = true;
    bool dumpConfig = false;
    abilityRecord->scheduler_ = nullptr;
    abilityRecord->DumpClientInfo(info, params, isClient, dumpConfig);
    abilityRecord->scheduler_ = new AbilityScheduler();
    abilityRecord->isReady_ = false;
    abilityRecord->DumpClientInfo(info, params, isClient, dumpConfig);
    abilityRecord->isReady_ = true;
    abilityRecord->DumpClientInfo(info, params, isClient, dumpConfig);
    dumpConfig = true;
    abilityRecord->DumpClientInfo(info, params, isClient, dumpConfig);
}

/*
 * Feature: AbilityRecord
 * Function: DumpAbilityInfoDone
 * SubFunction: DumpAbilityInfoDone
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord DumpAbilityInfoDone
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_DumpAbilityInfoDone_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::vector<std::string> infos;
    abilityRecord->isDumpTimeout_ = true;
    abilityRecord->DumpAbilityInfoDone(infos);
    abilityRecord->isDumpTimeout_ = false;
    abilityRecord->DumpAbilityInfoDone(infos);
}

/*
 * Feature: AbilityRecord
 * Function: GrantUriPermission
 * SubFunction: GrantUriPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GrantUriPermission
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GrantUriPermission_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Want want;
    want.SetFlags(1);
    abilityRecord->GrantUriPermission(want);
}

/*
 * Feature: AbilityRecord
 * Function: HandleDlpClosed
 * SubFunction: HandleDlpClosed
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord HandleDlpClosed
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_HandleDlpClosed_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.bundleName = DLP_BUNDLE_NAME;
    abilityRecord->appIndex_ = 1;
    abilityRecord->HandleDlpClosed();
}

/*
 * Feature: AbilityRecord
 * Function: GetCurrentAccountId
 * SubFunction: GetCurrentAccountId
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetCurrentAccountId
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetCurrentAccountId_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->GetCurrentAccountId();
}

/*
 * Feature: AbilityRecord
 * Function: CanRestartResident
 * SubFunction:
 * FunctionPoints: CanRestartResident
 * EnvConditions: NA
 * CaseDescription: Verify CanRestartResident return true when the ability is not a restart requestion.
 * @tc.require: issueI6588V
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CanRestartResident_001, TestSize.Level1)
{
    abilityRecord_->SetKeepAlive();
    EXPECT_TRUE(abilityRecord_->isKeepAlive_);

    abilityRecord_->SetRestarting(true, -1);
    EXPECT_TRUE(abilityRecord_->isRestarting_);
    EXPECT_EQ(abilityRecord_->restartCount_, -1);

    abilityRecord_->restartTime_ = 100000000;
    EXPECT_FALSE(abilityRecord_->CanRestartResident());
    abilityRecord_->restartTime_ = 0;
    // restart success
    abilityRecord_->SetAbilityState(AbilityState::ACTIVE);

    EXPECT_TRUE(abilityRecord_->CanRestartResident());
}

/*
 * Feature: AbilityRecord
 * Function: CanRestartResident
 * SubFunction:
 * FunctionPoints: CanRestartResident
 * EnvConditions: NA
 * CaseDescription: Verify CanRestartResident return true when the restartCount is out of max times but the interval
 *  time is over configuration.
 * @tc.require: issueI6588V
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CanRestartResident_002, TestSize.Level1)
{
    abilityRecord_->SetKeepAlive();
    EXPECT_TRUE(abilityRecord_->isKeepAlive_);

    abilityRecord_->SetRestarting(true, -1);
    EXPECT_TRUE(abilityRecord_->isRestarting_);
    EXPECT_EQ(abilityRecord_->restartCount_, -1);
    abilityRecord_->SetRestartTime(0);
    EXPECT_EQ(abilityRecord_->restartTime_, 0);

    EXPECT_TRUE(abilityRecord_->CanRestartResident());
}
}  // namespace AAFwk
}  // namespace OHOS
