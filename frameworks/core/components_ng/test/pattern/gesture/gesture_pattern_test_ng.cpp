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

#include <optional>

#include "gtest/gtest.h"

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/gestures/gesture_group.h"
#include "core/components_ng/gestures/gesture_referee.h"
#include "core/components_ng/gestures/long_press_gesture.h"
#include "core/components_ng/gestures/pan_gesture.h"
#include "core/components_ng/gestures/pinch_gesture.h"
#include "core/components_ng/gestures/recognizers/click_recognizer.h"
#include "core/components_ng/gestures/recognizers/exclusive_recognizer.h"
#include "core/components_ng/gestures/recognizers/gesture_recognizer.h"
#include "core/components_ng/gestures/recognizers/long_press_recognizer.h"
#include "core/components_ng/gestures/recognizers/multi_fingers_recognizer.h"
#include "core/components_ng/gestures/recognizers/pinch_recognizer.h"
#include "core/components_ng/gestures/recognizers/rotation_recognizer.h"
#include "core/components_ng/gestures/recognizers/swipe_recognizer.h"
#include "core/components_ng/gestures/rotation_gesture.h"
#include "core/components_ng/gestures/swipe_gesture.h"
#include "core/components_ng/gestures/tap_gesture.h"
#include "core/components_ng/layout/layout_property.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t FINGER_NUMBER = 5;
constexpr int32_t FINGER_NUMBER_OVER_MAX = 11;
constexpr int32_t SINGLE_FINGER_NUMBER = 1;
constexpr int32_t COUNT = 2;
constexpr int32_t TAPPED_COUNT = 1;
constexpr int32_t LONG_PRESS_DURATION = 3000;
constexpr double PINCH_GESTURE_DISTANCE = 100.0;
constexpr double ROTATION_GESTURE_ANGLE = 1.0;
constexpr double CLICK_MULTI_TAP_SLOP = 110.0;
constexpr double PI = 360.0;
constexpr double COMMON_VALUE_RANGE_CASE = 101.0;
constexpr double SPECIAL_VALUE_RANGE_CASE1 = 181.0;
constexpr double SPECIAL_VALUE_RANGE_CASE2 = -181.0;
constexpr double SWIPE_SPEED = 10.0;
constexpr double VERTICAL_ANGLE = 90.0;
constexpr double HORIZONTAL_ANGLE = 180.0;
} // namespace

class GesturePatternTestNg : public testing::Test {
public:
};

/**
 * @tc.name: GestureRecognizerTest001
 * @tc.desc: Test ClickRecognizer function: OnAccepted OnRejected
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ClickRecognizerTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ClickRecognizer.
     */
    ClickRecognizer clickRecognizer = ClickRecognizer(FINGER_NUMBER, COUNT);

    /**
     * @tc.steps: step2. call OnAccepted function and compare result.
     * @tc.expected: step2. result equals.
     */
    ClickCallback onClick;
    clickRecognizer.onClick_ = onClick;
    TouchEvent touchEvent;
    touchEvent.tiltX = 0.0f;
    touchEvent.tiltY = 0.0f;
    clickRecognizer.touchPoints_[0] = touchEvent;
    ClickCallback remoteMessage;
    clickRecognizer.remoteMessage_ = remoteMessage;

    clickRecognizer.OnAccepted();
    EXPECT_EQ(clickRecognizer.refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step3. call OnRejected function and compare result.
     * @tc.expected: step3. result equals.
     */
    clickRecognizer.OnRejected();
    EXPECT_EQ(clickRecognizer.refereeState_, RefereeState::FAIL);
}

/**
 * @tc.name: GestureRecognizerTest002
 * @tc.desc: Test ClickRecognizer function: HandleTouchMoveEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ClickRecognizerTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ClickRecognizer.
     */
    ClickRecognizer clickRecognizer = ClickRecognizer(FINGER_NUMBER, COUNT);

    /**
     * @tc.steps: step2. call HandleTouchMoveEvent function and compare result.
     * @tc.steps: case2: refereeState is SUCCESS,return
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    clickRecognizer.refereeState_ = RefereeState::SUCCEED;
    clickRecognizer.HandleTouchMoveEvent(touchEvent);
    EXPECT_EQ(clickRecognizer.touchPoints_.size(), 0);

    /**
     * @tc.steps: step2. call HandleTouchMoveEvent function and compare result.
     * @tc.steps: case1: normal case
     * @tc.expected: step2. result equals.
     */
    clickRecognizer.fingers_ = FINGER_NUMBER_OVER_MAX;
    clickRecognizer.touchPoints_.clear();
    clickRecognizer.touchPoints_[touchEvent.id] = touchEvent;
    clickRecognizer.HandleTouchMoveEvent(touchEvent);
    EXPECT_EQ(clickRecognizer.disposal_, GestureDisposal::NONE);
}

/**
 * @tc.name: GestureRecognizerTest003
 * @tc.desc: Test ClickRecognizer function: ComputeFocusPoint
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ClickRecognizerTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ClickRecognizer.
     */
    ClickRecognizer clickRecognizer = ClickRecognizer(FINGER_NUMBER, COUNT);

    /**
     * @tc.steps: step2. call ComputeFocusPoint function and compare result.
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    clickRecognizer.touchPoints_.clear();
    clickRecognizer.touchPoints_[touchEvent.id] = touchEvent;
    auto focusPoint = clickRecognizer.ComputeFocusPoint();
    EXPECT_EQ(focusPoint.GetX(), 0);
    EXPECT_EQ(focusPoint.GetY(), 0);
}

/**
 * @tc.name: GestureRecognizerTest004
 * @tc.desc: Test ClickRecognizer function: ExceedSlop
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ClickRecognizerTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ClickRecognizer.
     */
    ClickRecognizer clickRecognizer = ClickRecognizer(FINGER_NUMBER, COUNT);

    /**
     * @tc.steps: step2. call ComputeFocusPoint function and compare result.
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    clickRecognizer.touchPoints_.clear();
    clickRecognizer.touchPoints_[touchEvent.id] = touchEvent;
    auto result = clickRecognizer.ExceedSlop();
    EXPECT_EQ(result, false);

    touchEvent.x = CLICK_MULTI_TAP_SLOP;
    touchEvent.y = CLICK_MULTI_TAP_SLOP;
    clickRecognizer.touchPoints_[touchEvent.id] = touchEvent;
    clickRecognizer.tappedCount_ = TAPPED_COUNT;
    clickRecognizer.count_ = COUNT;
    result = clickRecognizer.ExceedSlop();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: GestureRecognizerTest005
 * @tc.desc: Test ClickRecognizer function: ReconcileFrom
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ClickRecognizerTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ClickRecognizer.
     */
    ClickRecognizer clickRecognizer = ClickRecognizer(FINGER_NUMBER, COUNT);
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case1: normal case
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    clickRecognizer.touchPoints_.clear();
    clickRecognizer.touchPoints_[touchEvent.id] = touchEvent;
    auto result = clickRecognizer.ReconcileFrom(clickRecognizerPtr);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case2: recognizerPtr is nullptr
     * @tc.expected: step2. result equals.
     */
    result = clickRecognizer.ReconcileFrom(nullptr);
    EXPECT_EQ(result, false);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case3: recognizerPtr count != count
     * @tc.expected: step2. result equals.
     */
    clickRecognizerPtr->count_ = 1;
    clickRecognizer.count_ = 0;
    result = clickRecognizer.ReconcileFrom(clickRecognizerPtr);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GestureRecognizerTest006
 * @tc.desc: Test ClickRecognizer function: HandleTouchDownEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ClickRecognizerTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ClickRecognizer.
     */
    ClickRecognizer clickRecognizer = ClickRecognizer(FINGER_NUMBER, COUNT);

    /**
     * @tc.steps: step2. call HandleTouchDownEvent function and compare result.
     * @tc.steps: case1: refereeState is SUCCESS,return
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    clickRecognizer.refereeState_ = RefereeState::SUCCEED;
    clickRecognizer.HandleTouchDownEvent(touchEvent);
    EXPECT_EQ(clickRecognizer.touchPoints_.size(), 0);

    /**
     * @tc.steps: step2. call HandleTouchDownEvent function and compare result.
     * @tc.steps: case2: refereeState is PENDING, tappedCount_ = 1, fingers > PointsNum
     * @tc.expected: step2. result equals.
     */
    clickRecognizer.refereeState_ = RefereeState::PENDING;
    clickRecognizer.fingers_ = FINGER_NUMBER;
    clickRecognizer.tappedCount_ = 1;
    clickRecognizer.currentTouchPointsNum_ = 0;
    clickRecognizer.HandleTouchDownEvent(touchEvent);
    EXPECT_EQ(clickRecognizer.currentTouchPointsNum_, 1);
    EXPECT_EQ(clickRecognizer.touchPoints_[touchEvent.id].id, touchEvent.id);

    /**
     * @tc.steps: step2. call HandleTouchDownEvent function and compare result.
     * @tc.steps: case3: refereeState is PENDING, tappedCount_ = 1, fingers = 1
     * @tc.expected: step2. result equals.
     */
    clickRecognizer.refereeState_ = RefereeState::PENDING;
    clickRecognizer.fingers_ = 1;
    clickRecognizer.tappedCount_ = 1;
    clickRecognizer.currentTouchPointsNum_ = 0;
    clickRecognizer.HandleTouchDownEvent(touchEvent);
    EXPECT_EQ(clickRecognizer.equalsToFingers_, true);
    EXPECT_EQ(clickRecognizer.refereeState_, RefereeState::DETECTING);
}

/**
 * @tc.name: GestureRecognizerTest007
 * @tc.desc: Test ClickRecognizer function: HandleTouchUpEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ClickRecognizerTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ClickRecognizer.
     */
    ClickRecognizer clickRecognizer = ClickRecognizer(FINGER_NUMBER, COUNT);

    /**
     * @tc.steps: step2. call HandleTouchUpEvent function and compare result.
     * @tc.steps: case1: refereeState is SUCCESS,return
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    clickRecognizer.currentTouchPointsNum_ = 0;
    clickRecognizer.refereeState_ = RefereeState::SUCCEED;
    clickRecognizer.HandleTouchUpEvent(touchEvent);
    EXPECT_EQ(clickRecognizer.touchPoints_.size(), 0);

    /**
     * @tc.steps: step2. call HandleTouchUpEvent function and compare result.
     * @tc.steps: case2: equalsToFingers = true, currentTouchPointsNum = 1, tap != count
     * @tc.expected: step2. result equals.
     */
    clickRecognizer.currentTouchPointsNum_ = 1;
    clickRecognizer.equalsToFingers_ = true;
    clickRecognizer.useCatchMode_ = false;
    clickRecognizer.refereeState_ = RefereeState::PENDING;
    clickRecognizer.HandleTouchUpEvent(touchEvent);
    EXPECT_EQ(clickRecognizer.equalsToFingers_, false);
    EXPECT_EQ(clickRecognizer.currentTouchPointsNum_, 0);
    EXPECT_EQ(clickRecognizer.touchPoints_[touchEvent.id].id, touchEvent.id);

    /**
     * @tc.steps: step2. call HandleTouchUpEvent function and compare result.
     * @tc.steps: case3: equalsToFingers = true, currentTouchPointsNum = 1,
     * @tc.steps: tap == count, useCatchMode_ = false
     * @tc.expected: step2. result equals.
     */
    clickRecognizer.currentTouchPointsNum_ = 1;
    clickRecognizer.equalsToFingers_ = true;
    clickRecognizer.useCatchMode_ = false;
    clickRecognizer.refereeState_ = RefereeState::PENDING;
    clickRecognizer.tappedCount_ = 0;
    clickRecognizer.count_ = 0;
    clickRecognizer.HandleTouchUpEvent(touchEvent);
    EXPECT_EQ(clickRecognizer.equalsToFingers_, false);
    EXPECT_EQ(clickRecognizer.currentTouchPointsNum_, 0);
    EXPECT_EQ(clickRecognizer.touchPoints_[touchEvent.id].id, touchEvent.id);

    /**
     * @tc.steps: step2. call HandleTouchUpEvent function and compare result.
     * @tc.steps: case3: equalsToFingers = true, currentTouchPointsNum = fingers,
     * @tc.expected: step2. result equals.
     */
    clickRecognizer.currentTouchPointsNum_ = FINGER_NUMBER;
    clickRecognizer.fingers_ = FINGER_NUMBER;
    clickRecognizer.equalsToFingers_ = true;
    clickRecognizer.refereeState_ = RefereeState::PENDING;
    clickRecognizer.HandleTouchUpEvent(touchEvent);
    EXPECT_EQ(clickRecognizer.equalsToFingers_, true);
}

/**
 * @tc.name: GestureRecognizerTest008
 * @tc.desc: Test ClickRecognizer function: HandleTouchCancelEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ClickRecognizerTest008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ClickRecognizer.
     */
    ClickRecognizer clickRecognizer = ClickRecognizer(FINGER_NUMBER, COUNT);

    /**
     * @tc.steps: step2. call HandleTouchUpEvent function and compare result.
     * @tc.steps: case1: refereeState is SUCCESS,return
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    clickRecognizer.currentTouchPointsNum_ = 0;
    clickRecognizer.refereeState_ = RefereeState::SUCCEED;
    clickRecognizer.HandleTouchCancelEvent(touchEvent);
    EXPECT_EQ(clickRecognizer.touchPoints_.size(), 0);

    clickRecognizer.refereeState_ = RefereeState::FAIL;
}

/**
 * @tc.name: GestureRecognizerTest009
 * @tc.desc: Test ClickRecognizer function: SendCallbackMsg
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ClickRecognizerTest009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ClickRecognizer.
     */
    ClickRecognizer clickRecognizer = ClickRecognizer(FINGER_NUMBER, COUNT);

    /**
     * @tc.steps: step2. call SendCallbackMsg function and compare result.
     * @tc.steps: case1: refereeState is SUCCESS,return
     * @tc.expected: step2. result equals.
     */
    std::unique_ptr<GestureEventFunc> onAction;
    TouchEvent touchEvent;
    touchEvent.tiltX = 0.0f;
    touchEvent.tiltY = 0.0f;
    clickRecognizer.touchPoints_[touchEvent.id] = touchEvent;
    clickRecognizer.SendCallbackMsg(onAction);
    EXPECT_EQ(clickRecognizer.touchPoints_.size(), 1);
}

