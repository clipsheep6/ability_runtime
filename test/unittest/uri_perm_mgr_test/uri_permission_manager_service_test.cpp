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
#define private public
#define protected public
#include "uri_permission_manager_service.h"
#undef private
#undef protected
using namespace testing::ext;
namespace OHOS {
namespace AAFwk {
class UriPermissionManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void UriPermissionManagerServiceTest::SetUpTestCase()
{}

void UriPermissionManagerServiceTest::TearDownTestCase()
{}

void UriPermissionManagerServiceTest::SetUp()
{}

void UriPermissionManagerServiceTest::TearDown()
{}

/**
 * @tc.number: OnStart_0100
 * @tc.name: OnStart
 * @tc.desc: Test whether OnStart and are called normally.
 */
HWTEST_F(UriPermissionManagerServiceTest, OnStart_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "UriPermissionManagerServiceTest OnStart_0100 start";
    DelayedSingleton<UriPermissionManagerService>::GetInstance()->OnStart();
    GTEST_LOG_(INFO) << "UriPermissionManagerServiceTest OnStart_0100 end";
}

/**
 * @tc.number: OnStop_0100
 * @tc.name: OnStop
 * @tc.desc: Test whether OnStop and are called normally.
 */
HWTEST_F(UriPermissionManagerServiceTest, OnStop_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "UriPermissionManagerServiceTest OnStop_0100 start";
    DelayedSingleton<UriPermissionManagerService>::GetInstance()->OnStop();
    GTEST_LOG_(INFO) << "UriPermissionManagerServiceTest OnStop_0100 end";
}

/**
 * @tc.number: IsServiceReady_0100
 * @tc.name: IsServiceReady
 * @tc.desc: Test whether IsServiceReady and are called normally.
 */
HWTEST_F(UriPermissionManagerServiceTest, IsServiceReady_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "UriPermissionManagerServiceTest IsServiceReady_0100 start";
    bool res = DelayedSingleton<UriPermissionManagerService>::GetInstance()->IsServiceReady();
    EXPECT_EQ(res, false);
    GTEST_LOG_(INFO) << "UriPermissionManagerServiceTest IsServiceReady_0100 end";
}

/**
 * @tc.number: Init_0100
 * @tc.name: Init
 * @tc.desc: Test whether Init and are called normally.
 */
HWTEST_F(UriPermissionManagerServiceTest, Init_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "UriPermissionManagerServiceTest Init_0100 start";
    bool res = DelayedSingleton<UriPermissionManagerService>::GetInstance()->Init();
    EXPECT_EQ(res, true);
    GTEST_LOG_(INFO) << "UriPermissionManagerServiceTest Init_0100 end";
}

/**
 * @tc.number: SelfClean_0100
 * @tc.name: SelfClean
 * @tc.desc: Test whether SelfClean and are called normally.
 */
HWTEST_F(UriPermissionManagerServiceTest, SelfClean_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "UriPermissionManagerServiceTest SelfClean_0100 start";
    DelayedSingleton<UriPermissionManagerService>::GetInstance()->SelfClean();
    GTEST_LOG_(INFO) << "UriPermissionManagerServiceTest SelfClean_0100 end";
}
}
}