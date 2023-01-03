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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define private public
#define protected public
#include "ability.h"
#include "ability_impl.h"
#include "abs_shared_result_set.h"
#include "bool_wrapper.h"
#include "hilog_wrapper.h"
#include "context_deal.h"
#include "continuation_manager.h"
#include "continuation_handler.h"
#include "data_ability_predicates.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_lifecycle_callbacks.h"
#include "mock_ability_token.h"
#include "mock_continuation_ability.h"
#include "mock_reverse_continuation_scheduler_replica_stub.h"
#include "ohos_application.h"
#include "page_ability_impl.h"
#include "values_bucket.h"
#undef protected
#undef private

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using testing::_;
using testing::Return;
const std::string SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME = "ohos.extra.param.key.supportContinuePageStack";
const int32_t CONTINUE_ABILITY_REJECTED = 29360197;
const int32_t CONTINUE_SAVE_DATA_FAILED = 29360198;
const int32_t CONTINUE_ON_CONTINUE_FAILED = 29360199;
const int32_t CONTINUE_ON_CONTINUE_MISMATCH = 29360204;
#ifdef SUPPORT_GRAPHICS
const int32_t CONTINUE_GET_CONTENT_FAILED = 29360200;
#endif
class ContinuationTest : public testing::Test {
public:
    ContinuationTest() : continuationManager_(nullptr), ability_(nullptr), abilityInfo_(nullptr),
        continueToken_(nullptr)
    {}
    ~ContinuationTest()
    {}
    std::shared_ptr<ContinuationManager> continuationManager_;
    std::shared_ptr<Ability> ability_;
    std::shared_ptr<MockContinuationAbility> mockAbility_;
    std::shared_ptr<AbilityInfo> abilityInfo_;
    sptr<IRemoteObject> continueToken_;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class MockReverseContinuationSchedulerPrimaryProxy : public IRemoteProxy<IReverseContinuationSchedulerPrimary> {
public:
    explicit MockReverseContinuationSchedulerPrimaryProxy(const sptr<IRemoteObject> &remoteObject) :
        IRemoteProxy<IReverseContinuationSchedulerPrimary>(remoteObject) {}
    virtual ~MockReverseContinuationSchedulerPrimaryProxy() = default;
    MOCK_METHOD0(NotifyReplicaTerminated, void());
    MOCK_METHOD1(ContinuationBack, bool(const AAFwk::Want &want));
};

class MockReverseContinuationSchedulerReplicaProxy : public IRemoteProxy<IReverseContinuationSchedulerReplica> {
public:
    explicit MockReverseContinuationSchedulerReplicaProxy(const sptr<IRemoteObject> &remoteObject) :
        IRemoteProxy<IReverseContinuationSchedulerReplica>(remoteObject) {}
    virtual ~MockReverseContinuationSchedulerReplicaProxy() = default;
    MOCK_METHOD1(PassPrimary, void(const sptr<IRemoteObject> &primary));
    MOCK_METHOD0(ReverseContinuation, bool());
    MOCK_METHOD1(NotifyReverseResult, void(int reverseResult));
};

void ContinuationTest::SetUpTestCase(void)
{}

void ContinuationTest::TearDownTestCase(void)
{}

void ContinuationTest::SetUp(void)
{
    continuationManager_ = std::make_shared<ContinuationManager>();
    continueToken_ = sptr<IRemoteObject>(new (std::nothrow)MockAbilityToken());
    abilityInfo_ = std::make_shared<AbilityInfo>();
    abilityInfo_->name = "ability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo_->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    ability_ = std::make_shared<Ability>();
    ability_->Init(abilityInfo_, application, handler, continueToken_);
    mockAbility_ = std::make_shared<MockContinuationAbility>();
    mockAbility_->Init(abilityInfo_, application, handler, continueToken_);
}

void ContinuationTest::TearDown(void)
{}

/*
 * @tc.name: continue_manager_init_001
 * @tc.desc: init ContinuationManager with illegal ability
 * @tc.type: FUNC
 * @tc.require: AR000GI8IP
 */
HWTEST_F(ContinuationTest, continue_manager_init_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_init_001 start";

    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    std::shared_ptr<ContinuationHandler> continuationHandler =
        std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    std::shared_ptr<Ability> ability = nullptr;
    bool result = continuationManager_->Init(ability, continueToken_, abilityInfo_, continuationHandler);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_init_001 end";
}

/*
 * @tc.name: continue_manager_init_002
 * @tc.desc: init ContinuationManager with illegal continueToken
 * @tc.type: FUNC
 * @tc.require: AR000GI8IP
 */
HWTEST_F(ContinuationTest, continue_manager_init_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_init_002 start";

    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    std::shared_ptr<ContinuationHandler> continuationHandler =
        std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> continueToken = nullptr;
    bool result = continuationManager_->Init(ability_, continueToken, abilityInfo_, continuationHandler);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_init_002 end";
}

/*
 * @tc.number: continue_manager_init_003
 * @tc.name: Init
 * @tc.desc: init ContinuationManager with null abilityInfo_
 */
HWTEST_F(ContinuationTest, continue_manager_init_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_init_003 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    ability_->abilityInfo_ = nullptr;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    bool result = continuationManager_->Init(ability_, continueToken, abilityInfo_, continuationHandler);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_init_003 end";
}

/*
 * @tc.number: continue_manager_init_004
 * @tc.name: Init
 * @tc.desc: init ContinuationManager success
 */
HWTEST_F(ContinuationTest, continue_manager_init_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_init_004 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    bool result = continuationManager_->Init(mockAbility_, continueToken, abilityInfo_, continuationHandler);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "continue_manager_init_004 end";
}

/*
 * @tc.number: continue_manager_start_continuation_001
 * @tc.name: StartContinuation
 * @tc.desc: call StartContinuation success
 */
HWTEST_F(ContinuationTest, continue_manager_start_continuation_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_start_continuation_001 start";
    EXPECT_CALL(*mockAbility_, OnStartContinuation()).Times(1).WillOnce(Return(true));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    std::shared_ptr<ContinuationHandler> continuationHandler =
        std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    bool startResult = continuationManager_->StartContinuation();
    EXPECT_TRUE(startResult);
    GTEST_LOG_(INFO) << "continue_manager_start_continuation_001 end";
}

/*
 * @tc.number: continue_manager_StartContinuation_002
 * @tc.name: StartContinuation
 * @tc.desc: test StartContinuation with DoScheduleStartContinuation failed
 */
HWTEST_F(ContinuationTest, continue_manager_StartContinuation_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_StartContinuation_002 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->ability_.reset();
    bool result = continuationManager_->StartContinuation();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_StartContinuation_002 end";
}

/*
 * @tc.number: continue_manager_save_data_001
 * @tc.name: SaveData
 * @tc.desc: call SaveData success
 */
HWTEST_F(ContinuationTest, continue_manager_save_data_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_save_data_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    EXPECT_CALL(*mockAbility_, OnSaveData(_)).Times(1).WillOnce(Return(true));
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    std::shared_ptr<ContinuationHandler> continuationHandler =
        std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    bool saveDataResult = continuationManager_->SaveData(wantParams);
    EXPECT_TRUE(saveDataResult);
    GTEST_LOG_(INFO) << "continue_manager_save_data_001 end";
}

/*
 * @tc.number: continue_manager_SaveData_002
 * @tc.name: SaveData
 * @tc.desc: call SaveData with DoScheduleSaveData failed
 */
HWTEST_F(ContinuationTest, continue_manager_SaveData_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_SaveData_002 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->progressState_ = ContinuationManager::ProgressState::WAITING_SCHEDULE;
    WantParams saveData;
    bool result = continuationManager_->SaveData(saveData);
    EXPECT_FALSE(result);
    EXPECT_EQ(ContinuationManager::ProgressState::INITIAL, continuationManager_->progressState_);
    GTEST_LOG_(INFO) << "continue_manager_SaveData_002 end";
}

/*
 * @tc.name: continue_handler_start_continue_001
 * @tc.desc: call HandleStartContinuation with illegal token
 * @tc.type: FUNC
 * @tc.require: AR000GI8IP
 */
HWTEST_F(ContinuationTest, continue_handler_start_continue_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_start_continue_001 start";

    std::shared_ptr<Ability> ability = std::make_shared<MockContinuationAbility>();
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability;
    std::shared_ptr<ContinuationHandler> continuationHandler =
        std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    bool result = continuationHandler->HandleStartContinuation(nullptr, "mock_deviceId");
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_start_continue_001 end";
}

/*
 * @tc.number: continue_handler_start_continue_002
 * @tc.name: HandleStartContinuation
 * @tc.desc: call HandleStartContinuation without Set AbilityInfo
 */
HWTEST_F(ContinuationTest, continue_handler_start_continue_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_start_continue_002 start";
    std::shared_ptr<Ability> ability = std::make_shared<MockContinuationAbility>();
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability;
    std::shared_ptr<ContinuationHandler> continuationHandler =
        std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    bool result = continuationHandler->HandleStartContinuation(continueToken, "mock_deviceId");
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_start_continue_002 end";
}

/*
 * @tc.number: continue_handler_HandleStartContinuation_003
 * @tc.name: HandleStartContinuation
 * @tc.desc: call HandleStartContinuation with null continuationManagerTmp
 */
HWTEST_F(ContinuationTest, continue_handler_HandleStartContinuation_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuation_003 start";
    std::shared_ptr<Ability> ability = std::make_shared<MockContinuationAbility>();
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    continuationManager.reset();
    std::weak_ptr<Ability> abilityTmp = ability;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    continuationHandler->SetAbilityInfo(abilityInfo);
    bool result = continuationHandler->HandleStartContinuation(continueToken, "mock_deviceId");
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuation_003 end";
}

/*
 * @tc.number: continue_handler_HandleStartContinuation_004
 * @tc.name: HandleStartContinuation
 * @tc.desc: call HandleStartContinuation with StartContinuation falied
 */
HWTEST_F(ContinuationTest, continue_handler_HandleStartContinuation_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuation_004 start";
    std::shared_ptr<Ability> ability = std::make_shared<MockContinuationAbility>();
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    continuationHandler->SetAbilityInfo(abilityInfo);
    bool result = continuationHandler->HandleStartContinuation(continueToken, "mock_deviceId");
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuation_004 end";
}

/*
 * @tc.number: continue_handler_HandleStartContinuation_005
 * @tc.name: HandleStartContinuation
 * @tc.desc: call HandleStartContinuation with ScheduleSaveData failed
 */
HWTEST_F(ContinuationTest, continue_handler_HandleStartContinuation_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuation_005 start";
    EXPECT_CALL(*mockAbility_, OnStartContinuation()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAbility_, OnSaveData(_)).Times(1).WillOnce(Return(false));
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    continuationHandler->SetAbilityInfo(abilityInfo);
    bool result = continuationHandler->HandleStartContinuation(continueToken, "mock_deviceId");
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuation_005 end";
}

/*
 * @tc.number: continue_handler_HandleStartContinuation_006
 * @tc.name: HandleStartContinuation
 * @tc.desc: call HandleStartContinuation with distClient_.startContinuation failed
 */
HWTEST_F(ContinuationTest, continue_handler_HandleStartContinuation_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuation_006 start";
    EXPECT_CALL(*mockAbility_, OnStartContinuation()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAbility_, OnSaveData(_)).Times(1).WillOnce(Return(true));
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    continuationHandler->SetAbilityInfo(abilityInfo);
    bool result = continuationHandler->HandleStartContinuation(continueToken, "mock_deviceId");
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuation_006 end";
}

/*
 * @tc.number: continue_handler_HandleReceiveRemoteScheduler_001
 * @tc.name: HandleReceiveRemoteScheduler
 * @tc.desc: call HandleReceiveRemoteScheduler with remoteReplica is null
 */
HWTEST_F(ContinuationTest, continue_handler_HandleReceiveRemoteScheduler_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_HandleReceiveRemoteScheduler_001 start";
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> primary = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    continuationHandler->SetPrimaryStub(primary);
    continuationHandler->HandleReceiveRemoteScheduler(nullptr);
    EXPECT_TRUE(continuationHandler->remotePrimaryProxy_ == nullptr);
    GTEST_LOG_(INFO) << "continue_handler_HandleReceiveRemoteScheduler_001 end";
}

/*
 * @tc.number: continue_handler_HandleReceiveRemoteScheduler_002
 * @tc.name: HandleReceiveRemoteScheduler
 * @tc.desc: call HandleReceiveRemoteScheduler using remoteReplicaProxy_ and DeathRecipient_ in different situations.
 */
HWTEST_F(ContinuationTest, continue_handler_HandleReceiveRemoteScheduler_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_HandleReceiveRemoteScheduler_002 start";
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<MockReverseContinuationSchedulerReplicaStub> remoteReplica =
        new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    EXPECT_CALL(*remoteReplica, PassPrimary(_)).Times(3);
    continuationHandler->HandleReceiveRemoteScheduler(remoteReplica);
    continuationHandler->HandleReceiveRemoteScheduler(remoteReplica);
    continuationHandler->schedulerDeathRecipient_ = nullptr;
    continuationHandler->HandleReceiveRemoteScheduler(remoteReplica);
    GTEST_LOG_(INFO) << "continue_handler_HandleReceiveRemoteScheduler_002 end";
}

/*
 * @tc.number: continue_handler_HandleCompleteContinuation_001
 * @tc.name: HandleStartContinuation
 * @tc.desc: call HandleCompleteContinuation with null continuationManager
 */
HWTEST_F(ContinuationTest, continue_handler_HandleCompleteContinuation_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_HandleCompleteContinuation_001 start";
    EXPECT_CALL(*mockAbility_, OnCompleteContinuation(_)).Times(0);
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    continuationManager.reset();
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    int result = 0;
    continuationHandler->HandleCompleteContinuation(result);
    GTEST_LOG_(INFO) << "continue_handler_HandleCompleteContinuation_001 end";
}

/*
 * @tc.number: continue_handler_HandleCompleteContinuation_002
 * @tc.name: HandleStartContinuation
 * @tc.desc: call HandleCompleteContinuation success
 */
HWTEST_F(ContinuationTest, continue_handler_HandleCompleteContinuation_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_HandleCompleteContinuation_002 start";
    EXPECT_CALL(*mockAbility_, OnCompleteContinuation(_)).Times(1);
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    int result = 0;
    continuationHandler->HandleCompleteContinuation(result);
    GTEST_LOG_(INFO) << "continue_handler_HandleCompleteContinuation_002 end";
}

/*
 * @tc.number: continue_handler_PassPrimary_001
 * @tc.name: PassPrimary
 * @tc.desc: call PassPrimary success
 */
HWTEST_F(ContinuationTest, continue_handler_PassPrimary_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_PassPrimary_001 start";
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> primary = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    continuationHandler->PassPrimary(primary);
    EXPECT_TRUE(continuationHandler->remotePrimaryProxy_ != nullptr);
    GTEST_LOG_(INFO) << "continue_handler_PassPrimary_001 end";
}

/*
 * @tc.number: continue_handler_ReverseContinuation_001
 * @tc.name: ReverseContinuation
 * @tc.desc: call ReverseContinuation with remotePrimaryProxy is null
 */
HWTEST_F(ContinuationTest, continue_handler_ReverseContinuation_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinuation_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationHandler->SetAbilityInfo(abilityInfo_);
    bool result = continuationHandler->ReverseContinuation();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinuation_001 end";
}

/*
 * @tc.number: continue_handler_ReverseContinuation_002
 * @tc.name: ReverseContinuation
 * @tc.desc: call ReverseContinuation with abilityInfo_ is null
 */
HWTEST_F(ContinuationTest, continue_handler_ReverseContinuation_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinuation_002 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> primary = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    continuationHandler->PassPrimary(primary);
    bool result = continuationHandler->ReverseContinuation();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinuation_002 end";
}

/*
 * @tc.number: continue_handler_ReverseContinuation_003
 * @tc.name: ReverseContinuation
 * @tc.desc: call ReverseContinuation with continuationManagerTmp is null
 */
HWTEST_F(ContinuationTest, continue_handler_ReverseContinuation_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinuation_003 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    continuationManager.reset();
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> primary = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    continuationHandler->PassPrimary(primary);
    continuationHandler->SetAbilityInfo(abilityInfo_);
    bool result = continuationHandler->ReverseContinuation();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinuation_003 end";
}

/*
 * @tc.number: continue_handler_ReverseContinuation_004
 * @tc.name: ReverseContinuation
 * @tc.desc: call ReverseContinuation with OnStartContinuation failed
 */
HWTEST_F(ContinuationTest, continue_handler_ReverseContinuation_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinuation_004 start";
    EXPECT_CALL(*mockAbility_, OnStartContinuation()).Times(1).WillOnce(Return(false));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    sptr<IRemoteObject> primary = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    continuationHandler->PassPrimary(primary);
    continuationHandler->SetAbilityInfo(abilityInfo_);
    bool result = continuationHandler->ReverseContinuation();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinuation_004 end";
}

/*
 * @tc.number: continue_handler_ReverseContinuation_005
 * @tc.name: ReverseContinuation
 * @tc.desc: call ReverseContinuation with OnSaveData failed
 */
HWTEST_F(ContinuationTest, continue_handler_ReverseContinuation_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinuation_005 start";
    EXPECT_CALL(*mockAbility_, OnStartContinuation()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAbility_, OnSaveData(_)).Times(1).WillOnce(Return(false));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    sptr<IRemoteObject> primary = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    continuationHandler->PassPrimary(primary);
    continuationHandler->SetAbilityInfo(abilityInfo_);
    bool result = continuationHandler->ReverseContinuation();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinuation_005 end";
}

/*
 * @tc.number: continue_handler_ReverseContinuation_006
 * @tc.name: ReverseContinuation
 * @tc.desc: call ReverseContinuation with ContinuationBack failed
 */
HWTEST_F(ContinuationTest, continue_handler_ReverseContinuation_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinuation_006 start";
    EXPECT_CALL(*mockAbility_, OnStartContinuation()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAbility_, OnSaveData(_)).Times(1).WillOnce(Return(true));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    sptr<IRemoteObject> remoteObject = new (std::nothrow) AAFwk::AbilityConnectCallback();
    sptr<MockReverseContinuationSchedulerPrimaryProxy> primary =
        new (std::nothrow) MockReverseContinuationSchedulerPrimaryProxy(remoteObject);
    EXPECT_CALL(*primary, ContinuationBack(_)).Times(1).WillOnce(Return(true));
    continuationHandler->remotePrimaryProxy_ = primary;
    continuationHandler->SetAbilityInfo(abilityInfo_);
    bool result = continuationHandler->ReverseContinuation();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinuation_006 end";
}

/*
 * @tc.number: continue_handler_ReverseContinuation_007
 * @tc.name: ReverseContinuation
 * @tc.desc: call ReverseContinuation with success
 */
HWTEST_F(ContinuationTest, continue_handler_ReverseContinuation_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinuation_007 start";
    EXPECT_CALL(*mockAbility_, OnStartContinuation()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAbility_, OnSaveData(_)).Times(1).WillOnce(Return(true));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    sptr<IRemoteObject> remoteObject = new (std::nothrow) AAFwk::AbilityConnectCallback();
    sptr<MockReverseContinuationSchedulerPrimaryProxy> primary =
        new (std::nothrow) MockReverseContinuationSchedulerPrimaryProxy(remoteObject);
    EXPECT_CALL(*primary, ContinuationBack(_)).Times(1).WillOnce(Return(false));
    continuationHandler->remotePrimaryProxy_ = primary;
    continuationHandler->SetAbilityInfo(abilityInfo_);
    bool result = continuationHandler->ReverseContinuation();
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinuation_007 end";
}

/*
 * @tc.number: continue_handler_NotifyReverseResult_001
 * @tc.name: NotifyReverseResult
 * @tc.desc: call NotifyReverseResult with reverseResult == 0
 */
HWTEST_F(ContinuationTest, continue_handler_NotifyReverseResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_NotifyReverseResult_001 start";
    EXPECT_CALL(*mockAbility_, TerminateAbility()).Times(1);
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    int result = 0;
    continuationHandler->NotifyReverseResult(result);
    GTEST_LOG_(INFO) << "continue_handler_NotifyReverseResult_001 end";
}

/*
 * @tc.number: continue_handler_NotifyReverseResult_002
 * @tc.name: NotifyReverseResult
 * @tc.desc: call NotifyReverseResult with reverseResult == 0 but ability_ is null
 */
HWTEST_F(ContinuationTest, continue_handler_NotifyReverseResult_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_NotifyReverseResult_002 start";
    EXPECT_CALL(*mockAbility_, TerminateAbility()).Times(0);
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationHandler->ability_.reset();
    int result = 0;
    continuationHandler->NotifyReverseResult(result);
    GTEST_LOG_(INFO) << "continue_handler_NotifyReverseResult_002 end";
}

/*
 * @tc.number: continue_handler_NotifyReverseResult_003
 * @tc.name: NotifyReverseResult
 * @tc.desc: call NotifyReverseResult with reverseResult != 0
 */
HWTEST_F(ContinuationTest, continue_handler_NotifyReverseResult_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_NotifyReverseResult_003 start";
    EXPECT_CALL(*mockAbility_, TerminateAbility()).Times(0);
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    int result = 1;
    continuationHandler->NotifyReverseResult(result);
    GTEST_LOG_(INFO) << "continue_handler_NotifyReverseResult_003 end";
}

/*
 * @tc.number: continue_handler_ContinuationBack_001
 * @tc.name: ContinuationBack
 * @tc.desc: call ContinuationBack with continuationManager is null
 */
HWTEST_F(ContinuationTest, continue_handler_ContinuationBack_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_ContinuationBack_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    continuationManager.reset();
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    Want want;
    bool result = continuationHandler->ContinuationBack(want);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_ContinuationBack_001 end";
}

/*
 * @tc.number: continue_handler_ContinuationBack_002
 * @tc.name: ContinuationBack
 * @tc.desc: call ContinuationBack with RestoreFromRemote failed
 */
HWTEST_F(ContinuationTest, continue_handler_ContinuationBack_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_ContinuationBack_002 start";
    EXPECT_CALL(*mockAbility_, OnRestoreData(_)).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*mockAbility_, TerminateAbility()).Times(0);
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    Want want;
    sptr<IRemoteObject> remoteReplica = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    continuationHandler->remoteReplicaProxy_ = iface_cast<IReverseContinuationSchedulerReplica>(remoteReplica);
    bool result = continuationHandler->ContinuationBack(want);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "continue_handler_ContinuationBack_002 end";
}

