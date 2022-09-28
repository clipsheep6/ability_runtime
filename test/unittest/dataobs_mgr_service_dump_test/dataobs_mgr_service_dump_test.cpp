/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "dataobs_mgr_service.h"
#include "app_mgr_service.h"
#undef private
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk  {
class DataobsMgrServiceDumpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void DataobsMgrServiceDumpTest::SetUpTestCase(void) {}

void DataobsMgrServiceDumpTest::TearDownTestCase(void) {}

void DataobsMgrServiceDumpTest::SetUp() {}

void DataobsMgrServiceDumpTest::TearDown() {}

/*
 * @tc.number    : DataobsMgrServiceDump_0100
 * @tc.name      : DataobsMgrService dump
 * @tc.desc      : 1.Test dump interface
 */
HWTEST_F(DataobsMgrServiceDumpTest, DataobsMgrServiceDump_0100, TestSize.Level1)
{
    HILOG_INFO("DataobsMgrServiceDump_0100 start");

    auto dataobsMgrService = std::make_shared<DataObsMgrService>();
    EXPECT_NE(dataobsMgrService, nullptr);

    constexpr int fd(0);
    std::vector<std::u16string> args;
    auto arg = Str8ToStr16("-h");
    args.emplace_back(arg);
    auto result = dataobsMgrService->Dump(fd, args);
    EXPECT_EQ(result, ERR_OK);

    HILOG_INFO("DataobsMgrServiceDump_0100 end");
}

/*
 * @tc.number    : DataobsMgrServiceDump_0200
 * @tc.name      : DataobsMgrService dump
 * @tc.desc      : 1.Test dump interface
 */
HWTEST_F(DataobsMgrServiceDumpTest, DataobsMgrServiceDump_0200, TestSize.Level1)
{
    HILOG_INFO("DataobsMgrServiceDump_0200 start");

    auto dataobsMgrService = std::make_shared<DataObsMgrService>();
    EXPECT_NE(dataobsMgrService, nullptr);

    constexpr int fd(0);
    std::vector<std::u16string> args;
    auto result = dataobsMgrService->Dump(fd, args);
    EXPECT_EQ(result, ERR_OK);

    HILOG_INFO("DataobsMgrServiceDump_0200 end");
}

/*
 * @tc.number    : DataobsMgrServiceDump_0300
 * @tc.name      : DataobsMgrService dump
 * @tc.desc      : 1.Test dump interface
 */
HWTEST_F(DataobsMgrServiceDumpTest, DataobsMgrServiceDump_0300, TestSize.Level1)
{
    HILOG_INFO("DataobsMgrServiceDump_0300 start");

    auto dataobsMgrService = std::make_shared<DataObsMgrService>();
    EXPECT_NE(dataobsMgrService, nullptr);

    constexpr int fd(0);
    std::vector<std::u16string> args;
    auto arg = Str8ToStr16("-i");
    args.emplace_back(arg);
    auto result = dataobsMgrService->Dump(fd, args);
    EXPECT_EQ(result, ERR_OK);

    HILOG_INFO("DataobsMgrServiceDump_0300 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
