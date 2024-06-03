/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "app_mgr_event.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {

class AppMgrEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrEventTest::SetUpTestCase(void)
{}

void AppMgrEventTest::TearDownTestCase(void)
{}

void AppMgrEventTest::SetUp()
{}

void AppMgrEventTest::TearDown()
{}

/**
 * @tc.name: SendCreateAtomicServiceProcessEvent
 * @tc.desc: Send Create Atomic service process event, appRecord is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrEventTest, SendCreateAtomicServiceProcessEvent_0100, TestSize.Level1)
{
    std::shared_ptr<AppRunningRecord> callerAppRecord = nullptr;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    std::string moduleName = "testModuleName";
    std::string abilityName = "testAbilityName";
    bool ret = AppMgrEventUtil::SendCreateAtomicServiceProcessEvent(callerAppRecord, appRecord,
        moduleName, abilityName);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: SendCreateAtomicServiceProcessEvent
 * @tc.desc: Send Create Atomic service process event, callerAppRecord is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrEventTest, SendCreateAtomicServiceProcessEvent_0200, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    EXPECT_NE(appInfo, nullptr);
    appInfo->bundleName = "testBundleName";
    appInfo->name = "testBundleName";

    int32_t recordId = 1;
    std::string processName = "testProcess";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRecord->SetCallerUid(-1);
    EXPECT_NE(appRecord, nullptr);

    std::shared_ptr<AppRunningRecord> callerAppRecord = nullptr;
    std::string moduleName = "testModuleName";
    std::string abilityName = "testAbilityName";
    bool ret = AppMgrEventUtil::SendCreateAtomicServiceProcessEvent(callerAppRecord, appRecord,
        moduleName, abilityName);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: SendCreateAtomicServiceProcessEvent
 * @tc.desc: Send Create Atomic service process event, callerAppRecord is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrEventTest, SendCreateAtomicServiceProcessEvent_0300, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    EXPECT_NE(appInfo, nullptr);
    appInfo->bundleName = "testBundleName";
    appInfo->name = "testBundleName";

    int32_t recordId = 1;
    std::string processName = "testProcess";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRecord, nullptr);
    appRecord->SetCallerUid(-1);
    
    auto callerAppInfo = std::make_shared<ApplicationInfo>();
    EXPECT_NE(callerAppInfo, nullptr);
    callerAppInfo->bundleName = "testCallerBundleName";
    callerAppInfo->name = "testCallerBundleName";
    std::string callerProcessName = "testCallerProcess";
    int32_t callerRecordId = 2;
    auto callerAppRecord = std::make_shared<AppRunningRecord>(callerAppInfo, callerRecordId, callerProcessName);
    EXPECT_NE(callerAppRecord, nullptr);

    std::string moduleName = "testModuleName";
    std::string abilityName = "testAbilityName";
    bool ret = AppMgrEventUtil::SendCreateAtomicServiceProcessEvent(callerAppRecord, appRecord,
        moduleName, abilityName);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: SendProcessStartEvent
 * @tc.desc: Send process start event, appRecord is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrEventTest, SendProcessStartEvent_0100, TestSize.Level1)
{
    std::shared_ptr<AppRunningRecord> callerAppRecord = nullptr;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    AAFwk::EventInfo eventInfo;
    bool ret = AppMgrEventUtil::SendProcessStartEvent(callerAppRecord, appRecord, eventInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: SendProcessStartEvent
 * @tc.desc: Send process start event: callerAppRecord is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrEventTest, SendProcessStartEvent_0200, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    EXPECT_NE(appInfo, nullptr);

    appInfo->bundleName = "testBundleName";
    appInfo->name = "testBundleName";

    int32_t recordId = 1;
    std::string processName = "testProcess";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRecord->SetCallerUid(-1);
    appRecord->priorityObject_->SetPid(1001);
    EXPECT_NE(appRecord, nullptr);

    std::shared_ptr<AppRunningRecord> callerAppRecord = nullptr;

    AAFwk::EventInfo eventInfo;
    bool ret = AppMgrEventUtil::SendProcessStartEvent(callerAppRecord, appRecord, eventInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: SendProcessStartEvent
 * @tc.desc: Send process start event, callerAppRecord is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrEventTest, SendProcessStartEvent_0300, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    EXPECT_NE(appInfo, nullptr);
    appInfo->bundleName = "testBundleName";
    appInfo->name = "testBundleName";

    int32_t recordId = 1;
    std::string processName = "testProcess";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRecord, nullptr);
    appRecord->SetCallerUid(-1);
    
    auto callerAppInfo = std::make_shared<ApplicationInfo>();
    EXPECT_NE(callerAppInfo, nullptr);
    callerAppInfo->bundleName = "testCallerBundleName";
    callerAppInfo->name = "testCallerBundleName";
    std::string callerProcessName = "testCallerProcess";
    int32_t callerRecordId = 2;
    auto callerAppRecord = std::make_shared<AppRunningRecord>(callerAppInfo, callerRecordId, callerProcessName);
    EXPECT_NE(callerAppRecord, nullptr);
    
    AAFwk::EventInfo eventInfo;
    bool ret = AppMgrEventUtil::SendProcessStartEvent(callerAppRecord, appRecord, eventInfo);
    EXPECT_EQ(ret, true);
}
} // namespace AppExecFwk
} // namespace OHOS