/*
 * @tc.number: continue_handler_ContinuationBack_003
 * @tc.name: ContinuationBack
 * @tc.desc: call ContinuationBack with RestoreFromRemote success
 */
HWTEST_F(ContinuationTest, continue_handler_ContinuationBack_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_ContinuationBack_003 start";
    EXPECT_CALL(*mockAbility_, OnRestoreData(_)).Times(1).WillOnce(Return(true));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    Want want;
    sptr<IRemoteObject> remoteReplica = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    continuationHandler->remoteReplicaProxy_ = iface_cast<IReverseContinuationSchedulerReplica>(remoteReplica);
    bool result = continuationHandler->ContinuationBack(want);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "continue_handler_ContinuationBack_003 end";
}

/*
 * @tc.number: continue_handler_NotifyReplicaTerminated_001
 * @tc.name: NotifyReplicaTerminated
 * @tc.desc: call NotifyReplicaTerminated success
 */
HWTEST_F(ContinuationTest, continue_handler_NotifyReplicaTerminated_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_NotifyReplicaTerminated_001 start";
    EXPECT_CALL(*mockAbility_, OnRemoteTerminated()).Times(1);
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken, abilityInfo_, continuationHandler);
    continuationHandler->NotifyReplicaTerminated();
    GTEST_LOG_(INFO) << "continue_handler_NotifyReplicaTerminated_001 end";
}

