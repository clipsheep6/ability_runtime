/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "frameworks/bridge/common/dom/dom_rich_text.h"

#include "core/components/declaration/richtext/rich_text_declaration.h"
#include "frameworks/bridge/common/dom/dom_type.h"
#include "frameworks/bridge/common/utils/utils.h"

namespace OHOS::Ace::Framework {

DOMRichText::DOMRichText(NodeId nodeId, const std::string& nodeName) : DOMNode(nodeId, nodeName)
{
    childComponent_ = AceType::MakeRefPtr<RichTextComponent>(nodeName);
}

void DOMRichText::PrepareSpecializedComponent()
{
    auto richTextDeclaration = AceType::DynamicCast<RichTextDeclaration>(declaration_);
    if (!richTextDeclaration) {
        return;
    }
    childComponent_->SetDeclaration(richTextDeclaration);
}

} // namespace OHOS::Ace::Framework