/**
 * @tc.name: ExclusiveRecognizerTest001
 * @tc.desc: Test ExclusiveRecognizer function: OnAccepted OnRejected OnPending OnBlocked
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ExclusiveRecognizerTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ExclusiveRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    ExclusiveRecognizer exclusiveRecognizer = ExclusiveRecognizer(recognizers);

    /**
     * @tc.steps: step2. call OnAccepted function and compare result.
     * @tc.expected: step2. result equals.
     */
    exclusiveRecognizer.OnAccepted();
    EXPECT_EQ(exclusiveRecognizer.refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step3. call OnRejected function and compare result.
     * @tc.expected: step3. result equals.
     */
    exclusiveRecognizer.OnRejected();
    EXPECT_EQ(exclusiveRecognizer.refereeState_, RefereeState::FAIL);

    /**
     * @tc.steps: step4. call OnPending function and compare result.
     * @tc.expected: step4. result equals.
     */
    exclusiveRecognizer.OnPending();
    EXPECT_EQ(exclusiveRecognizer.refereeState_, RefereeState::PENDING);

    /**
     * @tc.steps: step5. call OnBlocked function and compare result.
     * @tc.expected: step5. result equals.
     */
    exclusiveRecognizer.disposal_ = GestureDisposal::ACCEPT;
    exclusiveRecognizer.OnBlocked();
    EXPECT_EQ(exclusiveRecognizer.refereeState_, RefereeState::SUCCEED_BLOCKED);
    exclusiveRecognizer.disposal_ = GestureDisposal::PENDING;
    exclusiveRecognizer.OnBlocked();
    EXPECT_EQ(exclusiveRecognizer.refereeState_, RefereeState::PENDING_BLOCKED);
}

/**
 * @tc.name: ExclusiveRecognizerTest002
 * @tc.desc: Test ExclusiveRecognizer function: HandleEvent OnResetStatus
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ExclusiveRecognizerTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ExclusiveRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    ExclusiveRecognizer exclusiveRecognizer = ExclusiveRecognizer(recognizers);

    /**
     * @tc.steps: step2. create touchEvent and call HandleEvent function and compare result.
     * @tc.expected: step2. result equals.
     */
    bool result;
    TouchEvent touchEvent;
    touchEvent.type = TouchType::MOVE;
    result = exclusiveRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(result, true);
    touchEvent.type = TouchType::DOWN;
    result = exclusiveRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(result, true);
    touchEvent.type = TouchType::UP;
    result = exclusiveRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(result, true);
    touchEvent.type = TouchType::CANCEL;
    result = exclusiveRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step3. create axisEvent and call HandleEvent function and compare result.
     * @tc.expected: step3. result equals.
     */
    AxisEvent axisEvent;
    axisEvent.action = AxisAction::BEGIN;
    result = exclusiveRecognizer.HandleEvent(axisEvent);
    EXPECT_EQ(result, true);
    axisEvent.action = AxisAction::UPDATE;
    result = exclusiveRecognizer.HandleEvent(axisEvent);
    EXPECT_EQ(result, true);
    axisEvent.action = AxisAction::END;
    result = exclusiveRecognizer.HandleEvent(axisEvent);
    EXPECT_EQ(result, true);
    axisEvent.action = AxisAction::NONE;
    result = exclusiveRecognizer.HandleEvent(axisEvent);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step4. call OnResetStatus function and compare result.
     * @tc.expected: step4. result equals.
     */
    exclusiveRecognizer.OnResetStatus();
    EXPECT_EQ(exclusiveRecognizer.activeRecognizer_, nullptr);
}

/**
 * @tc.name: ExclusiveRecognizerTest003
 * @tc.desc: Test ExclusiveRecognizer function: CheckNeedBlocked
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ExclusiveRecognizerTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ExclusiveRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    ExclusiveRecognizer exclusiveRecognizer = ExclusiveRecognizer(recognizers);
    
    /**
     * @tc.steps: step2. call CheckNeedBlocked function and compare result.
     * @tc.expected: step2. result equals.
     */
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    auto result = exclusiveRecognizer.CheckNeedBlocked(clickRecognizerPtr);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: ExclusiveRecognizerTest004
 * @tc.desc: Test ExclusiveRecognizer function: UnBlockGesture
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ExclusiveRecognizerTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ExclusiveRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    ExclusiveRecognizer exclusiveRecognizer = ExclusiveRecognizer(recognizers);
    
    /**
     * @tc.steps: step2. call UnBlockGesture function and compare result.
     * @tc.expected: step2. result equals.
     */
    auto result = exclusiveRecognizer.UnBlockGesture();
    EXPECT_EQ(result, nullptr);

    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    clickRecognizerPtr->refereeState_ = RefereeState::SUCCEED_BLOCKED;
    exclusiveRecognizer.recognizers_.push_back(clickRecognizerPtr);
    result = exclusiveRecognizer.UnBlockGesture();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: ExclusiveRecognizerTest005
 * @tc.desc: Test ExclusiveRecognizer function: BatchAdjudicate, and GestureDisposal
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ExclusiveRecognizerTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ExclusiveRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    ExclusiveRecognizer exclusiveRecognizer = ExclusiveRecognizer(recognizers);
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    
    /**
     * @tc.steps: step2. call GestureDisposal function and compare result.
     * @tc.steps: case1: refereeState_ is SUCCESS, return
     * @tc.expected: step2. result equals.
     */
    exclusiveRecognizer.refereeState_ = RefereeState::SUCCEED;
    exclusiveRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::ACCEPT);
    EXPECT_EQ(exclusiveRecognizer.activeRecognizer_, nullptr);
    
    /**
     * @tc.steps: step2. call GestureDisposal function and compare result.
     * @tc.steps: case2: GestureDisposal::ACCEPT, recognizer is SUCCESS
     * @tc.expected: step2. result equals.
     */
    exclusiveRecognizer.refereeState_ = RefereeState::PENDING;
    clickRecognizerPtr->refereeState_ = RefereeState::SUCCEED;
    exclusiveRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::ACCEPT);
    EXPECT_EQ(exclusiveRecognizer.activeRecognizer_, nullptr);
    
    /**
     * @tc.steps: step2. call GestureDisposal function and compare result.
     * @tc.steps: case3: GestureDisposal::PENDING, recognizer is PENDING
     * @tc.expected: step2. result equals.
     */
    exclusiveRecognizer.refereeState_ = RefereeState::PENDING;
    clickRecognizerPtr->refereeState_ = RefereeState::PENDING;
    exclusiveRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::PENDING);
    EXPECT_EQ(exclusiveRecognizer.activeRecognizer_, nullptr);
    
    /**
     * @tc.steps: step2. call GestureDisposal function and compare result.
     * @tc.steps: case4: GestureDisposal::REJECT, recognizer is FAIL
     * @tc.expected: step2. result equals.
     */
    exclusiveRecognizer.refereeState_ = RefereeState::PENDING;
    clickRecognizerPtr->refereeState_ = RefereeState::FAIL;
    exclusiveRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::REJECT);
    EXPECT_EQ(exclusiveRecognizer.activeRecognizer_, nullptr);
}

/**
 * @tc.name: ExclusiveRecognizerTest006
 * @tc.desc: Test ExclusiveRecognizer function: ReconcileFrom
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ExclusiveRecognizerTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ClickRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    ExclusiveRecognizer exclusiveRecognizer = ExclusiveRecognizer(recognizers);
    RefPtr<ExclusiveRecognizer> exclusiveRecognizerPtr =
        AceType::MakeRefPtr<ExclusiveRecognizer>(recognizers);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case1: normal case
     * @tc.expected: step2. result equals.
     */
    auto result = exclusiveRecognizer.ReconcileFrom(exclusiveRecognizerPtr);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case2: recognizerPtr is nullptr
     * @tc.expected: step2. result equals.
     */
    result = exclusiveRecognizer.ReconcileFrom(nullptr);
    EXPECT_EQ(result, false);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case3: recognizerPtr count != count
     * @tc.expected: step2. result equals.
     */
    exclusiveRecognizer.recognizers_.push_back(nullptr);
    result = exclusiveRecognizer.ReconcileFrom(exclusiveRecognizerPtr);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: LongPressRecognizerTest001
 * @tc.desc: Test LongPressRecognizer function: OnAccepted OnRejected
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, LongPressRecognizerTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create LongPressRecognizer.
     */
    LongPressRecognizer longPressRecognizer =
        LongPressRecognizer(LONG_PRESS_DURATION, FINGER_NUMBER, false);

    /**
     * @tc.steps: step2. call OnAccepted function and compare result.
     * @tc.expected: step2. result equals.
     */
    OnLongPress onLongPress;
    TouchEvent touchEvent;
    longPressRecognizer.touchPoints_[touchEvent.id] = touchEvent;
    longPressRecognizer.repeat_ = true;
    longPressRecognizer.OnAccepted();
    EXPECT_EQ(longPressRecognizer.refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step3. call OnRejected function and compare result.
     * @tc.expected: step3. result equals.
     */
    longPressRecognizer.OnRejected();
    EXPECT_EQ(longPressRecognizer.refereeState_, RefereeState::FAIL);
}

/**
 * @tc.name: LongPressRecognizerTest002
 * @tc.desc: Test LongPressRecognizer function: HandleTouchMoveEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, LongPressRecognizerTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create LongPressRecognizer.
     */
    LongPressRecognizer longPressRecognizer =
        LongPressRecognizer(LONG_PRESS_DURATION, FINGER_NUMBER, false);

    /**
     * @tc.steps: step2. call HandleTouchUpEvent function and compare result.
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    longPressRecognizer.touchPoints_[touchEvent.id] = touchEvent;
    longPressRecognizer.HandleTouchMoveEvent(touchEvent);
    EXPECT_EQ(longPressRecognizer.time_, touchEvent.time);
}

/**
 * @tc.name: LongPressRecognizerTest003
 * @tc.desc: Test LongPressRecognizer function: HandleTouchDownEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, LongPressRecognizerTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create LongPressRecognizer.
     */
    LongPressRecognizer longPressRecognizer =
        LongPressRecognizer(LONG_PRESS_DURATION, FINGER_NUMBER, false);

    /**
     * @tc.steps: step2. call HandleTouchUpEvent function and compare result.
     * @tc.steps: case1: pointsCount == fingers, useCatchMode_ is true
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    longPressRecognizer.HandleTouchDownEvent(touchEvent);
    EXPECT_EQ(longPressRecognizer.touchPoints_.size(), 1);
    
    /**
     * @tc.steps: step2. call HandleTouchUpEvent function and compare result.
     * @tc.steps: case2: pointsCount == fingers, useCatchMode_ is true
     * @tc.expected: step2. result equals.
     */
    touchEvent.sourceType = SourceType::MOUSE;
    longPressRecognizer.isForDrag_ = true;
    longPressRecognizer.touchPoints_[touchEvent.id] = touchEvent;
    longPressRecognizer.fingers_ = 1;
    longPressRecognizer.useCatchMode_ = true;
    longPressRecognizer.HandleTouchDownEvent(touchEvent);
    EXPECT_EQ(longPressRecognizer.globalPoint_.GetX(), touchEvent.x);
    EXPECT_EQ(longPressRecognizer.globalPoint_.GetY(), touchEvent.y);
    EXPECT_EQ(longPressRecognizer.refereeState_, RefereeState::DETECTING);

    /**
     * @tc.steps: step2. call HandleTouchUpEvent function and compare result.
     * @tc.steps: case3: pointsCount == fingers, useCatchMode_ is false
     * @tc.expected: step2. result equals.
     */
    longPressRecognizer.useCatchMode_ = false;
    longPressRecognizer.HandleTouchDownEvent(touchEvent);
    EXPECT_EQ(longPressRecognizer.refereeState_, RefereeState::DETECTING);
}

/**
 * @tc.name: LongPressRecognizerTest004
 * @tc.desc: Test LongPressRecognizer function: HandleTouchCancelEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, LongPressRecognizerTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create LongPressRecognizer.
     */
    LongPressRecognizer longPressRecognizer =
        LongPressRecognizer(LONG_PRESS_DURATION, FINGER_NUMBER, false);

    /**
     * @tc.steps: step2. call HandleTouchUpEvent function and compare result.
     * @tc.steps: refereeState == RefereeState::SUCCEED
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    longPressRecognizer.refereeState_ = RefereeState::SUCCEED;
    longPressRecognizer.HandleTouchCancelEvent(touchEvent);
    EXPECT_EQ(longPressRecognizer.touchPoints_.size(), 0);
}

/**
 * @tc.name: LongPressRecognizerTest005
 * @tc.desc: Test LongPressRecognizer function: SendCallbackMsg
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, LongPressRecognizerTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create LongPressRecognizer.
     */
    LongPressRecognizer longPressRecognizer =
        LongPressRecognizer(LONG_PRESS_DURATION, FINGER_NUMBER, false);

    /**
     * @tc.steps: step2. call SendCallbackMsg function and compare result.
     * @tc.steps: case1: refereeState is SUCCESS,return
     * @tc.expected: step2. result equals.
     */
    std::unique_ptr<GestureEventFunc> onAction;
    TouchEvent touchEvent;
    touchEvent.tiltX = 0.0f;
    touchEvent.tiltY = 0.0f;
    longPressRecognizer.touchPoints_[touchEvent.id] = touchEvent;
    longPressRecognizer.SendCallbackMsg(onAction, false);
    EXPECT_EQ(longPressRecognizer.touchPoints_.size(), 1);
}

