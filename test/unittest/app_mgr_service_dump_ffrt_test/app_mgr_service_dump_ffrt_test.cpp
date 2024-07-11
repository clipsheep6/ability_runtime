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

#include "mock_ipc_skeleton.h"

#define private public
#include "app_mgr_service.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "mock_app_mgr_service_inner.h"
#include "app_mgr_service_dump_error_code.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t HIDUMPER_SERVICE_UID = 1212;
const int32_t RANDOM_SERVICE_UID = 1500;
const int32_t NUMBER_VALID_PID_1 = 2000;
const int32_t NUMBER_VALID_PID_2 = 2001;
const int32_t NUMBER_VALID_PID_3 = 2002;
const int32_t NUMBER_VALID_PID_4 = 2003;
const int32_t NUMBER_INVALID_PID_1 = -1001;
const int32_t NUMBER_INVALID_PID_2 = -1002;
const int32_t NUMBER_INVALID_PID_3 = -1003;
const int32_t NUMBER_PID_APP_RUNING_RECORD_NOT_EXIST_1 = 3000;
const int32_t NUMBER_PID_APP_RUNING_RECORD_NOT_EXIST_2 = 3001;
const int32_t NUMBER_PID_APP_RUNING_RECORD_NOT_EXIST_3 = 3002;
const std::string STRING_INVALID_PID_1 = "invalid";
const std::string STRING_INVALID_PID_2 = "invalid1000";
const std::string STRING_OPTION_FFRT = "--ffrt";
} // namespace

class AppMgrServiceDumpFFRTTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrServiceDumpFFRTTest::SetUpTestCase(void)
{}

void AppMgrServiceDumpFFRTTest::TearDownTestCase(void)
{}

void AppMgrServiceDumpFFRTTest::SetUp()
{}

void AppMgrServiceDumpFFRTTest::TearDown()
{}

/*
 * @tc.number    : AppMgrServiceDumpFFRT_0100
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ffrt
 */
HWTEST_F(AppMgrServiceDumpFFRTTest, AppMgrServiceDumpFFRT_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0100 start");

    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_NE(mockAppMgrServiceInner, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner);
    EXPECT_NE(appMgrService->appMgrServiceInner_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    EXPECT_NE(appMgrService->eventHandler_, nullptr);

    // IsHidumperServiceCall
    IPCSkeleton::SetCallingUid(HIDUMPER_SERVICE_UID);
    EXPECT_TRUE((IPCSkeleton::GetCallingUid() == HIDUMPER_SERVICE_UID));

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_FFRT);
    args.emplace_back(arg0);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_INVALID_NUM_ARGS_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0100 end");
}

/*
 * @tc.number    : AppMgrServiceDumpFFRT_0200
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ffrt <valid pid> without correct permission
 */
HWTEST_F(AppMgrServiceDumpFFRTTest, AppMgrServiceDumpFFRT_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0200 start");

    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_NE(mockAppMgrServiceInner, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner);
    EXPECT_NE(appMgrService->appMgrServiceInner_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    EXPECT_NE(appMgrService->eventHandler_, nullptr);

    // Not HidumperServiceCall
    IPCSkeleton::SetCallingUid(RANDOM_SERVICE_UID);
    EXPECT_FALSE((IPCSkeleton::GetCallingUid() == HIDUMPER_SERVICE_UID));

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_FFRT);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16(std::to_string(NUMBER_VALID_PID_1));
    args.emplace_back(arg1);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_PERMISSION_DENY_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0200 end");
}

/*
 * @tc.number    : AppMgrServiceDumpFFRT_0300
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ffrt pid1|pid2|pid3
 */
HWTEST_F(AppMgrServiceDumpFFRTTest, AppMgrServiceDumpFFRT_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0300 start");

    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_NE(mockAppMgrServiceInner, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner);
    EXPECT_NE(appMgrService->appMgrServiceInner_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    EXPECT_NE(appMgrService->eventHandler_, nullptr);

    // IsHidumperServiceCall
    IPCSkeleton::SetCallingUid(HIDUMPER_SERVICE_UID);
    EXPECT_TRUE((IPCSkeleton::GetCallingUid() == HIDUMPER_SERVICE_UID));

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_FFRT);
    args.emplace_back(arg0);
    std::string pids;
    pids.append(std::to_string(NUMBER_VALID_PID_1))
        .append("|")
        .append(std::to_string(NUMBER_VALID_PID_2))
        .append("|")
        .append(std::to_string(NUMBER_VALID_PID_3));
    auto arg1 = Str8ToStr16(pids);
    args.emplace_back(arg1);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_INVALID_PID_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0300 end");
}

