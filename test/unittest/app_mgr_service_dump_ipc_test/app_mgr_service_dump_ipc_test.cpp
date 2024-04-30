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
#include "mock_permission_verification.h"

#define private public
#include "app_mgr_service.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "mock_app_mgr_service_inner.h"
#include "app_mgr_service_dump_error_code.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t HIDUMPER_SERVICE_UID = 1212;
const int32_t RANDOM_SERVICE_UID = 1500;
const int32_t NUMBER_VALID_PID = 2000;
const int32_t NUMBER_INVALID_PID = 2001;
const std::string STRING_INVALID_PID = "invalid";
const std::string STRING_CMD_START_STAT = "--start-stat";
const std::string STRING_CMD_STOP_STAT = "--stop-stat";
const std::string STRING_CMD_STAT = "--stat";
const std::string STRING_CMD_INVALID = "--invalid-cmd";
const std::string STRING_OPTION_IPC = "--ipc";
} // namespace

class AppMgrServiceDumpIPCTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrServiceDumpIPCTest::SetUpTestCase(void)
{}

void AppMgrServiceDumpIPCTest::TearDownTestCase(void)
{}

void AppMgrServiceDumpIPCTest::SetUp()
{}

void AppMgrServiceDumpIPCTest::TearDown()
{}

/*
 * @tc.number    : AppMgrServiceDumpIPC_0100
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc all --start-stat
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0100 start");

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

    // IsShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_SHELL_CALL;
    EXPECT_TRUE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16("all");
    args.emplace_back(arg1);
    auto arg2 = Str8ToStr16(STRING_CMD_START_STAT);
    args.emplace_back(arg2);
    EXPECT_CALL(*mockAppMgrServiceInner, DumpIpcAllStart(_))
        .Times(1).WillOnce(Return(DumpErrorCode::ERR_OK));
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0100 end");
}

/*
 * @tc.number    : AppMgrServiceDumpIPC_0200
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc all --stop-stat
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0200 start");

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

    // IsShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_SHELL_CALL;
    EXPECT_TRUE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16("all");
    args.emplace_back(arg1);
    auto arg2 = Str8ToStr16(STRING_CMD_STOP_STAT);
    args.emplace_back(arg2);
    EXPECT_CALL(*mockAppMgrServiceInner, DumpIpcAllStop(_))
        .Times(1).WillOnce(Return(DumpErrorCode::ERR_OK));
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0200 end");
}
/*
 * @tc.number    : AppMgrServiceDumpIPC_0300
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc all --stat
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0300 start");

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

    // IsShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_SHELL_CALL;
    EXPECT_TRUE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16("all");
    args.emplace_back(arg1);
    auto arg2 = Str8ToStr16(STRING_CMD_STAT);
    args.emplace_back(arg2);
    EXPECT_CALL(*mockAppMgrServiceInner, DumpIpcAllStat(_))
        .Times(1).WillOnce(Return(DumpErrorCode::ERR_OK));
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0300 end");
}

/*
 * @tc.number    : AppMgrServiceDumpIPC_0400
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc <valid pid> --start-stat
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0400 start");

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

    // IsShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_SHELL_CALL;
    EXPECT_TRUE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16(std::to_string(NUMBER_VALID_PID));
    args.emplace_back(arg1);
    auto arg2 = Str8ToStr16(STRING_CMD_START_STAT);
    args.emplace_back(arg2);
    EXPECT_CALL(*mockAppMgrServiceInner, DumpIpcStart(NUMBER_VALID_PID, _))
        .Times(1).WillOnce(Return(DumpErrorCode::ERR_OK));
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0400 end");
}

/*
 * @tc.number    : AppMgrServiceDumpIPC_0500
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc <valid pid> --stop-stat
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0500 start");

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

    // IsShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_SHELL_CALL;
    EXPECT_TRUE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16(std::to_string(NUMBER_VALID_PID));
    args.emplace_back(arg1);
    auto arg2 = Str8ToStr16(STRING_CMD_STOP_STAT);
    args.emplace_back(arg2);
    EXPECT_CALL(*mockAppMgrServiceInner, DumpIpcStop(NUMBER_VALID_PID, _))
        .Times(1).WillOnce(Return(DumpErrorCode::ERR_OK));
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0500 end");
}
/*
 * @tc.number    : AppMgrServiceDumpIPC_0600
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc <valid pid> --stat
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0600 start");

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

    // IsShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_SHELL_CALL;
    EXPECT_TRUE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16(std::to_string(NUMBER_VALID_PID));
    args.emplace_back(arg1);
    auto arg2 = Str8ToStr16(STRING_CMD_STAT);
    args.emplace_back(arg2);
    EXPECT_CALL(*mockAppMgrServiceInner, DumpIpcStat(NUMBER_VALID_PID, _))
        .Times(1).WillOnce(Return(DumpErrorCode::ERR_OK));
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0600 end");
}

/*
 * @tc.number    : AppMgrServiceDumpIPC_0700
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc <invalid pid> --start-stat
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0700 start");

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

    // IsShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_SHELL_CALL;
    EXPECT_TRUE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16(std::to_string(NUMBER_INVALID_PID));
    args.emplace_back(arg1);
    auto arg2 = Str8ToStr16(STRING_CMD_START_STAT);
    args.emplace_back(arg2);
    EXPECT_CALL(*mockAppMgrServiceInner, DumpIpcStart(NUMBER_INVALID_PID, _))
        .Times(1).WillOnce(Return(DumpErrorCode::ERR_INVALID_PID_ERROR));
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_INVALID_PID_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0700 end");
}

/*
 * @tc.number    : AppMgrServiceDumpIPC_0800
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc <invalid pid> --stop-stat
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0800 start");

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

    // IsShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_SHELL_CALL;
    EXPECT_TRUE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16(std::to_string(NUMBER_INVALID_PID));
    args.emplace_back(arg1);
    auto arg2 = Str8ToStr16(STRING_CMD_STOP_STAT);
    args.emplace_back(arg2);
    EXPECT_CALL(*mockAppMgrServiceInner, DumpIpcStop(NUMBER_INVALID_PID, _))
        .Times(1).WillOnce(Return(DumpErrorCode::ERR_INVALID_PID_ERROR));
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_INVALID_PID_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0800 end");
}

/*
 * @tc.number    : AppMgrServiceDumpIPC_0900
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc <invalid pid> --stat
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0900 start");

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

    // IsShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_SHELL_CALL;
    EXPECT_TRUE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16(std::to_string(NUMBER_INVALID_PID));
    args.emplace_back(arg1);
    auto arg2 = Str8ToStr16(STRING_CMD_STAT);
    args.emplace_back(arg2);
    EXPECT_CALL(*mockAppMgrServiceInner, DumpIpcStat(NUMBER_INVALID_PID, _))
        .Times(1).WillOnce(Return(DumpErrorCode::ERR_INVALID_PID_ERROR));
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_INVALID_PID_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_0900 end");
}

/*
 * @tc.number    : AppMgrServiceDumpIPC_1000
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc <invalid pid> --start-stat
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_1000, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_1000 start");

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

    // IsShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_SHELL_CALL;
    EXPECT_TRUE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16(STRING_INVALID_PID);
    args.emplace_back(arg1);
    auto arg2 = Str8ToStr16(STRING_CMD_START_STAT);
    args.emplace_back(arg2);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_INVALID_PID_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_1000 end");
}

/*
 * @tc.number    : AppMgrServiceDumpIPC_1100
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc <invalid pid> --stop-stat
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_1100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_1100 start");

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

    // IsShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_SHELL_CALL;
    EXPECT_TRUE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16(STRING_INVALID_PID);
    args.emplace_back(arg1);
    auto arg2 = Str8ToStr16(STRING_CMD_STOP_STAT);
    args.emplace_back(arg2);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_INVALID_PID_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_1100 end");
}

/*
 * @tc.number    : AppMgrServiceDumpIPC_1200
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc <invalid pid> --stat
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_1200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_1200 start");

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

    // IsShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_SHELL_CALL;
    EXPECT_TRUE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16(STRING_INVALID_PID);
    args.emplace_back(arg1);
    auto arg2 = Str8ToStr16(STRING_CMD_STAT);
    args.emplace_back(arg2);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_INVALID_PID_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_1200 end");
}

/*
 * @tc.number    : AppMgrServiceDumpIPC_1300
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc <valid pid> <invalid cmd>
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_1300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_1300 start");

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

    // IsShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_SHELL_CALL;
    EXPECT_TRUE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16(std::to_string(NUMBER_VALID_PID));
    args.emplace_back(arg1);
    auto arg2 = Str8ToStr16(STRING_CMD_INVALID);
    args.emplace_back(arg2);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_INVALID_CMD_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_1300 end");
}

/*
 * @tc.number    : AppMgrServiceDumpIPC_1400
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc <valid pid>
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_1400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_1400 start");

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

    // IsShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_SHELL_CALL;
    EXPECT_TRUE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16(std::to_string(NUMBER_VALID_PID));
    args.emplace_back(arg1);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_INVALID_NUM_ARGS_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_1400 end");
}

/*
 * @tc.number    : AppMgrServiceDumpIPC_1500
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test with args --ipc <valid pid> <valid cmd> without correct permission
 */
HWTEST_F(AppMgrServiceDumpIPCTest, AppMgrServiceDumpIPC_1500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_1500 start");

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

    // Not ShellCall
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_INVALID_CALL;
    EXPECT_FALSE(AAFwk::PermissionVerification::GetInstance()->IsShellCall());

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16(STRING_OPTION_IPC);
    args.emplace_back(arg0);
    auto arg1 = Str8ToStr16(std::to_string(NUMBER_VALID_PID));
    args.emplace_back(arg1);
    auto arg2 = Str8ToStr16(STRING_CMD_START_STAT);
    args.emplace_back(arg2);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_PERMISSION_DENY_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDumpIPC_1500 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
