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

#include <algorithm>
#include <gtest/gtest.h>

#define private public
#define protected public
#include "dump_ipc_helper.h"
#undef private
#undef protected

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class DumpIpcHelperTest : public testing::Test {
public:
    DumpIpcHelperTest()
    {}
    ~DumpIpcHelperTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DumpIpcHelperTest::SetUpTestCase(void)
{}

void DumpIpcHelperTest::TearDownTestCase(void)
{}

void DumpIpcHelperTest::SetUp(void)
{}

void DumpIpcHelperTest::TearDown(void)
{}

/**
 * @tc.number: DumpIpcStart_0100
 * @tc.name: DumpIpcStart
 * @tc.desc: Test whether DumpIpcStart and are called normally.
 */
HWTEST_F(DumpIpcHelperTest, DumpIpcStart_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpIpcHelperTest DumpIpcStart_0100 start";
    std::string result = "";
    DumpIpcHelper::DumpIpcStart(result);
    EXPECT_NE(result, "");

    GTEST_LOG_(INFO) << "DumpIpcHelperTest DumpIpcStart_0100 end";
}

/**
 * @tc.number: DumpIpcStop_0100
 * @tc.name: DumpIpcStop
 * @tc.desc: Test whether DumpIpcStop and are called normally.
 */
HWTEST_F(DumpIpcHelperTest, DumpIpcStop_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpIpcHelperTest DumpIpcStop_0100 start";
    std::string result = "";
    DumpIpcHelper::DumpIpcStop(result);
    EXPECT_NE(result, "");

    GTEST_LOG_(INFO) << "DumpIpcHelperTest DumpIpcStop_0100 end";
}

/**
 * @tc.number: DumpIpcStat_0100
 * @tc.name: DumpIpcStat
 * @tc.desc: Test whether DumpIpcStat and are called normally.
 */
HWTEST_F(DumpIpcHelperTest, DumpIpcStat_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpIpcHelperTest DumpIpcStat_0100 start";
    std::string result = "";
    DumpIpcHelper::DumpIpcStat(result);
    EXPECT_NE(result, "");

    GTEST_LOG_(INFO) << "DumpIpcHelperTest DumpIpcStat_0100 end";
}

}
}