/*
 * @tc.number    : AppMgrServiceDumpFFRT_0400
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ffrt ' ,<invalid pid1>,<invalid pid2>'
 */
HWTEST_F(AppMgrServiceDumpFFRTTest, AppMgrServiceDumpFFRT_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0400 start");

    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_NE(mockAppMgrServiceInner, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner);
    EXPECT_NE(appMgrService->appMgrServiceInner_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    EXPECT_NE(appMgrService->eventHandler_, nullptr);

    // IsHidumperServiceCall
    IPCSkeleton::SetCallingUid(HIDUMPER_SERVICE_UID);
    EXPECT_TRUE((IPCSkeleton::GetCallingUid() == HIDUMPER_SERVICE_UID));

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_FFRT);
    args.emplace_back(arg0);
    std::string pids;
    pids.append(" ,")
        .append(std::to_string(NUMBER_INVALID_PID_1))
        .append(",")
        .append(STRING_INVALID_PID_2);
    auto arg1 = Str8ToStr16(pids);
    args.emplace_back(arg1);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_INVALID_PID_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0400 end");
}

/*
 * @tc.number    : AppMgrServiceDumpFFRT_0500
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ffrt <invalid pid1>,<invalid pid2>,<invalid pid3>,<valid pid1>
 */
HWTEST_F(AppMgrServiceDumpFFRTTest, AppMgrServiceDumpFFRT_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0500 start");

    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_NE(mockAppMgrServiceInner, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner);
    EXPECT_NE(appMgrService->appMgrServiceInner_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    EXPECT_NE(appMgrService->eventHandler_, nullptr);

    // IsHidumperServiceCall
    IPCSkeleton::SetCallingUid(HIDUMPER_SERVICE_UID);
    EXPECT_TRUE((IPCSkeleton::GetCallingUid() == HIDUMPER_SERVICE_UID));

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_FFRT);
    args.emplace_back(arg0);
    std::string pids;
    pids.append(std::to_string(NUMBER_INVALID_PID_1))
        .append(",")
        .append(std::to_string(NUMBER_INVALID_PID_2))
        .append(",")
        .append(std::to_string(NUMBER_INVALID_PID_3))
        .append(",")
        .append(std::to_string(NUMBER_VALID_PID_1));
    auto arg1 = Str8ToStr16(pids);
    args.emplace_back(arg1);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_INVALID_PID_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0500 end");
}

/*
 * @tc.number    : AppMgrServiceDumpFFRT_0600
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ffrt <valid pid> --stat
 */
HWTEST_F(AppMgrServiceDumpFFRTTest, AppMgrServiceDumpFFRT_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0600 start");

    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_NE(mockAppMgrServiceInner, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner);
    EXPECT_NE(appMgrService->appMgrServiceInner_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    EXPECT_NE(appMgrService->eventHandler_, nullptr);

    // IsHidumperServiceCall
    IPCSkeleton::SetCallingUid(HIDUMPER_SERVICE_UID);
    EXPECT_TRUE((IPCSkeleton::GetCallingUid() == HIDUMPER_SERVICE_UID));

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_FFRT);
    args.emplace_back(arg0);
    std::string pids;
    pids.append(std::to_string(NUMBER_PID_APP_RUNING_RECORD_NOT_EXIST_1))
        .append(",")
        .append(std::to_string(NUMBER_PID_APP_RUNING_RECORD_NOT_EXIST_2))
        .append(",")
        .append(std::to_string(NUMBER_PID_APP_RUNING_RECORD_NOT_EXIST_3));
    auto arg1 = Str8ToStr16(pids);
    args.emplace_back(arg1);
    EXPECT_CALL(*mockAppMgrServiceInner, DumpFfrt(_, _))
        .Times(1).WillOnce(Return(DumpErrorCode::ERR_INVALID_PID_ERROR));
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_INVALID_PID_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0600 end");
}

