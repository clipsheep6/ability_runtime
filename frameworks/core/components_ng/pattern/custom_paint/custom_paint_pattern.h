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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CUSTOM_PAINT_CUSTOM_PAINT_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CUSTOM_PAINT_CUSTOM_PAINT_PATTERN_H

#include "base/memory/referenced.h"
#include "core/components/common/properties/paint_state.h"
#include "core/components_ng/pattern/custom_paint/custom_paint_event_hub.h"
#include "core/components_ng/pattern/custom_paint/custom_paint_layout_algorithm.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
class CanvasPaintMethod;
class OffscreenCanvasPattern;
// CustomPaintPattern is the base class for custom paint render node to perform paint canvas.
class ACE_EXPORT CustomPaintPattern : public Pattern {
    DECLARE_ACE_TYPE(CustomPaintPattern, Pattern);

public:
    CustomPaintPattern() = default;
    ~CustomPaintPattern() override = default;

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override;

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<CustomPaintLayoutAlgorithm>();
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<CustomPaintEventHub>();
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    void FillRect(const Rect& rect);
    void StrokeRect(const Rect& rect);
    void Fill();
    void Fill(const RefPtr<CanvasPath2D>& path);
    void Stroke();
    void Stroke(const RefPtr<CanvasPath2D>& path);
    void BeginPath();
    void ClosePath();
    void MoveTo(double x, double y);
    void LineTo(double x, double y);
    void Arc(const ArcParam& param);
    void ArcTo(const ArcToParam& param);
    void AddRect(const Rect& rect);
    void Ellipse(const EllipseParam& param);
    void BezierCurveTo(const BezierCurveParam& param);
    void QuadraticCurveTo(const QuadraticCurveParam& param);

    void DrawImage(const Ace::CanvasImage& image, double width, double height);
    void DrawPixelMap(RefPtr<PixelMap> pixelMap, const Ace::CanvasImage& image);
    std::unique_ptr<Ace::ImageData> GetImageData(double left, double top, double width, double height);
    void PutImageData(const Ace::ImageData& imageData);
    void TransferFromImageBitmap(const RefPtr<OffscreenCanvasPattern>& offscreenCanvasPattern);

    void UpdateFillColor(const Color& color);
    void UpdateFillRuleForPath(const CanvasFillRule rule);
    void UpdateFillRuleForPath2D(const CanvasFillRule rule);

private:
    void OnAttachToFrameNode() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    RefPtr<CanvasPaintMethod> paintMethod_;

    ACE_DISALLOW_COPY_AND_MOVE(CustomPaintPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CUSTOM_PAINT_CUSTOM_PAINT_PATTERN_H