/*
 * @tc.number: continue_handler_NotifyReplicaTerminated_002
 * @tc.name: NotifyReplicaTerminated
 * @tc.desc: call NotifyReplicaTerminated with null continuationManager
 */
HWTEST_F(ContinuationTest, continue_handler_NotifyReplicaTerminated_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_NotifyReplicaTerminated_002 start";

    EXPECT_CALL(*mockAbility_, OnRemoteTerminated()).Times(0);
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationHandler->continuationManager_.reset();
    continuationHandler->NotifyReplicaTerminated();
    GTEST_LOG_(INFO) << "continue_handler_NotifyReplicaTerminated_002 end";
}

/*
 * @tc.number: continue_handler_NotifyTerminationToPrimary_001
 * @tc.name: NotifyTerminationToPrimary
 * @tc.desc: call NotifyTerminationToPrimary with null remotePrimaryProxy_
 */
HWTEST_F(ContinuationTest, continue_handler_NotifyTerminationToPrimary_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_NotifyTerminationToPrimary_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> remoteObject = new (std::nothrow) AAFwk::AbilityConnectCallback();
    sptr<MockReverseContinuationSchedulerPrimaryProxy> primary =
        new (std::nothrow) MockReverseContinuationSchedulerPrimaryProxy(remoteObject);
    EXPECT_CALL(*primary, NotifyReplicaTerminated()).Times(0);
    continuationHandler->NotifyTerminationToPrimary();
    GTEST_LOG_(INFO) << "continue_handler_NotifyTerminationToPrimary_001 end";
}

/*
 * @tc.number: continue_handler_NotifyTerminationToPrimary_002
 * @tc.name: NotifyTerminationToPrimary
 * @tc.desc: call NotifyTerminationToPrimary success
 */
HWTEST_F(ContinuationTest, continue_handler_NotifyTerminationToPrimary_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_NotifyTerminationToPrimary_002 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> remoteObject = new (std::nothrow) AAFwk::AbilityConnectCallback();
    sptr<MockReverseContinuationSchedulerPrimaryProxy> primary =
        new (std::nothrow) MockReverseContinuationSchedulerPrimaryProxy(remoteObject);
    EXPECT_CALL(*primary, NotifyReplicaTerminated()).Times(1);
    continuationHandler->remotePrimaryProxy_ = primary;
    continuationHandler->NotifyTerminationToPrimary();
    GTEST_LOG_(INFO) << "continue_handler_NotifyTerminationToPrimary_002 end";
}

/*
 * @tc.number: continue_handler_SetReversible_001
 * @tc.name: SetReversible
 * @tc.desc: call SetReversible success
 */
