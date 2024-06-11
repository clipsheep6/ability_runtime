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
#include "idle_time.h"
#include "main_thread.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AppExecFwk {
class IdleTimeTest : public testing::Test {
public:
    IdleTimeTest()
    {
        std::function<void(int32_t)> callback = nullptr;
        std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
        idleTime_ = std::make_shared<IdleTime>(mainHandler, callback);
    }
    ~IdleTimeTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<IdleTime> idleTime_ = nullptr;
};

void IdleTimeTest::SetUpTestCase(void)
{
}

void IdleTimeTest::TearDownTestCase(void)
{}

void IdleTimeTest::SetUp(void)
{
}

void IdleTimeTest::TearDown(void)
{}

/**
 * @tc.number: Preload_0100
 * @tc.name: Preload
 * @tc.desc: Test whether Preload and are called normally.
 */
HWTEST_F(IdleTimeTest, Preload_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "IdleTimeTest Preload_0100 start";

    int64_t time = 0;
    time = idleTime_->GetSysTimeNs();
    EXPECT_NE(time, 0);

    GTEST_LOG_(INFO) << "IdleTimeTest Preload_0100 end";
}

/**
 * @tc.number: InitVSyncReceiver_0100
 * @tc.name: InitVSyncReceiver
 * @tc.desc: Test whether InitVSyncReceiver and are called normally.
 */
HWTEST_F(IdleTimeTest, InitVSyncReceiver_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "IdleTimeTest InitVSyncReceiver_0100 start";
    idleTime_->needStop_ = true;
    idleTime_->receiver_ = nullptr;
    idleTime_->InitVSyncReceiver();
    EXPECT_EQ(idleTime_->receiver_, nullptr);
    GTEST_LOG_(INFO) << "IdleTimeTest InitVSyncReceiver_0100 end";
}

/**
 * @tc.number: EventTask_0100
 * @tc.name: EventTask
 * @tc.desc: Test whether EventTask and are called normally.
 */
HWTEST_F(IdleTimeTest, EventTask_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "IdleTimeTest EventTask_0100 start";
    idleTime_->needStop_ = false;
    idleTime_->EventTask();
    EXPECT_EQ(idleTime_->callback_, nullptr);

    GTEST_LOG_(INFO) << "IdleTimeTest EventTask_0100 end";
}

/**
 * @tc.number: PostTask_0100
 * @tc.name: PostTask
 * @tc.desc: Test whether PostTask and are called normally.
 */
HWTEST_F(IdleTimeTest, PostTask_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "IdleTimeTest PostTask_0100 start";
    idleTime_->needStop_ = true;
    idleTime_->PostTask();
    EXPECT_NE(idleTime_->eventHandler_, nullptr);

    GTEST_LOG_(INFO) << "IdleTimeTest PostTask_0100 end";
}

/**
 * @tc.number: GetIdleNotifyFunc_0100
 * @tc.name: GetIdleNotifyFunc
 * @tc.desc: Test whether GetIdleNotifyFunc and are called normally.
 */
HWTEST_F(IdleTimeTest, GetIdleNotifyFunc_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "IdleTimeTest GetIdleNotifyFunc_0100 start";
    IdleNotifyStatusCallback callBack = idleTime_->GetIdleNotifyFunc();
    EXPECT_NE(callBack, nullptr);

    GTEST_LOG_(INFO) << "IdleTimeTest GetIdleNotifyFunc_0100 end";
}
}
}