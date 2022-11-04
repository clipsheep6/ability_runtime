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
#include "app_mgr_service.h"
#undef private


using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrServiceTest::SetUpTestCase(void)
{}

void AppMgrServiceTest::TearDownTestCase(void)
{}

void AppMgrServiceTest::SetUp()
{}

void AppMgrServiceTest::TearDown()
{}

/**
 * @tc.name: PreStartNWebSpawnProcess_001
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceTest, PreStartNWebSpawnProcess_001, TestSize.Level0)
{
    auto ams = std::make_shared<AppMgrService>();
    ams->SetInnerService(std::make_shared<AppMgrServiceInner>());
    ams->Init();
    int ret = ams->PreStartNWebSpawnProcess();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: PreStartNWebSpawnProcess_002
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceTest, PreStartNWebSpawnProcess_002, TestSize.Level0)
{
    auto ams = std::make_shared<AppMgrService>();
    ams->SetInnerService(nullptr);
    int ret = ams->PreStartNWebSpawnProcess();
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}
} // namespace AppExecFwk
} // namespace OHOS
