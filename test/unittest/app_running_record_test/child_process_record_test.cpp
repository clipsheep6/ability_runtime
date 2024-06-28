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

#include "app_running_record.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t RECORD_ID = 1;
}
class ChildProcessRecordTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ChildProcessRecordTest::SetUpTestCase(void)
{}

void ChildProcessRecordTest::TearDownTestCase(void)
{}

void ChildProcessRecordTest::SetUp()
{}

void ChildProcessRecordTest::TearDown()
{}

/**
 * @tc.name: ChildProcessRecord_0100
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_0100 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", appRecord, 0, false);
    auto hostPid = childRecord->GetHostPid();
    EXPECT_EQ(hostPid, 101);
}

/**
 * @tc.name: ChildProcessRecord_0200
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_0200, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_0200 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", appRecord, 0, false);
    childRecord->SetUid(100);
    auto uid = childRecord->GetUid();
    EXPECT_EQ(uid, 100);
}

/**
 * @tc.name: ChildProcessRecord_0300
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_0300, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_0300 called.");

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", nullptr, 0, false);
    auto record = childRecord->GetHostRecord();
    EXPECT_EQ(record, nullptr);
}

/**
 * @tc.name: ChildProcessRecord_0400
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_0400, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_0400 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", appRecord, 0, false);
    auto processName = childRecord->GetProcessName();
    EXPECT_TRUE(processName.length() > 0);
}

/**
 * @tc.name: ChildProcessRecord_0500
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_0500, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_0500 called.");

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", nullptr, 0, false);
    auto processName = childRecord->GetProcessName();
    EXPECT_TRUE(processName.length() <= 0);
}

/**
 * @tc.name: ChildProcessRecord_0600
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_0600, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_0600 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "", appRecord, 0, false);
    auto processName = childRecord->GetProcessName();
    EXPECT_TRUE(processName.length() <= 0);
}

/**
 * @tc.name: ChildProcessRecord_0700
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_0700, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_0700 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", appRecord, 0, false);
    auto srcEntry = childRecord->GetSrcEntry();
    EXPECT_EQ(srcEntry, "./ets/AProcess.ts");
}

/**
 * @tc.name: ChildProcessRecord_0800
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_0800, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_0800 called.");

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", nullptr, 0, false);
    sptr<IChildScheduler> scheduler;
    childRecord->SetScheduler(scheduler);
    EXPECT_NE(childRecord, nullptr);
}

/**
 * @tc.name: ChildProcessRecord_0900
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_0900, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_0900 called.");

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", nullptr, 0, false);
    sptr<IChildScheduler> scheduler;
    childRecord->SetScheduler(scheduler);
    EXPECT_EQ(childRecord->GetScheduler(), scheduler);
}

/**
 * @tc.name: ChildProcessRecord_1000
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_1000, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_1000 called.");

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", nullptr, 0, false);
    sptr<AppDeathRecipient> recipient;
    childRecord->SetDeathRecipient(recipient);
    EXPECT_NE(childRecord, nullptr);
}

/**
 * @tc.name: ChildProcessRecord_1100
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_1100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_1100 called.");

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", nullptr, 0, false);
    sptr<IChildScheduler> scheduler;
    childRecord->SetScheduler(scheduler);
    sptr<AppDeathRecipient> recipient;
    childRecord->SetDeathRecipient(recipient);
    childRecord->RegisterDeathRecipient();
    EXPECT_NE(childRecord, nullptr);
}

/**
 * @tc.name: ChildProcessRecord_1200
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_1200, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_1200 called.");

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", nullptr, 0, false);
    childRecord->RemoveDeathRecipient();
    EXPECT_NE(childRecord, nullptr);
}

/**
 * @tc.name: ChildProcessRecord_1300
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_1300, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_1300 called.");

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", nullptr, 0, false);
    sptr<IChildScheduler> scheduler;
    childRecord->SetScheduler(scheduler);
    childRecord->RemoveDeathRecipient();
    EXPECT_NE(childRecord, nullptr);
}

/**
 * @tc.name: ChildProcessRecord_1400
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_1400, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_1400 called.");

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", nullptr, 0, false);
    sptr<IChildScheduler> scheduler;
    childRecord->SetScheduler(scheduler);
    childRecord->ScheduleExitProcessSafely();
    EXPECT_NE(childRecord, nullptr);
}

/**
 * @tc.name: ChildProcessRecord_1500
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_1500, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_1500 called.");

    auto childRecord = std::make_shared<ChildProcessRecord>(101, "./ets/AProcess.ts", nullptr, 0, false);
    childRecord->ScheduleExitProcessSafely();
    EXPECT_NE(childRecord, nullptr);
}

/**
 * @tc.name: ChildProcessRecord_1600
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_1600, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_1600 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);

    auto childRecord = ChildProcessRecord::CreateChildProcessRecord(0, "./ets/AProcess.ts", appRecord, 0, false);
    EXPECT_EQ(childRecord, nullptr);
}

/**
 * @tc.name: ChildProcessRecord_1700
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_1700, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_1700 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);

    auto childRecord = ChildProcessRecord::CreateChildProcessRecord(101, "", appRecord, 0, false);
    EXPECT_EQ(childRecord, nullptr);
}

/**
 * @tc.name: ChildProcessRecord_1800
 * @tc.desc: Test ChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessRecordTest, ChildProcessRecord_1800, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessRecord_1800 called.");
    auto childRecord = ChildProcessRecord::CreateChildProcessRecord(101, "./ets/AProcess.ts", nullptr, 0, false);
    EXPECT_EQ(childRecord, nullptr);
}
} // namespace AppExecFwk
} // namespace OHOS