/**
 * @tc.name: LongPressRecognizerTest006
 * @tc.desc: Test LongPressRecognizer function: ReconcileFrom
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, LongPressRecognizerTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create LongPressRecognizer.
     */
    LongPressRecognizer longPressRecognizer =
        LongPressRecognizer(LONG_PRESS_DURATION, FINGER_NUMBER, false);
    RefPtr<LongPressRecognizer> longPressRecognizerPtr =
        AceType::MakeRefPtr<LongPressRecognizer>(LONG_PRESS_DURATION, FINGER_NUMBER, false);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case1: normal case
     * @tc.expected: step2. result equals.
     */
    auto result = longPressRecognizer.ReconcileFrom(longPressRecognizerPtr);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case2: recognizerPtr is nullptr
     * @tc.expected: step2. result equals.
     */
    result = longPressRecognizer.ReconcileFrom(nullptr);
    EXPECT_EQ(result, false);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case3: recognizerPtr count != count
     * @tc.expected: step2. result equals.
     */
    longPressRecognizer.duration_ = 0;
    result = longPressRecognizer.ReconcileFrom(longPressRecognizerPtr);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: PanRecognizerTest001
 * @tc.desc: Test PanRecognizer function: OnAccepted OnRejected
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);

    /**
     * @tc.steps: step2. call OnAccepted function and compare result.
     * @tc.expected: step2. result equals.
     */
    panRecognizer.OnAccepted();
    EXPECT_EQ(panRecognizer.refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step3. call OnRejected function and compare result.
     * @tc.expected: step3. result equals.
     */
    panRecognizer.OnRejected();
    EXPECT_EQ(panRecognizer.refereeState_, RefereeState::FAIL);
}

/**
 * @tc.name: PanRecognizerTest002
 * @tc.desc: Test PanRecognizer function: HandleTouchDownEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);

    /**
     * @tc.steps: step2. call HandleTouchDown function and compare result.
     * @tc.steps: case1: call TouchEvent, refereeState is SUCCEED, return
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    panRecognizer.refereeState_ = RefereeState::SUCCEED;
    panRecognizer.HandleTouchDownEvent(touchEvent);
    EXPECT_EQ(panRecognizer.touchPoints_.size(), 0);

    /**
     * @tc.steps: step2. call HandleTouchDown function and compare result.
     * @tc.steps: case2: call TouchEvent, normal case
     * @tc.expected: step2. result equals.
     */
    panRecognizer.refereeState_ = RefereeState::PENDING;
    panRecognizer.HandleTouchDownEvent(touchEvent);
    EXPECT_EQ(panRecognizer.deviceId_, touchEvent.deviceId);
    EXPECT_EQ(panRecognizer.deviceType_, touchEvent.sourceType);
    EXPECT_EQ(panRecognizer.inputEventType_, InputEventType::TOUCH_SCREEN);

    /**
     * @tc.steps: step2. call HandleTouchDown function and compare result.
     * @tc.steps: case3: call TouchEvent, set fingerNum = fingers_
     * @tc.expected: step2. result equals.
     */
    panRecognizer.fingers_ = 1;
    panRecognizer.HandleTouchDownEvent(touchEvent);
    EXPECT_EQ(panRecognizer.refereeState_, RefereeState::DETECTING);

    /**
     * @tc.steps: step2. call HandleTouchDown function and compare result.
     * @tc.steps: case4: call AxisEvent, refereeState is SUCCEED, return
     * @tc.expected: step2. result equals.
     */
    AxisEvent axisEvent;
    panRecognizer.refereeState_ = RefereeState::SUCCEED;
    panRecognizer.touchPoints_.clear();
    panRecognizer.HandleTouchDownEvent(axisEvent);
    EXPECT_EQ(panRecognizer.touchPoints_.size(), 0);

    /**
     * @tc.steps: step2. call HandleTouchDown function and compare result.
     * @tc.steps: case5: call AxisEvent, refereeState is PENDING, normal case
     * @tc.expected: step2. result equals.
     */
    panRecognizer.refereeState_ = RefereeState::PENDING;
    panRecognizer.HandleTouchDownEvent(axisEvent);
    EXPECT_EQ(panRecognizer.deviceId_, axisEvent.deviceId);
    EXPECT_EQ(panRecognizer.deviceType_, axisEvent.sourceType);
    EXPECT_EQ(panRecognizer.inputEventType_, InputEventType::AXIS);
    EXPECT_EQ(panRecognizer.refereeState_, RefereeState::DETECTING);
}

/**
 * @tc.name: PanRecognizerTest003
 * @tc.desc: Test PanRecognizer function: HandleTouchUpEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);

    /**
     * @tc.steps: step2. call HandleTouchUp function and compare result.
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    panRecognizer.refereeState_ = RefereeState::SUCCEED;
    panRecognizer.HandleTouchUpEvent(touchEvent);
    EXPECT_EQ(panRecognizer.globalPoint_.GetX(), touchEvent.x);
    EXPECT_EQ(panRecognizer.globalPoint_.GetY(), touchEvent.y);
    EXPECT_EQ(panRecognizer.lastTouchEvent_.id, touchEvent.id);

    AxisEvent axisEvent;
    panRecognizer.refereeState_ = RefereeState::SUCCEED;
    panRecognizer.HandleTouchUpEvent(axisEvent);
    EXPECT_EQ(panRecognizer.globalPoint_.GetX(), axisEvent.x);
    EXPECT_EQ(panRecognizer.globalPoint_.GetY(), axisEvent.y);
}

/**
 * @tc.name: PanRecognizerTest004
 * @tc.desc: Test PanRecognizer function: HandleTouchMoveEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);

    /**
     * @tc.steps: step2. call HandleTouchUp function and compare result.
     * @tc.steps: case1: direction.type is VERTICAL
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    panRecognizer.refereeState_ = RefereeState::SUCCEED;
    panRecognizer.touchPoints_[touchEvent.id] = touchEvent;
    panRecognizer.direction_.type = PanDirection::VERTICAL;
    panRecognizer.isFlushTouchEventsEnd_ = true;
    panRecognizer.HandleTouchMoveEvent(touchEvent);
    EXPECT_EQ(panRecognizer.globalPoint_.GetX(), touchEvent.x);
    EXPECT_EQ(panRecognizer.globalPoint_.GetY(), touchEvent.y);
    EXPECT_EQ(panRecognizer.delta_.GetX(), 0);
    EXPECT_EQ(panRecognizer.delta_.GetY(), 0);
    EXPECT_EQ(panRecognizer.averageDistance_.GetX(), 0);

    /**
     * @tc.steps: step2. call HandleTouchUp function and compare result.
     * @tc.steps: case2: direction.type is HORIZONTAL
     * @tc.expected: step2. result equals.
     */
    panRecognizer.direction_.type = PanDirection::HORIZONTAL;
    panRecognizer.HandleTouchMoveEvent(touchEvent);
    EXPECT_EQ(panRecognizer.averageDistance_.GetY(), 0);
}

/**
 * @tc.name: PanRecognizerTest005
 * @tc.desc: Test PanRecognizer function: OnFlushTouchEvents begin end
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);

    /**
     * @tc.steps: step2. call HandleTouchUp function and compare result.
     * @tc.expected: step2. result equals.
     */
    panRecognizer.OnFlushTouchEventsBegin();
    EXPECT_EQ(panRecognizer.isFlushTouchEventsEnd_, false);
    panRecognizer.OnFlushTouchEventsEnd();
    EXPECT_EQ(panRecognizer.isFlushTouchEventsEnd_, true);
}

/**
 * @tc.name: PanRecognizerTest006
 * @tc.desc: Test PanRecognizer function: IsPanGestureAccept
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);

    /**
     * @tc.steps: step2. case1: PanDirection::ALL, fabs(offset) >= distance_.
     * @tc.expected: step2. return ACCEPT.
     */
    PanRecognizer::GestureAcceptResult result;
    panRecognizer.direction_.type = PanDirection::ALL;
    result = panRecognizer.IsPanGestureAccept();
    EXPECT_EQ(result, PanRecognizer::GestureAcceptResult::ACCEPT);

    /**
     * @tc.steps: step2. case2: PanDirection::ALL, fabs(offset) < distance_.
     * @tc.expected: step2. return DETECTING.
     */
    panRecognizer.direction_.type = PanDirection::ALL;
    panRecognizer.distance_ = 1;
    result = panRecognizer.IsPanGestureAccept();
    EXPECT_EQ(result, PanRecognizer::GestureAcceptResult::DETECTING);

    /**
     * @tc.steps: step2. case3: GetX > GetY, GetX < distance, PanDirection::HORIZONTAL.
     * @tc.expected: step2. return DETECTING.
     */
    panRecognizer.direction_.type = PanDirection::HORIZONTAL;
    panRecognizer.averageDistance_ = Offset(0, -1);
    panRecognizer.distance_ = 1;
    result = panRecognizer.IsPanGestureAccept();
    EXPECT_EQ(result, PanRecognizer::GestureAcceptResult::DETECTING);
    
    /**
     * @tc.steps: step2. case4: GetX > GetY, GetX > 0, GetX > distance_, PanDirection::LEFT.
     * @tc.expected: step2. return REJECT.
     */
    panRecognizer.direction_.type = PanDirection::LEFT;
    panRecognizer.averageDistance_ = Offset(1, 0);
    panRecognizer.distance_ = 0;
    result = panRecognizer.IsPanGestureAccept();
    EXPECT_EQ(result, PanRecognizer::GestureAcceptResult::REJECT);
    
    /**
     * @tc.steps: step2. case5: GetX > GetY, GetX > 0, GetX > distance_, PanDirection::VERTICAL.
     * @tc.expected: step2. return REJECT.
     */
    panRecognizer.direction_.type = PanDirection::VERTICAL;
    panRecognizer.averageDistance_ = Offset(1, 0);
    panRecognizer.distance_ = 0;
    result = panRecognizer.IsPanGestureAccept();
    EXPECT_EQ(result, PanRecognizer::GestureAcceptResult::DETECTING);

    /**
     * @tc.steps: step2. case6: GetX > GetY, GetX = distance_ = 0, PanDirection::VERTICAL.
     * @tc.expected: step2. return ACCEPT.
     */
    panRecognizer.direction_.type = PanDirection::VERTICAL;
    panRecognizer.averageDistance_ = Offset(0, -1);
    panRecognizer.distance_ = 0;
    result = panRecognizer.IsPanGestureAccept();
    EXPECT_EQ(result, PanRecognizer::GestureAcceptResult::ACCEPT);
}

/**
 * @tc.name: PanRecognizerTest007
 * @tc.desc: Test PanRecognizer function: IsPanGestureAccept
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);

    /**
     * @tc.steps: step2. case7: GetX <= GetY, GetY < distance_, PanDirection::VERTICAL.
     * @tc.expected: step2. return DETECTING.
     */
    PanRecognizer::GestureAcceptResult result;
    panRecognizer.direction_.type = PanDirection::VERTICAL;
    panRecognizer.averageDistance_ = Offset(-1, 0);
    panRecognizer.distance_ = 1;
    result = panRecognizer.IsPanGestureAccept();
    EXPECT_EQ(result, PanRecognizer::GestureAcceptResult::DETECTING);
    
    /**
     * @tc.steps: step2. case8: GetX <= GetY, GetY > distance_, GetY < 0, PanDirection::DOWN.
     * @tc.expected: step2. return ACCEPT.
     */
    panRecognizer.direction_.type = PanDirection::DOWN;
    panRecognizer.averageDistance_ = Offset(-1, 1);
    panRecognizer.distance_ = 0;
    result = panRecognizer.IsPanGestureAccept();
    EXPECT_EQ(result, PanRecognizer::GestureAcceptResult::ACCEPT);
    
    /**
     * @tc.steps: step2. case9: GetX <= GetY, GetY > distance_, GetY > 0, PanDirection::UP.
     * @tc.expected: step2. return REJECT.
     */
    panRecognizer.direction_.type = PanDirection::UP;
    panRecognizer.averageDistance_ = Offset(-1, 1);
    panRecognizer.distance_ = 0;
    result = panRecognizer.IsPanGestureAccept();
    EXPECT_EQ(result, PanRecognizer::GestureAcceptResult::REJECT);

    /**
     * @tc.steps: step2. case10: GetX <= GetY, GetY = distance_ = 0, PanDirection::UP.
     * @tc.expected: step2. return DETECTING.
     */
    panRecognizer.direction_.type = PanDirection::UP;
    panRecognizer.averageDistance_ = Offset(-1, 0);
    panRecognizer.distance_ = 0;
    result = panRecognizer.IsPanGestureAccept();
    EXPECT_EQ(result, PanRecognizer::GestureAcceptResult::DETECTING);
}

/**
 * @tc.name: PanRecognizerTest008
 * @tc.desc: Test PanRecognizer function: HandleTouchCancelEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);

    /**
     * @tc.steps: step2. call HandleTouchCancel function and compare result.
     * @tc.steps: case1: input is TouchEvent
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    panRecognizer.refereeState_ = RefereeState::SUCCEED;
    panRecognizer.HandleTouchCancelEvent(touchEvent);
    EXPECT_EQ(panRecognizer.touchPoints_.size(), 0);

    /**
     * @tc.steps: step2. call HandleTouchCancel function and compare result.
     * @tc.steps: case2: input is AxisEvent
     * @tc.expected: step2. result equals.
     */
    AxisEvent axisEvent;
    panRecognizer.refereeState_ = RefereeState::SUCCEED;
    panRecognizer.HandleTouchCancelEvent(touchEvent);
    EXPECT_EQ(panRecognizer.touchPoints_.size(), 0);
}

