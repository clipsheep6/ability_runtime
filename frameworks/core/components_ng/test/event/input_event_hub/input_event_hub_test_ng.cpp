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

#include <cstddef>

#include "gtest/gtest.h"

#define private public
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/test/mock/mock_pipeline_base.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace {
bool OHOS::Ace::SystemProperties::GetDebugEnabled()
{
    return false;
}
} // namespace OHOS::Ace
namespace OHOS::Ace::NG {
namespace {
constexpr uint32_t INPUT_EVENTS_SIZE = 1;
constexpr uint32_t INPUT_EVENTS_SIZE_2 = 0;
constexpr uint32_t MOUSE_RESULT_SIZE = 1;
constexpr uint32_t HOVER_RESULT_SIZE = 1;
constexpr uint32_t AXIS_RESULT_SIZE = 1;
const HoverEffectType HOVER_EFFECT_TYPE = HoverEffectType::BOARD;
constexpr float WIDTH = 400.0f;
constexpr float HEIGHT = 400.0f;
const OffsetF COORDINATE_OFFSET(WIDTH, HEIGHT);
} // namespace

class InputEventHubTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp() override;
    void TearDown() override;
};

void InputEventHubTestNg::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "InputEventHubTestNg SetUpTestCase";
}

void InputEventHubTestNg::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "InputEventHubTestNg TearDownTestCase";
}

void InputEventHubTestNg::SetUp()
{
    MockPipelineBase::SetUp();
}

void InputEventHubTestNg::TearDown()
{
    MockPipelineBase::TearDown();
}
/**
 * @tc.name: InputEventHubCreateTest001
 * @tc.desc: Create InputEventHub
 * @tc.type: FUNC
 */
HWTEST_F(InputEventHubTestNg, InputEventHubCreateTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create InputEventHub.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto inputEventHub = AceType::MakeRefPtr<InputEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    EXPECT_NE(inputEventHub, nullptr);
}

/**
 * @tc.name: InputEventHubMouseEventTest002
 * @tc.desc: Create InputEventHub and set, add and remove mouse event.
 * @tc.type: FUNC
 */
HWTEST_F(InputEventHubTestNg, InputEventHubMouseEventTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create InputEventHub.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto inputEventHub = AceType::MakeRefPtr<InputEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    EXPECT_NE(inputEventHub, nullptr);
    EXPECT_TRUE(
        inputEventHub->GetFrameNode() != nullptr && inputEventHub->GetFrameNode()->GetTag() == V2::TEXT_ETS_TAG);

    /**
     * @tc.steps: step2. Create mouse event and set it to inputEventHub.
     * @tc.expected: mouseEventActuator_ will be initialized.
     */
    const OnMouseEventFunc onMouse = [](MouseInfo& info) {};
    OnMouseEventFunc onMouse1 = onMouse;
    inputEventHub->SetMouseEvent(std::move(onMouse1));
    EXPECT_NE(inputEventHub->mouseEventActuator_, nullptr);

    /**
     * @tc.steps: step3. Make mouseEventActuator_ as nullptr, and create input event and add it to inputEventHub.
     * @tc.expected: mouseEventActuator_ will be initialized and the list of inputEvents size is 1.
     */
    inputEventHub->mouseEventActuator_ = nullptr;
    OnMouseEventFunc onMouse2 = onMouse;
    auto inputEvent = AceType::MakeRefPtr<InputEvent>(std::move(onMouse2));
    inputEventHub->AddOnMouseEvent(inputEvent);
    EXPECT_EQ(inputEventHub->mouseEventActuator_->inputEvents_.size(), INPUT_EVENTS_SIZE);

    /**
     * @tc.steps: step4. Remove OnMouseEvent.
     * @tc.expected: inputEvent will be remove from the list of inputEvents and the size will be 0. If
     * mouseEventActuator_ is nullptr, the function will return directly.
     */
    inputEventHub->RemoveOnMouseEvent(inputEvent);
    EXPECT_EQ(inputEventHub->mouseEventActuator_->inputEvents_.size(), INPUT_EVENTS_SIZE_2);
    inputEventHub->mouseEventActuator_ = nullptr;
    inputEventHub->RemoveOnMouseEvent(inputEvent);
}

