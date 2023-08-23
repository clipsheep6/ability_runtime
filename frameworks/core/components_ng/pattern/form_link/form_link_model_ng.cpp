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

#include "core/components_ng/pattern/form_link/form_link_model_ng.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/form_link/form_link_pattern.h"

namespace OHOS::Ace::NG {
void FormLinkModelNG::Create(const std::string& action)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::FORM_LINK_ETS_TAG, stack->ClaimNodeId(), []() { return AceType::MakeRefPtr<FormLinkPattern>(); });
    auto pattern = frameNode->GetPattern<FormLinkPattern>();
    pattern->SetAction(action);
    stack->Push(frameNode);
}
} // namespace OHOS::Ace::NG
