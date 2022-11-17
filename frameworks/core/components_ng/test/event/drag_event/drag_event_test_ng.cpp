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

#define private public
#define protected public
#include "gtest/gtest.h"

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/event/drag_event.h"
#include "core/components_ng/event/event_hub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr double GESTURE_EVENT_PROPERTY_VALUE = 10.0;
const PanDirection DRAG_DIRECTION = { PanDirection::LEFT };
const TouchRestrict DRAG_TOUCH_RESTRICT = { TouchRestrict::CLICK };
constexpr int32_t FINGERS_NUMBER = 2;
constexpr int32_t TOUCH_TEST_RESULT_SIZE = 1;
constexpr float DISTANCE = 10.5f;
const float WIDTH = 400.0f;
const float HEIGHT = 400.0f;
const OffsetF COORDINATE_OFFSET(WIDTH, HEIGHT);
} // namespace

class DragEventTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp() override;
    void TearDown() override;
};

void DragEventTestNg::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "DragEventTestNg SetUpTestCase";
}

void DragEventTestNg::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "DragEventTestNg TearDownTestCase";
}

void DragEventTestNg::SetUp()
{
    GTEST_LOG_(INFO) << "DragEventTestNg SetUp";
}

void DragEventTestNg::TearDown()
{
    GTEST_LOG_(INFO) << "DragEventTestNg TearDown";
}

/**
 * @tc.name: DragEventTest001
 * @tc.desc: Create DragEvent and execute its callback functions.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GestureEventFunc as the arguments of the construction of DragEvent.
     */
    double unknownPropertyValue = 0.0;
    GestureEventFunc actionStart = [&unknownPropertyValue](
                                       GestureEvent& info) { unknownPropertyValue = info.GetScale(); };
    GestureEventFunc actionUpdate = [&unknownPropertyValue](
                                        GestureEvent& info) { unknownPropertyValue = info.GetAngle(); };
    GestureEventFunc actionEnd = [&unknownPropertyValue](
                                     GestureEvent& info) { unknownPropertyValue = info.GetOffsetX(); };
    GestureEventNoParameter actionCancel = [&unknownPropertyValue]() {
        unknownPropertyValue = GESTURE_EVENT_PROPERTY_VALUE;
    };

    /**
     * @tc.steps: step2. Create DragEvent.
     */
    const DragEvent dragEvent =
        DragEvent(std::move(actionStart), std::move(actionUpdate), std::move(actionEnd), std::move(actionCancel));

    /**
     * @tc.steps: step3. Get and execute DragEvent ActionStartEvent.
     * @tc.expected: Execute ActionStartEvent which unknownPropertyValue is assigned in.
     */
    GestureEvent info = GestureEvent();
    info.SetScale(GESTURE_EVENT_PROPERTY_VALUE);
    dragEvent.GetActionStartEventFunc()(info);
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);

    /**
     * @tc.steps: step4. Get and execute DragEvent ActionUpdateEvent.
     * @tc.expected: Execute ActionUpdateEvent which unknownPropertyValue is assigned in.
     */
    unknownPropertyValue = 0.0;
    info.SetAngle(GESTURE_EVENT_PROPERTY_VALUE);
    dragEvent.GetActionUpdateEventFunc()(info);
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);

    /**
     * @tc.steps: step5. Get and execute DragEvent ActionEndEvent.
     * @tc.expected: Execute ActionEndEvent which unknownPropertyValue is assigned in.
     */
    unknownPropertyValue = 0.0;
    info.SetOffsetX(GESTURE_EVENT_PROPERTY_VALUE);
    dragEvent.GetActionEndEventFunc()(info);
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);

    /**
     * @tc.steps: step6. Get and execute DragEvent ActionCancelEvent.
     * @tc.expected: Execute ActionCancelEvent which unknownPropertyValue is assigned in.
     */
    unknownPropertyValue = 0.0;
    dragEvent.GetActionCancelEventFunc()();
    EXPECT_EQ(unknownPropertyValue, GESTURE_EVENT_PROPERTY_VALUE);
}

