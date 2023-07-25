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

#include "core/components_ng/pattern/stepper/stepper_model_ng.h"

#include "base/i18n/localization.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/stepper/stepper_node.h"
#include "core/components_ng/pattern/stepper/stepper_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

void StepperModelNG::Create(uint32_t index)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto stepperNode = StepperNode::GetOrCreateStepperNode(
        V2::STEPPER_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<StepperPattern>(); });
    stack->Push(stepperNode);
    bool hasSwiperNode = stepperNode->HasSwiperNode();
    auto swiperId = stepperNode->GetSwiperId();
    if (!hasSwiperNode) {
        auto swiperNode = CreateSwiperChild(swiperId, index);
        swiperNode->MountToParent(stepperNode);
        ACE_UPDATE_LAYOUT_PROPERTY(StepperLayoutProperty, Index, index);
    } else {
        auto swiperNode = AceType::DynamicCast<FrameNode>(
            stepperNode->GetChildAtIndex(stepperNode->GetChildIndexById(stepperNode->GetSwiperId())));
        CHECK_NULL_VOID(swiperNode);
        auto swiperController = swiperNode->GetPattern<SwiperPattern>()->GetSwiperController();
        swiperController->SwipeTo(index);
    }
}

void StepperModelNG::SetOnFinish(RoutineCallbackEvent&& eventOnFinish)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<StepperEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetFinishEvent(std::move(eventOnFinish));
}

void StepperModelNG::SetOnSkip(RoutineCallbackEvent&& eventOnSkip)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<StepperEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetSkipEvent(std::move(eventOnSkip));
}

void StepperModelNG::SetOnChange(IndexCallbackEvent&& eventOnChange)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<StepperEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetChangeEvent(std::move(eventOnChange));
}

void StepperModelNG::SetOnNext(IndexCallbackEvent&& eventOnNext)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<StepperEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetNextEvent(std::move(eventOnNext));
}

void StepperModelNG::SetOnPrevious(IndexCallbackEvent&& eventOnPrevious)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<StepperEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetPreviousEvent(std::move(eventOnPrevious));
}

RefPtr<FrameNode> StepperModelNG::CreateSwiperChild(int32_t id, uint32_t index)
{
    auto swiperNode =
        FrameNode::GetOrCreateFrameNode(V2::SWIPER_ETS_TAG, id, []() { return AceType::MakeRefPtr<SwiperPattern>(); });
    auto swiperPaintProperty = swiperNode->GetPaintProperty<SwiperPaintProperty>();
    swiperPaintProperty->UpdateLoop(false);
    swiperPaintProperty->UpdateEdgeEffect(EdgeEffect::NONE);
    swiperPaintProperty->UpdateDisableSwipe(true);
    swiperNode->GetLayoutProperty<SwiperLayoutProperty>()->UpdateCachedCount(0);
    swiperNode->GetLayoutProperty<SwiperLayoutProperty>()->UpdateIndex(static_cast<int32_t>(index));
    swiperNode->GetLayoutProperty<SwiperLayoutProperty>()->UpdateShowIndicator(false);
    swiperNode->MarkModifyDone();
    return swiperNode;
}

void StepperModelNG::SetOnChangeEvent(IndexChangeEvent&& onChangeEvent)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<StepperEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnChangeEvent(std::move(onChangeEvent));
}
} // namespace OHOS::Ace::NG
