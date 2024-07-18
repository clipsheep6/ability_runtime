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
#include "ability_manager_service.h"
#undef private
#undef protected
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "ability_bundle_event_callback.h"
#include "session/host/include/session.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;

constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
constexpr const char* DEBUG_APP = "debugApp";
constexpr const char* START_ABILITY_TYPE = "ABILITY_INNER_START_WITH_ACCOUNT";

constexpr int32_t FOUNDATION_UID = 5523;

namespace OHOS {
namespace AAFwk {
class AbilityManagerServiceFourthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);
};

void AbilityManagerServiceFourthTest::SetUpTestCase() {}

void AbilityManagerServiceFourthTest::TearDownTestCase() {}

void AbilityManagerServiceFourthTest::SetUp() {}

void AbilityManagerServiceFourthTest::TearDown() {}

sptr<SessionInfo> AbilityManagerServiceFourthTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

std::shared_ptr<AbilityRecord> AbilityManagerServiceFourthTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> AbilityManagerServiceFourthTest::MockToken(AbilityType abilityType)
{
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}


/*
 * Feature: AbilityManagerService
 * Function: AddFreeInstallObserver
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AddFreeInstallObserver
 */
HWTEST_F(AbilityManagerServiceFourthTest, AddFreeInstallObserver_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest AddFreeInstallObserver_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<AbilityRuntime::IFreeInstallObserver> observer;
    EXPECT_EQ(abilityMs_->AddFreeInstallObserver(nullptr, observer), ERR_INVALID_VALUE);

    abilityMs_->freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    EXPECT_EQ(abilityMs_->AddFreeInstallObserver(nullptr, observer), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest AddFreeInstallObserver_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: VerifyPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService VerifyPermission
 */
HWTEST_F(AbilityManagerServiceFourthTest, VerifyPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest VerifyPermission_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();

    std::string permission = "test_permission";
    int pid = 0;
    int uid = 0;
    EXPECT_EQ(abilityMs_->VerifyPermission(permission, pid, uid), CHECK_PERMISSION_FAILED);

    std::string permission2 = "";
    EXPECT_EQ(abilityMs_->VerifyPermission(permission2, pid, uid), CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest VerifyPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireShareData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireShareData
 */
HWTEST_F(AbilityManagerServiceFourthTest, AcquireShareData_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest AcquireShareData_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();

    int32_t missionId = 1;
    sptr<IAcquireShareDataCallback> shareData = nullptr;
    EXPECT_EQ(abilityMs_->AcquireShareData(missionId, shareData), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest AcquireShareData_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ShareDataDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ShareDataDone
 */
HWTEST_F(AbilityManagerServiceFourthTest, ShareDataDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ShareDataDone_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();

    sptr<IRemoteObject> token = nullptr;
    int32_t resultCode = 1;
    int32_t uniqueId = 1;
    WantParams wantParam;
    EXPECT_EQ(abilityMs_->ShareDataDone(token, resultCode, uniqueId, wantParam), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ShareDataDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: NotifySaveAsResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifySaveAsResult
 */
HWTEST_F(AbilityManagerServiceFourthTest, NotifySaveAsResult_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest NotifySaveAsResult_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();

    Want want;
    auto result = abilityMs_->NotifySaveAsResult(want, 0, 0);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest NotifySaveAsResult_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: InitDefaultRecoveryList
 * FunctionPoints: AbilityManagerService InitDefaultRecoveryList
 */
HWTEST_F(AbilityManagerServiceFourthTest, InitDefaultRecoveryList_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->InitDefaultRecoveryList();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnStop
 * FunctionPoints: AbilityManagerService OnStop
 */
HWTEST_F(AbilityManagerServiceFourthTest, OnStop_001, TestSize.Level1)
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
HWTEST_F(AbilityManagerServiceFourthTest, OnStop_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->OnStop();
    EXPECT_EQ(abilityMs->abilityBundleEventCallback_, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: GetConfiguration
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConfiguration
 */
HWTEST_F(AbilityManagerServiceFourthTest, GetConfiguration_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest GetConfiguration_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AppExecFwk::Configuration config;
    abilityMs_->SubscribeBackgroundTask();
    EXPECT_EQ(abilityMs_->GetConfiguration(config), 0);
    abilityMs_->UnSubscribeBackgroundTask();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest GetConfiguration_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReportAbilitStartInfoToRSS
 * FunctionPoints: AbilityManagerService ReportAbilitStartInfoToRSS
 */
HWTEST_F(AbilityManagerServiceFourthTest, ReportAbilitStartInfoToRSS_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ReportAbilitStartInfoToRSS start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityMs->ReportAbilitStartInfoToRSS(abilityInfo);
    EXPECT_EQ(abilityInfo.type, AppExecFwk::AbilityType::PAGE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ReportAbilitStartInfoToRSS end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReportAbilitAssociatedStartInfoToRSS
 * FunctionPoints: AbilityManagerService ReportAbilitAssociatedStartInfoToRSS
 */
HWTEST_F(AbilityManagerServiceFourthTest, ReportAbilitAssociatedStartInfoToRSS_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ReportAbilitAssociatedStartInfoToRSS start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> callerToken = nullptr;
    int64_t type = 0;
    abilityMs->ReportAbilitAssociatedStartInfoToRSS(abilityInfo, type, callerToken);
    callerToken = MockToken(AbilityType::PAGE);
    abilityMs->ReportAbilitAssociatedStartInfoToRSS(abilityInfo, type, callerToken);
    EXPECT_EQ(abilityInfo.type, AppExecFwk::AbilityType::PAGE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ReportAbilitAssociatedStartInfoToRSS end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbilityInner
 * FunctionPoints: AbilityManagerService StartExtensionAbilityInner
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartExtensionAbilityInner_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartExtensionAbilityInner_004 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t userId = 0;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::VPN;
    bool checkSystemCaller = true;
    bool isImplicit = true;
    bool isDlp = true;
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    auto result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_IMPLICIT_START_ABILITY_FAIL);

    abilityMs-> implicitStartProcessor_ = std::make_shared<ImplicitStartProcessor>();
    result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_IMPLICIT_START_ABILITY_FAIL);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartExtensionAbilityInner_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWithSpecifyTokenId
 * FunctionPoints: AbilityManagerService StartAbilityWithSpecifyTokenId
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityWithSpecifyTokenId_002, TestSize.Level1)
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
 * Function: StartUIAbilityBySCB
 * FunctionPoints: AbilityManagerService StartUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartUIAbilityBySCB_003, TestSize.Level1)
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
 * Function: RequestDialogServiceInner
 * FunctionPoints: AbilityManagerService RequestDialogServiceInner
 */
HWTEST_F(AbilityManagerServiceFourthTest, RequestDialogServiceInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RequestDialogServiceInner_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    const sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->RequestDialogServiceInner(want, callerToken, requestCode, userId);
    EXPECT_EQ(result, ERR_INVALID_CALLER);

    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    sptr<IRemoteObject> callerToken2 = abilityRecord->GetToken();

    result = abilityMs->RequestDialogServiceInner(want, callerToken2, requestCode, userId);
    EXPECT_EQ(result, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RequestDialogServiceInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForOptionInner
 * FunctionPoints: AbilityManagerService StartAbilityForOptionInner
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityForOptionInner_001, TestSize.Level1)
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
 * Function: InitDeepLinkReserve
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService InitDeepLinkReserve
 */
HWTEST_F(AbilityManagerServiceFourthTest, InitDeepLinkReserve_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitDeepLinkReserve_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->InitDeepLinkReserve();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitDeepLinkReserve_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: InitInterceptor
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService InitInterceptor
 */
HWTEST_F(AbilityManagerServiceFourthTest, InitInterceptor_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitInterceptor_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->InitInterceptor();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitInterceptor_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: InitStartupFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService InitStartupFlag
 */
HWTEST_F(AbilityManagerServiceFourthTest, InitStartupFlag_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitStartupFlag_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->InitStartupFlag();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitStartupFlag_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: InitStartAbilityChain
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService InitStartAbilityChain
 */
HWTEST_F(AbilityManagerServiceFourthTest, InitStartAbilityChain_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitStartAbilityChain_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->InitStartAbilityChain();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitStartAbilityChain_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: QueryServiceState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService QueryServiceState
 */
HWTEST_F(AbilityManagerServiceFourthTest, QueryServiceState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest QueryServiceState_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->QueryServiceState();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest QueryServiceState_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbility_001 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->StartAbility(want, 0, 0);
    want.SetParam(DEBUG_APP, true);
    want.SetParam(DEVELOPER_MODE_STATE, false);
    abilityMs_->StartAbility(want, 0, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbility_002 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->StartAbility(want, callerToken, 0, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbility_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByFreeInstall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByFreeInstall
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityByFreeInstall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByFreeInstall_001 start");
    Want want;
    want.SetParam(START_ABILITY_TYPE, true);
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->StartAbilityByFreeInstall(want, callerToken, 0, 0);
    want.SetParam(START_ABILITY_TYPE, false);
    abilityMs_->StartAbilityByFreeInstall(want, callerToken, 0, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByFreeInstall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWithSpecifyTokenIdInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityWithSpecifyTokenIdInner
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityWithSpecifyTokenIdInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityWithSpecifyTokenIdInner_001 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->StartAbilityWithSpecifyTokenIdInner(want, callerToken, 0, 0, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityWithSpecifyTokenIdInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWithSpecifyTokenIdInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityWithSpecifyTokenIdInner
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityWithSpecifyTokenIdInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityWithSpecifyTokenIdInner_002 start");
    Want want;
    StartOptions startOptions;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->StartAbilityWithSpecifyTokenIdInner(want, startOptions, callerToken, 0, 0, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityWithSpecifyTokenIdInner_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByInsightIntent
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByInsightIntent
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityByInsightIntent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByInsightIntent_001 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->StartAbilityByInsightIntent(want, callerToken, 0, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByInsightIntent_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityByUIContentSession_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByUIContentSession_001 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->StartAbilityByUIContentSession(want, callerToken, MockSessionInfo(0), 0, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByUIContentSession_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityByUIContentSession_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByUIContentSession_002 start");
    Want want;
    StartOptions startOptions;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->StartAbilityByUIContentSession(want, startOptions, callerToken, MockSessionInfo(0), 0, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByUIContentSession_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCaller
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityAsCaller_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityAsCaller_001 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->StartAbilityAsCaller(want, callerToken, callerToken, 0, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityAsCaller_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ImplicitStartAbilityAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ImplicitStartAbilityAsCaller
 */
HWTEST_F(AbilityManagerServiceFourthTest, ImplicitStartAbilityAsCaller_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ImplicitStartAbilityAsCaller_001 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->ImplicitStartAbilityAsCaller(want, callerToken, callerToken, 0, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ImplicitStartAbilityAsCaller_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCallerDetails
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCallerDetails
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityAsCallerDetails_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityAsCallerDetails_001 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->StartAbilityAsCallerDetails(want, callerToken, callerToken, 0, 0, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityAsCallerDetails_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityPublicPrechainCheck
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityPublicPrechainCheck
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityPublicPrechainCheck_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityPublicPrechainCheck_001 start");
    Want want;
    StartAbilityParams startAbilityParams(want);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->StartAbilityPublicPrechainCheck(startAbilityParams);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityPublicPrechainCheck_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityPrechainInterceptor
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityPrechainInterceptor
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityPrechainInterceptor_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityPrechainInterceptor_001 start");
    Want want;
    StartAbilityParams startAbilityParams(want);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->StartAbilityPrechainInterceptor(startAbilityParams);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityPrechainInterceptor_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SetReserveInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetReserveInfo
 */
HWTEST_F(AbilityManagerServiceFourthTest, SetReserveInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest SetReserveInfo_001 start");
    std::string linkString;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->SetReserveInfo(linkString);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest SetReserveInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckExtensionCallPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckExtensionCallPermission
 */
HWTEST_F(AbilityManagerServiceFourthTest, CheckExtensionCallPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckExtensionCallPermission_001 start");
    Want want;
    AbilityRequest abilityRequest;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->CheckExtensionCallPermission(want, abilityRequest);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckExtensionCallPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckServiceCallPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckServiceCallPermission
 */
HWTEST_F(AbilityManagerServiceFourthTest, CheckServiceCallPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckServiceCallPermission_001 start");
    Want want;
    AbilityRequest abilityRequest;
    AppExecFwk::AbilityInfo abilityInfo;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->CheckServiceCallPermission(abilityRequest, abilityInfo);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckServiceCallPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckBrokerCallPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckBrokerCallPermission
 */
HWTEST_F(AbilityManagerServiceFourthTest, CheckBrokerCallPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckBrokerCallPermission_001 start");
    Want want;
    AbilityRequest abilityRequest;
    AppExecFwk::AbilityInfo abilityInfo;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->CheckBrokerCallPermission(abilityRequest, abilityInfo);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckBrokerCallPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckAbilityCallPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckAbilityCallPermission
 */
HWTEST_F(AbilityManagerServiceFourthTest, CheckAbilityCallPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckAbilityCallPermission_001 start");
    Want want;
    AbilityRequest abilityRequest;
    AppExecFwk::AbilityInfo abilityInfo;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->CheckAbilityCallPermission(abilityRequest, abilityInfo, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckAbilityCallPermission_001 end");
}

} // namespace AAFwk
} // namespace OHOS