/**
 * @tc.name: InputEventHubHoverEventTest003
 * @tc.desc: Create InputEventHub and invoke hover related functions.
 * @tc.type: FUNC
 */
HWTEST_F(InputEventHubTestNg, InputEventHubHoverEventTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create InputEventHub.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto inputEventHub = AceType::MakeRefPtr<InputEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    EXPECT_NE(inputEventHub, nullptr);

    /**
     * @tc.steps: step2. Set hover animation.
     * @tc.expected: mouseEventActuator_ will be initialized and get the hoverEffect which will be assigned the correct
     * value.
     */
    inputEventHub->SetHoverAnimation(HOVER_EFFECT_TYPE);
    EXPECT_NE(inputEventHub->mouseEventActuator_, nullptr);
    EXPECT_EQ(inputEventHub->GetHoverEffect(), HOVER_EFFECT_TYPE);

    /**
     * @tc.steps: step3. Set HoverEvent.
     * @tc.expected: hoverEventActuator_ will be initialized.
     */
    OnHoverEventFunc onHover = [](bool) {};
    inputEventHub->SetHoverEvent(std::move(onHover));
    EXPECT_NE(inputEventHub->hoverEventActuator_, nullptr);

    /**
     * @tc.steps: step4. Add OnHoverEvent.
     * @tc.expected: hoverEventActuator_ will be initialized and and the list of inputEvents size is 1.
     */
    inputEventHub->hoverEventActuator_ = nullptr;
    OnMouseEventFunc onHover2 = [](MouseInfo& info) {};
    auto onHoverEvent = AceType::MakeRefPtr<InputEvent>(std::move(onHover2));
    inputEventHub->AddOnHoverEvent(onHoverEvent);
    EXPECT_NE(inputEventHub->hoverEventActuator_, nullptr);
    EXPECT_EQ(inputEventHub->hoverEventActuator_->inputEvents_.size(), INPUT_EVENTS_SIZE);

    /**
     * @tc.steps: step5. Remove OnHoverEvent.
     * @tc.expected: onHoverEvent will be remove from the list of inputEvents and the size will be 0. If
     * hoverEventActuator_ is nullptr, the function will return directly.
     */
    inputEventHub->RemoveOnHoverEvent(onHoverEvent);
    EXPECT_EQ(inputEventHub->hoverEventActuator_->inputEvents_.size(), INPUT_EVENTS_SIZE_2);
    inputEventHub->hoverEventActuator_ = nullptr;
    inputEventHub->RemoveOnHoverEvent(onHoverEvent);
}

/**
 * @tc.name: InputEventHubAxisEventTest004
 * @tc.desc: Create InputEventHub and invoke Axis related functions.
 * @tc.type: FUNC
 */
HWTEST_F(InputEventHubTestNg, InputEventHubAxisEventTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create InputEventHub.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto inputEventHub = AceType::MakeRefPtr<InputEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    EXPECT_NE(inputEventHub, nullptr);

    /**
     * @tc.steps: step2. Add OnAxisEvent.
     * @tc.expected: axisEventActuator_ will be initialized, and the list of inputEvents and the size will be 1.
     */
    OnMouseEventFunc onAxis = [](MouseInfo& info) {};
    auto onAxisEvent = AceType::MakeRefPtr<InputEvent>(std::move(onAxis));
    inputEventHub->AddOnAxisEvent(onAxisEvent);
    EXPECT_NE(inputEventHub->axisEventActuator_, nullptr);
    EXPECT_EQ(inputEventHub->axisEventActuator_->inputEvents_.size(), INPUT_EVENTS_SIZE);

    /**
     * @tc.steps: step3. Remove OnAxisEvent.
     * @tc.expected: onAxisEvent will be remove from the list of inputEvents and the size will be 0. If
     * axisEventActuator_ is nullptr, the function will return directly.
     */
    inputEventHub->RemoveOnAxisEvent(onAxisEvent);
    EXPECT_EQ(inputEventHub->axisEventActuator_->inputEvents_.size(), INPUT_EVENTS_SIZE_2);
    inputEventHub->axisEventActuator_ = nullptr;
    inputEventHub->RemoveOnAxisEvent(onAxisEvent);
}