/**
 * @tc.name: PanRecognizerTest009
 * @tc.desc: Test PanRecognizer function: SendCallbackMsg
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);

    /**
     * @tc.steps: step2. call SendCallbackMsg function and compare result.
     * @tc.steps: case1: inputEventType is AXIS
     * @tc.expected: step2. result equals.
     */
    std::unique_ptr<GestureEventFunc> callback;
    TouchEvent touchEvent;
    touchEvent.tiltX = 0.0f;
    touchEvent.tiltY = 0.0f;
    panRecognizer.lastTouchEvent_ = touchEvent;
    panRecognizer.inputEventType_ = InputEventType::AXIS;
    panRecognizer.SendCallbackMsg(callback);
    EXPECT_EQ(panRecognizer.touchPoints_.size(), 0);

    /**
     * @tc.steps: step2. call SendCallbackMsg function and compare result.
     * @tc.steps: case2: inputEventType is AXIS
     * @tc.expected: step2. result equals.
     */
    panRecognizer.inputEventType_ = InputEventType::KEYBOARD;
    panRecognizer.SendCallbackMsg(callback);
    EXPECT_EQ(panRecognizer.touchPoints_.size(), 0);
}

/**
 * @tc.name: PanRecognizerTest010
 * @tc.desc: Test PanRecognizer function: ReconcileFrom
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);
    RefPtr<PanRecognizer> panRecognizerPtr =
        AceType::MakeRefPtr<PanRecognizer>(panGestureOption);
    
    /**
     * @tc.steps: step2. call ReconcileFrom function
     * @tc.steps: case1: recognizer is nullptr
     * @tc.expected: step2. result equals.
     */
    auto result = panRecognizer.ReconcileFrom(nullptr);
    EXPECT_EQ(result, false);

    /**
     * @tc.steps: step2. call ReconcileFrom function
     * @tc.steps: case2: recognizer is normal
     * @tc.expected: step2. result equals.
     */
    result = panRecognizer.ReconcileFrom(panRecognizerPtr);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step2. call ReconcileFrom function
     * @tc.steps: case3: recognizer is normal but need reset
     * @tc.expected: step2. result equals.
     */
    panRecognizer.fingers_ = 1;
    result = panRecognizer.ReconcileFrom(panRecognizerPtr);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: PanRecognizerTest011
 * @tc.desc: Test PanRecognizer function: SetDirection
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);
    PanDirection panDirection;
    
    /**
     * @tc.steps: step2. call SetDirection function
     * @tc.steps: case1: panDirection is VERTICAL
     * @tc.expected: step2. result equals.
     */
    panDirection.type = PanDirection::VERTICAL;
    panRecognizer.SetDirection(panDirection);
    EXPECT_EQ(panRecognizer.velocityTracker_.mainAxis_, Axis::VERTICAL);

    /**
     * @tc.steps: step2. call SetDirection function
     * @tc.steps: case2: panDirection is HORIZONTAL
     * @tc.expected: step2. result equals.
     */
    panDirection.type = PanDirection::HORIZONTAL;
    panRecognizer.SetDirection(panDirection);
    EXPECT_EQ(panRecognizer.velocityTracker_.mainAxis_, Axis::HORIZONTAL);
}

/**
 * @tc.name: PanRecognizerTest012
 * @tc.desc: Test PanRecognizer function: ChangeFingers
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);
    
    /**
     * @tc.steps: step2. call ChangeFingers function
     * @tc.expected: step2. result equals.
     */
    panRecognizer.fingers_ = 0;
    panRecognizer.ChangeFingers(1);
    EXPECT_EQ(panRecognizer.newFingers_, 1);
}

/**
 * @tc.name: PanRecognizerTest013
 * @tc.desc: Test PanRecognizer function: ChangeDirection
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);
    
    /**
     * @tc.steps: step2. call ChangeDirection function
     * @tc.expected: step2. result equals.
     */
    PanDirection panDirection;
    panDirection.type = PanDirection::VERTICAL;
    panRecognizer.direction_.type = PanDirection::HORIZONTAL;
    panRecognizer.ChangeDirection(panDirection);
    EXPECT_EQ(panRecognizer.direction_.type, PanDirection::VERTICAL);
    EXPECT_EQ(panRecognizer.newDirection_.type, PanDirection::VERTICAL);
}

/**
 * @tc.name: PanRecognizerTest014
 * @tc.desc: Test PanRecognizer function: ChangeDistance
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);
    
    /**
     * @tc.steps: step2. call ChangeDistance function
     * @tc.expected: step2. result equals.
     */
    panRecognizer.distance_ = 0;
    panRecognizer.ChangeDistance(1);
    panRecognizer.refereeState_ = RefereeState::READY;
    EXPECT_EQ(panRecognizer.distance_, 1);
    EXPECT_EQ(panRecognizer.newDistance_, 1);
}

/**
 * @tc.name: PanRecognizerTest015
 * @tc.desc: Test PanRecognizer function: GetMainAxisDelta
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PanRecognizerTest015, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PanRecognizer.
     */
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    PanRecognizer panRecognizer = PanRecognizer(panGestureOption);
    
    /**
     * @tc.steps: step2. call ChangeDistance function
     * @tc.steps: case1: directionType is ALL
     * @tc.expected: step2. result equals.
     */
    PanDirection panDirection;
    panRecognizer.direction_ = panDirection;
    panRecognizer.direction_.type = PanDirection::ALL;
    auto result = panRecognizer.GetMainAxisDelta();
    EXPECT_EQ(result, 0);

    /**
     * @tc.steps: step2. call ChangeDistance function
     * @tc.steps: case2: directionType is HORIZONTAL
     * @tc.expected: step2. result equals.
     */
    panRecognizer.direction_.type = PanDirection::HORIZONTAL;
    result = panRecognizer.GetMainAxisDelta();
    EXPECT_EQ(result, 0);

    /**
     * @tc.steps: step2. call ChangeDistance function
     * @tc.steps: case3: directionType is VERTICAL
     * @tc.expected: step2. result equals.
     */
    panRecognizer.direction_.type = PanDirection::VERTICAL;
    result = panRecognizer.GetMainAxisDelta();
    EXPECT_EQ(result, 0);

    /**
     * @tc.steps: step2. call ChangeDistance function
     * @tc.steps: case4: directionType is LEFT
     * @tc.expected: step2. result equals.
     */
    panRecognizer.direction_.type = PanDirection::LEFT;
    result = panRecognizer.GetMainAxisDelta();
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: ParallelRecognizerTest001
 * @tc.desc: Test ParallelRecognizer function: OnAccepted OnRejected OnPending OnBlock
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ParallelRecognizerTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ParallelRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    ParallelRecognizer parallelRecognizer = ParallelRecognizer(recognizers);
    RefPtr<NGGestureRecognizer> gestureRecognizerPtr;
    parallelRecognizer.currentBatchRecognizer_ = gestureRecognizerPtr;
    
    /**
     * @tc.steps: step2. call OnAccepted function and compare result.
     * @tc.expected: step2. result equals.
     */
    parallelRecognizer.OnAccepted();
    EXPECT_EQ(parallelRecognizer.refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step3. call OnRejected function and compare result.
     * @tc.expected: step3. result equals.
     */
    parallelRecognizer.OnRejected();
    EXPECT_EQ(parallelRecognizer.refereeState_, RefereeState::FAIL);

    /**
     * @tc.steps: step4. call OnPending function and compare result.
     * @tc.expected: step4. result equals.
     */
    parallelRecognizer.OnPending();
    EXPECT_EQ(parallelRecognizer.refereeState_, RefereeState::PENDING);

    /**
     * @tc.steps: step5. call OnBlocked function and compare result.
     * @tc.steps: case1: disposal is ACCEPT
     * @tc.expected: step5. result equals.
     */
    parallelRecognizer.disposal_ = GestureDisposal::ACCEPT;
    parallelRecognizer.OnBlocked();
    EXPECT_EQ(parallelRecognizer.refereeState_, RefereeState::SUCCEED_BLOCKED);

    /**
     * @tc.steps: step5. call OnBlocked function and compare result.
     * @tc.steps: case2: disposal is PENDING
     * @tc.expected: step5. result equals.
     */
    parallelRecognizer.disposal_ = GestureDisposal::PENDING;
    parallelRecognizer.OnBlocked();
    EXPECT_EQ(parallelRecognizer.refereeState_, RefereeState::PENDING_BLOCKED);
}

/**
 * @tc.name: ParallelRecognizerTest002
 * @tc.desc: Test ParallelRecognizer function: HandleEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ParallelRecognizerTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ParallelRecognizer.
     */
    RefPtr<NGGestureRecognizer> gestureRecognizerPtr;
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {gestureRecognizerPtr};
    ParallelRecognizer parallelRecognizer = ParallelRecognizer(recognizers);
    
    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.expected: step2. result equals.
     */
    parallelRecognizer.refereeState_ = RefereeState::READY;
    TouchEvent touchEvent;
    auto result = parallelRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(parallelRecognizer.refereeState_, RefereeState::DETECTING);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: ParallelRecognizerTest003
 * @tc.desc: Test ParallelRecognizer function: BatchAdjudicate
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ParallelRecognizerTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ParallelRecognizer.
     */
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    ParallelRecognizer parallelRecognizer = ParallelRecognizer(recognizers);
    
    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case1: disposal is ACCEPT, refereeState is SUCCEED
     * @tc.expected: step2. expect return
     */
    clickRecognizerPtr->refereeState_ = RefereeState::SUCCEED;
    parallelRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::ACCEPT);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case2: disposal is ACCEPT, refereeState is PENDING, refereeState_ is SUCCEED
     * @tc.expected: step2. expect SUCCESS
     */
    clickRecognizerPtr->refereeState_ = RefereeState::PENDING;
    parallelRecognizer.refereeState_ = RefereeState::SUCCEED;
    parallelRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::ACCEPT);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case3: disposal is ACCEPT, refereeState is PENDING, refereeState_ is PENDING_BLOCKED
     * @tc.expected: step2. expect PENDING
     */
    clickRecognizerPtr->refereeState_ = RefereeState::PENDING;
    parallelRecognizer.refereeState_ = RefereeState::PENDING_BLOCKED;
    parallelRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::ACCEPT);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::PENDING);

    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case4: disposal is REJECT, refereeState is FAIL
     * @tc.expected: step2. expect FAIL
     */
    clickRecognizerPtr->refereeState_ = RefereeState::FAIL;
    parallelRecognizer.refereeState_ = RefereeState::FAIL;
    parallelRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::REJECT);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::FAIL);

    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case5: disposal is PENDING, refereeState is PENDING, refereeState_ is PENDING
     * @tc.expected: step2. expect PENDING
     */
    clickRecognizerPtr->refereeState_ = RefereeState::PENDING;
    parallelRecognizer.refereeState_ = RefereeState::PENDING;
    parallelRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::PENDING);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::PENDING);

    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case6: disposal is PENDING, refereeState is READY, refereeState_ is PENDING
     * @tc.expected: step2. expect PENDING
     */
    clickRecognizerPtr->refereeState_ = RefereeState::READY;
    parallelRecognizer.refereeState_ = RefereeState::PENDING;
    parallelRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::PENDING);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::PENDING);

    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case7: disposal is PENDING, refereeState is READY, refereeState_ is PENDING_BLOCKED
     * @tc.expected: step2. expect PENDING_CLOCKED
     */
    clickRecognizerPtr->refereeState_ = RefereeState::READY;
    parallelRecognizer.refereeState_ = RefereeState::PENDING_BLOCKED;
    parallelRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::PENDING);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::READY);
}

/**
 * @tc.name: ParallelRecognizerTest004
 * @tc.desc: Test ParallelRecognizer function: ReconcileFrom
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, ParallelRecognizerTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ParallelRecognizer.
     */
    RefPtr<NGGestureRecognizer> gestureRecognizerPtr;
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {gestureRecognizerPtr};
    ParallelRecognizer parallelRecognizer = ParallelRecognizer(recognizers);
    RefPtr<ParallelRecognizer> parallelRecognizerPtr =
        AceType::MakeRefPtr<ParallelRecognizer>(recognizers);
    bool result = false;

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case1: normal case
     * @tc.expected: step2. result equals.
     */
    result = parallelRecognizer.ReconcileFrom(parallelRecognizerPtr);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case2: recognizerPtr is nullptr
     * @tc.expected: step2. result equals.
     */
    result = parallelRecognizer.ReconcileFrom(nullptr);
    EXPECT_EQ(result, false);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case3: recognizers_ = {nullptr};
     * @tc.expected: step2. result equals.
     */
    parallelRecognizer.recognizers_ = {nullptr};
    result = parallelRecognizer.ReconcileFrom(parallelRecognizerPtr);
    EXPECT_EQ(result, false);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case4: recognizers_ = {};
     * @tc.expected: step2. result equals.
     */
    parallelRecognizer.recognizers_ = {};
    result = parallelRecognizer.ReconcileFrom(parallelRecognizerPtr);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case5: curr->priorityMask_ != priorityMask_
     * @tc.expected: step2. result equals.
     */
    parallelRecognizer.priorityMask_ = GestureMask::Normal;
    parallelRecognizerPtr->priorityMask_ = GestureMask::Begin;
    result = parallelRecognizer.ReconcileFrom(parallelRecognizerPtr);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: PinchRecognizerTest001
 * @tc.desc: Test PinchRecognizer function: OnAccepted OnRejected
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PinchRecognizerTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PinchRecognizer.
     */
    PinchRecognizer pinchRecognizer = PinchRecognizer(FINGER_NUMBER, PINCH_GESTURE_DISTANCE);

    /**
     * @tc.steps: step2. call OnAccepted function and compare result.
     * @tc.expected: step2. result equals.
     */
    pinchRecognizer.OnAccepted();
    EXPECT_EQ(pinchRecognizer.refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step3. call OnRejected function and compare result.
     * @tc.expected: step3. result equals.
     */
    pinchRecognizer.OnRejected();
    EXPECT_EQ(pinchRecognizer.refereeState_, RefereeState::FAIL);
}

