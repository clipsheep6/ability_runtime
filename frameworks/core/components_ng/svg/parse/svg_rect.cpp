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

#include "frameworks/core/components_ng/svg/parse/svg_rect.h"

#include "frameworks/core/components/declaration/svg/svg_rect_declaration.h"

namespace OHOS::Ace::NG {

SvgRect::SvgRect() : SvgGraphic()
{
    declaration_ = AceType::MakeRefPtr<SvgRectDeclaration>();
    declaration_->Init();
    declaration_->InitializeStyle();
}

RefPtr<SvgNode> SvgRect::Create()
{
    return AceType::MakeRefPtr<SvgRect>();
}

SkPath SvgRect::AsPath(const Size& viewPort) const
{
    auto declaration = AceType::DynamicCast<SvgRectDeclaration>(declaration_);
    if (declaration == nullptr) {
        return SkPath();
    }
    double rx = 0.0;
    if (GreatOrEqual(declaration->GetRx().Value(), 0.0)) {
        rx = ConvertDimensionToPx(declaration->GetRx(), viewPort, SvgLengthType::HORIZONTAL);
    } else {
        if (GreatNotEqual(declaration->GetRy().Value(), 0.0)) {
            rx = ConvertDimensionToPx(declaration->GetRy(), viewPort, SvgLengthType::VERTICAL);
        }
    }
    double ry = 0.0;
    if (GreatOrEqual(declaration->GetRy().Value(), 0.0)) {
        ry = ConvertDimensionToPx(declaration->GetRy(), viewPort, SvgLengthType::VERTICAL);
    } else {
        if (GreatNotEqual(declaration->GetRx().Value(), 0.0)) {
            ry = ConvertDimensionToPx(declaration->GetRx(), viewPort, SvgLengthType::HORIZONTAL);
        }
    }
    SkRRect roundRect = SkRRect::MakeRectXY(
        SkRect::MakeXYWH(ConvertDimensionToPx(declaration->GetX(), viewPort, SvgLengthType::HORIZONTAL),
            ConvertDimensionToPx(declaration->GetY(), viewPort, SvgLengthType::VERTICAL),
            ConvertDimensionToPx(declaration->GetWidth(), viewPort, SvgLengthType::HORIZONTAL),
            ConvertDimensionToPx(declaration->GetHeight(), viewPort, SvgLengthType::VERTICAL)),
        rx, ry);
    SkPath path;
    path.addRRect(roundRect);
    return path;
}

} // namespace OHOS::Ace::NG