/**
 * @tc.name: InputEventHubProcessMouseTest005
 * @tc.desc: Create InputEventHub and invoke ProcessMouseTestHit functions.
 * @tc.type: FUNC
 */
HWTEST_F(InputEventHubTestNg, InputEventHubProcessMouseTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create InputEventHub.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto inputEventHub = AceType::MakeRefPtr<InputEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    EXPECT_NE(inputEventHub, nullptr);

    /**
     * @tc.steps: step2. Invoke ProcessMouseTestHit when eventHub is nullptr.
     * @tc.expected: ProcessMouseTestHit return false.
     */
    MouseTestResult onMouseResult;
    MouseTestResult onHoverResult;
    auto hoverNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    auto inputEventHub2 = AceType::MakeRefPtr<InputEventHub>(nullptr);
    EXPECT_FALSE(inputEventHub2->ProcessMouseTestHit(COORDINATE_OFFSET, onMouseResult, onHoverResult, hoverNode));

    /**
     * @tc.steps: step3. Initialize mouseEventActuator_ and mouseEventActuator_, their inputEvents_ is empty and
     * userCallback_ is nullptr.
     * @tc.expected: OnCollectMouseEvent will return directly, and ProcessMouseTestHit return false.
     */
    inputEventHub->mouseEventActuator_ =
        AceType::MakeRefPtr<InputEventActuator>(AceType::WeakClaim(AceType::RawPtr(inputEventHub)));
    inputEventHub->hoverEventActuator_ =
        AceType::MakeRefPtr<InputEventActuator>(AceType::WeakClaim(AceType::RawPtr(inputEventHub)));
    EXPECT_FALSE(inputEventHub->ProcessMouseTestHit(COORDINATE_OFFSET, onMouseResult, onHoverResult, hoverNode));

    /**
     * @tc.steps: step4. Set MouseEvent and mouseEventActuator_ and userCallback_ will be initialized.
     */
    const OnMouseEventFunc onMouse = [](MouseInfo& info) {};
    OnMouseEventFunc onMouse1 = onMouse;
    inputEventHub->SetMouseEvent(std::move(onMouse1));
    EXPECT_NE(inputEventHub->mouseEventActuator_->userCallback_, nullptr);

    /**
     * @tc.steps: step5. Set HoverEvent and hoverEventActuator_ and userCallback_ will be initialized.
     */
    const OnHoverEventFunc onHover = [](bool) {};
    OnHoverEventFunc onHover1 = onHover;
    inputEventHub->SetHoverEvent(std::move(onHover1));
    EXPECT_NE(inputEventHub->hoverEventActuator_->userCallback_, nullptr);

    /**
     * @tc.steps: step6. Add OnMouseEvent and inputEvents_ will not be empty.
     */
    OnMouseEventFunc onMouse2 = onMouse;
    auto inputEvent = AceType::MakeRefPtr<InputEvent>(std::move(onMouse2));
    inputEventHub->AddOnMouseEvent(inputEvent);
    EXPECT_EQ(inputEventHub->mouseEventActuator_->inputEvents_.size(), INPUT_EVENTS_SIZE);

    /**
     * @tc.steps: step7. Set HoverEvent and inputEvents_ will not be empty.
     */
    OnHoverEventFunc onHover2 = onHover;
    auto onHoverEvent = AceType::MakeRefPtr<InputEvent>(std::move(onHover2));
    inputEventHub->AddOnHoverEvent(onHoverEvent);
    EXPECT_EQ(inputEventHub->hoverEventActuator_->inputEvents_.size(), INPUT_EVENTS_SIZE);

    /**
     * @tc.steps: step8. Invoke ProcessMouseTestHit when inputEvents_ is not empty and userCallback_ has already been
     * initialized.
     * @tc.expected: ProcessMouseTestHit return false, mouse and hover result size has been increased one.
     */
    EXPECT_FALSE(inputEventHub->ProcessMouseTestHit(COORDINATE_OFFSET, onMouseResult, onHoverResult, hoverNode));
    EXPECT_EQ(inputEventHub->mouseEventActuator_->eventTarget_->coordinateOffset_, COORDINATE_OFFSET);
    EXPECT_EQ(inputEventHub->hoverEventActuator_->eventTarget_->coordinateOffset_, COORDINATE_OFFSET);
    EXPECT_EQ(onMouseResult.size(), MOUSE_RESULT_SIZE);
    EXPECT_EQ(onHoverResult.size(), HOVER_RESULT_SIZE);
}