HWTEST_F(ContinuationTest, continue_handler_SetReversible_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_SetReversible_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    bool reversible = true;
    continuationHandler->SetReversible(reversible);
    EXPECT_TRUE(continuationHandler->reversible_);
    GTEST_LOG_(INFO) << "continue_handler_SetReversible_001 end";
}

/*
 * @tc.number: continue_handler_SetAbilityInfo_001
 * @tc.name: SetAbilityInfo
 * @tc.desc: call SetAbilityInfo success
 */
HWTEST_F(ContinuationTest, continue_handler_SetAbilityInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_SetAbilityInfo_001 start";
    std::shared_ptr<Ability> ability = std::make_shared<MockContinuationAbility>();
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    continuationHandler->SetAbilityInfo(abilityInfo);
    EXPECT_TRUE(continuationHandler->abilityInfo_ != nullptr);
    GTEST_LOG_(INFO) << "continue_handler_SetAbilityInfo_001 end";
}

/*
 * @tc.number: continue_handler_SetPrimaryStub_001
 * @tc.name: SetPrimaryStub
 * @tc.desc: call SetPrimaryStub success
 */
HWTEST_F(ContinuationTest, continue_handler_SetPrimaryStub_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_SetPrimaryStub_001 start";
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationHandler->SetPrimaryStub(continueToken);
    EXPECT_TRUE(continuationHandler->remotePrimaryStub_ != nullptr);
    GTEST_LOG_(INFO) << "continue_handler_SetPrimaryStub_001 end";
}

/*
 * @tc.number: continue_handler_ReverseContinueAbility_001
 * @tc.name: ReverseContinueAbility
 * @tc.desc: call ReverseContinueAbility with null remoteReplicaProxy_
 */
HWTEST_F(ContinuationTest, continue_handler_ReverseContinueAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_SetPrimaryStub_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    auto result = continuationHandler->ReverseContinueAbility();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinueAbility_001 end";
}

/*
 * @tc.number: continue_handler_ReverseContinueAbility_002
 * @tc.name: ReverseContinueAbility
 * @tc.desc: call ReverseContinueAbility success
 */
HWTEST_F(ContinuationTest, continue_handler_ReverseContinueAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_SetPrimaryStub_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> stub = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    sptr<MockReverseContinuationSchedulerReplicaProxy> primary =
        new (std::nothrow) MockReverseContinuationSchedulerReplicaProxy(stub);
    EXPECT_CALL(*primary, ReverseContinuation()).Times(1).WillOnce(Return(true));
    continuationHandler->remoteReplicaProxy_ = primary;
    auto result = continuationHandler->ReverseContinueAbility();
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "continue_handler_ReverseContinueAbility_002 end";
}

/*
 * @tc.number: continue_handler_HandleStartContinuationWithStack_001
 * @tc.name: HandleStartContinuationWithStack
 * @tc.desc: call HandleStartContinuationWithStack with nullptr token
 */
HWTEST_F(ContinuationTest, continue_handler_HandleStartContinuationWithStack_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuationWithStack_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    uint32_t versionCode = 0;
    auto result = continuationHandler->HandleStartContinuationWithStack(nullptr, "deviceId", versionCode);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuationWithStack_001 end";
}

/*
 * @tc.number: continue_handler_HandleStartContinuationWithStack_002
 * @tc.name: HandleStartContinuationWithStack
 * @tc.desc: call HandleStartContinuationWithStack with no abilityInfo_
 */
HWTEST_F(ContinuationTest, continue_handler_HandleStartContinuationWithStack_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuationWithStack_002 start";
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    uint32_t versionCode = 0;
    auto result = continuationHandler->HandleStartContinuationWithStack(continueToken, "deviceId", versionCode);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuationWithStack_002 end";
}

/*
 * @tc.number: continue_handler_HandleStartContinuationWithStack_003
 * @tc.name: HandleStartContinuationWithStack
 * @tc.desc: call HandleStartContinuationWithStack with null continuationManager
 */
HWTEST_F(ContinuationTest, continue_handler_HandleStartContinuationWithStack_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuationWithStack_003 start";
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    continuationManager.reset();
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    uint32_t versionCode = 0;
    continuationHandler->SetAbilityInfo(abilityInfo_);
    auto result = continuationHandler->HandleStartContinuationWithStack(continueToken, "deviceId", versionCode);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuationWithStack_003 end";
}

/*
 * @tc.number: continue_handler_HandleStartContinuationWithStack_004
 * @tc.name: HandleStartContinuationWithStack
 * @tc.desc: call HandleStartContinuationWithStack with OnContinue failed
 */
HWTEST_F(ContinuationTest, continue_handler_HandleStartContinuationWithStack_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuationWithStack_004 start";
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken, abilityInfo_, continuationHandler);
    uint32_t versionCode = 0;
    continuationHandler->SetAbilityInfo(abilityInfo_);
    auto result = continuationHandler->HandleStartContinuationWithStack(continueToken, "deviceId", versionCode);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuationWithStack_004 end";
}

/*
 * @tc.number: continue_handler_HandleStartContinuationWithStack_005
 * @tc.name: HandleStartContinuationWithStack
 * @tc.desc: call HandleStartContinuationWithStack success
 */
HWTEST_F(ContinuationTest, continue_handler_HandleStartContinuationWithStack_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuationWithStack_005 start";
    EXPECT_CALL(*mockAbility_, OnContinue(_)).Times(1).WillOnce(Return(ContinuationManager::OnContinueResult::AGREE));
    EXPECT_CALL(*mockAbility_, GetContentInfo()).Times(1).WillOnce(Return("ContentInfo"));
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    uint32_t versionCode = 0;
    ability_->abilityInfo_->isStageBasedModel = true;
    continuationHandler->SetAbilityInfo(abilityInfo_);
    auto result = continuationHandler->HandleStartContinuationWithStack(continueToken, "deviceId", versionCode);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_HandleStartContinuationWithStack_005 end";
}

/*
 * @tc.number: continue_manager_GetContinuationState_001
 * @tc.name: GetContinuationState
 * @tc.desc: call GetContinuationState success
 */
HWTEST_F(ContinuationTest, continue_manager_GetContinuationState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_GetContinuationState_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    auto result = static_cast<int>(continuationManager_->GetContinuationState());
    EXPECT_EQ(static_cast<int>(ContinuationState::LOCAL_RUNNING), result);
    GTEST_LOG_(INFO) << "continue_manager_GetContinuationState_001 end";
}

/*
 * @tc.number: continue_manager_GetOriginalDeviceId_001
 * @tc.name: GetOriginalDeviceId
 * @tc.desc: call GetOriginalDeviceId success
 */
HWTEST_F(ContinuationTest, continue_manager_GetOriginalDeviceId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_GetOriginalDeviceId_001 start";

    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->originalDeviceId_ = "deviceId";
    std::string result = continuationManager_->GetOriginalDeviceId();
    EXPECT_STREQ("deviceId", result.c_str());
    GTEST_LOG_(INFO) << "continue_manager_GetOriginalDeviceId_001 end";
}

/*
 * @tc.number: continue_manager_ContinueAbilityWithStack_001
 * @tc.name: ContinueAbilityWithStack
 * @tc.desc: call ContinueAbilityWithStack
 */
HWTEST_F(ContinuationTest, continue_manager_ContinueAbilityWithStack_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_ContinueAbilityWithStack_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    uint32_t versionCode = 0;
    mockAbility_->abilityInfo_->isStageBasedModel = true;
    continuationHandler->SetAbilityInfo(abilityInfo_);
    continuationManager_->ContinueAbilityWithStack("deviceId", versionCode);
    EXPECT_TRUE(continuationManager_->mainHandler_ != nullptr);
    GTEST_LOG_(INFO) << "continue_manager_ContinueAbilityWithStack_001 end";
}

/*
 * @tc.number: continue_manager_HandleContinueAbilityWithStack_001
 * @tc.name: HandleContinueAbilityWithStack
 * @tc.desc: call HandleContinueAbilityWithStack with checkAbilityToken failed
 */
HWTEST_F(ContinuationTest, continue_manager_HandleContinueAbilityWithStack_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_HandleContinueAbilityWithStack_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, nullptr, abilityInfo_, continuationHandler);
    uint32_t versionCode = 0;
    bool result = continuationManager_->HandleContinueAbilityWithStack("deviceId", versionCode);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_HandleContinueAbilityWithStack_001 end";
}

/*
 * @tc.number: continue_manager_HandleContinueAbilityWithStack_002
 * @tc.name: HandleContinueAbilityWithStack
 * @tc.desc: call HandleContinueAbilityWithStack with null continuationHandler 
 */
HWTEST_F(ContinuationTest, continue_manager_HandleContinueAbilityWithStack_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_HandleContinueAbilityWithStack_002 start";
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, nullptr);
    uint32_t versionCode = 0;
    bool result = continuationManager_->HandleContinueAbilityWithStack("deviceId", versionCode);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_HandleContinueAbilityWithStack_002 end";
}

