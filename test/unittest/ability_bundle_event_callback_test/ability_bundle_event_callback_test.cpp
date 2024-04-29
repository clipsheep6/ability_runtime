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
#define protected public
#include "ability_bundle_event_callback.h"
#include "ability_event_util.h"
#undef private
#undef protected
#include "hilog_wrapper.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace AAFwk {
class AbilityBundleEventCallbackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void AbilityBundleEventCallbackTest::SetUpTestCase(void) {}
void AbilityBundleEventCallbackTest::TearDownTestCase(void) {}
void AbilityBundleEventCallbackTest::TearDown() {}
void AbilityBundleEventCallbackTest::SetUp() {}

/**
 * @tc.name: AbilityBundleEventCallbackTest_OnReceiveEvent_0100
 * @tc.desc: Test the state of OnReceiveEvent
 * @tc.type: FUNC
 */
HWTEST_F(AbilityBundleEventCallbackTest, OnReceiveEvent_0100, TestSize.Level1)
{
    sptr<AbilityBundleEventCallback> abilityBundleEventCallback_ =
        new (std::nothrow) AbilityBundleEventCallback(nullptr, nullptr);
    EXPECT_NE(abilityBundleEventCallback_, nullptr);
    EventFwk::CommonEventData eventData;
    abilityBundleEventCallback_->OnReceiveEvent(eventData);
    EXPECT_EQ(abilityBundleEventCallback_->taskHandler_, nullptr);
}

/**
 * @tc.name: AbilityBundleEventCallbackTest_OnReceiveEvent_0200
 * @tc.desc: Test the state of OnReceiveEvent
 * @tc.type: FUNC
 */
HWTEST_F(AbilityBundleEventCallbackTest, OnReceiveEvent_0200, TestSize.Level1)
{
    sptr<AbilityBundleEventCallback> abilityBundleEventCallback_ =
        new (std::nothrow) AbilityBundleEventCallback(nullptr, nullptr);
    EXPECT_NE(abilityBundleEventCallback_, nullptr);
    abilityBundleEventCallback_->taskHandler_ = TaskHandlerWrap::CreateQueueHandler("AbilityBundleEventCallbackTest");
    EventFwk::CommonEventData eventData;
    abilityBundleEventCallback_->OnReceiveEvent(eventData);
    EXPECT_NE(abilityBundleEventCallback_->taskHandler_, nullptr);
}
} // namespace AAFwk
} // namespace OHOS
