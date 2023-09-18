/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include <iremote_object.h>
#include <iremote_stub.h>

#include "ability_connect_callback_interface.h"
#include "ability_loader.h"
#include "ability_thread.h"
#include "fa_ability_thread.h"
#define private public
#define protected public
#include "ability_record.h"
#include "call_record.h"
#include "mission.h"
#include "mission_info_mgr.h"
#include "mission_list.h"
#include "mission_list_manager.h"

namespace OHOS {
namespace AAFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace {
}

class MissionListManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    std::unique_ptr<MissionListManager> missionListMgr_ = nullptr;
};

void MissionListManagerTest::SetUpTestCase(void)
{}

void MissionListManagerTest::TearDownTestCase(void)
{}

void MissionListManagerTest::SetUp(void)
{}

void MissionListManagerTest::TearDown(void)
{}

class MissionListManagerTestStub : public IRemoteStub<IAbilityConnection> {
public:
    MissionListManagerTestStub() {};
    virtual ~MissionListManagerTestStub() {};

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return 0;
    };

    virtual void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) {};

    /**
     * OnAbilityDisconnectDone, AbilityMs notify caller ability the result of disconnect.
     *
     * @param element, service ability's ElementName.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    virtual void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) {};
};

class MissionListManagerTestAbilityThreadStub : public AbilityRuntime::FAAbilityThread {
public:
    MissionListManagerTestAbilityThreadStub() {};
    ~MissionListManagerTestAbilityThreadStub() {};

    void CallRequest()
    {
        return;
    }
};

/**
 * @tc.number: MissionListManager_001
 * @tc.name: OnCallConnectDied
 * @tc.desc: MissionListManager to process OnCallConnectDied success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_001 begin";
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;

    int32_t callerUid;
    sptr<IAbilityConnection> connCallback = new (std::nothrow) MissionListManagerTestStub();
    sptr<IRemoteObject> callToken = nullptr;
    AppExecFwk::ElementName element;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<CallRecord> callRecord =
        std::make_shared<CallRecord>(callerUid, abilityRecord, connCallback, callToken);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord, "");
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();

    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;
    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);

    auto testValue = static_cast<int>(callContainer->callRecordMap_.size());
    EXPECT_EQ(1, testValue);
    missionListMgr->OnCallConnectDied(callRecord);

    testValue = static_cast<int>(callContainer->callRecordMap_.size());
    EXPECT_EQ(0, testValue);
    GTEST_LOG_(INFO) << "MissionListManager_001 end";
}

/**
 * @tc.number: MissionListManager_002
 * @tc.name: OnCallConnectDied
 * @tc.desc: MissionListManager to process OnCallConnectDied success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_002 begin";
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;

    int32_t callerUid;
    sptr<IAbilityConnection> connCallback = new (std::nothrow) MissionListManagerTestStub();
    sptr<IRemoteObject> callToken = nullptr;
    AppExecFwk::ElementName element;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<CallRecord> callRecord =
        std::make_shared<CallRecord>(callerUid, abilityRecord, connCallback, callToken);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord, "");
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();

    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;
    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);

    auto testValue = static_cast<int>(callContainer->callRecordMap_.size());
    EXPECT_EQ(1, testValue);
    missionListMgr->OnCallConnectDied(nullptr);

    testValue = static_cast<int>(callContainer->callRecordMap_.size());
    EXPECT_EQ(1, testValue);
    GTEST_LOG_(INFO) << "MissionListManager_002 end";
}

/**
 * @tc.number: MissionListManager_003
 * @tc.name: OnCallConnectDied
 * @tc.desc: MissionListManager to process OnCallConnectDied success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_003 begin";
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;

    sptr<IAbilityConnection> connCallback = new (std::nothrow) MissionListManagerTestStub();
    sptr<IAbilityConnection> connCallback1 = new (std::nothrow) MissionListManagerTestStub();
    sptr<IRemoteObject> callToken = nullptr;
    AppExecFwk::ElementName element;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<CallRecord> callRecord =
        std::make_shared<CallRecord>(1, abilityRecord, connCallback, callToken);
    std::shared_ptr<CallRecord> callRecord1 =
        std::make_shared<CallRecord>(2, abilityRecord, connCallback1, callToken);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord, "");
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();

    callContainer->AddCallRecord(connCallback1, callRecord1);
    abilityRecord->callContainer_ = callContainer;
    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);

    auto testValue = static_cast<int>(callContainer->callRecordMap_.size());
    EXPECT_EQ(1, testValue);
    missionListMgr->OnCallConnectDied(callRecord);

    testValue = static_cast<int>(callContainer->callRecordMap_.size());
    EXPECT_EQ(1, testValue);
    GTEST_LOG_(INFO) << "MissionListManager_003 end";
}

/**
 * @tc.number: MissionListManager_004
 * @tc.name: CallAbilityLocked
 * @tc.desc: MissionListManager to process CallAbilityLocked success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_004, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_004 begin";

    AbilityRequest abilityRequest;
    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);

    int testRet = missionListMgr->CallAbilityLocked(abilityRequest);

    EXPECT_EQ(ERR_INVALID_VALUE, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_004 end";
}

/**
 * @tc.number: MissionListManager_005
 * @tc.name: CallAbilityLocked
 * @tc.desc: MissionListManager to process CallAbilityLocked success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_005, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_005 begin";

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.abilityInfo.bundleName = "test_bundle";
    abilityRequest.abilityInfo.name = "test_name";
    abilityRequest.abilityInfo.moduleName = "test_moduleName";
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;

    Want want;
    AppExecFwk::AbilityInfo abilityInfo = abilityRequest.abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    int32_t id;
    std::string missionName = "#" + abilityRequest.abilityInfo.bundleName + ":" +
        abilityRequest.abilityInfo.moduleName + ":" + abilityRequest.abilityInfo.name;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    missionListMgr->Init();
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(id, abilityRecord, missionName);
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();
    sptr<MissionListManagerTestStub> connCallback = new (std::nothrow) MissionListManagerTestStub();
    std::shared_ptr<CallRecord> callRecord = std::make_shared<CallRecord>(0, abilityRecord, connCallback, nullptr);

    callRecord->connCallback_ = connCallback;
    callRecord->callRemoteObject_ = connCallback->AsObject();
    callRecord->SetCallState(CallState::INIT);
    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;
    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);

    int testRet = missionListMgr->CallAbilityLocked(abilityRequest);

    EXPECT_EQ(RESOLVE_CALL_ABILITY_INNER_ERR, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_005 end";
}

/**
 * @tc.number: MissionListManager_006
 * @tc.name: CallAbilityLocked
 * @tc.desc: MissionListManager to process CallAbilityLocked success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_006, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_006 begin";

    sptr<MissionListManagerTestStub> connCallback = new (std::nothrow) MissionListManagerTestStub();
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.abilityInfo.bundleName = "test_bundle";
    abilityRequest.abilityInfo.name = "test_name";
    abilityRequest.abilityInfo.moduleName = "test_moduleName";
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    abilityRequest.connect = connCallback;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo = abilityRequest.abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    int32_t id;
    std::string missionName = "#" + abilityRequest.abilityInfo.bundleName + ":" +
        abilityRequest.abilityInfo.moduleName + ":" + abilityRequest.abilityInfo.name;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(id, abilityRecord, missionName);
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();
    std::shared_ptr<CallRecord> callRecord = std::make_shared<CallRecord>(0, abilityRecord, connCallback, nullptr);
    callRecord->connCallback_ = connCallback;
    callRecord->callRemoteObject_ = connCallback->AsObject();
    callRecord->SetCallState(CallState::REQUESTED);
    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;
    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);

    int testRet = missionListMgr->CallAbilityLocked(abilityRequest);

    EXPECT_NE(ERR_OK, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_006 end";
}

/**
 * @tc.number: MissionListManager_007
 * @tc.name: ResolveAbility
 * @tc.desc: MissionListManager to process ResolveAbility success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_007, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_007 begin";

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);

    int testRet = missionListMgr->ResolveAbility(abilityRecord, abilityRequest);

    EXPECT_EQ(ResolveResultType::NG_INNER_ERROR, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_007 end";
}


/**
 * @tc.number: MissionListManager_008
 * @tc.name: GetAbilityRecordByName
 * @tc.desc: MissionListManager to process GetAbilityRecordByName success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_008, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_008 begin";

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    AppExecFwk::ElementName element;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();

    missionListMgr->currentMissionLists_.push_front(missionList);
    missionListMgr->launcherList_ = missionList;
    missionListMgr->defaultStandardList_ = missionList;
    missionListMgr->defaultSingleList_ = missionList;

    auto testRet = missionListMgr->GetAbilityRecordByName(element);

    EXPECT_TRUE(nullptr == testRet);
    GTEST_LOG_(INFO) << "MissionListManager_008 end";
}

/**
 * @tc.number: MissionListManager_009
 * @tc.name: GetAbilityRecordByName
 * @tc.desc: MissionListManager to process GetAbilityRecordByName success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_009, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_009 begin";

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    AppExecFwk::ElementName element;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord, "");

    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);

    auto testRet = missionListMgr->GetAbilityRecordByName(element);

    EXPECT_TRUE(nullptr != testRet);
    GTEST_LOG_(INFO) << "MissionListManager_009 end";
}

/**
 * @tc.number: MissionListManager_010
 * @tc.name: ResolveAbility
 * @tc.desc: MissionListManager to process ResolveAbility success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_010, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_010 begin";

    sptr<MissionListManagerTestStub> connCallback = new (std::nothrow) MissionListManagerTestStub();
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = connCallback;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<CallRecord> callRecord = std::make_shared<CallRecord>(0, abilityRecord, connCallback, nullptr);
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();

    callRecord->connCallback_ = connCallback;
    callRecord->callRemoteObject_ = connCallback->AsObject();
    callRecord->SetCallState(CallState::REQUESTED);
    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;

    int testRet = missionListMgr->ResolveAbility(abilityRecord, abilityRequest);

    EXPECT_NE(ResolveResultType::OK_HAS_REMOTE_OBJ, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_010 end";
}

/**
 * @tc.number: MissionListManager_011
 * @tc.name: ResolveAbility
 * @tc.desc: MissionListManager to process ResolveAbility success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_011, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_011 begin";

    sptr<MissionListManagerTestStub> connCallback = new (std::nothrow) MissionListManagerTestStub();
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = connCallback;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<CallRecord> callRecord = std::make_shared<CallRecord>(0, abilityRecord, connCallback, nullptr);
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();

    callRecord->connCallback_ = connCallback;
    callRecord->callRemoteObject_ = connCallback->AsObject();
    callRecord->SetCallState(CallState::INIT);
    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;
    abilityRecord->isReady_ = true;
    abilityRecord->scheduler_ = new (std::nothrow) MissionListManagerTestAbilityThreadStub();

    int testRet = missionListMgr->ResolveAbility(abilityRecord, abilityRequest);

    EXPECT_EQ(ResolveResultType::OK_HAS_REMOTE_OBJ, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_011 end";
}

/**
 * @tc.number: MissionListManager_012
 * @tc.name: ResolveAbility
 * @tc.desc: MissionListManager to process ResolveAbility success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_012, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_012 begin";

    sptr<MissionListManagerTestStub> connCallback = new (std::nothrow) MissionListManagerTestStub();
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = connCallback;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<CallRecord> callRecord = std::make_shared<CallRecord>(0, abilityRecord, connCallback, nullptr);
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();

    callRecord->connCallback_ = connCallback;
    callRecord->callRemoteObject_ = connCallback->AsObject();
    callRecord->SetCallState(CallState::INIT);
    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;
    abilityRecord->isReady_ = false;

    int testRet = missionListMgr->ResolveAbility(abilityRecord, abilityRequest);

    EXPECT_EQ(ResolveResultType::OK_NO_REMOTE_OBJ, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_012 end";
}

/**
 * @tc.number: MissionListManager_013
 * @tc.name: ResolveLocked
 * @tc.desc: MissionListManager to process ResolveLocked success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_013, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_013 begin";
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::INVALID_TYPE;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);

    int testRet = missionListMgr->ResolveLocked(abilityRequest);

    EXPECT_EQ(RESOLVE_CALL_ABILITY_INNER_ERR, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_013 end";
}


/**
 * @tc.number: MissionListManager_014
 * @tc.name: ResolveLocked
 * @tc.desc: MissionListManager to process ResolveLocked success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_014, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_014 begin";


    sptr<MissionListManagerTestStub> connCallback = new (std::nothrow) MissionListManagerTestStub();
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.abilityInfo.bundleName = "test_bundle";
    abilityRequest.abilityInfo.name = "test_name";
    abilityRequest.abilityInfo.moduleName = "test_moduleName";
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    abilityRequest.connect = connCallback;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo = abilityRequest.abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    int32_t id;
    std::string missionName = "#" + abilityRequest.abilityInfo.bundleName + ":" +
        abilityRequest.abilityInfo.moduleName + ":" + abilityRequest.abilityInfo.name;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(id, abilityRecord, missionName);
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();
    std::shared_ptr<CallRecord> callRecord = std::make_shared<CallRecord>(0, abilityRecord, connCallback, nullptr);
    callRecord->connCallback_ = connCallback;
    callRecord->callRemoteObject_ = connCallback->AsObject();
    callRecord->SetCallState(CallState::REQUESTED);
    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;
    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);

    int testRet = missionListMgr->ResolveLocked(abilityRequest);

    EXPECT_NE(ERR_OK, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_014 end";
}

/**
 * @tc.number: MissionListManager_015
 * @tc.name: CallAbilityLocked
 * @tc.desc: MissionListManager test CallAbilityLocked is not SINGLETON.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_015, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_015 begin";
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    abilityInfo.applicationInfo = applicationInfo;
    want.SetElementName("test.bundle.name", "test.ability.name");
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.abilityInfo.bundleName = "test_bundle";
    abilityRequest.abilityInfo.name = "test_name";
    abilityRequest.abilityInfo.moduleName = "test_moduleName";
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    abilityRequest.startRecent = true;
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, abilityInfo.applicationInfo);
    abilityRecord->isReady_ = true;
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    missionListMgr->defaultStandardList_ = missionList;
    std::shared_ptr<Mission> mission =
        std::make_shared<Mission>(0, abilityRecord, missionListMgr->GetMissionName(abilityRequest));
    missionList->AddMissionToTop(mission);
    EXPECT_EQ(RESOLVE_CALL_ABILITY_INNER_ERR, missionListMgr->CallAbilityLocked(abilityRequest));
    GTEST_LOG_(INFO) << "MissionListManager_015 end";
}

/**
 * @tc.number: MissionListManager_016
 * @tc.name: GetAbilityRecordsByName
 * @tc.desc: MissionListManager to process GetAbilityRecordsByName success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_016, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_016 begin";
    AppExecFwk::ElementName element;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord, "");
    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);
    missionListMgr->currentMissionLists_.push_front(nullptr);
    missionListMgr->launcherList_ = missionList;
    missionListMgr->defaultStandardList_ = missionList;
    auto ret = missionListMgr->GetAbilityRecordsByName(element);
    EXPECT_FALSE(ret.empty());
    GTEST_LOG_(INFO) << "MissionListManager_016 end";
}

/**
 * @tc.number: MissionListManager_017
 * @tc.name: GetAbilityRecordsByName
 * @tc.desc: MissionListManager to process GetAbilityRecordsByName success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_017, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_017 begin";
    AppExecFwk::ElementName element;
    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    auto ret = missionListMgr->GetAbilityRecordsByName(element);
    EXPECT_TRUE(ret.empty());
    GTEST_LOG_(INFO) << "MissionListManager_017 end";
}

/**
 * @tc.number: MissionListManager_018
 * @tc.name: GetAbilityRecordsByName
 * @tc.desc: MissionListManager to process GetAbilityRecordsByName success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_018, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_018 begin";
    AppExecFwk::ElementName element;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord, "");
    missionListMgr->defaultSingleList_ = missionList;
    auto ret = missionListMgr->GetAbilityRecordsByName(element);
    EXPECT_TRUE(ret.empty());
    GTEST_LOG_(INFO) << "MissionListManager_018 end";
}

/**
 * @tc.number: MissionListManager_019
 * @tc.name: ReleaseCallLocked
 * @tc.desc: call ReleaseCallLocked interface and find_if fails and return RELEASE_CALL_ABILITY_INNER_ERR.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_019, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_019 begin";
    sptr<IAbilityConnection> connect = new (std::nothrow) MissionListManagerTestStub();
    AppExecFwk::ElementName element;
    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    missionListMgr->currentMissionLists_.push_front(nullptr);
    auto ret = missionListMgr->ReleaseCallLocked(connect, element);
    EXPECT_EQ(ret, RELEASE_CALL_ABILITY_INNER_ERR);
    GTEST_LOG_(INFO) << "MissionListManager_019 end";
}

/**
 * @tc.number: MissionListManager_020
 * @tc.name: ReleaseCallLocked
 * @tc.desc: call ReleaseCallLocked interface and return ERR_OK.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_020, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_020 begin";
    sptr<IAbilityConnection> connect = new (std::nothrow) MissionListManagerTestStub();
    sptr<IRemoteObject> callToken = nullptr;
    AppExecFwk::ElementName element;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<CallRecord> callRecord =
        std::make_shared<CallRecord>(2, abilityRecord, connect, callToken);
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord, "");
    abilityRecord->callContainer_ = callContainer;
    callContainer->AddCallRecord(connect, callRecord);
    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);
    auto ret = missionListMgr->ReleaseCallLocked(connect, element);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "MissionListManager_020 end";
}
}  // namespace AAFwk
}  // namespace OHOS
