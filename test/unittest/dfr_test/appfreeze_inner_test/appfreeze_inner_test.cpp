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

#define private public
#define protected public
#include "appfreeze_inner.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
class AppfreezeInnerTest : public testing::Test {
public:
    AppfreezeInnerTest()
    {}
    ~AppfreezeInnerTest()
    {}
    std::shared_ptr<AppfreezeInner> appfreezeInner = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppfreezeInnerTest::SetUpTestCase(void)
{}

void AppfreezeInnerTest::TearDownTestCase(void)
{}

void AppfreezeInnerTest::SetUp(void)
{
    appfreezeInner = AppfreezeInner::GetInstance();
}

void AppfreezeInnerTest::TearDown(void)
{
    AppfreezeInner::DestroyInstance();
}

/**
 * @tc.number: AppfreezeInnerTest_SetMainHandler_001
 * @tc.name: SetMainHandler
 * @tc.desc: Verify that function SetMainHandler.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__SetMainHandler_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__SetMainHandler_001 start";
    std::shared_ptr<EventHandler> eventHandler = std::make_shared<EventHandler>();
    EXPECT_TRUE(eventHandler != nullptr);
    AppfreezeInner::SetMainHandler(eventHandler);
    GTEST_LOG_(INFO) << "AppfreezeInner__SetMainHandler_001 end";
}

/**
 * @tc.number: AppfreezeInnerTest_SetApplicationInfo_001
 * @tc.name: SetApplicationInfo
 * @tc.desc: Verify that function SetApplicationInfo.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__SetApplicationInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__SetApplicationInfo_001 start";
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    EXPECT_TRUE(applicationInfo != nullptr);
    appfreezeInner->SetApplicationInfo(applicationInfo);
    GTEST_LOG_(INFO) << "AppfreezeInner__SetApplicationInfo_001 end";
}

/**
 * @tc.number: AppfreezeInnerTest_ThreadBlock_001
 * @tc.name: ThreadBlock
 * @tc.desc: Verify that function ThreadBlock.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__ThreadBlock_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__ThreadBlock_001 start";
    std::atomic_bool isSixSecondEvent = false;
    appfreezeInner->ThreadBlock(isSixSecondEvent);
    EXPECT_TRUE(isSixSecondEvent);
    GTEST_LOG_(INFO) << "AppfreezeInner__ThreadBlock_001 end";
}

/**
 * @tc.number: AppfreezeInnerTest_ThreadBlock_002
 * @tc.name: ThreadBlock
 * @tc.desc: Verify that function ThreadBlock.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__ThreadBlock_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__ThreadBlock_002 start";
    std::atomic_bool isSixSecondEvent = true;
    appfreezeInner->ThreadBlock(isSixSecondEvent);
    EXPECT_TRUE(isSixSecondEvent);
    GTEST_LOG_(INFO) << "AppfreezeInner__ThreadBlock_002 end";
}

/**
 * @tc.number: AppfreezeInner__AppfreezeHandle_001
 * @tc.name: AppfreezeHandle
 * @tc.desc: Verify that function AppfreezeHandle.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__AppfreezeHandle_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__AppfreezeHandle_001 start";
    FaultData faultData;
    faultData.errorObject.message = "App main thread is not response!";
    faultData.faultType = FaultDataType::APP_FREEZE;
    faultData.timeoutMarkers = "";
    bool onlyMainThread = true;
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    appfreezeInner->SetApplicationInfo(applicationInfo);
    int ret = appfreezeInner->AppfreezeHandle(faultData, onlyMainThread);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "AppfreezeInner__AppfreezeHandle_001 end";
}

/**
 * @tc.number: AppfreezeInner__AcquireStack_001
 * @tc.name: AcquireStack
 * @tc.desc: Verify that function AcquireStack.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__AcquireStack_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__AcquireStack_001 start";

    FaultData faultData;
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    bool onlyMainThread = true;
    int ret = appfreezeInner->AcquireStack(faultData, onlyMainThread);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "AppfreezeInner__AcquireStack_001 end";
}

/**
 * @tc.number: AppfreezeInner__Dump_001
 * @tc.name: Dump
 * @tc.desc: Verify that function Dump.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__Dump_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__Dump_001 start";
    std::string msgContent = "App main thread is not response!";
    MainHandlerDumper handlerDumper;
    handlerDumper.Dump(msgContent);
    EXPECT_EQ(msgContent, handlerDumper.GetDumpInfo());
    GTEST_LOG_(INFO) << "AppfreezeInner__Dump_001 end";
}

/**
 * @tc.number: AppfreezeInner__Dump_002
 * @tc.name: Dump
 * @tc.desc: Verify that function Dump.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__Dump_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__Dump_002 start";
    MainHandlerDumper handlerDumper;
    EXPECT_EQ("", handlerDumper.GetTag());
    GTEST_LOG_(INFO) << "AppfreezeInner__Dump_002 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
