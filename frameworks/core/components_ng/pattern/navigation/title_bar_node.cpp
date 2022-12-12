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

#include "core/components_ng/pattern/navigation/title_bar_node.h"

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/pattern/navigation/title_bar_pattern.h"

namespace OHOS::Ace::NG {

TitleBarNode::TitleBarNode(const std::string& tag, int32_t nodeId)
    : FrameNode(tag, nodeId, MakeRefPtr<TitleBarPattern>())
{}

RefPtr<TitleBarNode> TitleBarNode::GetOrCreateTitleBarNode(
    const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator)
{
    auto frameNode = GetFrameNode(tag, nodeId);
    CHECK_NULL_RETURN_NOLOG(!frameNode, AceType::DynamicCast<TitleBarNode>(frameNode));
    auto pattern = patternCreator ? patternCreator() : MakeRefPtr<Pattern>();
    auto titleBarNode = AceType::MakeRefPtr<TitleBarNode>(tag, nodeId, pattern);
    titleBarNode->InitializePatternAndContext();
    ElementRegister::GetInstance()->AddUINode(titleBarNode);
    return titleBarNode;
}

} // namespace OHOS::Ace::NG