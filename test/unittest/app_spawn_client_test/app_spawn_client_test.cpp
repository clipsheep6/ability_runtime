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

#include <gtest/gtest.h>

#define private public
#include "app_spawn_client.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppSpawnClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppSpawnClientTest::SetUpTestCase(void)
{}

void AppSpawnClientTest::TearDownTestCase(void)
{}

void AppSpawnClientTest::SetUp()
{}

void AppSpawnClientTest::TearDown()
{}

/**
 * @tc.name: PreStartNWebSpawnProcess_001
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, PreStartNWebSpawnProcess_001, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    int ret = asc->PreStartNWebSpawnProcess();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: PreStartNWebSpawnProcessImpl_001
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, PreStartNWebSpawnProcessImpl_001, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    int ret = asc->PreStartNWebSpawnProcessImpl();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: PreStartNWebSpawnProcessImpl_002
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, PreStartNWebSpawnProcessImpl_002, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    asc->OpenConnection();
    int ret = asc->PreStartNWebSpawnProcessImpl();
    EXPECT_EQ(ret, ERR_OK);
}
} // namespace AppExecFwk
} // namespace OHOS
