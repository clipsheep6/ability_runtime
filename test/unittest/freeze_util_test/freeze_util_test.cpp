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

#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "ipc_object_stub.h"
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class FreezeUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void FreezeUtilTest::SetUpTestCase(void)
{}

void FreezeUtilTest::TearDownTestCase(void)
{}

void FreezeUtilTest::SetUp()
{}

void FreezeUtilTest::TearDown()
{}

/*
 * @tc.number    : FreezeUtilTest_001
 * @tc.name      : FreezeUtilTest
 * @tc.desc      : Test Function FreezeUtil::GetInstance() and AddLifecycleEvent() and GetLifecycleEvent()
 */
HWTEST_F(FreezeUtilTest, FreezeUtilTest_001, TestSize.Level1)
{
    FreezeUtil::LifecycleFlow flow;
    EXPECT_EQ(FreezeUtil::GetInstance().GetLifecycleEvent(flow), "");
    flow.state = FreezeUtil::TimeoutState::FOREGROUND;
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, "firstEntry");
    EXPECT_EQ(FreezeUtil::GetInstance().GetLifecycleEvent(flow), "firstEntry");

    FreezeUtil::GetInstance().AddLifecycleEvent(flow, "secondEntry");
    EXPECT_EQ(FreezeUtil::GetInstance().GetLifecycleEvent(flow), "firstEntry\nsecondEntry");
    TAG_LOGI(AAFwkTag::TEST, "FreezeUtilTest_001 is end");
}

/*
 * @tc.number    : FreezeUtilTest_002
 * @tc.name      : FreezeUtilTest
 * @tc.desc      : Test Function FreezeUtil::GetInstance() and DeleteLifecycleEvent() and GetLifecycleEvent()
 */
HWTEST_F(FreezeUtilTest, FreezeUtilTest_002, TestSize.Level1)
{
    FreezeUtil::LifecycleFlow flow;
    flow.state = FreezeUtil::TimeoutState::LOAD;
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, "testDeleteEntry");
    EXPECT_EQ(FreezeUtil::GetInstance().GetLifecycleEvent(flow), "testDeleteEntry");
    FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);
    EXPECT_EQ(FreezeUtil::GetInstance().GetLifecycleEvent(flow), "");
    TAG_LOGI(AAFwkTag::TEST, "FreezeUtilTest_002 is end");
}

/*
 * @tc.number    : FreezeUtilTest_003
 * @tc.name      : FreezeUtilTest
 * @tc.desc      : Test Function DeleteLifecycleEvent() and DeleteLifecycleEventInner()
 */
HWTEST_F(FreezeUtilTest, FreezeUtilTest_003, TestSize.Level1)
{
    sptr<IRemoteObject> token_(new IPCObjectStub());
    FreezeUtil::LifecycleFlow foregroundFlow = { token_, FreezeUtil::TimeoutState::FOREGROUND };
    FreezeUtil::GetInstance().AddLifecycleEvent(foregroundFlow, "testDeleteLifecyleEventForground");
    EXPECT_EQ(FreezeUtil::GetInstance().GetLifecycleEvent(foregroundFlow), "testDeleteLifecyleEventForground");

    FreezeUtil::LifecycleFlow backgroundFlow = { token_, FreezeUtil::TimeoutState::BACKGROUND };
    FreezeUtil::GetInstance().AddLifecycleEvent(backgroundFlow, "testDeleteLifecyleEventBackground");
    EXPECT_EQ(FreezeUtil::GetInstance().GetLifecycleEvent(backgroundFlow), "testDeleteLifecyleEventBackground");

    FreezeUtil::GetInstance().DeleteLifecycleEvent(token_);
    EXPECT_EQ(FreezeUtil::GetInstance().GetLifecycleEvent(foregroundFlow), "");
    EXPECT_EQ(FreezeUtil::GetInstance().GetLifecycleEvent(backgroundFlow), "");
    TAG_LOGI(AAFwkTag::TEST, "FreezeUtilTest_003 is end");
}
}
}
