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

#include "core/components_ng/pattern/form/form_model_ng.h"

#include "base/geometry/dimension.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/form/form_layout_property.h"
#include "core/components_ng/pattern/form/form_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
void FormModelNG::Create(const RequestFormInfo& formInfo)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto frameNode = FormNode::GetOrCreateFormNode(
        V2::FORM_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<FormPattern>(); });
    stack->Push(frameNode);

    ACE_UPDATE_LAYOUT_PROPERTY(FormLayoutProperty, RequestFormInfo, formInfo);
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, Visibility, VisibleType::INVISIBLE);
    ACE_UPDATE_LAYOUT_PROPERTY(FormLayoutProperty, VisibleType, VisibleType::VISIBLE);
}

void FormModelNG::SetDimension(int32_t dimension)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto property = frameNode->GetLayoutProperty<FormLayoutProperty>();
    CHECK_NULL_VOID(property);
    if (!property->HasRequestFormInfo()) {
        return;
    }
    auto formInfo = property->GetRequestFormInfoValue();
    formInfo.dimension = dimension;
    ACE_UPDATE_LAYOUT_PROPERTY(FormLayoutProperty, RequestFormInfo, formInfo);
}


void FormModelNG::SetSize(const Dimension& width, const Dimension& height) {}

void FormModelNG::AllowUpdate(bool allowUpdate)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto property = frameNode->GetLayoutProperty<FormLayoutProperty>();
    CHECK_NULL_VOID(property);
    if (!property->HasRequestFormInfo()) {
        return;
    }
    auto formInfo = property->GetRequestFormInfoValue();
    formInfo.allowUpdate = allowUpdate;
    property->UpdateRequestFormInfo(formInfo);
}

void FormModelNG::SetVisible(VisibleType visible)
{
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, Visibility, visible);
}

void FormModelNG::SetVisibility(VisibleType visible)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto formPattern = frameNode->GetPattern<FormPattern>();
    CHECK_NULL_VOID(formPattern);
    auto isLoaded = formPattern->GetIsLoaded();
    auto layoutProperty = frameNode->GetLayoutProperty<FormLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (isLoaded || visible != VisibleType::VISIBLE) {
        layoutProperty->UpdateVisibility(visible, true);
    } else {
        layoutProperty->UpdateVisibility(VisibleType::INVISIBLE, true);
    }

    ACE_UPDATE_LAYOUT_PROPERTY(FormLayoutProperty, VisibleType, visible);
}

void FormModelNG::SetModuleName(const std::string& moduleName)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto property = frameNode->GetLayoutProperty<FormLayoutProperty>();
    CHECK_NULL_VOID(property);
    if (!property->HasRequestFormInfo()) {
        return;
    }
    auto formInfo = property->GetRequestFormInfoValue();
    formInfo.moduleName = moduleName;
    property->UpdateRequestFormInfo(formInfo);
}

void FormModelNG::SetOnAcquired(std::function<void(const std::string&)>&& onAcquired)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnAcquired(std::move(onAcquired));
}

void FormModelNG::SetOnError(std::function<void(const std::string&)>&& onError)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnError(std::move(onError));
}

void FormModelNG::SetOnUninstall(std::function<void(const std::string&)>&& onUninstall)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnUninstall(std::move(onUninstall));
}

void FormModelNG::SetOnRouter(std::function<void(const std::string&)>&& onRouter)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnRouter(std::move(onRouter));
}

void FormModelNG::SetOnLoad(std::function<void(const std::string&)>&& onLoad)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnLoad(std::move(onLoad));
}
} // namespace OHOS::Ace::NG
