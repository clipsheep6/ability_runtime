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
#include "app_running_record.h"
#undef private
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
} // namespace AppExecFwk
} // namespace OHOS