/**
 * @tc.name: InputEventHubProcessAxisTestHitTest006
 * @tc.desc: Create InputEventHub and invoke ProcessAxisTestHit functions.
 * @tc.type: FUNC
 */
HWTEST_F(InputEventHubTestNg, InputEventHubProcessAxisTestHitTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create InputEventHub.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    eventHub->AttachHost(frameNode);
    auto inputEventHub = AceType::MakeRefPtr<InputEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    EXPECT_NE(inputEventHub, nullptr);

    /**
     * @tc.steps: step2. Invoke ProcessAxisTestHit when eventHub is nullptr.
     * @tc.expected: ProcessMouseTestHit return false.
     */
    AxisTestResult onAxisResult;
    auto inputEventHub2 = AceType::MakeRefPtr<InputEventHub>(nullptr);
    EXPECT_FALSE(inputEventHub2->ProcessAxisTestHit(COORDINATE_OFFSET, onAxisResult));

    /**
     * @tc.steps: step3. Initialize axisEventActuator_ and its inputEvents_ is empty and userCallback_ is nullptr.
     * @tc.expected: OnCollectAxisEvent will return directly, and ProcessAxisTestHit return false.
     */
    inputEventHub->axisEventActuator_ =
        AceType::MakeRefPtr<InputEventActuator>(AceType::WeakClaim(AceType::RawPtr(inputEventHub)));
    EXPECT_FALSE(inputEventHub2->ProcessAxisTestHit(COORDINATE_OFFSET, onAxisResult));

    /**
     * @tc.steps: step4. Initialize userCallback_.
     */
    OnMouseEventFunc onAxis = [](MouseInfo& info) {};
    auto onAxisEvent = AceType::MakeRefPtr<InputEvent>(std::move(onAxis));
    inputEventHub->axisEventActuator_->userCallback_ = onAxisEvent;

    /**
     * @tc.steps: step5. Add OnAxisEvent.
     * @tc.expected: axisEventActuator_ will be initialized, and the list of inputEvents and the size will be 1.
     */
    inputEventHub->AddOnAxisEvent(onAxisEvent);
    EXPECT_EQ(inputEventHub->axisEventActuator_->inputEvents_.size(), INPUT_EVENTS_SIZE);

    /**
     * @tc.steps: step6. Invoke ProcessAxisTestHit when inputEvents_ is not empty and userCallback_ has already been
     * initialized.
     * @tc.expected: ProcessAxisTestHit return false, axis result size has been increased one.
     */
    EXPECT_FALSE(inputEventHub->ProcessAxisTestHit(COORDINATE_OFFSET, onAxisResult));
    EXPECT_EQ(inputEventHub->axisEventActuator_->axisEventTarget_->coordinateOffset_, COORDINATE_OFFSET);
    EXPECT_EQ(onAxisResult.size(), AXIS_RESULT_SIZE);
}
} // namespace OHOS::Ace::NG