/*
 * @tc.number: continue_manager_HandleContinueAbilityWithStack_003
 * @tc.name: HandleContinueAbilityWithStack
 * @tc.desc: call HandleContinueAbilityWithStack success
 */
HWTEST_F(ContinuationTest, continue_manager_HandleContinueAbilityWithStack_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_HandleContinueAbilityWithStack_003 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    uint32_t versionCode = 0;
    mockAbility_->abilityInfo_->isStageBasedModel = true;
    continuationHandler->SetAbilityInfo(abilityInfo_);
    bool result = continuationManager_->HandleContinueAbilityWithStack("deviceId", versionCode);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "continue_manager_HandleContinueAbilityWithStack_003 end";
}

/*
 * @tc.number: continue_manager_ContinueAbility_001
 * @tc.name: ContinueAbility
 * @tc.desc: call ContinueAbility with CheckContinuation Illegal
 */
HWTEST_F(ContinuationTest, continue_manager_ContinueAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_ContinueAbility_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    mockAbility_->abilityLifecycleExecutor_->DispatchLifecycleState(
        AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED);
    bool reversible = false;
    continuationManager_->ContinueAbility(reversible, "deviceId");
    EXPECT_TRUE(continuationManager_->CheckContinuationIllegal());
    GTEST_LOG_(INFO) << "continue_manager_ContinueAbility_001 end";
}

/*
 * @tc.number: continue_manager_ContinueAbility_002
 * @tc.name: ContinueAbility
 * @tc.desc: call ContinueAbility with progressState_ is IN_PROGRESS
 */
HWTEST_F(ContinuationTest, continue_manager_ContinueAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_ContinueAbility_002 start";
    bool reversible = false;
    continuationManager_->progressState_ = ContinuationManager::ProgressState::IN_PROGRESS;
    continuationManager_->ContinueAbility(reversible, "deviceId");
    EXPECT_NE(ContinuationManager::ProgressState::WAITING_SCHEDULE, continuationManager_->progressState_);
    GTEST_LOG_(INFO) << "continue_manager_ContinueAbility_002 end";
}

/*
 * @tc.number: continue_manager_ContinueAbility_003
 * @tc.name: ContinueAbility
 * @tc.desc: call ContinueAbility with continuationState_ is REMOTE_RUNNING
 */
HWTEST_F(ContinuationTest, continue_manager_ContinueAbility_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_ContinueAbility_003 start";
    bool reversible = false;
    continuationManager_->continuationState_ = ContinuationState::REMOTE_RUNNING;
    continuationManager_->ContinueAbility(reversible, "deviceId");
    EXPECT_NE(ContinuationManager::ProgressState::WAITING_SCHEDULE, continuationManager_->progressState_);
    GTEST_LOG_(INFO) << "continue_manager_ContinueAbility_003 end";
}

/*
 * @tc.number: continue_manager_ContinueAbility_004
 * @tc.name: ContinueAbility
 * @tc.desc: call ContinueAbility with HandleContinueAbility return true
 */
HWTEST_F(ContinuationTest, continue_manager_ContinueAbility_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_ContinueAbility_004 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    bool reversible = false;
    continuationManager_->ContinueAbility(reversible, "deviceId");
    EXPECT_EQ(ContinuationManager::ProgressState::WAITING_SCHEDULE, continuationManager_->progressState_);
    GTEST_LOG_(INFO) << "continue_manager_ContinueAbility_004 end";
}

/*
 * @tc.number: continue_manager_HandleContinueAbility_001
 * @tc.name: HandleContinueAbility
 * @tc.desc: call HandleContinueAbility with CheckAbilityToken failed
 */
HWTEST_F(ContinuationTest, continue_manager_HandleContinueAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_HandleContinueAbility_001 start";
    bool reversible = false;
    bool result = continuationManager_->HandleContinueAbility(reversible, "deviceId");
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_HandleContinueAbility_001 end";
}

/*
 * @tc.number: continue_manager_HandleContinueAbility_002
 * @tc.name: HandleContinueAbility
 * @tc.desc: call HandleContinueAbility with continuationHandler null
 */
HWTEST_F(ContinuationTest, continue_manager_HandleContinueAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_HandleContinueAbility_002 start";
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, nullptr);
    bool reversible = false;
    bool result = continuationManager_->HandleContinueAbility(reversible, "deviceId");
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_HandleContinueAbility_002 end";
}

/*
 * @tc.number: continue_manager_HandleContinueAbility_003
 * @tc.name: HandleContinueAbility
 * @tc.desc: call HandleContinueAbility success
 */
HWTEST_F(ContinuationTest, continue_manager_HandleContinueAbility_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_HandleContinueAbility_003 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    bool reversible = false;
    bool result = continuationManager_->HandleContinueAbility(reversible, "deviceId");
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "continue_manager_HandleContinueAbility_003 end";
}

/*
 * @tc.number: continue_manager_ReverseContinueAbility_001
 * @tc.name: ReverseContinueAbility
 * @tc.desc: call ReverseContinueAbility with wrong ProgressState_
 */
HWTEST_F(ContinuationTest, continue_manager_ReverseContinueAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_ReverseContinueAbility_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->progressState_ = ContinuationManager::ProgressState::WAITING_SCHEDULE;
    continuationManager_->continuationState_ = ContinuationState::REMOTE_RUNNING;
    bool result = continuationManager_->ReverseContinueAbility();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_ReverseContinueAbility_001 end";
}

/*
 * @tc.number: continue_manager_ReverseContinueAbility_002
 * @tc.name: ReverseContinueAbility
 * @tc.desc: call ReverseContinueAbility with wrong continuationState_
 */
HWTEST_F(ContinuationTest, continue_manager_ReverseContinueAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_ReverseContinueAbility_002 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->progressState_ = ContinuationManager::ProgressState::INITIAL;
    continuationManager_->continuationState_ = ContinuationState::LOCAL_RUNNING;
    bool result = continuationManager_->ReverseContinueAbility();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_ReverseContinueAbility_002 end";
}

/*
 * @tc.number: continue_manager_ReverseContinueAbility_003
 * @tc.name: ReverseContinueAbility
 * @tc.desc: call ReverseContinueAbility with null continuationHandler_
 */
HWTEST_F(ContinuationTest, continue_manager_ReverseContinueAbility_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_ReverseContinueAbility_003 start";
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, nullptr);
    continuationManager_->progressState_ = ContinuationManager::ProgressState::INITIAL;
    continuationManager_->continuationState_ = ContinuationState::REMOTE_RUNNING;
    bool result = continuationManager_->ReverseContinueAbility();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_ReverseContinueAbility_003 end";
}

/*
 * @tc.number: continue_manager_ReverseContinueAbility_004
 * @tc.name: ReverseContinueAbility
 * @tc.desc: call ReverseContinueAbility with requestSuccess is true
 */
HWTEST_F(ContinuationTest, continue_manager_ReverseContinueAbility_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_ReverseContinueAbility_004 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->continuationState_ = ContinuationState::REMOTE_RUNNING;
    sptr<IRemoteObject> stub = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    sptr<MockReverseContinuationSchedulerReplicaProxy> primary =
        new (std::nothrow) MockReverseContinuationSchedulerReplicaProxy(stub);
    EXPECT_CALL(*primary, ReverseContinuation()).Times(1).WillOnce(Return(true));
    continuationHandler->remoteReplicaProxy_ = primary;
    bool result = continuationManager_->ReverseContinueAbility();
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "continue_manager_ReverseContinueAbility_004 end";
}

/*
 * @tc.number: continue_manager_ReverseContinueAbility_005
 * @tc.name: ReverseContinueAbility
 * @tc.desc: call ReverseContinueAbility with requestSuccess is false
 */
HWTEST_F(ContinuationTest, continue_manager_ReverseContinueAbility_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_ReverseContinueAbility_005 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->continuationState_ = ContinuationState::REMOTE_RUNNING;
    sptr<IRemoteObject> stub = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    sptr<MockReverseContinuationSchedulerReplicaProxy> primary =
        new (std::nothrow) MockReverseContinuationSchedulerReplicaProxy(stub);
    EXPECT_CALL(*primary, ReverseContinuation()).Times(1).WillOnce(Return(false));
    continuationHandler->remoteReplicaProxy_ = primary;
    bool result = continuationManager_->ReverseContinueAbility();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_ReverseContinueAbility_005 end";
}

/*
 * @tc.number: continue_manager_OnContinue_001
 * @tc.name: OnContinue
 * @tc.desc: call OnContinue with ability_ is null or abilityInfo is nullptr
 */
HWTEST_F(ContinuationTest, continue_manager_OnContinue_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_OnContinue_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    ability_->abilityInfo_ = nullptr;
    int32_t result = continuationManager_->OnContinue(wantParams);
    EXPECT_EQ(ERR_INVALID_VALUE, result);
    continuationManager_->ability_.reset();
    result = continuationManager_->OnContinue(wantParams);
    EXPECT_EQ(ERR_INVALID_VALUE, result);
    GTEST_LOG_(INFO) << "continue_manager_OnContinue_001 end";
}

