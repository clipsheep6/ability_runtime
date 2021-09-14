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

#include "frameworks/core/components/svg/svg_fe_offset_component.h"

#include "frameworks/core/components/svg/render_svg_fe_offset.h"
#include "frameworks/core/components/svg/svg_fe_offset_element.h"

namespace OHOS::Ace {

SvgFeOffsetComponent::SvgFeOffsetComponent()
{
    InitDeclaration();
}

SvgFeOffsetComponent::SvgFeOffsetComponent(const std::list<RefPtr<Component>>& children)
    : ComponentGroup(children)
{
    InitDeclaration();
}

void SvgFeOffsetComponent::InitDeclaration()
{
    if (!declaration_) {
        declaration_ = AceType::MakeRefPtr<SvgFeOffsetDeclaration>();
        declaration_->Init();
        declaration_->InitializeStyle();
    }
}

RefPtr<Element> SvgFeOffsetComponent::CreateElement()
{
    return AceType::MakeRefPtr<SvgFeOffsetElement>();
}


RefPtr<RenderNode> SvgFeOffsetComponent::CreateRenderNode()
{
    return RenderSvgFeOffset::Create();
}

void SvgFeOffsetComponent::SetDeclaration(const RefPtr<SvgFeOffsetDeclaration>& declaration)
{
    if (declaration) {
        declaration_ = declaration;
    }
}

} // namespace OHOS::Ace
