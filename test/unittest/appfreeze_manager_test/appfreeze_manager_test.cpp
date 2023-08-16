/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "appfreeze_manager.h"
#include "fault_data.h"

using namespace testing;
using namespace testing::ext;

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
 * @tc.number: AppfreezeManagerTest_AppfreezeHandle_001
 * @tc.name: AppfreezeHandle
 * @tc.desc: Verify that function AppfreezeHandle.
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_AppfreezeHandle_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeManagerTest_AppfreezeHandle_001 start";
    AppfreezeManager::AppInfo appInfo;
    appInfo.bundleName = "";
    FaultData faultData;
    faultData.errorObject.name = AppFreezeType::APP_INPUT_BLOCK;
    faultData.timeoutMarkers = "\f\n\r";
    int ret = appfreezeManager-> AppfreezeHandle(faultData, appInfo);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "AppfreezeManagerTest_AppfreezeHandle_001 end";
}

/**
 * @tc.number: AppfreezeManagerTest_AppfreezeHandle_002
 * @tc.name: AppfreezeHandle
 * @tc.desc: Verify that function AppfreezeHandle.
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_AppfreezeHandle_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeManagerTest_AppfreezeHandle_002 start";
    AppfreezeManager::AppInfo appInfo;
    appInfo.bundleName = "com.application.myapplication";
    FaultData faultData;
    faultData.errorObject.name = AppFreezeType::LIFECYCLE_TIMEOUT;
    faultData.timeoutMarkers = "\f\n\r";
    int ret = appfreezeManager-> AppfreezeHandle(faultData, appInfo);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "AppFreezeManagerTest_AppfreezeHandle_002 end";
}

/**
 * @tc.number: AppfreezeManagerTest_AppfreezeHandle_003
 * @tc.name: AppfreezeHandle
 * @tc.desc: Verify that function AppfreezeHandle.
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_AppfreezeHandle_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeManagerTest_AppfreezeHandle_003 start";
    AppfreezeManager::AppInfo appInfo;
    appInfo.bundleName = "";
    FaultData faultData;
    faultData.errorObject.name = "";
    faultData.errorObject.message = "5678";
    faultData.errorObject.stack = "90";
    int ret = appfreezeManager->AppfreezeHandle(faultData, appInfo);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "AppfreezeManagerTest_AppfreezeHandle_003 end";
}

/**
 * @tc.number: AppfreezeManagerTest_AppfreezeHandleWithStack_001
 * @tc.name: AppfreezeHandleWithStack
 * @tc.desc: Verify that function AppfreezeHandleWithStack.
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_AppfreezeHandleWithStack_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeManagerTest_AppfreezeHandleWithStack_001 start";
    AppfreezeManager::AppInfo appInfo;
    appInfo.bundleName = "";
    FaultData faultData;
    faultData.errorObject.name = "1234";
    faultData.errorObject.message = "5678";
    faultData.errorObject.stack = "90";
    int ret = appfreezeManager->AppfreezeHandleWithStack(faultData, appInfo);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "AppfreezeManagerTest_AppfreezeHandleWithStack_001 end";
}

/**
 * @tc.number: AppfreezeManagerTest_LifecycleTimeoutHandle_001
 * @tc.name: LifecycleTimeoutHandle
 * @tc.desc: Verify that function LifecycleTimeoutHandle.
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_LifecycleTimeoutHandle_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeManagerTest_LifecycleTimeoutHandle_001 start";
    std::string bundleName = "com.is.hiserice";
    std::string msg = "message";
    std::string eventName = AppFreezeType::THREAD_BLOCK_3S;
    int32_t pid = 8124;
    int typeId = AppfreezeManager::TypeAttribute::NORMAL_TIMEOUT;
    int ret1 = appfreezeManager->LifecycleTimeoutHandle(typeId, pid, eventName, bundleName, msg);
    EXPECT_EQ(ret1, -1);
    typeId = AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
    int ret2 = appfreezeManager->LifecycleTimeoutHandle(typeId, pid, eventName, bundleName, msg);
    EXPECT_EQ(ret2, -1);
    int ret3 = appfreezeManager->LifecycleTimeoutHandle(typeId, pid, eventName, "", msg);
    EXPECT_EQ(ret3, -1);
    eventName = AppFreezeType::LIFECYCLE_HALF_TIMEOUT;
    int ret4 = appfreezeManager->LifecycleTimeoutHandle(typeId, pid, eventName, bundleName, msg);
    EXPECT_EQ(ret4, -1);
    GTEST_LOG_(INFO) << "AppfreezeManagerTest_LifecycleTimeoutHandle_001 end";
}

/**
 * @tc.number: AppfreezeManagerTest_IsHandleAppfreeze_001
 * @tc.name: IsHandleAppfreeze
 * @tc.desc: Verify that function IsHandleAppfreeze.
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_IsHandleAppfreeze_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppFreezeManagerTest_IsHandleAppfreeze_001 start";
    bool ret1 = appfreezeManager->IsHandleAppfreeze("");
    EXPECT_EQ(true, ret1);
    std::string bundleName = "com.is.hiserice";
    bool ret2 = appfreezeManager->IsHandleAppfreeze(bundleName);
    EXPECT_EQ(true, ret2);
    GTEST_LOG_(INFO) << "AppfreezeManagerTest_IsHandleAppfreeze_001 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