/**
 * @tc.name: PinchRecognizerTest002
 * @tc.desc: Test PinchRecognizer function: HandleTouchDownEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PinchRecognizerTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PinchRecognizer.
     */
    PinchRecognizer pinchRecognizer = PinchRecognizer(SINGLE_FINGER_NUMBER, PINCH_GESTURE_DISTANCE);

    /**
     * @tc.steps: step2. call HandleTouchDownEvent function and compare result.
     * @tc.steps: case1: input is TouchEvent. refereeState_ is SUCCESS.
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    pinchRecognizer.refereeState_ = RefereeState::SUCCEED;
    pinchRecognizer.HandleTouchDownEvent(touchEvent);
    EXPECT_EQ(static_cast<int32_t>(pinchRecognizer.touchPoints_.size()), 0);

    /**
     * @tc.steps: step2. call HandleTouchDownEvent function and compare result.
     * @tc.steps: case2: input is TouchEvent. refereeState_ is PENDING.
     * @tc.expected: step2. result equals.
     */
    pinchRecognizer.refereeState_ = RefereeState::PENDING;
    pinchRecognizer.HandleTouchDownEvent(touchEvent);
    EXPECT_EQ(pinchRecognizer.touchPoints_[touchEvent.id].id, touchEvent.id);
    EXPECT_EQ(static_cast<int32_t>(pinchRecognizer.touchPoints_.size()), pinchRecognizer.fingers_);
    EXPECT_EQ(pinchRecognizer.refereeState_, RefereeState::DETECTING);

    /**
     * @tc.steps: step2. call HandleTouchDownEvent function and compare result.
     * @tc.steps: case3: input is AxisEvent. refereeState_ is SUCCESS.
     * @tc.expected: step2. result equals.
     */
    AxisEvent axisEvent;
    pinchRecognizer.refereeState_ = RefereeState::SUCCEED;
    pinchRecognizer.HandleTouchDownEvent(axisEvent);
    EXPECT_EQ(pinchRecognizer.refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step2. call HandleTouchDownEvent function and compare result.
     * @tc.steps: case4: input is AxisEvent. refereeState_ is PENDING.
     * @tc.expected: step2. result equals.
     */
    pinchRecognizer.refereeState_ = RefereeState::PENDING;
    pinchRecognizer.HandleTouchDownEvent(axisEvent);
    EXPECT_EQ(pinchRecognizer.pinchCenter_.GetX(), axisEvent.x);
    EXPECT_EQ(pinchRecognizer.pinchCenter_.GetY(), axisEvent.y);
    EXPECT_EQ(pinchRecognizer.refereeState_, RefereeState::DETECTING);
}

/**
 * @tc.name: PinchRecognizerTest003
 * @tc.desc: Test PinchRecognizer function: HandleMove HandleUp HandleCancel
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PinchRecognizerTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PinchRecognizer.
     */
    PinchRecognizer pinchRecognizer = PinchRecognizer(SINGLE_FINGER_NUMBER, PINCH_GESTURE_DISTANCE);

    /**
     * @tc.steps: step2. call HandleTouchMoveEvent function and compare result.
     * @tc.steps: case1: input is TouchEvent
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    pinchRecognizer.refereeState_ = RefereeState::SUCCEED;
    pinchRecognizer.HandleTouchMoveEvent(touchEvent);
    pinchRecognizer.HandleTouchUpEvent(touchEvent);
    pinchRecognizer.HandleTouchCancelEvent(touchEvent);
    EXPECT_EQ(pinchRecognizer.touchPoints_[touchEvent.id].id, touchEvent.id);
    EXPECT_EQ(pinchRecognizer.lastTouchEvent_.id, touchEvent.id);

    /**
     * @tc.steps: step2. call HandleTouchMoveEvent function and compare result.
     * @tc.steps: case2: input is AxisEvent
     * @tc.expected: step2. result equals.
     */
    AxisEvent axisEvent;
    pinchRecognizer.refereeState_ = RefereeState::SUCCEED;
    pinchRecognizer.HandleTouchMoveEvent(axisEvent);
    EXPECT_EQ(pinchRecognizer.touchPoints_[touchEvent.id].id, touchEvent.id);
    EXPECT_EQ(pinchRecognizer.lastTouchEvent_.id, touchEvent.id);
    EXPECT_EQ(pinchRecognizer.scale_, axisEvent.pinchAxisScale);
}

/**
 * @tc.name: PinchRecognizerTest004
 * @tc.desc: Test PinchRecognizer function: OnFlushTouchEvent begin end
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PinchRecognizerTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PinchRecognizer.
     */
    PinchRecognizer pinchRecognizer = PinchRecognizer(SINGLE_FINGER_NUMBER, PINCH_GESTURE_DISTANCE);

    /**
     * @tc.steps: step2. call OnFlushTouchEvent function and compare result.
     * @tc.expected: step2. result equals.
     */
    pinchRecognizer.OnFlushTouchEventsBegin();
    EXPECT_EQ(pinchRecognizer.isFlushTouchEventsEnd_, false);
    pinchRecognizer.OnFlushTouchEventsEnd();
    EXPECT_EQ(pinchRecognizer.isFlushTouchEventsEnd_, true);
}

/**
 * @tc.name: PinchRecognizerTest005
 * @tc.desc: Test PinchRecognizer function: ComputeAverageDeviation
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PinchRecognizerTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PinchRecognizer.
     */
    PinchRecognizer pinchRecognizer = PinchRecognizer(SINGLE_FINGER_NUMBER, PINCH_GESTURE_DISTANCE);

    /**
     * @tc.steps: step2. call ComputeAverageDeviation function and compare result.
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    pinchRecognizer.touchPoints_[touchEvent.id] = touchEvent;
    auto result = pinchRecognizer.ComputeAverageDeviation();
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: PinchRecognizerTest006
 * @tc.desc: Test PinchRecognizer function: SendCallbackMsg
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PinchRecognizerTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PinchRecognizer.
     */
    PinchRecognizer pinchRecognizer = PinchRecognizer(SINGLE_FINGER_NUMBER, PINCH_GESTURE_DISTANCE);

    /**
     * @tc.steps: step2. call SendCallbackMsg function and compare result.
     * @tc.steps: case1: refereeState is SUCCESS,return
     * @tc.expected: step2. result equals.
     */
    std::unique_ptr<GestureEventFunc> onAction;
    TouchEvent touchEvent;
    touchEvent.tiltX = 0.0f;
    touchEvent.tiltY = 0.0f;
    pinchRecognizer.touchPoints_[touchEvent.id] = touchEvent;
    pinchRecognizer.lastTouchEvent_ = touchEvent;
    pinchRecognizer.SendCallbackMsg(onAction);
    EXPECT_EQ(pinchRecognizer.touchPoints_.size(), 1);
}

/**
 * @tc.name: PinchRecognizerTest007
 * @tc.desc: Test PinchRecognizer function: ReconcileFrom
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, PinchRecognizerTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PinchRecognizer.
     */
    PinchRecognizer pinchRecognizer = PinchRecognizer(SINGLE_FINGER_NUMBER, PINCH_GESTURE_DISTANCE);
    RefPtr<PinchRecognizer> pinchRecognizerPtr =
        AceType::MakeRefPtr<PinchRecognizer>(SINGLE_FINGER_NUMBER, PINCH_GESTURE_DISTANCE);
    bool result = false;

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case1: normal case
     * @tc.expected: step2. result equals.
     */
    result = pinchRecognizer.ReconcileFrom(pinchRecognizerPtr);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case2: recognizerPtr is nullptr
     * @tc.expected: step2. result equals.
     */
    result = pinchRecognizer.ReconcileFrom(nullptr);
    EXPECT_EQ(result, false);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case3: fingers_ != curr->fingers_;
     * @tc.expected: step2. result equals.
     */
    pinchRecognizer.fingers_ = 1;
    pinchRecognizerPtr->fingers_ = 0;
    result = pinchRecognizer.ReconcileFrom(pinchRecognizerPtr);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: RotationRecognizerTest001
 * @tc.desc: Test RotationRecognizer function: OnAccepted OnRejected
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, RotationRecognizerTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create RotationRecognizer.
     */
    RotationRecognizer rotationRecognizer =
        RotationRecognizer(SINGLE_FINGER_NUMBER, ROTATION_GESTURE_ANGLE);

    /**
     * @tc.steps: step2. call OnAccepted function and compare result.
     * @tc.expected: step2. result equals.
     */
    rotationRecognizer.OnAccepted();
    EXPECT_EQ(rotationRecognizer.refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step3. call OnRejected function and compare result.
     * @tc.expected: step3. result equals.
     */
    rotationRecognizer.OnRejected();
    EXPECT_EQ(rotationRecognizer.refereeState_, RefereeState::FAIL);
}

/**
 * @tc.name: RotationRecognizerTest002
 * @tc.desc: Test RotationRecognizer function: TouchDown TouchUp TouchMove
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, RotationRecognizerTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create RotationRecognizer.
     */
    RotationRecognizer rotationRecognizer =
        RotationRecognizer(SINGLE_FINGER_NUMBER, ROTATION_GESTURE_ANGLE);

    /**
     * @tc.steps: step2. call HandleTouchDownEvent function and compare result.
     * @tc.steps: case1: touchPoints.size == fingers
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    rotationRecognizer.HandleTouchDownEvent(touchEvent);
    EXPECT_EQ(rotationRecognizer.touchPoints_[touchEvent.id].id, touchEvent.id);
    EXPECT_EQ(rotationRecognizer.refereeState_, RefereeState::DETECTING);

    /**
     * @tc.steps: step2. call HandleTouchDownEvent function and compare result.
     * @tc.steps: case2: touchPoints.size < fingers
     * @tc.expected: step2. result equals.
     */
    rotationRecognizer.fingers_ = FINGER_NUMBER;
    rotationRecognizer.refereeState_ = RefereeState::SUCCEED;
    rotationRecognizer.HandleTouchDownEvent(touchEvent);
    rotationRecognizer.HandleTouchUpEvent(touchEvent);
    rotationRecognizer.HandleTouchMoveEvent(touchEvent);
    rotationRecognizer.HandleTouchCancelEvent(touchEvent);
    EXPECT_EQ(rotationRecognizer.refereeState_, RefereeState::SUCCEED);
    EXPECT_EQ(rotationRecognizer.resultAngle_, 0);
}

/**
 * @tc.name: RotationRecognizerTest003
 * @tc.desc: Test RotationRecognizer function: ChangeValueRange
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, RotationRecognizerTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create RotationRecognizer.
     */
    RotationRecognizer rotationRecognizer =
        RotationRecognizer(SINGLE_FINGER_NUMBER, ROTATION_GESTURE_ANGLE);

    /**
     * @tc.steps: step2. call ChangeValueRange function and compare result.
     * @tc.expected: step2. result equals.
     */
    auto result = rotationRecognizer.ChangeValueRange(COMMON_VALUE_RANGE_CASE);
    EXPECT_EQ(result, COMMON_VALUE_RANGE_CASE);

    result = rotationRecognizer.ChangeValueRange(SPECIAL_VALUE_RANGE_CASE1);
    EXPECT_EQ(result, SPECIAL_VALUE_RANGE_CASE1 - PI);

    result = rotationRecognizer.ChangeValueRange(SPECIAL_VALUE_RANGE_CASE2);
    EXPECT_EQ(result, SPECIAL_VALUE_RANGE_CASE2 + PI);
}

/**
 * @tc.name: RotationRecognizerTest004
 * @tc.desc: Test RotationRecognizer function: ComputeAngle
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, RotationRecognizerTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create RotationRecognizer.
     */
    RotationRecognizer rotationRecognizer =
        RotationRecognizer(SINGLE_FINGER_NUMBER, ROTATION_GESTURE_ANGLE);

    /**
     * @tc.steps: step2. call ComputeAngle function and compare result.
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEventStart;
    touchEventStart.id = 0;
    rotationRecognizer.touchPoints_[0] = touchEventStart;
    TouchEvent touchEventEnd;
    touchEventEnd.id = 1;
    rotationRecognizer.touchPoints_[1] = touchEventEnd;
    auto result = rotationRecognizer.ComputeAngle();
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: RotationRecognizerTest005
 * @tc.desc: Test RotationRecognizer function: OnResetStatus
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, RotationRecognizerTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create RotationRecognizer.
     */
    RotationRecognizer rotationRecognizer =
        RotationRecognizer(SINGLE_FINGER_NUMBER, ROTATION_GESTURE_ANGLE);

    /**
     * @tc.steps: step2. call OnResetStatus function and compare result.
     * @tc.expected: step2. result equals.
     */
    rotationRecognizer.OnResetStatus();
    EXPECT_EQ(rotationRecognizer.initialAngle_, 0.0);
    EXPECT_EQ(rotationRecognizer.currentAngle_, 0.0);
    EXPECT_EQ(rotationRecognizer.resultAngle_, 0.0);
}