/*
 * @tc.number: continue_manager_OnContinue_002
 * @tc.name: OnContinue
 * @tc.desc: call OnContinue with stageBased is false
 */
HWTEST_F(ContinuationTest, continue_manager_OnContinue_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_OnContinue_002 start";
    EXPECT_CALL(*mockAbility_, OnStartContinuation()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAbility_, OnSaveData(_)).Times(1).WillOnce(Return(true));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    ability_->abilityInfo_->isStageBasedModel = false;
    int32_t result = continuationManager_->OnContinue(wantParams);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "continue_manager_OnContinue_002 end";
}

/*
 * @tc.number: continue_manager_OnContinue_003
 * @tc.name: OnContinue
 * @tc.desc: call OnContinue with stageBased is true
 */
HWTEST_F(ContinuationTest, continue_manager_OnContinue_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_OnContinue_003 start";
    EXPECT_CALL(*mockAbility_, OnContinue(_)).Times(1).WillOnce(Return(ContinuationManager::OnContinueResult::AGREE));
    EXPECT_CALL(*mockAbility_, GetContentInfo()).Times(1).WillOnce(Return("ContentInfo"));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    ability_->abilityInfo_->isStageBasedModel = true;
    int32_t result = continuationManager_->OnContinue(wantParams);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "continue_manager_OnContinue_003 end";
}

/*
 * @tc.number: continue_manager_OnStartAndSaveData_001
 * @tc.name: OnStartAndSaveData
 * @tc.desc: call OnStartAndSaveData with ability_ is null
 */
HWTEST_F(ContinuationTest, continue_manager_OnStartAndSaveData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_OnStartAndSaveData_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->ability_.reset();
    WantParams wantParams;
    int result = continuationManager_->OnStartAndSaveData(wantParams);
    EXPECT_EQ(ERR_INVALID_VALUE, result);
    GTEST_LOG_(INFO) << "continue_manager_OnStartAndSaveData_001 end";
}

/*
 * @tc.number: continue_manager_OnStartAndSaveData_002
 * @tc.name: OnStartAndSaveData
 * @tc.desc: call OnStartAndSaveData with OnStartContinuation failed
 */
HWTEST_F(ContinuationTest, continue_manager_OnStartAndSaveData_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_OnStartAndSaveData_002 start";
    EXPECT_CALL(*mockAbility_, OnStartContinuation()).Times(1).WillOnce(Return(false));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    int32_t result = continuationManager_->OnStartAndSaveData(wantParams);
    EXPECT_EQ(CONTINUE_ABILITY_REJECTED, result);
    GTEST_LOG_(INFO) << "continue_manager_OnStartAndSaveData_002 end";
}

/*
 * @tc.number: continue_manager_OnStartAndSaveData_003
 * @tc.name: OnStartAndSaveData
 * @tc.desc: call OnStartAndSaveData with OnSaveData failed
 */
HWTEST_F(ContinuationTest, continue_manager_OnStartAndSaveData_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_OnStartAndSaveData_003 start";
    EXPECT_CALL(*mockAbility_, OnStartContinuation()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAbility_, OnSaveData(_)).Times(1).WillOnce(Return(false));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    int32_t result = continuationManager_->OnStartAndSaveData(wantParams);
    EXPECT_EQ(CONTINUE_SAVE_DATA_FAILED, result);
    GTEST_LOG_(INFO) << "continue_manager_OnStartAndSaveData_003 end";
}

/*
 * @tc.number: continue_manager_OnStartAndSaveData_004
 * @tc.name: OnStartAndSaveData
 * @tc.desc: call OnStartAndSaveData with success
 */
HWTEST_F(ContinuationTest, continue_manager_OnStartAndSaveData_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_OnStartAndSaveData_004 start";
    EXPECT_CALL(*mockAbility_, OnStartContinuation()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAbility_, OnSaveData(_)).Times(1).WillOnce(Return(true));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    int32_t result = continuationManager_->OnStartAndSaveData(wantParams);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "continue_manager_OnStartAndSaveData_004 end";
}

/*
 * @tc.number: continue_manager_IsContinuePageStack_001
 * @tc.name: IsContinuePageStack
 * @tc.desc: call IsContinuePageStack success
 */
HWTEST_F(ContinuationTest, continue_manager_IsContinuePageStack_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_IsContinuePageStack_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    sptr<IInterface> iInterface = Boolean::Parse("true");
    wantParams.SetParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME, iInterface);
    bool result = continuationManager_->IsContinuePageStack(wantParams);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "continue_manager_IsContinuePageStack_001 end";
}

/*
 * @tc.number: continue_manager_IsContinuePageStack_002
 * @tc.name: IsContinuePageStack
 * @tc.desc: call IsContinuePageStack with no params
 */
HWTEST_F(ContinuationTest, continue_manager_IsContinuePageStack_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_IsContinuePageStack_002 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    sptr<IInterface> iInterface = Boolean::Parse("false");
    wantParams.SetParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME, iInterface);
    bool result = continuationManager_->IsContinuePageStack(wantParams);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_IsContinuePageStack_002 end";
}

/*
 * @tc.number: continue_manager_OnContinueAndGetContent_001
 * @tc.name: OnContinueAndGetContent
 * @tc.desc: call OnContinueAndGetContent with ability_ is null
 */
HWTEST_F(ContinuationTest, continue_manager_OnContinueAndGetContent_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_OnContinueAndGetContent_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->ability_.reset();
    WantParams wantParams;
    int32_t result = continuationManager_->OnContinueAndGetContent(wantParams);
    EXPECT_EQ(ERR_INVALID_VALUE, result);
    GTEST_LOG_(INFO) << "continue_manager_OnContinueAndGetContent_001 end";
}

/*
 * @tc.number: continue_manager_OnContinueAndGetContent_002
 * @tc.name: OnContinueAndGetContent
 * @tc.desc: call OnContinueAndGetContent with OnContinue failed
 * @tc.require: ljhCommit
 */
HWTEST_F(ContinuationTest, continue_manager_OnContinueAndGetContent_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_OnContinueAndGetContent_002 start";
    EXPECT_CALL(*mockAbility_, OnContinue(_)).Times(1).WillOnce(Return(ContinuationManager::OnContinueResult::REJECT));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    int32_t result = continuationManager_->OnContinueAndGetContent(wantParams);
    EXPECT_EQ(CONTINUE_ON_CONTINUE_FAILED, result);
    GTEST_LOG_(INFO) << "continue_manager_OnContinueAndGetContent_002 end";
}

/*
 * @tc.number: continue_manager_OnContinueAndGetContent_003
 * @tc.name: OnContinueAndGetContent
 * @tc.desc: call OnContinueAndGetContent with OnContinue version mismatch
 */
HWTEST_F(ContinuationTest, continue_manager_OnContinueAndGetContent_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_OnContinueAndGetContent_003 start";
    EXPECT_CALL(*mockAbility_, OnContinue(_)).Times(1).WillOnce(Return(ContinuationManager::OnContinueResult::MISMATCH));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    int32_t result = continuationManager_->OnContinueAndGetContent(wantParams);
    EXPECT_EQ(CONTINUE_ON_CONTINUE_MISMATCH, result);
    GTEST_LOG_(INFO) << "continue_manager_OnContinueAndGetContent_003 end";
}

/*
 * @tc.number: continue_manager_OnContinueAndGetContent_004
 * @tc.name: OnContinueAndGetContent
 * @tc.desc: call OnContinueAndGetContent with IsContinuePageStack is true but GetContentInfo failed
 */
HWTEST_F(ContinuationTest, continue_manager_OnContinueAndGetContent_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_OnContinueAndGetContent_004 start";

    EXPECT_CALL(*mockAbility_, OnContinue(_)).Times(1).WillOnce(Return(ContinuationManager::OnContinueResult::AGREE));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    int32_t result = continuationManager_->OnContinueAndGetContent(wantParams);
    EXPECT_EQ(CONTINUE_GET_CONTENT_FAILED, result);
    GTEST_LOG_(INFO) << "continue_manager_OnContinueAndGetContent_004 end";
}

/*
 * @tc.number: continue_manager_OnContinueAndGetContent_005
 * @tc.name: OnContinueAndGetContent
 * @tc.desc: call OnContinueAndGetContent success with IsContinuePageStack is true
 */
HWTEST_F(ContinuationTest, continue_manager_OnContinueAndGetContent_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_OnContinueAndGetContent_005 start";
    EXPECT_CALL(*mockAbility_, OnContinue(_)).Times(1).WillOnce(Return(ContinuationManager::OnContinueResult::AGREE));
    EXPECT_CALL(*mockAbility_, GetContentInfo()).Times(1).WillOnce(Return("ContentInfo"));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    int32_t result = continuationManager_->OnContinueAndGetContent(wantParams);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "continue_manager_OnContinueAndGetContent_005 end";
}

