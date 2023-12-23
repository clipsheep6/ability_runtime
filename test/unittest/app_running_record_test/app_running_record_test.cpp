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
#include "ability_running_record.h"
#include "ability_window_configuration.h"
#include "app_running_record.h"
#include "app_mgr_service_event_handler.h"
#include "app_mgr_service_inner.h"
#include "child_process_record.h"
#include "module_running_record.h"
#include "want_params.h"
#undef private
#undef protected
#include "hilog_wrapper.h"
#include "mock_ability_token.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int64_t TIMEOUT = 1;
constexpr int32_t RECORD_ID = 1;
constexpr int32_t EVENT_ID_1 = 1;
constexpr int32_t EVENT_ID_2 = 2;
constexpr int32_t START_PROCESS_SPECIFY_ABILITY_EVENT_ID = 1;
constexpr int32_t ADD_ABILITY_STAGE_INFO_EVENT_ID = 2;
constexpr int32_t TERMINATE_ABILITY_SIZE = 0;
}
class AppRunningRecordTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppRunningRecordTest::SetUpTestCase(void)
{}

void AppRunningRecordTest::TearDownTestCase(void)
{}

void AppRunningRecordTest::SetUp()
{}

void AppRunningRecordTest::TearDown()
{}

/**
 * @tc.name: AppRunningRecord_SendEvent_0100
 * @tc.desc: Test the status of SendEvent when msg is a value of 4.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SendEvent_0100, TestSize.Level1)
{
    uint32_t msg = AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG;
    int64_t timeOut = TIMEOUT;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRunningRecord, nullptr);
    appRunningRecord->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appRunningRecord->eventHandler_ =
        std::make_shared<AMSEventHandler>(appRunningRecord->taskHandler_, appRunningRecord->appMgrServiceInner_);
    appRunningRecord->isDebugApp_ = false;
    appRunningRecord->isNativeDebug_ = false;
    appRunningRecord->isAttachDebug_ = false;
    appRunningRecord->SendEvent(msg, timeOut);
    EXPECT_EQ(appRunningRecord->eventId_, EVENT_ID_1);
    EXPECT_EQ(appRunningRecord->startProcessSpecifiedAbilityEventId_, START_PROCESS_SPECIFY_ABILITY_EVENT_ID);
}

/**
 * @tc.name: AppRunningRecord_SendEvent_0200
 * @tc.desc: Test the status of SendEvent when msg is a value of 2.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SendEvent_0200, TestSize.Level1)
{
    uint32_t msg = AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG;
    int64_t timeOut = TIMEOUT;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRunningRecord, nullptr);
    appRunningRecord->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appRunningRecord->eventHandler_ =
        std::make_shared<AMSEventHandler>(appRunningRecord->taskHandler_, appRunningRecord->appMgrServiceInner_);
    appRunningRecord->isDebugApp_ = false;
    appRunningRecord->isNativeDebug_ = false;
    appRunningRecord->isAttachDebug_ = false;
    appRunningRecord->SendEvent(msg, timeOut);
    EXPECT_EQ(appRunningRecord->eventId_, EVENT_ID_2);
    EXPECT_EQ(appRunningRecord->addAbilityStageInfoEventId_, ADD_ABILITY_STAGE_INFO_EVENT_ID);
}

/**
 * @tc.name: AppRunningRecord_SetAttachDebug_0100
 * @tc.desc: Test the status of SetAttachDebug based on the isAttachDebug value being true.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SetAttachDebug_0100, TestSize.Level1)
{
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRunningRecord, nullptr);
    appRunningRecord->SetAttachDebug(isAttachDebug);
    EXPECT_EQ(appRunningRecord->isAttachDebug_, true);
}

/**
 * @tc.name: AppRunningRecord_SetAttachDebug_0200
 * @tc.desc: Test the status of SetAttachDebug based on the isAttachDebug value being false.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SetAttachDebug_0200, TestSize.Level1)
{
    bool isAttachDebug = false;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRunningRecord, nullptr);
    appRunningRecord->SetAttachDebug(isAttachDebug);
    EXPECT_EQ(appRunningRecord->isAttachDebug_, false);
}

/**
 * @tc.name: AppRunningRecord_TerminateAbility_0100
 * @tc.desc: Test the status of TerminateAbility.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_TerminateAbility_0100, TestSize.Level1)
{
    bool isForce = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRunningRecord, nullptr);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto info = nullptr;
    auto eventHandler = nullptr;
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(info, eventHandler);
    EXPECT_NE(moduleRecord, nullptr);
    appRunningRecord->TerminateAbility(token, isForce);
    EXPECT_EQ(moduleRecord->terminateAbilities_.size(), TERMINATE_ABILITY_SIZE);
}

/**
 * @tc.name: AppRunningRecord_AbilityTerminated_0100
 * @tc.desc: Test the status of AbilityTerminated.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_AbilityTerminated_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token;
    bool isForce = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRunningRecord, nullptr);
    appRunningRecord->AbilityTerminated(token);
    EXPECT_EQ(appRunningRecord->processType_, ProcessType::NORMAL);
}

/**
 * @tc.name: AppRunningRecord_AddChildProcessRecord_0100
 * @tc.desc: Test AddChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_AddChildProcessRecord_0100, TestSize.Level1)
{
    HILOG_DEBUG("AppRunningRecord_AddChildProcessRecord_0100 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);
    
    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", appRecord);
    pid_t childPid = 201;
    childRecord->pid_ = childPid;
    appRecord->AddChildProcessRecord(childPid, childRecord);

    auto childProcessRecordMap = appRecord->childProcessRecordMap_;
    auto iter = childProcessRecordMap.find(childPid);
    EXPECT_NE(iter, childProcessRecordMap.end());
}

/**
 * @tc.name: AppRunningRecord_RemoveChildProcessRecord_0100
 * @tc.desc: Test RemoveChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_RemoveChildProcessRecord_0100, TestSize.Level1)
{
    HILOG_DEBUG("AppRunningRecord_RemoveChildProcessRecord_0100 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);
    
    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", appRecord);
    pid_t childPid = 201;
    childRecord->pid_ = childPid;
    appRecord->childProcessRecordMap_.emplace(childPid, childRecord);

    appRecord->RemoveChildProcessRecord(childRecord);
    auto childProcessRecordMap = appRecord->childProcessRecordMap_;
    auto iter = childProcessRecordMap.find(childPid);
    EXPECT_EQ(iter, childProcessRecordMap.end());
}

/**
 * @tc.name: AppRunningRecord_GetChildProcessRecordByPid_0100
 * @tc.desc: Test GetChildProcessRecordByPid works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetChildProcessRecordByPid_0100, TestSize.Level1)
{
    HILOG_DEBUG("AppRunningRecord_GetChildProcessRecordByPid_0100 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);
    
    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", appRecord);
    pid_t childPid = 201;
    childRecord->pid_ = childPid;
    appRecord->childProcessRecordMap_.emplace(childPid, childRecord);

    auto record = appRecord->GetChildProcessRecordByPid(childPid);
    EXPECT_NE(record, nullptr);
}

/**
 * @tc.name: AppRunningRecord_GetChildProcessRecordMap_0100
 * @tc.desc: Test GetChildProcessRecordByPid works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetChildProcessRecordMap_0100, TestSize.Level1)
{
    HILOG_DEBUG("AppRunningRecord_GetChildProcessRecordMap_0100 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);

    auto childProcessRecordMap = appRecord->GetChildProcessRecordMap();
    EXPECT_EQ(childProcessRecordMap.size(), 0);
}

/**
 * @tc.name: GetSplitModeAndFloatingMode_0100
 * @tc.desc: Test the return when abilityWant is not nullptr and
 *      the first determine of windowMode is conformed.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, GetSplitModeAndFloatingMode_001, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> info = nullptr;
    int32_t recordId = 0;
    std::string processName = "appRunningRecordProcessName";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AbilityRunningRecord> abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token);

    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    want->SetParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE,
        AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING);
    abilityRecord->SetWant(want);
    moduleRunningRecord->abilities_.emplace(nullptr, abilityRecord);

    std::vector<std::shared_ptr<ModuleRunningRecord>> hapModulesVector;
    hapModulesVector.emplace_back(moduleRunningRecord);
    std::string hapModulesString = "hapModulesString";
    appRunningRecord->hapModules_.emplace(hapModulesString, hapModulesVector);

    bool isSplitScreenMode = false;
    bool isFloatingWindowMode = false;
    appRunningRecord->GetSplitModeAndFloatingMode(isSplitScreenMode, isFloatingWindowMode);
    EXPECT_EQ(true, isFloatingWindowMode);
}
} // namespace AppExecFwk
} // namespace OHOS