/**
 * @tc.name: RotationRecognizerTest006
 * @tc.desc: Test RotationRecognizer function: SendCallbackMsg
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, RotationRecognizerTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create RotationRecognizer.
     */
    RotationRecognizer rotationRecognizer =
        RotationRecognizer(SINGLE_FINGER_NUMBER, ROTATION_GESTURE_ANGLE);

    /**
     * @tc.steps: step2. call SendCallbackMsg function and compare result.
     * @tc.steps: case1: callback is null
     * @tc.expected: step2. result equals.
     */
    rotationRecognizer.SendCallbackMsg(nullptr);
    EXPECT_EQ(rotationRecognizer.touchPoints_.size(), 0);

    /**
     * @tc.steps: step2. call SendCallbackMsg function and compare result.
     * @tc.steps: case2: callback is ptr, have no tiltX and tileY
     * @tc.expected: step2. result equals.
     */
    std::unique_ptr<GestureEventFunc> onAction;
    TouchEvent touchEvent1;
    rotationRecognizer.touchPoints_[touchEvent1.id] = touchEvent1;
    rotationRecognizer.SendCallbackMsg(onAction);
    EXPECT_EQ(rotationRecognizer.touchPoints_.size(), 1);

    /**
     * @tc.steps: step2. call SendCallbackMsg function and compare result.
     * @tc.steps: case3: callback is ptr, have no tiltX
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent2;
    touchEvent2.tiltY = 0.0f;
    rotationRecognizer.touchPoints_[touchEvent2.id] = touchEvent2;
    rotationRecognizer.SendCallbackMsg(onAction);
    EXPECT_EQ(rotationRecognizer.touchPoints_.size(), 1);

    /**
     * @tc.steps: step2. call SendCallbackMsg function and compare result.
     * @tc.steps: case4: callback is ptr, have tiltX and tiltY
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent3;
    touchEvent3.tiltX = 0.0f;
    touchEvent3.tiltY = 0.0f;
    rotationRecognizer.touchPoints_[touchEvent3.id] = touchEvent3;
    rotationRecognizer.SendCallbackMsg(onAction);
    EXPECT_EQ(rotationRecognizer.touchPoints_.size(), 1);
}

/**
 * @tc.name: RotationRecognizerTest007
 * @tc.desc: Test RotationRecognizer function: ReconcileFrom
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, RotationRecognizerTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ClickRecognizer.
     */
    RotationRecognizer rotationRecognizer =
        RotationRecognizer(SINGLE_FINGER_NUMBER, ROTATION_GESTURE_ANGLE);
    RefPtr<RotationRecognizer> rotationRecognizerPtr =
        AceType::MakeRefPtr<RotationRecognizer>(SINGLE_FINGER_NUMBER, ROTATION_GESTURE_ANGLE);
    bool result = false;

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case1: recognizerPtr is nullptr
     * @tc.expected: step2. result equals.
     */
    result = rotationRecognizer.ReconcileFrom(nullptr);
    EXPECT_EQ(result, false);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case2: recognizerPtr is normal, curr->fingers != fingers
     * @tc.expected: step2. result equals.
     */
    rotationRecognizer.fingers_ = rotationRecognizerPtr->fingers_ + 1;
    result = rotationRecognizer.ReconcileFrom(rotationRecognizerPtr);
    EXPECT_EQ(result, false);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case2: recognizerPtr is normal, curr->angle != angle
     * @tc.expected: step2. result equals.
     */
    rotationRecognizer.fingers_ = rotationRecognizerPtr->fingers_;
    rotationRecognizer.angle_ = rotationRecognizerPtr->angle_ + 1;
    result = rotationRecognizer.ReconcileFrom(rotationRecognizerPtr);
    EXPECT_EQ(result, false);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case2: recognizerPtr is normal, curr->priorityMask != priorityMask
     * @tc.expected: step2. result equals.
     */
    rotationRecognizer.fingers_ = rotationRecognizerPtr->fingers_;
    rotationRecognizer.angle_ = rotationRecognizerPtr->angle_;
    rotationRecognizer.priorityMask_ = GestureMask::Begin;
    result = rotationRecognizer.ReconcileFrom(rotationRecognizerPtr);
    EXPECT_EQ(result, false);

    /**
     * @tc.steps: step2. call ReconcileFrom function and compare result.
     * @tc.steps: case2: recognizerPtr is normal
     * @tc.expected: step2. result equals.
     */
    rotationRecognizer.fingers_ = rotationRecognizerPtr->fingers_;
    rotationRecognizer.angle_ = rotationRecognizerPtr->angle_;
    rotationRecognizer.priorityMask_ = rotationRecognizerPtr->priorityMask_;
    result = rotationRecognizer.ReconcileFrom(rotationRecognizerPtr);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: SequencedRecognizerTest001
 * @tc.desc: Test SequencedRecognizer function: OnAccepted
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SequencedRecognizerTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SequencedRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    SequencedRecognizer sequencedRecognizer = SequencedRecognizer(recognizers);
    
    /**
     * @tc.steps: step2. call OnAccepted function and compare result.
     * @tc.steps: case1: recognizers_ is empty
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.OnAccepted();
    EXPECT_EQ(sequencedRecognizer.refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step2. call OnAccepted function and compare result.
     * @tc.steps: case2: recognizers_ is not empty, have nullptr
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.recognizers_.push_back(nullptr);
    sequencedRecognizer.OnAccepted();
    EXPECT_EQ(sequencedRecognizer.refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step2. call OnAccepted function and compare result.
     * @tc.steps: case3: recognizers_ is not empty, have click ptr
     * @tc.expected: step2. result equals.
     */
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    sequencedRecognizer.recognizers_.clear();
    sequencedRecognizer.recognizers_.push_back(clickRecognizerPtr);
    sequencedRecognizer.OnAccepted();
    EXPECT_EQ(sequencedRecognizer.refereeState_, RefereeState::SUCCEED);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::SUCCEED);
}

/**
 * @tc.name: SequencedRecognizerTest002
 * @tc.desc: Test SequencedRecognizer function: OnRejected
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SequencedRecognizerTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SequencedRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    SequencedRecognizer sequencedRecognizer = SequencedRecognizer(recognizers);
    
    /**
     * @tc.steps: step2. call OnRejected function and compare result.
     * @tc.steps: case1: recognizers_ is empty
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.OnRejected();
    EXPECT_EQ(sequencedRecognizer.refereeState_, RefereeState::FAIL);

    /**
     * @tc.steps: step2. call OnRejected function and compare result.
     * @tc.steps: case2: recognizers_ is not empty, have nullptr
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.currentIndex_ = -1;
    sequencedRecognizer.recognizers_.push_back(nullptr);
    sequencedRecognizer.OnRejected();
    EXPECT_EQ(sequencedRecognizer.refereeState_, RefereeState::FAIL);

    /**
     * @tc.steps: step2. call OnAccepted function and compare result.
     * @tc.steps: case3: recognizers_ is not empty, have click ptr
     * @tc.expected: step2. result equals.
     */
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    sequencedRecognizer.currentIndex_ = 0;
    sequencedRecognizer.recognizers_.clear();
    sequencedRecognizer.recognizers_.push_back(clickRecognizerPtr);
    sequencedRecognizer.OnRejected();
    EXPECT_EQ(sequencedRecognizer.refereeState_, RefereeState::FAIL);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::FAIL);
}

/**
 * @tc.name: SequencedRecognizerTest003
 * @tc.desc: Test SequencedRecognizer function: OnPending
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SequencedRecognizerTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SequencedRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    SequencedRecognizer sequencedRecognizer = SequencedRecognizer(recognizers);
    
    /**
     * @tc.steps: step2. call OnPending function and compare result.
     * @tc.steps: case1: recognizers_ is empty
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.OnPending();
    EXPECT_EQ(sequencedRecognizer.refereeState_, RefereeState::PENDING);

    /**
     * @tc.steps: step2. call OnPending function and compare result.
     * @tc.steps: case2: recognizers_ is not empty, have nullptr
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.recognizers_.push_back(nullptr);
    sequencedRecognizer.OnPending();
    EXPECT_EQ(sequencedRecognizer.refereeState_, RefereeState::PENDING);

    /**
     * @tc.steps: step2. call OnPending function and compare result.
     * @tc.steps: case3: recognizers_ is not empty, have click ptr, ACCEPT
     * @tc.expected: step2. result equals.
     */
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    clickRecognizerPtr->disposal_ = GestureDisposal::ACCEPT;
    sequencedRecognizer.recognizers_.clear();
    sequencedRecognizer.recognizers_.push_back(clickRecognizerPtr);
    sequencedRecognizer.OnPending();
    EXPECT_EQ(sequencedRecognizer.refereeState_, RefereeState::PENDING);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::SUCCEED);
    
    /**
     * @tc.steps: step2. call OnPending function and compare result.
     * @tc.steps: case4: recognizers_ is not empty, have click ptr, PENDING
     * @tc.expected: step2. result equals.
     */
    clickRecognizerPtr->disposal_ = GestureDisposal::PENDING;
    sequencedRecognizer.recognizers_.clear();
    sequencedRecognizer.recognizers_.push_back(clickRecognizerPtr);
    sequencedRecognizer.OnPending();
    EXPECT_EQ(sequencedRecognizer.refereeState_, RefereeState::PENDING);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::PENDING);
}

/**
 * @tc.name: SequencedRecognizerTest004
 * @tc.desc: Test SequencedRecognizer function: OnBlocked
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SequencedRecognizerTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SequencedRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    SequencedRecognizer sequencedRecognizer = SequencedRecognizer(recognizers);
    
    /**
     * @tc.steps: step2. call OnBlocked function and compare result.
     * @tc.steps: case1: recognizers_ is empty
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.OnBlocked();
    EXPECT_EQ(sequencedRecognizer.refereeState_, RefereeState::READY);

    /**
     * @tc.steps: step2. call OnBlocked function and compare result.
     * @tc.steps: case2: recognizers_ is not empty, have nullptr
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.recognizers_.push_back(nullptr);
    sequencedRecognizer.OnBlocked();
    EXPECT_EQ(sequencedRecognizer.refereeState_, RefereeState::READY);

    /**
     * @tc.steps: step2. call OnBlocked function and compare result.
     * @tc.steps: case3: recognizers_ is not empty, disposal is ACCEPT
     * @tc.expected: step2. result equals.
     */
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    sequencedRecognizer.disposal_ = GestureDisposal::ACCEPT;
    sequencedRecognizer.recognizers_.clear();
    sequencedRecognizer.recognizers_.push_back(clickRecognizerPtr);
    sequencedRecognizer.OnBlocked();
    EXPECT_EQ(sequencedRecognizer.refereeState_, RefereeState::SUCCEED_BLOCKED);
    
    /**
     * @tc.steps: step2. call OnBlocked function and compare result.
     * @tc.steps: case4: recognizers_ is not empty, disposal is PENDING
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.disposal_ = GestureDisposal::PENDING;
    sequencedRecognizer.OnBlocked();
    EXPECT_EQ(sequencedRecognizer.refereeState_, RefereeState::PENDING_BLOCKED);
}

/**
 * @tc.name: SequencedRecognizerTest005
 * @tc.desc: Test SequencedRecognizer function: HandleEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SequencedRecognizerTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SequencedRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    SequencedRecognizer sequencedRecognizer = SequencedRecognizer(recognizers);
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    sequencedRecognizer.recognizers_.clear();
    sequencedRecognizer.recognizers_.push_back(clickRecognizerPtr);
    sequencedRecognizer.recognizers_.push_back(clickRecognizerPtr);
    bool result = false;
    TouchEvent touchEvent;
    
    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case1: currentIndex = 0
     * @tc.expected: step2. result equals.
     */
    result = sequencedRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(result, true);

    /**
    //  * @tc.steps: step2. call HandleEvent function and compare result.
    //  * @tc.steps: case2: currentIndex = 1, prevState = SUCCESS
    //  * @tc.expected: step2. result equals.
    //  */
    sequencedRecognizer.currentIndex_ = 1;
    clickRecognizerPtr->refereeState_ = RefereeState::SUCCEED;
    result = sequencedRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case3: currentIndex = 1, prevState = READY
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.currentIndex_ = 1;
    clickRecognizerPtr->refereeState_ = RefereeState::READY;
    result = sequencedRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: SequencedRecognizerTest006
 * @tc.desc: Test SequencedRecognizer function: HandleEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SequencedRecognizerTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SequencedRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    SequencedRecognizer sequencedRecognizer = SequencedRecognizer(recognizers);
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    sequencedRecognizer.recognizers_.clear();
    sequencedRecognizer.recognizers_.push_back(clickRecognizerPtr);
    bool result = false;
    TouchEvent touchEvent;

    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case4: point.type = DOWN, size = 1
     * @tc.expected: step2. result equals.
     */
    touchEvent.type = TouchType::DOWN;
    result = sequencedRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case5: point.type = MOVE
     * @tc.expected: step2. result equals.
     */
    touchEvent.type = TouchType::MOVE;
    result = sequencedRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case6: point.type = UP
     * @tc.expected: step2. result equals.
     */
    touchEvent.type = TouchType::UP;
    result = sequencedRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case7: point.type = CANCEL
     * @tc.expected: step2. result equals.
     */
    touchEvent.type = TouchType::CANCEL;
    result = sequencedRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: SequencedRecognizerTest007
 * @tc.desc: Test SequencedRecognizer function: HandleEvent
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SequencedRecognizerTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SequencedRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    SequencedRecognizer sequencedRecognizer = SequencedRecognizer(recognizers);
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    sequencedRecognizer.recognizers_.clear();
    sequencedRecognizer.recognizers_.push_back(clickRecognizerPtr);
    bool result = false;
    TouchEvent touchEvent;

    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case8: point.type = UNKOWN
     * @tc.expected: step2. result equals.
     */
    touchEvent.type = TouchType::UNKNOWN;
    result = sequencedRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case9: point.type = UP and refereeState = PENDING
     * @tc.expected: step2. result equals.
     */
    touchEvent.type = TouchType::UP;
    sequencedRecognizer.refereeState_ = RefereeState::PENDING;
    result = sequencedRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step2. call HandleEvent function and compare result.
     * @tc.steps: case10: point.type != UP and refereeState = PENDING
     * @tc.expected: step2. result equals.
     */
    touchEvent.type = TouchType::DOWN;
    sequencedRecognizer.refereeState_ = RefereeState::PENDING;
    result = sequencedRecognizer.HandleEvent(touchEvent);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: SequencedRecognizerTest008
 * @tc.desc: Test SequencedRecognizer function: BatchAdjudicate, and GestureDisposal
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SequencedRecognizerTest008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SequencedRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    SequencedRecognizer sequencedRecognizer = SequencedRecognizer(recognizers);
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    
    /**
     * @tc.steps: step2. call GestureDisposal function and compare result.
     * @tc.steps: case1: disposal: ACCEPT, refereeState: SUCCEED
     * @tc.expected: step2. result equals.
     */
    clickRecognizerPtr->refereeState_ = RefereeState::SUCCEED;
    sequencedRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::ACCEPT);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step2. call GestureDisposal function and compare result.
     * @tc.steps: case2: disposal: ACCEPT, refereeState: PENDING, currentIndex = 0
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.currentIndex_ = 0;
    sequencedRecognizer.refereeState_ = RefereeState::PENDING;
    sequencedRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::ACCEPT);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step2. call GestureDisposal function and compare result.
     * @tc.steps: case3: disposal: REJECT, refereeState: FAIL
     * @tc.expected: step2. result equals.
     */
    clickRecognizerPtr->refereeState_ = RefereeState::FAIL;
    sequencedRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::REJECT);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::FAIL);

    /**
     * @tc.steps: step2. call GestureDisposal function and compare result.
     * @tc.steps: case4: disposal: REJECT, refereeState: SUCCESS, refereeState_ = FAIL
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.refereeState_ = RefereeState::FAIL;
    clickRecognizerPtr->refereeState_ = RefereeState::FAIL;
    sequencedRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::REJECT);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::FAIL);

    /**
     * @tc.steps: step2. call GestureDisposal function and compare result.
     * @tc.steps: case5: disposal: PENDING, refereeState: PENDING
     * @tc.expected: step2. result equals.
     */
    clickRecognizerPtr->refereeState_ = RefereeState::PENDING;
    sequencedRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::PENDING);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::PENDING);

    /**
     * @tc.steps: step2. call GestureDisposal function and compare result.
     * @tc.steps: case5: disposal: PENDING, refereeState: SUCCESS, refereeState_: PENDING
     * @tc.expected: step2. result equals.
     */
    clickRecognizerPtr->refereeState_ = RefereeState::SUCCEED;
    sequencedRecognizer.refereeState_ = RefereeState::PENDING;
    sequencedRecognizer.BatchAdjudicate(clickRecognizerPtr, GestureDisposal::PENDING);
    EXPECT_EQ(clickRecognizerPtr->refereeState_, RefereeState::PENDING);
}