/*
 * @tc.number: continue_manager_OnContinueAndGetContent_006
 * @tc.name: OnContinueAndGetContent
 * @tc.desc: call OnContinueAndGetContent success with IsContinuePageStack is false
 */
HWTEST_F(ContinuationTest, continue_manager_OnContinueAndGetContent_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_OnContinueAndGetContent_006 start";
    EXPECT_CALL(*mockAbility_, OnContinue(_)).Times(1).WillOnce(Return(ContinuationManager::OnContinueResult::AGREE));
    EXPECT_CALL(*mockAbility_, GetContentInfo()).Times(1).WillOnce(Return("ContentInfo"));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    sptr<IInterface> iInterface = Boolean::Parse("true");
    wantParams.SetParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME, iInterface);
    int32_t result = continuationManager_->OnContinueAndGetContent(wantParams);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "continue_manager_OnContinueAndGetContent_006 end";
}

/*
 * @tc.number: continue_manager_RestoreData_001
 * @tc.name: RestoreData
 * @tc.desc: call RestoreData with DoScheduleRestoreData failed and reversible is true
 */
HWTEST_F(ContinuationTest, continue_manager_RestoreData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_RestoreData_001 start";
    EXPECT_CALL(*mockAbility_, OnRestoreData(_)).Times(1).WillOnce(Return(false));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    WantParams restoreData;
    bool reversible = true;
    bool result = continuationManager_->RestoreData(restoreData, reversible, "originalDeviceId");
    EXPECT_FALSE(result);
    EXPECT_EQ(ContinuationState::REPLICA_RUNNING, continuationManager_->continuationState_);
    GTEST_LOG_(INFO) << "continue_manager_RestoreData_001 end";
}

/*
 * @tc.number: continue_manager_RestoreData_002
 * @tc.name: RestoreData
 * @tc.desc: call RestoreData with reversible is false
 */
HWTEST_F(ContinuationTest, continue_manager_RestoreData_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_RestoreData_002 start";
    EXPECT_CALL(*mockAbility_, OnRestoreData(_)).Times(1).WillOnce(Return(true));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    WantParams restoreData;
    bool reversible = false;
    bool result = continuationManager_->RestoreData(restoreData, reversible, "originalDeviceId");
    EXPECT_TRUE(result);
    EXPECT_EQ(ContinuationState::LOCAL_RUNNING, continuationManager_->continuationState_);
    GTEST_LOG_(INFO) << "continue_manager_RestoreData_002 end";
}

/*
 * @tc.number: continue_manager_CompleteContinuation_001
 * @tc.name: CompleteContinuation
 * @tc.desc: call CompleteContinuation with CheckContinuation Illegal
 */
HWTEST_F(ContinuationTest, continue_manager_CompleteContinuation_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_CompleteContinuation_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    ability_->abilityLifecycleExecutor_->DispatchLifecycleState(
        AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED);
    int result = 0;
    continuationManager_->CompleteContinuation(result);
    EXPECT_TRUE(continuationManager_->CheckContinuationIllegal());
    GTEST_LOG_(INFO) << "continue_manager_CompleteContinuation_001 end";
}

/*
 * @tc.number: continue_manager_CompleteContinuation_002
 * @tc.name: CompleteContinuation
 * @tc.desc: call CompleteContinuation with ability is null
 */
HWTEST_F(ContinuationTest, continue_manager_CompleteContinuation_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_CompleteContinuation_002 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->ability_.reset();
    int result = 0;
    continuationManager_->progressState_ = ContinuationManager::ProgressState::WAITING_SCHEDULE;
    continuationManager_->CompleteContinuation(result);
    EXPECT_EQ(ContinuationManager::ProgressState::WAITING_SCHEDULE, continuationManager_->progressState_);
    GTEST_LOG_(INFO) << "continue_manager_CompleteContinuation_002 end";
}

/*
 * @tc.number: continue_manager_CompleteContinuation_003
 * @tc.name: CompleteContinuation
 * @tc.desc: call CompleteContinuation with result == 0 or 1 and reversible_ is true or false
 */
HWTEST_F(ContinuationTest, continue_manager_CompleteContinuation_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_CompleteContinuation_003 start";
    EXPECT_CALL(*mockAbility_, OnCompleteContinuation(_)).Times(4);
    EXPECT_CALL(*mockAbility_, TerminateAbility()).Times(2);
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    int resultZero = 0;
    int resultOne = 1;
    continuationManager_->reversible_ = true;
    continuationManager_->CompleteContinuation(resultZero);
    continuationManager_->CompleteContinuation(resultOne);
    continuationManager_->reversible_ = false;
    continuationManager_->CompleteContinuation(resultZero);
    continuationManager_->CompleteContinuation(resultOne);
    GTEST_LOG_(INFO) << "continue_manager_CompleteContinuation_003 end";
}

/*
 * @tc.number: continue_manager_RestoreFromRemote_001
 * @tc.name: RestoreFromRemote
 * @tc.desc: call RestoreFromRemote with DoRestoreFromRemote failed
 */
HWTEST_F(ContinuationTest, continue_manager_RestoreFromRemote_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_RestoreFromRemote_001 start";
    EXPECT_CALL(*mockAbility_, OnRemoteTerminated()).Times(0);
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->ability_.reset();
    WantParams restoreData;
    bool result = continuationManager_->RestoreFromRemote(restoreData);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_RestoreFromRemote_001 end";
}

/*
 * @tc.number: continue_manager_RestoreFromRemote_002
 * @tc.name: RestoreFromRemote
 * @tc.desc: call RestoreFromRemote success
 */
HWTEST_F(ContinuationTest, continue_manager_RestoreFromRemote_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_RestoreFromRemote_002 start";
    EXPECT_CALL(*mockAbility_, OnRestoreData(_)).Times(1).WillOnce(Return(true));
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    WantParams restoreData;
    bool result = continuationManager_->RestoreFromRemote(restoreData);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "continue_manager_RestoreFromRemote_002 end";
}

/*
 * @tc.number: continue_manager_NotifyRemoteTerminated_001
 * @tc.name: NotifyRemoteTerminated
 * @tc.desc: call NotifyRemoteTerminated with null ability_
 */
HWTEST_F(ContinuationTest, continue_manager_NotifyRemoteTerminated_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_NotifyRemoteTerminated_001 start";
    EXPECT_CALL(*mockAbility_, OnRemoteTerminated()).Times(0);
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->ability_.reset();
    continuationManager_->NotifyRemoteTerminated();
    GTEST_LOG_(INFO) << "continue_manager_NotifyRemoteTerminated_001 end";
}

/*
 * @tc.number: continue_manager_NotifyRemoteTerminated_002
 * @tc.name: NotifyRemoteTerminated
 * @tc.desc: call NotifyRemoteTerminated success
 */
HWTEST_F(ContinuationTest, continue_manager_NotifyRemoteTerminated_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_NotifyRemoteTerminated_002 start";
    EXPECT_CALL(*mockAbility_, OnRemoteTerminated()).Times(1);
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->NotifyRemoteTerminated();
    GTEST_LOG_(INFO) << "continue_manager_NotifyRemoteTerminated_002 end";
}

/*
 * @tc.number: continue_manager_ChangeProcessStateToInit_001
 * @tc.name: ChangeProcessStateToInit
 * @tc.desc: call ChangeProcessStateToInit with null mainHandler_
 */
HWTEST_F(ContinuationTest, continue_manager_ChangeProcessStateToInit_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_ChangeProcessStateToInit_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->progressState_ = ContinuationManager::ProgressState::WAITING_SCHEDULE;
    continuationManager_->ChangeProcessStateToInit();
    EXPECT_EQ(ContinuationManager::ProgressState::INITIAL, continuationManager_->progressState_);
    GTEST_LOG_(INFO) << "continue_manager_ChangeProcessStateToInit_001 end";
}

/*
 * @tc.number: continue_manager_ChangeProcessStateToInit_002
 * @tc.name: ChangeProcessStateToInit
 * @tc.desc: call ChangeProcessStateToInit init success
 */
HWTEST_F(ContinuationTest, continue_manager_ChangeProcessStateToInit_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_ChangeProcessStateToInit_002 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->InitMainHandlerIfNeed();
    continuationManager_->progressState_ = ContinuationManager::ProgressState::WAITING_SCHEDULE;
    continuationManager_->ChangeProcessStateToInit();
    EXPECT_EQ(ContinuationManager::ProgressState::INITIAL, continuationManager_->progressState_);
    GTEST_LOG_(INFO) << "continue_manager_ChangeProcessStateToInit_002 end";
}

/*
 * @tc.name: continue_manager_DoScheduleStartContinuation_001
 * @tc.name: DoScheduleStartContinuation
 * @tc.desc: call DoScheduleStartContinuation with CheckContinuationIllegal failed
 */
HWTEST_F(ContinuationTest, continue_manager_DoScheduleStartContinuation_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_DoScheduleStartContinuation_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    mockAbility_->abilityLifecycleExecutor_->DispatchLifecycleState(
        AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED);
    bool result = continuationManager_->DoScheduleStartContinuation();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_DoScheduleStartContinuation_001 end";
}

