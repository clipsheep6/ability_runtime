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
#include <gmock/gmock.h>

#include "hilog_wrapper.h"
#include "mission_listener_controller.h"
#include "mission_listener_stub.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
namespace {
int32_t TEST_MISSION_ID = 100;
}
class MyMissionListener : public MissionListenerStub {
public:
    MyMissionListener() = default;
    ~MyMissionListener() = default;

    void OnMissionCreated(int32_t missionId) override
    {
        isMissionCreated_ = true;
    }

    void OnMissionDestroyed(int32_t missionId) override
    {
        isMissionDestroyed_ = true;
    }

    void OnMissionSnapshotChanged(int32_t missionId) override
    {
        isMissionSnapshotChanged_ = true;
    }

    void OnMissionMovedToFront(int32_t missionId) override
    {
        isMissionMovedToFront_ = true;
    }

    void OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<OHOS::Media::PixelMap> &icon) override
    {
        isMissionIconUpdated_ = true;
    }

    void OnMissionClosed(int32_t missionId) override
    {
        isMissionClosed_ = true;
    }

    void OnMissionLabelUpdated(int32_t missionId) override
    {
        isMissionLabelUpdated_ = true;
    }

    bool IsMissionCreated() const
    {
        return isMissionCreated_;
    }

    bool IsMissionDestroyed() const
    {
        return isMissionDestroyed_;
    }

    bool IsMissionSnapshotChanged() const
    {
        return isMissionSnapshotChanged_;
    }

    bool IsMissionMovedToFront() const
    {
        return isMissionMovedToFront_;
    }

    bool IsMissionIconUpdated() const
    {
        return isMissionIconUpdated_;
    }

    bool IsMissionClosed() const
    {
        return isMissionClosed_;
    }

    bool IsMissionLabelUpdated() const
    {
        return isMissionLabelUpdated_;
    }

private:
    bool isMissionCreated_ = false;
    bool isMissionDestroyed_ = false;
    bool isMissionSnapshotChanged_ = false;
    bool isMissionMovedToFront_ = false;
    bool isMissionIconUpdated_ = false;
    bool isMissionClosed_ = false;
    bool isMissionLabelUpdated_ = false;
};

class MissionListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<MissionListenerController> GetController();

private:
    std::shared_ptr<MissionListenerController> controller_;
};

void MissionListenerTest::SetUpTestCase(void) {}

void MissionListenerTest::TearDownTestCase(void) {}

void MissionListenerTest::SetUp(void) {}

void MissionListenerTest::TearDown(void) {}

std::shared_ptr<MissionListenerController> MissionListenerTest::GetController()
{
    if (!controller_) {
        controller_ = std::make_shared<MissionListenerController>();
        controller_->Init();
    }
    return controller_;
}

/**
 * @tc.name: MissionListener_Register_0100
 * @tc.desc: test register and unregister.
 * @tc.type: FUNC
 * @tc.require: I5OB2Y
 */
HWTEST_F(MissionListenerTest, MissionListener_Register_0100, TestSize.Level1)
{
    HILOG_INFO("MissionListener_Register_0100 start");

    auto controller = GetController();
    ASSERT_TRUE(controller);

    sptr<IMissionListener> listener = nullptr;
    auto result = controller->AddMissionListener(listener);
    EXPECT_NE(0, result);

    listener = new MyMissionListener();
    result = controller->AddMissionListener(listener);
    EXPECT_EQ(result, 0);

    controller->DelMissionListener(listener);
    HILOG_INFO("MissionListener_Register_0100 end");
}

/**
 * @tc.name: MissionListener_Callback_0100
 * @tc.desc: test callback function.
 * @tc.type: FUNC
 * @tc.require: I5OB2Y
 */
HWTEST_F(MissionListenerTest, MissionListener_Callback_0100, TestSize.Level2)
{
    HILOG_INFO("MissionListener_Callback_0100 start");

    auto controller = GetController();
    ASSERT_TRUE(controller);

    sptr<MyMissionListener> listener = new MyMissionListener();
    auto result = controller->AddMissionListener(listener);
    EXPECT_EQ(0, result);

    EXPECT_FALSE(listener->IsMissionCreated());
    EXPECT_FALSE(listener->IsMissionDestroyed());
    EXPECT_FALSE(listener->IsMissionSnapshotChanged());
    EXPECT_FALSE(listener->IsMissionMovedToFront());
    EXPECT_FALSE(listener->IsMissionIconUpdated());
    EXPECT_FALSE(listener->IsMissionClosed());
    EXPECT_FALSE(listener->IsMissionLabelUpdated());

    controller->NotifyMissionCreated(TEST_MISSION_ID);
    controller->NotifyMissionDestroyed(TEST_MISSION_ID);
    controller->NotifyMissionSnapshotChanged(TEST_MISSION_ID);
    controller->NotifyMissionIconChanged(TEST_MISSION_ID, nullptr);
    controller->NotifyMissionMovedToFront(TEST_MISSION_ID);
    controller->NotifyMissionClosed(TEST_MISSION_ID);
    controller->NotifyMissionLabelUpdated(TEST_MISSION_ID);

    sleep(2);

    EXPECT_TRUE(listener->IsMissionCreated());
    EXPECT_TRUE(listener->IsMissionDestroyed());
    EXPECT_TRUE(listener->IsMissionSnapshotChanged());
    EXPECT_TRUE(listener->IsMissionMovedToFront());
    EXPECT_TRUE(listener->IsMissionIconUpdated());
    EXPECT_TRUE(listener->IsMissionClosed());
    EXPECT_TRUE(listener->IsMissionLabelUpdated());

    controller->DelMissionListener(listener);

    HILOG_INFO("MissionListener_Callback_0100 end");
}

/**
 * @tc.name: MissionListener_Callback_0200
 * @tc.desc: test callback function.
 * @tc.type: FUNC
 * @tc.require: I5OB2Y
 */
HWTEST_F(MissionListenerTest, MissionListener_Callback_0200, TestSize.Level2)
{
    HILOG_INFO("MissionListener_Callback_0200 start");

    std::shared_ptr<MissionListenerController> controller = std::make_shared<MissionListenerController>();
    ASSERT_TRUE(controller);

    sptr<MyMissionListener> listener = new MyMissionListener();
    auto result = controller->AddMissionListener(listener);
    EXPECT_EQ(0, result);

    controller->NotifyMissionLabelUpdated(TEST_MISSION_ID);

    sleep(2);

    EXPECT_FALSE(listener->IsMissionLabelUpdated());

    HILOG_INFO("MissionListener_Callback_0200 end");
}
}  // namespace AAFwk
}  // namespace OHOS