/**
 * @tc.name: SequencedRecognizerTest009
 * @tc.desc: Test SequencedRecognizer function: UpdateCurrentIndex
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SequencedRecognizerTest009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SequencedRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    SequencedRecognizer sequencedRecognizer = SequencedRecognizer(recognizers);
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    
    /**
     * @tc.steps: step2. call UpdateCurrentIndex function and compare result.
     * @tc.steps: case1: currentIndex == size - 1
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.currentIndex_ = -1;
    sequencedRecognizer.UpdateCurrentIndex();
    EXPECT_EQ(sequencedRecognizer.currentIndex_, -1);

    /**
     * @tc.steps: step2. call UpdateCurrentIndex function and compare result.
     * @tc.steps: case1: currentIndex != size - 1
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.currentIndex_ = 0;
    sequencedRecognizer.UpdateCurrentIndex();
    EXPECT_EQ(sequencedRecognizer.currentIndex_, 1);
}

/**
 * @tc.name: SequencedRecognizerTest010
 * @tc.desc: Test SequencedRecognizer function: ReconcileFrom
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SequencedRecognizerTest010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SwipeRecognizer.
     */
    std::vector<RefPtr<NGGestureRecognizer>> recognizers = {};
    SequencedRecognizer sequencedRecognizer = SequencedRecognizer(recognizers);
    RefPtr<SequencedRecognizer> sequencedRecognizerPtr =
        AceType::MakeRefPtr<SequencedRecognizer>(recognizers);
    bool result = false;
    
    /**
     * @tc.steps: step2. call ReconcileFrom function
     * @tc.steps: case1: recognizer is nullptr
     * @tc.expected: step2. result equals.
     */
    result = sequencedRecognizer.ReconcileFrom(nullptr);
    EXPECT_EQ(result, false);

    /**
     * @tc.steps: step2. call ReconcileFrom function
     * @tc.steps: case2: size not same, priorityMask not same
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.recognizers_.clear();
    sequencedRecognizer.recognizers_.push_back(nullptr);
    sequencedRecognizer.priorityMask_ = GestureMask::Begin;
    result = sequencedRecognizer.ReconcileFrom(sequencedRecognizerPtr);
    EXPECT_EQ(result, false);

    /**
     * @tc.steps: step2. call ReconcileFrom function
     * @tc.steps: case3: size not same, priorityMask same
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.recognizers_.clear();
    sequencedRecognizer.recognizers_.push_back(nullptr);
    sequencedRecognizer.priorityMask_ = GestureMask::Normal;
    result = sequencedRecognizer.ReconcileFrom(sequencedRecognizerPtr);
    EXPECT_EQ(result, false);
    
    /**
     * @tc.steps: step2. call ReconcileFrom function
     * @tc.steps: case4: size same, priorityMask not same
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.recognizers_.clear();
    sequencedRecognizer.priorityMask_ = GestureMask::Begin;
    result = sequencedRecognizer.ReconcileFrom(sequencedRecognizerPtr);
    EXPECT_EQ(result, false);
    
    /**
     * @tc.steps: step2. call ReconcileFrom function
     * @tc.steps: case4: size same, priorityMask same, child is nullptr
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.recognizers_.clear();
    sequencedRecognizer.recognizers_.push_back(nullptr);
    sequencedRecognizerPtr->recognizers_.clear();
    sequencedRecognizerPtr->recognizers_.push_back(nullptr);
    sequencedRecognizer.priorityMask_ = GestureMask::Normal;
    result = sequencedRecognizer.ReconcileFrom(sequencedRecognizerPtr);
    EXPECT_EQ(result, false);
    
    /**
     * @tc.steps: step2. call ReconcileFrom function
     * @tc.steps: case4: size same, priorityMask same, child is ptr
     * @tc.expected: step2. result equals.
     */
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    sequencedRecognizer.recognizers_.clear();
    sequencedRecognizer.recognizers_.push_back(clickRecognizerPtr);
    sequencedRecognizerPtr->recognizers_.clear();
    sequencedRecognizerPtr->recognizers_.push_back(clickRecognizerPtr);
    sequencedRecognizer.priorityMask_ = GestureMask::Normal;
    result = sequencedRecognizer.ReconcileFrom(sequencedRecognizerPtr);
    EXPECT_EQ(result, true);

    /**
     * @tc.steps: step2. call ReconcileFrom function
     * @tc.steps: case4: size same, priorityMask same, child is ptr and nullptr
     * @tc.expected: step2. result equals.
     */
    sequencedRecognizer.recognizers_.clear();
    sequencedRecognizer.recognizers_.push_back(clickRecognizerPtr);
    sequencedRecognizerPtr->recognizers_.clear();
    sequencedRecognizerPtr->recognizers_.push_back(nullptr);
    sequencedRecognizer.priorityMask_ = GestureMask::Normal;
    result = sequencedRecognizer.ReconcileFrom(sequencedRecognizerPtr);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: SwipeRecognizerTest001
 * @tc.desc: Test SwipeRecognizer function: OnAccepted OnRejected
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SwipeRecognizerTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SwipeRecognizer.
     */
    SwipeDirection swipeDirection;
    SwipeRecognizer swipeRecognizer =
        SwipeRecognizer(SINGLE_FINGER_NUMBER, swipeDirection, SWIPE_SPEED);
    
    /**
     * @tc.steps: step2. call OnAccepted function and compare result.
     * @tc.expected: step2. result equals.
     */
    swipeRecognizer.OnAccepted();
    EXPECT_EQ(swipeRecognizer.refereeState_, RefereeState::SUCCEED);

    /**
     * @tc.steps: step3. call OnRejected function and compare result.
     * @tc.expected: step3. result equals.
     */
    swipeRecognizer.OnRejected();
    EXPECT_EQ(swipeRecognizer.refereeState_, RefereeState::FAIL);
}

/**
 * @tc.name: SwipeRecognizerTest002
 * @tc.desc: Test SwipeRecognizer function: HandleTouchDown
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SwipeRecognizerTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SwipeRecognizer.
     */
    SwipeDirection swipeDirection;
    SwipeRecognizer swipeRecognizer =
        SwipeRecognizer(SINGLE_FINGER_NUMBER, swipeDirection, SWIPE_SPEED);
    
    /**
     * @tc.steps: step2. call HandleTouchDown function
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    swipeRecognizer.fingers_ = 1;
    swipeRecognizer.HandleTouchDownEvent(touchEvent);
    EXPECT_EQ(swipeRecognizer.lastTouchEvent_.id, touchEvent.id);
    EXPECT_EQ(swipeRecognizer.refereeState_, RefereeState::DETECTING);

    AxisEvent axisEvent;
    swipeRecognizer.HandleTouchDownEvent(axisEvent);
    EXPECT_EQ(swipeRecognizer.axisVerticalTotal_, 0.0);
    EXPECT_EQ(swipeRecognizer.axisHorizontalTotal_, 0.0);
    EXPECT_EQ(swipeRecognizer.refereeState_, RefereeState::DETECTING);
}

/**
 * @tc.name: SwipeRecognizerTest003
 * @tc.desc: Test SwipeRecognizer function: HandleTouchUp
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SwipeRecognizerTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SwipeRecognizer.
     */
    SwipeDirection swipeDirection;
    SwipeRecognizer swipeRecognizer =
        SwipeRecognizer(SINGLE_FINGER_NUMBER, swipeDirection, SWIPE_SPEED);
    
    /**
     * @tc.steps: step2. call HandleTouchUp function
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    swipeRecognizer.refereeState_ = RefereeState::FAIL;
    swipeRecognizer.downEvents_[touchEvent.id] = touchEvent;
    swipeRecognizer.HandleTouchUpEvent(touchEvent);
    EXPECT_EQ(swipeRecognizer.globalPoint_.GetX(), touchEvent.x);
    EXPECT_EQ(swipeRecognizer.globalPoint_.GetY(), touchEvent.y);
    EXPECT_EQ(swipeRecognizer.lastTouchEvent_.id, touchEvent.id);

    AxisEvent axisEvent;
    swipeRecognizer.refereeState_ = RefereeState::FAIL;
    swipeRecognizer.HandleTouchUpEvent(touchEvent);
    EXPECT_EQ(swipeRecognizer.globalPoint_.GetX(), axisEvent.x);
    EXPECT_EQ(swipeRecognizer.globalPoint_.GetY(), axisEvent.y);
}

/**
 * @tc.name: SwipeRecognizerTest004
 * @tc.desc: Test SwipeRecognizer function: HandleTouchMove
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SwipeRecognizerTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SwipeRecognizer.
     */
    SwipeDirection swipeDirection;
    SwipeRecognizer swipeRecognizer =
        SwipeRecognizer(SINGLE_FINGER_NUMBER, swipeDirection, SWIPE_SPEED);
    
    /**
     * @tc.steps: step2. call HandleTouchMove function
     * @tc.expected: step2. result equals.
     */
    TouchEvent touchEvent;
    swipeRecognizer.refereeState_ = RefereeState::FAIL;
    swipeRecognizer.downEvents_[touchEvent.id] = touchEvent;
    touchEvent.x = 0;
    touchEvent.y = 1;
    swipeRecognizer.HandleTouchUpEvent(touchEvent);
    EXPECT_EQ(swipeRecognizer.globalPoint_.GetX(), touchEvent.x);
    EXPECT_EQ(swipeRecognizer.globalPoint_.GetY(), touchEvent.y);
    EXPECT_EQ(swipeRecognizer.lastTouchEvent_.id, touchEvent.id);

    AxisEvent axisEvent;
    swipeRecognizer.refereeState_ = RefereeState::FAIL;
    swipeRecognizer.HandleTouchUpEvent(axisEvent);
    EXPECT_EQ(swipeRecognizer.globalPoint_.GetX(), axisEvent.x);
    EXPECT_EQ(swipeRecognizer.globalPoint_.GetY(), axisEvent.y);
    EXPECT_EQ(swipeRecognizer.axisVerticalTotal_, 0);
    EXPECT_EQ(swipeRecognizer.axisHorizontalTotal_, 0);
}

/**
 * @tc.name: SwipeRecognizerTest005
 * @tc.desc: Test SwipeRecognizer function: CheckAngle
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SwipeRecognizerTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SwipeRecognizer.
     */
    SwipeDirection swipeDirection;
    SwipeRecognizer swipeRecognizer =
        SwipeRecognizer(SINGLE_FINGER_NUMBER, swipeDirection, SWIPE_SPEED);
    
    /**
     * @tc.steps: step2. call CheckAngle function
     * @tc.expected: step2. result equals.
     */
    swipeRecognizer.prevAngle_ = std::make_optional(VERTICAL_ANGLE);
    auto result = swipeRecognizer.CheckAngle(0);
    EXPECT_EQ(result, false);
    
    swipeRecognizer.prevAngle_ = std::make_optional(VERTICAL_ANGLE);
    result = swipeRecognizer.CheckAngle(VERTICAL_ANGLE);
    EXPECT_EQ(result, true);

    swipeRecognizer.prevAngle_ = std::make_optional(VERTICAL_ANGLE);
    swipeRecognizer.direction_.type = SwipeDirection::HORIZONTAL;
    result = swipeRecognizer.CheckAngle(VERTICAL_ANGLE);
    EXPECT_EQ(result, false);

    swipeRecognizer.prevAngle_ = std::make_optional(VERTICAL_ANGLE);
    swipeRecognizer.direction_.type = SwipeDirection::VERTICAL;
    result = swipeRecognizer.CheckAngle(HORIZONTAL_ANGLE);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: SwipeRecognizerTest006
 * @tc.desc: Test SwipeRecognizer function: OnResetStatus
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SwipeRecognizerTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SwipeRecognizer.
     */
    SwipeDirection swipeDirection;
    SwipeRecognizer swipeRecognizer =
        SwipeRecognizer(SINGLE_FINGER_NUMBER, swipeDirection, SWIPE_SPEED);
    
    /**
     * @tc.steps: step2. call OnResetStatus function
     * @tc.expected: step2. result equals.
     */
    swipeRecognizer.OnResetStatus();
    EXPECT_EQ(swipeRecognizer.axisHorizontalTotal_, 0.0);
    EXPECT_EQ(swipeRecognizer.axisVerticalTotal_, 0.0);
    EXPECT_EQ(swipeRecognizer.resultSpeed_, 0.0);
    EXPECT_EQ(swipeRecognizer.globalPoint_.GetX(), 0.0);
    EXPECT_EQ(swipeRecognizer.globalPoint_.GetY(), 0.0);
}