/*
 * @tc.number    : AppMgrServiceDumpFFRT_0700
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ffrt pid1,pid2,pid3,pid4
 */
HWTEST_F(AppMgrServiceDumpFFRTTest, AppMgrServiceDumpFFRT_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0700 start");

    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_NE(mockAppMgrServiceInner, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner);
    EXPECT_NE(appMgrService->appMgrServiceInner_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    EXPECT_NE(appMgrService->eventHandler_, nullptr);

    // IsHidumperServiceCall
    IPCSkeleton::SetCallingUid(HIDUMPER_SERVICE_UID);
    EXPECT_TRUE((IPCSkeleton::GetCallingUid() == HIDUMPER_SERVICE_UID));

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_FFRT);
    args.emplace_back(arg0);
    std::string pids;
    pids.append(std::to_string(NUMBER_VALID_PID_1))
        .append(",")
        .append(std::to_string(NUMBER_VALID_PID_2))
        .append(",")
        .append(std::to_string(NUMBER_VALID_PID_3))
        .append(",")
        .append(std::to_string(NUMBER_VALID_PID_4));
    auto arg1 = Str8ToStr16(pids);
    args.emplace_back(arg1);
    std::vector<int32_t> validPids = {NUMBER_VALID_PID_1, NUMBER_VALID_PID_2, NUMBER_VALID_PID_3};
    EXPECT_CALL(*mockAppMgrServiceInner, DumpFfrt(validPids, _))
        .Times(1).WillOnce(Return(DumpErrorCode::ERR_OK));
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0700 end");
}

/*
 * @tc.number    : AppMgrServiceDumpFFRT_0800
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ffrt pid1,<invalid pid>,pid2,pid3
 */
HWTEST_F(AppMgrServiceDumpFFRTTest, AppMgrServiceDumpFFRT_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0800 start");

    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_NE(mockAppMgrServiceInner, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner);
    EXPECT_NE(appMgrService->appMgrServiceInner_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    EXPECT_NE(appMgrService->eventHandler_, nullptr);

    // IsHidumperServiceCall
    IPCSkeleton::SetCallingUid(HIDUMPER_SERVICE_UID);
    EXPECT_TRUE((IPCSkeleton::GetCallingUid() == HIDUMPER_SERVICE_UID));

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_FFRT);
    args.emplace_back(arg0);
    std::string pids;
    pids.append(std::to_string(NUMBER_VALID_PID_1))
        .append(",")
        .append(STRING_INVALID_PID_1)
        .append(",")
        .append(std::to_string(NUMBER_VALID_PID_2))
        .append(",")
        .append(std::to_string(NUMBER_VALID_PID_3));
    auto arg1 = Str8ToStr16(pids);
    args.emplace_back(arg1);
    std::vector<int32_t> validPids = {NUMBER_VALID_PID_1, NUMBER_VALID_PID_2};
    EXPECT_CALL(*mockAppMgrServiceInner, DumpFfrt(validPids, _))
        .Times(1).WillOnce(Return(DumpErrorCode::ERR_OK));
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0800 end");
}

/*
 * @tc.number    : AppMgrServiceDumpFFRT_0900
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ffrt <invalid pid>|pid2|pid3
 */
HWTEST_F(AppMgrServiceDumpFFRTTest, AppMgrServiceDumpFFRT_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0900 start");

    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_NE(mockAppMgrServiceInner, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner);
    EXPECT_NE(appMgrService->appMgrServiceInner_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    EXPECT_NE(appMgrService->eventHandler_, nullptr);

    // IsHidumperServiceCall
    IPCSkeleton::SetCallingUid(HIDUMPER_SERVICE_UID);
    EXPECT_TRUE((IPCSkeleton::GetCallingUid() == HIDUMPER_SERVICE_UID));

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_FFRT);
    args.emplace_back(arg0);
    std::string pids;
    pids.append(STRING_INVALID_PID_1)
        .append("|")
        .append(std::to_string(NUMBER_VALID_PID_2))
        .append("|")
        .append(std::to_string(NUMBER_VALID_PID_3));
    auto arg1 = Str8ToStr16(pids);
    args.emplace_back(arg1);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_INVALID_PID_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpFFRT_0900 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