/**
 * @tc.name: DragEventActuatorPropertyTest002
 * @tc.desc: Create DragEventActuator and test its property value.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventActuatorPropertyTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    DragEventActuator dragEventActuator = DragEventActuator(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);

    /**
     * @tc.steps: step2. Get DragEventActuator direction, fingers_ and distance_.
     * @tc.expected:  DragEventActuator's direction, fingers_ and distance_ are equal with the parameters passed in the
     * constructor.
     */
    EXPECT_EQ(dragEventActuator.GetDirection().type, DRAG_DIRECTION.type);
    EXPECT_EQ(dragEventActuator.fingers_, FINGERS_NUMBER);
    EXPECT_EQ(dragEventActuator.distance_, DISTANCE);

    /**
     * @tc.steps: step3. Create DragEvent and set as DragEventActuator's DragEvent and CustomDragEvent.
     * @tc.expected:  Get DragEventActuator's DragEvent and CustomDragEvent which are equal with the DragEvent create
     * before.
     */
    GestureEventFunc actionStart = [](GestureEvent& info) {};
    GestureEventFunc actionUpdate = [](GestureEvent& info) {};
    GestureEventFunc actionEnd = [](GestureEvent& info) {};
    GestureEventNoParameter actionCancel = []() {};

    auto dragEvent = AceType::MakeRefPtr<DragEvent>(
        std::move(actionStart), std::move(actionUpdate), std::move(actionEnd), std::move(actionCancel));
    dragEventActuator.ReplaceDragEvent(dragEvent);
    EXPECT_EQ(dragEvent, dragEventActuator.userCallback_);
    dragEventActuator.SetCustomDragEvent(dragEvent);
    EXPECT_EQ(dragEvent, dragEventActuator.customCallback_);
}

/**
 * @tc.name: DragEventActuatorOnCollectTouchTargetTest003
 * @tc.desc: Create DragEventActuator and invoke OnCollectTouchTarget function.
 * @tc.type: FUNC
 */
HWTEST_F(DragEventTestNg, DragEventActuatorOnCollectTouchTargetTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create DragEventActuator.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto gestureEventHub = AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    DragEventActuator dragEventActuator = DragEventActuator(
        AceType::WeakClaim(AceType::RawPtr(gestureEventHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);

    /**
     * @tc.steps: step2. Invoke OnCollectTouchTarget without setting userCallback_.
     * @tc.expected:  userCallback_ is null and return directly.
     */
    auto getEventTargetImpl = eventHub->CreateGetEventTargetImpl();
    EXPECT_FALSE(getEventTargetImpl == nullptr);
    TouchTestResult finalResult;
    dragEventActuator.OnCollectTouchTarget(COORDINATE_OFFSET, DRAG_TOUCH_RESTRICT, getEventTargetImpl, finalResult);
    EXPECT_TRUE(dragEventActuator.panRecognizer_->onActionStart_ == nullptr);
    EXPECT_TRUE(dragEventActuator.panRecognizer_->onActionStart_ == nullptr);
    EXPECT_TRUE(dragEventActuator.panRecognizer_->onActionUpdate_ == nullptr);
    EXPECT_TRUE(dragEventActuator.panRecognizer_->onActionEnd_ == nullptr);
    EXPECT_TRUE(dragEventActuator.panRecognizer_->onActionCancel_ == nullptr);
    EXPECT_TRUE(finalResult.empty());

    /**
     * @tc.steps: step3. Create DragEvent and set as DragEventActuator's DragEvent.
     * @tc.expected: DragEventActuator's userCallback_ is not null.
     */
    GestureEventFunc actionStart = [](GestureEvent& info) {};
    GestureEventFunc actionUpdate = [](GestureEvent& info) {};
    GestureEventFunc actionEnd = [](GestureEvent& info) {};
    GestureEventNoParameter actionCancel = []() {};
    auto dragEvent = AceType::MakeRefPtr<DragEvent>(
        std::move(actionStart), std::move(actionUpdate), std::move(actionEnd), std::move(actionCancel));
    dragEventActuator.ReplaceDragEvent(dragEvent);
    EXPECT_TRUE(dragEventActuator.userCallback_ != nullptr);

    /**
     * @tc.steps: step3. Invoke OnCollectTouchTarget when userCallback_ is not null.
     * @tc.expected: panRecognizer_ action and finalResult will be assigned value.
     */
    dragEventActuator.OnCollectTouchTarget(COORDINATE_OFFSET, DRAG_TOUCH_RESTRICT, getEventTargetImpl, finalResult);

    EXPECT_FALSE(dragEventActuator.panRecognizer_->onActionStart_ == nullptr);
    EXPECT_FALSE(dragEventActuator.panRecognizer_->onActionUpdate_ == nullptr);
    EXPECT_FALSE(dragEventActuator.panRecognizer_->onActionEnd_ == nullptr);
    EXPECT_FALSE(dragEventActuator.panRecognizer_->onActionCancel_ == nullptr);
    EXPECT_TRUE(finalResult.size() == TOUCH_TEST_RESULT_SIZE);
}
} // namespace OHOS::Ace::NG
