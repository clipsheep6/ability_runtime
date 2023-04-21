/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "core/components_v2/list/rosen_render_list.h"
#include "core/components_v2/list/render_list_item_group.h"

#include "render_service_client/core/ui/rs_node.h"

#include "base/utils/utils.h"
#include "core/components/common/painter/rosen_scroll_bar_painter.h"
#include "core/pipeline/base/rosen_render_context.h"

namespace OHOS::Ace::V2 {

void RosenRenderList::Update(const RefPtr<Component>& component)
{
    RenderList::Update(component);
    auto rsNode = GetRSNode();
    if (rsNode == nullptr) {
        LOGE("rsNode is null");
        return;
    }
    rsNode->SetClipToFrame(true);
}

void RosenRenderList::DrawDividerOnNode(const std::shared_ptr<RSNode>& rsNode, const SkPaint& paint, bool isVertical,
    double startCrossAxis, double mainAxis, double endCrossAxis)
{
    rsNode->DrawOnNode(Rosen::RSModifierType::CONTENT_STYLE,
        [isVertical = vertical_, startX = startCrossAxis, startY = mainAxis, endX = endCrossAxis, endY = mainAxis,
            paint](const std::shared_ptr<SkCanvas>& canvas) mutable {
            if (!canvas) {
                LOGE("sk canvas is nullptr when try paint divider for list");
                return;
            }
            if (!isVertical) {
                std::swap(startX, startY);
                std::swap(endX, endY);
            }
            canvas->drawLine(startX, startY, endX, endY, paint);
        });
}

void RosenRenderList::Paint(RenderContext& context, const Offset& offset)
{
    int indexStep = 0;
    for (const auto& child : items_) {
        if (child == currentStickyItem_ || child == selectedItem_) {
            continue;
        }

        if (child->IsSelected()) {
            PaintItemZone(context, child);
        }
        if (drivenRender_) {
            auto childRSNode = child->GetRSNode();
            if (childRSNode != nullptr) {
                childRSNode->MarkDrivenRenderItemIndex(GetStartIndex() + indexStep);
            }
        }
        indexStep++;
        PaintChild(child, context, offset);
    }

    const auto renderContext = static_cast<RosenRenderContext*>(&context);
    auto canvas = renderContext->GetCanvas();
    auto rsNode = renderContext->GetRSNode();
    if (!canvas || !rsNode) {
        LOGE("canvas is null");
        return;
    }
    rsNode->MarkDrivenRender(drivenRender_);
    if (drivenRender_) {
        if (IsFramePaintStateValid()) {
            rsNode->MarkDrivenRenderFramePaintState(GetPaintState());
        }
    }
    MarkFramePaintStateValid(false);
    // if we call [SetPaintOrder(true)], all draw commands that are directly generated by [RosenRenderList] will be
    // executed at last, which means the paint action of [divider] and [scrollBar_] is at the last(after
    // [currentStickyItem_] being painted). Because [currentStickyItem_] is painted by [context.PiantChild] so that it has its
    // own seperate recording. In order to accomplish the effect that [divider] is covered by [currentStickyItem_], we
    // should draw it before [currentStickyItem_] being painted.
    rsNode->SetPaintOrder(true);
    // paint custom effect
    if (scrollEffect_) {
        scrollEffect_->Paint(context, viewPort_, offset);
    }
    PaintDivider(rsNode);

    if (currentStickyItem_) {
        PaintChild(currentStickyItem_, context, offset);
    }

    if (selectedItem_) {
        selectedItem_->SetPosition(MakeValue<Offset>(selectedItemMainAxis_, 0.0));
        PaintChild(selectedItem_, context, offset);
    }

    // paint scrollBar
    if (scrollBar_ && scrollBar_->NeedPaint()) {
        bool needPaint = false;
        if (scrollBar_->IsActive() || scrollBar_->GetDisplayMode() == DisplayMode::ON) {
            scrollBarOpacity_ = UINT8_MAX;
            needPaint = true;
        } else {
            // for DisplayMode::Auto
            if (scrollBarOpacity_ != 0) {
                needPaint = true;
            }
        }
        if (needPaint) {
            scrollBar_->UpdateScrollBarRegion(offset, GetLayoutSize(), GetLastOffset(), GetEstimatedHeight());
            RefPtr<RosenScrollBarPainter> scrollBarPainter = AceType::MakeRefPtr<RosenScrollBarPainter>();
            scrollBarPainter->PaintBar(
                canvas, offset, GetPaintRect(), scrollBar_, GetGlobalOffset(), scrollBarOpacity_);
        }
    }
    if (scrollBarProxy_) {
        scrollBarProxy_->NotifyScrollBar(AceType::WeakClaim(this));
    }

    PaintSelectedZone(context);
}

void RosenRenderList::PaintDivider(const std::shared_ptr<RSNode>& rsNode)
{
    const auto& layoutSize = GetLayoutSize();
    const auto& divider = component_->GetItemDivider();
    if (!divider || divider->color.GetAlpha() <= 0x00 || LessOrEqual(divider->strokeWidth.Value(), 0.0)) {
        return;
    }
    const double crossSize = GetCrossSize(layoutSize);
    const double strokeWidth = NormalizePercentToPx(divider->strokeWidth, vertical_);
    const double halfSpaceWidth = std::max(spaceWidth_, strokeWidth) / 2.0;
    const double startMargin = NormalizePercentToPx(divider->startMargin, !vertical_);
    const double endMargin = NormalizePercentToPx(divider->endMargin, !vertical_);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(divider->color.GetValue());
    paint.setStyle(SkPaint::Style::kStroke_Style);
    paint.setStrokeWidth(strokeWidth);
    bool lastIsItemGroup = false;
    bool isFirstItem = (startIndex_ == 0);
    int lane = 0;

    for (const auto& child : items_) {
        bool isItemGroup = static_cast<bool>(AceType::DynamicCast<RenderListItemGroup>(child));
        double mainAxis = GetMainAxis(child->GetPosition());
        if (!isFirstItem && child != selectedItem_) {
            mainAxis -= halfSpaceWidth;
            if (GetLanes() > 1 && !lastIsItemGroup && !isItemGroup) {
                double start = crossSize / GetLanes() * lane + startMargin;
                double end = crossSize / GetLanes() * (lane + 1) - endMargin;
                DrawDividerOnNode(rsNode, paint, vertical_, start, mainAxis, end);
            } else {
                DrawDividerOnNode(rsNode, paint, vertical_, startMargin, mainAxis, crossSize - endMargin);
            }
        }
        lastIsItemGroup = isItemGroup;
        lane = (GetLanes() <= 1 || (lane + 1) >= GetLanes() || isItemGroup) ? 0 : lane + 1;
        isFirstItem = isFirstItem ? lane > 0 : false;
    }

    if (selectedItem_) {
        double mainAxis = targetMainAxis_ - halfSpaceWidth;
        DrawDividerOnNode(rsNode, paint, vertical_, startMargin, mainAxis, crossSize - endMargin);
    }
}

void RosenRenderList::PaintSelectedZone(RenderContext& context)
{
    auto canvas = static_cast<RosenRenderContext*>(&context)->GetCanvas();
    if (canvas == nullptr) {
        LOGE("skia canvas is null");
        return;
    }

    SkRect skRect = SkRect::MakeXYWH(mouseStartOffset_.GetX(), mouseStartOffset_.GetY(),
        mouseEndOffset_.GetX() - mouseStartOffset_.GetX(), mouseEndOffset_.GetY() - mouseStartOffset_.GetY());

    SkPaint fillGeometry;
    fillGeometry.setAntiAlias(true);
    fillGeometry.setStyle(SkPaint::Style::kFill_Style);
    fillGeometry.setColor(0x1A000000);

    SkPaint strokeGeometry;
    strokeGeometry.setAntiAlias(true);
    strokeGeometry.setStyle(SkPaint::Style::kStroke_Style);
    strokeGeometry.setColor(0x33FFFFFF);
    strokeGeometry.setStrokeWidth(NormalizeToPx(1.0_vp));

    canvas->drawRect(skRect, fillGeometry);
    canvas->drawRect(skRect, strokeGeometry);
}

void RosenRenderList::PaintItemZone(RenderContext& context, const RefPtr<RenderListItem>& item)
{
    auto canvas = static_cast<RosenRenderContext*>(&context)->GetCanvas();
    if (canvas == nullptr) {
        LOGE("skia canvas is null");
        return;
    }

    SkRect skRect = SkRect::MakeXYWH(item->GetPaintRect().GetOffset().GetX(), item->GetPaintRect().GetOffset().GetY(),
        item->GetPaintRect().Width(), item->GetPaintRect().Height());

    SkRRect rrect = SkRRect::MakeEmpty();
    SkVector rectRadii[4] = { { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 } };
    rectRadii[SkRRect::kUpperLeft_Corner] =
        SkPoint::Make(NormalizeToPx(item->GetBorderRadius()), NormalizeToPx(item->GetBorderRadius()));
    rectRadii[SkRRect::kUpperRight_Corner] =
        SkPoint::Make(NormalizeToPx(item->GetBorderRadius()), NormalizeToPx(item->GetBorderRadius()));
    rectRadii[SkRRect::kLowerRight_Corner] =
        SkPoint::Make(NormalizeToPx(item->GetBorderRadius()), NormalizeToPx(item->GetBorderRadius()));
    rectRadii[SkRRect::kLowerLeft_Corner] =
        SkPoint::Make(NormalizeToPx(item->GetBorderRadius()), NormalizeToPx(item->GetBorderRadius()));
    rrect.setRectRadii(skRect, rectRadii);

    SkPaint fillGeometry;
    fillGeometry.setAntiAlias(true);
    fillGeometry.setStyle(SkPaint::Style::kFill_Style);
    fillGeometry.setColor(0x1A0A59f7);

    canvas->drawRRect(rrect, fillGeometry);
}

} // namespace OHOS::Ace::V2