/**
 * @tc.name: SwipeRecognizerTest007
 * @tc.desc: Test SwipeRecognizer function: ReconcileFrom
 * @tc.type: FUNC
 */
HWTEST_F(GesturePatternTestNg, SwipeRecognizerTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create SwipeRecognizer.
     */
    SwipeDirection swipeDirection;
    SwipeRecognizer swipeRecognizer =
        SwipeRecognizer(SINGLE_FINGER_NUMBER, swipeDirection, SWIPE_SPEED);
    RefPtr<SwipeRecognizer> swipeRecognizerPtr =
        AceType::MakeRefPtr<SwipeRecognizer>(SINGLE_FINGER_NUMBER, swipeDirection, SWIPE_SPEED);
    
    /**
     * @tc.steps: step2. call ReconcileFrom function
     * @tc.expected: step2. result equals.
     */
    auto result = swipeRecognizer.ReconcileFrom(nullptr);
    EXPECT_EQ(result, false);

    result = swipeRecognizer.ReconcileFrom(swipeRecognizerPtr);
    EXPECT_EQ(result, true);

    swipeRecognizer.fingers_ = 1;
    result = swipeRecognizer.ReconcileFrom(swipeRecognizerPtr);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: GestureGroupTest001
 * @tc.desc: Test GestureGroup CreateRecognizer function
 */
HWTEST_F(GesturePatternTestNg, GestureGroupTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create GestureGroup.
     */
    GestureGroup gestureGroup = GestureGroup(GestureMode::Sequence);

    /**
     * @tc.steps: step2. call CreateRecognizer function and compare result
     * @tc.steps: case1: GestureMode::Begin
     */
    gestureGroup.priority_ = GesturePriority::Low;
    gestureGroup.gestureMask_ = GestureMask::Normal;
    gestureGroup.mode_ = GestureMode::Begin;
    auto groupRecognizer =  gestureGroup.CreateRecognizer();
    EXPECT_EQ(groupRecognizer, nullptr);
    
    /**
     * @tc.steps: step2. call CreateRecognizer function and compare result
     * @tc.steps: case2: GestureMode::Sequence
     */
    gestureGroup.priority_ = GesturePriority::Low;
    gestureGroup.gestureMask_ = GestureMask::Normal;
    gestureGroup.mode_ = GestureMode::Sequence;
    groupRecognizer =  gestureGroup.CreateRecognizer();
    EXPECT_NE(groupRecognizer, nullptr);
    EXPECT_EQ(groupRecognizer->GetPriority(), GesturePriority::Low);
    EXPECT_EQ(groupRecognizer->GetPriorityMask(), GestureMask::Normal);

    /**
     * @tc.steps: step2. call CreateRecognizer function and compare result
     * @tc.steps: case3: GestureMode::Parallel
     */
    gestureGroup.priority_ = GesturePriority::Low;
    gestureGroup.gestureMask_ = GestureMask::Normal;
    gestureGroup.mode_ = GestureMode::Parallel;
    groupRecognizer =  gestureGroup.CreateRecognizer();
    EXPECT_NE(groupRecognizer, nullptr);
    EXPECT_EQ(groupRecognizer->GetPriority(), GesturePriority::Low);
    EXPECT_EQ(groupRecognizer->GetPriorityMask(), GestureMask::Normal);

    /**
     * @tc.steps: step2. call CreateRecognizer function and compare result
     * @tc.steps: case4: GestureMode::Exclusive
     */
    gestureGroup.priority_ = GesturePriority::Low;
    gestureGroup.gestureMask_ = GestureMask::Normal;
    gestureGroup.mode_ = GestureMode::Exclusive;
    groupRecognizer =  gestureGroup.CreateRecognizer();
    EXPECT_NE(groupRecognizer, nullptr);
    EXPECT_EQ(groupRecognizer->GetPriority(), GesturePriority::Low);
    EXPECT_EQ(groupRecognizer->GetPriorityMask(), GestureMask::Normal);

    /**
     * @tc.steps: step2. call CreateRecognizer function and compare result
     * @tc.steps: case5: GestureMode::End
     */
    gestureGroup.priority_ = GesturePriority::Low;
    gestureGroup.gestureMask_ = GestureMask::Normal;
    gestureGroup.mode_ = GestureMode::End;
    groupRecognizer =  gestureGroup.CreateRecognizer();
    EXPECT_EQ(groupRecognizer, nullptr);
}

/**
 * @tc.name: GestureRefereeTest001
 * @tc.desc: Test GestureReferee Existed function
 */
HWTEST_F(GesturePatternTestNg, GestureRefereeTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create GestureScope and clickRecognizer.
     */
    GestureScope gestureScope = GestureScope(0);
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    gestureScope.recognizers_.insert(gestureScope.recognizers_.end(), clickRecognizerPtr);

    /**
     * @tc.steps: step2. call Existed function and compare result
     * @tc.steps: expected equal
     */
    auto result = gestureScope.Existed(clickRecognizerPtr);
    EXPECT_EQ(result, true);

    result = gestureScope.Existed(nullptr);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GestureRefereeTest002
 * @tc.desc: Test GestureReferee CheckNeedBlocked function
 */
HWTEST_F(GesturePatternTestNg, GestureRefereeTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create GestureScope and clickRecognizer.
     */
    GestureScope gestureScope = GestureScope(0);
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    gestureScope.recognizers_.insert(gestureScope.recognizers_.end(), clickRecognizerPtr);

    /**
     * @tc.steps: step2. call Existed function and compare result
     * @tc.steps: expected equal
     */
    auto result = gestureScope.CheckNeedBlocked(clickRecognizerPtr);
    EXPECT_EQ(result, false);
    
    RefPtr<ClickRecognizer> clickRecognizerPtrNotInsert =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    clickRecognizerPtr->refereeState_ = RefereeState::PENDING;
    result = gestureScope.CheckNeedBlocked(clickRecognizerPtrNotInsert);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: GestureRefereeTest003
 * @tc.desc: Test GestureReferee OnAcceptGesture function
 */
HWTEST_F(GesturePatternTestNg, GestureRefereeTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create GestureScope and clickRecognizer.
     */
    GestureScope gestureScope = GestureScope(0);
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    gestureScope.recognizers_.insert(gestureScope.recognizers_.end(), clickRecognizerPtr);

    /**
     * @tc.steps: step2. call OnAcceptGesture function and compare result
     * @tc.steps: expected equal
     */
    gestureScope.OnAcceptGesture(clickRecognizerPtr);
    EXPECT_EQ(gestureScope.hasGestureAccepted_, true);
}

/**
 * @tc.name: GestureRefereeTest004
 * @tc.desc: Test GestureReferee OnBlockGesture function
 */
HWTEST_F(GesturePatternTestNg, GestureRefereeTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create GestureScope and clickRecognizer.
     */
    GestureScope gestureScope = GestureScope(0);
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    gestureScope.recognizers_.insert(gestureScope.recognizers_.end(), clickRecognizerPtr);

    /**
     * @tc.steps: step2. call UnBlockGesture function and compare result
     * @tc.steps: expected equal
     */
    auto result = gestureScope.UnBlockGesture();
    EXPECT_EQ(result, nullptr);

    clickRecognizerPtr->refereeState_ = RefereeState::PENDING_BLOCKED;
    result = gestureScope.UnBlockGesture();
    EXPECT_EQ(result, clickRecognizerPtr);
}

/**
 * @tc.name: GestureRefereeTest005
 * @tc.desc: Test GestureReferee IsPending function
 */
HWTEST_F(GesturePatternTestNg, GestureRefereeTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create GestureScope and clickRecognizer.
     */
    GestureScope gestureScope = GestureScope(0);
    RefPtr<ClickRecognizer> clickRecognizerPtr =
        AceType::MakeRefPtr<ClickRecognizer>(FINGER_NUMBER, COUNT);
    gestureScope.recognizers_.insert(gestureScope.recognizers_.end(), clickRecognizerPtr);

    /**
     * @tc.steps: step2. call IsPending function and compare result
     * @tc.steps: expected equal
     */
    auto result = gestureScope.IsPending();
    EXPECT_EQ(result, false);

    clickRecognizerPtr->refereeState_ = RefereeState::PENDING;
    result = gestureScope.IsPending();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: GestureRefereeTest006
 * @tc.desc: Test GestureReferee AddGestureToScope function
 */
HWTEST_F(GesturePatternTestNg, GestureRefereeTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create GestureReferee.
     */
    GestureReferee gestureReferee;

    /**
     * @tc.steps: step2. call AddGestureToScope function and compare result
     * @tc.steps: expected equal
     */
    TouchTestResult touchTestResult;
    gestureReferee.AddGestureToScope(0, touchTestResult);
    EXPECT_EQ(gestureReferee.gestureScopes_.size(), 1);
}

/**
 * @tc.name: GestureRefereeTest007
 * @tc.desc: Test GestureReferee CleanGestureScope function
 */
HWTEST_F(GesturePatternTestNg, GestureRefereeTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create GestureReferee.
     */
    GestureReferee gestureReferee;
    
    /**
     * @tc.steps: step2. call CleanGestureScope function and compare result
     * @tc.steps: expected equal
     */
    gestureReferee.CleanGestureScope(0);
    EXPECT_EQ(gestureReferee.gestureScopes_.size(), 0);
}

/**
 * @tc.name: LongPressGestureTest001
 * @tc.desc: Test LongPressGesture CreateRecognizer function
 */
HWTEST_F(GesturePatternTestNg, LongPressGestureTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create LongPressGesture.
     */
    LongPressGesture longPressGesture =
        LongPressGesture(FINGER_NUMBER, false, LONG_PRESS_DURATION, false, false);
    EXPECT_EQ(longPressGesture.repeat_, false);
    EXPECT_EQ(longPressGesture.duration_, LONG_PRESS_DURATION);
    EXPECT_EQ(longPressGesture.isForDrag_, false);
    EXPECT_EQ(longPressGesture.isDisableMouseLeft_, false);

    /**
     * @tc.steps: step2. call CreateRecognizer function and compare result
     */
    auto longPressRecognizer =
        AceType::DynamicCast<LongPressRecognizer>(longPressGesture.CreateRecognizer());
    EXPECT_NE(longPressRecognizer, nullptr);
    EXPECT_EQ(longPressRecognizer->repeat_, false);
    EXPECT_EQ(longPressRecognizer->duration_, LONG_PRESS_DURATION);
    EXPECT_EQ(longPressRecognizer->isForDrag_, false);
    EXPECT_EQ(longPressRecognizer->isDisableMouseLeft_, false);
}

/**
 * @tc.name: PinchGestureTest001
 * @tc.desc: Test PinchGesture CreateRecognizer function
 */
HWTEST_F(GesturePatternTestNg, PinchGestureTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create PinchGestureGesture.
     */
    PinchGesture pinchGesture = PinchGesture(FINGER_NUMBER, PINCH_GESTURE_DISTANCE);
    EXPECT_EQ(pinchGesture.distance_, PINCH_GESTURE_DISTANCE);
}

/**
 * @tc.name: RotationGestureTest001
 * @tc.desc: Test RotationGesture CreateRecognizer function
 */
HWTEST_F(GesturePatternTestNg, RotationGestureTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create RotationGestureGesture.
     */
    RotationGesture rotationGesture = RotationGesture(FINGER_NUMBER, ROTATION_GESTURE_ANGLE);
    EXPECT_EQ(rotationGesture.angle_, ROTATION_GESTURE_ANGLE);

    /**
     * @tc.steps: step2. call CreateRecognizer function and compare result
     */
    rotationGesture.priority_ = GesturePriority::Low;
    rotationGesture.gestureMask_ = GestureMask::Normal;
    auto rotationRecognizer =
        AceType::DynamicCast<RotationRecognizer>(rotationGesture.CreateRecognizer());
    EXPECT_NE(rotationRecognizer, nullptr);
    EXPECT_EQ(rotationRecognizer->GetPriority(), GesturePriority::Low);
    EXPECT_EQ(rotationRecognizer->GetPriorityMask(), GestureMask::Normal);
}

/**
 * @tc.name: TapGestureTest001
 * @tc.desc: Test TapGesture CreateRecognizer function
 */
HWTEST_F(GesturePatternTestNg, TapGestureTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create TapGestureGesture.
     */
    TapGesture tapGesture = TapGesture(COUNT, FINGER_NUMBER);
    EXPECT_EQ(tapGesture.count_, COUNT);
    
    /**
     * @tc.steps: step2. call CreateRecognizer function and compare result
     */
    tapGesture.priority_ = GesturePriority::Low;
    tapGesture.gestureMask_ = GestureMask::Normal;
    auto tapRecognizer =
        AceType::DynamicCast<ClickRecognizer>(tapGesture.CreateRecognizer());
    EXPECT_NE(tapRecognizer, nullptr);
    EXPECT_EQ(tapRecognizer->GetPriority(), GesturePriority::Low);
    EXPECT_EQ(tapRecognizer->GetPriorityMask(), GestureMask::Normal);
}
} // namespace OHOS::Ace::NG

