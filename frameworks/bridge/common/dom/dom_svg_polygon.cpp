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

#include "frameworks/bridge/common/dom/dom_svg_polygon.h"

#include "frameworks/bridge/common/utils/utils.h"

namespace OHOS::Ace::Framework {

DOMSvgPolygon::DOMSvgPolygon(NodeId nodeId, const std::string& nodeName) : DOMSvgBase(nodeId, nodeName) {}

RefPtr<Component> DOMSvgPolygon::GetSpecializedComponent()
{
    return polygonComponent_;
}

void DOMSvgPolygon::OnChildNodeAdded(const RefPtr<DOMNode>& child, int32_t slot)
{
    polygonComponent_->InsertChild(slot, child->GetSpecializedComponent());
}

void DOMSvgPolygon::OnMounted(const RefPtr<DOMNode>& parentNode)
{
    DOMSvgBase::InheritAttrs(parentNode);
}

void DOMSvgPolygon::PrepareSpecializedComponent()
{
    if (!polygonComponent_) {
        polygonComponent_ = AceType::MakeRefPtr<SvgPolygonComponent>();
    }
    auto declaration = AceType::DynamicCast<SvgPolygonDeclaration>(declaration_);
    if (declaration) {
        polygonComponent_->SetDeclaration(declaration);
    }
}

} // namespace OHOS::Ace::Framework
