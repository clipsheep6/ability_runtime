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
#include "appfreeze_manager.h"
#undef private

#include "cpp/mutex.h"
#include "cpp/condition_variable.h"
#include "fault_data.h"
#include "freeze_util.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
class AppfreezeManagerTest : public testing::Test {
public:
    AppfreezeManagerTest()
    {}
    ~AppfreezeManagerTest()
    {}
    std::shared_ptr<AppfreezeManager> appfreezeManager = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppfreezeManagerTest::SetUpTestCase(void)
{}

void AppfreezeManagerTest::TearDownTestCase(void)
{}

void AppfreezeManagerTest::SetUp(void)
{
    appfreezeManager = AppfreezeManager::GetInstance();
}

void AppfreezeManagerTest::TearDown(void)
{
    AppfreezeManager::DestroyInstance();
}

/**
 * @tc.number: AppfreezeManagerTest_001
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_001, TestSize.Level1)
{
    bool ret = appfreezeManager->IsHandleAppfreeze("");
    EXPECT_TRUE(ret);
    ret = appfreezeManager->IsHandleAppfreeze("AppfreezeManagerTest_001");
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: AppfreezeManagerTest_002
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_002, TestSize.Level1)
{
    FaultData faultData;
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    faultData.faultType = FaultDataType::APP_FREEZE;
    AppfreezeManager::AppInfo appInfo;
    int ret = appfreezeManager->AppfreezeHandle(faultData, appInfo);
    EXPECT_EQ(ret, 0);
    ret = appfreezeManager->AppfreezeHandleWithStack(faultData, appInfo);
    EXPECT_EQ(ret, 0);
    ret = appfreezeManager->AcquireStack(faultData, appInfo);
    EXPECT_EQ(ret, 0);

    faultData.errorObject.name = AppFreezeType::APP_INPUT_BLOCK;
    ret = appfreezeManager->AppfreezeHandle(faultData, appInfo);
    EXPECT_EQ(ret, 0);
    ret = appfreezeManager->AppfreezeHandleWithStack(faultData, appInfo);
    EXPECT_EQ(ret, 0);
    ret = appfreezeManager->AcquireStack(faultData, appInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: AppfreezeManagerTest_003
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_003, TestSize.Level1)
{
    FaultData faultData;
    faultData.errorObject.name = AppFreezeType::APP_INPUT_BLOCK;
    AppfreezeManager::AppInfo appInfo = {
        .pid = 1,
        .uid = 1,
        .bundleName = "AppfreezeManagerTest_003",
        .processName = "AppfreezeManagerTest_003",
    };
    int ret = appfreezeManager->NotifyANR(faultData, appInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: AppfreezeManagerTest_004
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_004, TestSize.Level1)
{
    AppfreezeManager::ParamInfo info;
    int ret = appfreezeManager->LifecycleTimeoutHandle(info);
    EXPECT_EQ(ret, -1);
    AppfreezeManager::ParamInfo info1 = {
        .typeId = AppfreezeManager::TypeAttribute::NORMAL_TIMEOUT,
        .eventName = AppFreezeType::APP_INPUT_BLOCK,
        .bundleName = "",
        .msg = "Test",
    };
    ret = appfreezeManager->LifecycleTimeoutHandle(info1);
    EXPECT_EQ(ret, -1);
    AppfreezeManager::ParamInfo info2 = {
        .typeId = AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT,
        .eventName = AppFreezeType::APP_INPUT_BLOCK,
        .bundleName = "",
        .msg = "Test",
    };
    ret = appfreezeManager->LifecycleTimeoutHandle(info2);
    EXPECT_EQ(ret, -1);
    AppfreezeManager::ParamInfo info3 = {
        .typeId = AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT,
        .eventName = AppFreezeType::LIFECYCLE_HALF_TIMEOUT,
        .bundleName = "",
        .msg = "Test",
    };
    auto flow = std::make_unique<FreezeUtil::LifecycleFlow>();
    flow->state = AbilityRuntime::FreezeUtil::TimeoutState::FOREGROUND;
    ret = appfreezeManager->LifecycleTimeoutHandle(info3, std::move(flow));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: AppfreezeManagerTest_005
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_005, TestSize.Level1)
{
    std::map<int, std::set<int>> binderInfo;
    std::set<int> ids;
    ids.insert(1);
    binderInfo.insert(std::pair<int, std::set<int>>(0, ids));
    ids.insert(0);
    binderInfo.insert(std::pair<int, std::set<int>>(1, ids));
    std::set<int> pids;
    appfreezeManager->ParseBinderPids(binderInfo, pids, 2, 8);
    EXPECT_EQ(pids.size(), 0);
    appfreezeManager->ParseBinderPids(binderInfo, pids, 1, 0);
    EXPECT_EQ(pids.size(), 2);
}

/**
 * @tc.number: AppfreezeManagerTest_006
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_006, TestSize.Level1)
{
    std::string ret = appfreezeManager->CatcherStacktrace(0);
    printf("ret: %s\n", ret.c_str());
    ret = appfreezeManager->CatcherStacktrace(2);
    printf("ret: %s\n", ret.c_str());
    EXPECT_TRUE(!ret.empty());
}
}  // namespace AppExecFwk
}  // namespace OHOS