/*
 * @tc.name: continue_manager_DoScheduleSaveData_001
 * @tc.name: DoScheduleSaveData
 * @tc.desc: call DoScheduleSaveData with CheckContinuationIllegal failed
 */
HWTEST_F(ContinuationTest, continue_manager_DoScheduleSaveData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_DoScheduleSaveData_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    mockAbility_->abilityLifecycleExecutor_->DispatchLifecycleState(
        AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED);
    WantParams saveData;
    bool result = continuationManager_->DoScheduleSaveData(saveData);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_DoScheduleSaveData_001 end";
}

/*
 * @tc.name: continue_manager_DoScheduleSaveData_002
 * @tc.name: DoScheduleSaveData
 * @tc.desc: call DoScheduleSaveData with ability is null
 */
HWTEST_F(ContinuationTest, continue_manager_DoScheduleSaveData_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_DoScheduleSaveData_002 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->ability_.reset();
    WantParams saveData;
    bool result = continuationManager_->DoScheduleSaveData(saveData);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_DoScheduleSaveData_002 end";
}

/*
 * @tc.name: continue_manager_DoScheduleRestoreData_001
 * @tc.name: DoScheduleRestoreData
 * @tc.desc: call DoScheduleRestoreData with CheckContinuationIllegal failed
 */
HWTEST_F(ContinuationTest, continue_manager_DoScheduleRestoreData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_DoScheduleRestoreData_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    mockAbility_->abilityLifecycleExecutor_->DispatchLifecycleState(
        AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED);
    WantParams restoreData;
    bool result = continuationManager_->DoScheduleRestoreData(restoreData);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_DoScheduleSaveData_001 end";
}

/*
 * @tc.name: continue_manager_DoScheduleRestoreData_002
 * @tc.name: DoScheduleRestoreData
 * @tc.desc: call DoScheduleRestoreData with ability is null
 */
HWTEST_F(ContinuationTest, continue_manager_DoScheduleRestoreData_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_DoScheduleRestoreData_002 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(mockAbility_, continueToken_, abilityInfo_, continuationHandler);
    continuationManager_->ability_.reset();
    WantParams restoreData;
    bool result = continuationManager_->DoScheduleRestoreData(restoreData);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_DoScheduleRestoreData_002 end";
}

/*
 * @tc.number: continue_handler_OnReplicaDied_001
 * @tc.name: OnReplicaDied
 * @tc.desc: call OnReplicaDied with schedulerDeathRecipient_ is null
 */
HWTEST_F(ContinuationTest, continue_handler_OnReplicaDied_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_OnReplicaDied_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> stub = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    sptr<MockReverseContinuationSchedulerReplicaProxy> primary =
        new (std::nothrow) MockReverseContinuationSchedulerReplicaProxy(stub);
    continuationHandler->remoteReplicaProxy_ = primary;
    continuationHandler->OnReplicaDied(primary->AsObject());
    EXPECT_TRUE(continuationHandler->remoteReplicaProxy_ == nullptr);
    GTEST_LOG_(INFO) << "continue_handler_OnReplicaDied_001 end";
}

/*
 * @tc.number: continue_handler_OnReplicaDied_002
 * @tc.name: OnReplicaDied
 * @tc.desc: call OnReplicaDied with remoteReplicaProxy_ is null
 */
HWTEST_F(ContinuationTest, continue_handler_OnReplicaDied_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_OnReplicaDied_002 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> stub = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    sptr<MockReverseContinuationSchedulerReplicaProxy> primary =
        new (std::nothrow) MockReverseContinuationSchedulerReplicaProxy(stub);
    continuationHandler->OnReplicaDied(primary->AsObject());
    EXPECT_TRUE(continuationHandler->remoteReplicaProxy_ == nullptr);
    GTEST_LOG_(INFO) << "continue_handler_OnReplicaDied_002 end";
}

/*
 * @tc.number: continue_handler_OnReplicaDied_003
 * @tc.name: OnReplicaDied
 * @tc.desc: call OnReplicaDied with remote is null
 */
HWTEST_F(ContinuationTest, continue_handler_OnReplicaDied_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_OnReplicaDied_003 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> stub = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    sptr<MockReverseContinuationSchedulerReplicaProxy> primary =
        new (std::nothrow) MockReverseContinuationSchedulerReplicaProxy(stub);
    continuationHandler->remoteReplicaProxy_ = primary;
    continuationHandler->OnReplicaDied(nullptr);
    EXPECT_TRUE(continuationHandler->remoteReplicaProxy_ != nullptr);
    GTEST_LOG_(INFO) << "continue_handler_OnReplicaDied_003 end";
}

/*
 * @tc.number: continue_handler_OnReplicaDied_004
 * @tc.name: OnReplicaDied
 * @tc.desc: call OnReplicaDied with schedulerDeathRecipient_ is not null
 */
HWTEST_F(ContinuationTest, continue_handler_OnReplicaDied_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_OnReplicaDied_004 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> stub = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    sptr<MockReverseContinuationSchedulerReplicaProxy> primary =
        new (std::nothrow) MockReverseContinuationSchedulerReplicaProxy(stub);
    continuationHandler->remoteReplicaProxy_ = primary;
    continuationHandler->HandleReceiveRemoteScheduler(stub);
    continuationHandler->OnReplicaDied(primary->AsObject());
    EXPECT_TRUE(continuationHandler->remoteReplicaProxy_ == nullptr);
    GTEST_LOG_(INFO) << "continue_handler_OnReplicaDied_004 end";
}

/*
 * @tc.number: continue_handler_OnReplicaDied_005
 * @tc.name: OnReplicaDied
 * @tc.desc: call OnReplicaDied with remoteReplica is not matches with remote
 */
HWTEST_F(ContinuationTest, continue_handler_OnReplicaDied_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_OnReplicaDied_005 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> stub = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    sptr<IRemoteObject> stubTwo = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    sptr<MockReverseContinuationSchedulerReplicaProxy> primary =
        new (std::nothrow) MockReverseContinuationSchedulerReplicaProxy(stub);
    continuationHandler->remoteReplicaProxy_ = primary;
    continuationHandler->OnReplicaDied(stubTwo);
    EXPECT_TRUE(continuationHandler->remoteReplicaProxy_ != nullptr);
    GTEST_LOG_(INFO) << "continue_handler_OnReplicaDied_005 end";
}

/*
 * @tc.number: continue_handler_SetWantParams_001
 * @tc.name: SetWantParams
 * @tc.desc: call SetWantParams with reversible_ is true and launchMode is STANDARD
 */
HWTEST_F(ContinuationTest, continue_handler_SetWantParams_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_SetWantParams_001 start";
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = mockAbility_;
    auto continuationHandler = std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    WantParams wantParams;
    abilityInfo_->launchMode = LaunchMode::STANDARD;
    continuationHandler->abilityInfo_ = abilityInfo_;
    continuationHandler->reversible_ = true;
    auto result = continuationHandler->SetWantParams(wantParams);
    auto flags = result.GetFlags();
    EXPECT_EQ(1032U, flags);
    GTEST_LOG_(INFO) << "continue_handler_SetWantParams_001 end";
}
#ifdef SUPPORT_GRAPHICS
/*
 * @tc.number: continue_manager_GetContentInfo_001
 * @tc.name: GetContentInfo
 * @tc.desc: call GetContentInfo with ability is null
 */
HWTEST_F(ContinuationTest, continue_manager_GetContentInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_GetContentInfo_001 start";
    WantParams wantParams;
    bool result = continuationManager_->GetContentInfo(wantParams);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_GetContentInfo_001 end";
}
#endif
/*
 * @tc.number: continue_manager_NotifyCompleteContinuation_001
 * @tc.name: NotifyCompleteContinuation
 * @tc.desc: call NotifyCompleteContinuation success
 */
HWTEST_F(ContinuationTest, continue_manager_NotifyCompleteContinuation_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_GetContentInfo_001 start";
    WantParams wantParams;
    int sessionId = 0;
    bool success = true;
    sptr<IRemoteObject> reverseScheduler = new (std::nothrow) MockReverseContinuationSchedulerReplicaStub();
    continuationManager_->NotifyCompleteContinuation("originDeviceId", sessionId, success, reverseScheduler);
    GTEST_LOG_(INFO) << "continue_manager_NotifyCompleteContinuation_001 end";
}

/*
 * @tc.number: continue_manager_GetProcessState_001
 * @tc.name: GetProcessState
 * @tc.desc: call GetProcessState success
 */
HWTEST_F(ContinuationTest, continue_manager_GetProcessState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_GetProcessState_001 start";
    auto result = continuationManager_->GetProcessState();
    EXPECT_EQ(ContinuationManager::ProgressState::INITIAL, result);
    GTEST_LOG_(INFO) << "continue_manager_GetProcessState_001 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
