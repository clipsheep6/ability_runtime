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

#include "core/components_ng/pattern/canvas_renderer/canvas_renderer_model_ng.h"

#include "frameworks/core/components_ng/pattern/custom_paint/custom_paint_pattern.h"
#include "frameworks/core/components_ng/pattern/custom_paint/offscreen_canvas_pattern.h"

namespace OHOS::Ace::NG {
void CanvasRendererModelNG::SetFillText(const BaseInfo& baseInfo, const FillTextInfo& fillTextInfo)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->FillText(fillTextInfo.text, fillTextInfo.x, fillTextInfo.y,
            fillTextInfo.maxWidth, baseInfo.paintState);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->FillText(fillTextInfo.text, fillTextInfo.x, fillTextInfo.y, fillTextInfo.maxWidth);
    }
}

void CanvasRendererModelNG::SetStrokeText(const BaseInfo& baseInfo, const FillTextInfo& fillTextInfo)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->StrokeText(fillTextInfo.text, fillTextInfo.x, fillTextInfo.y,
            fillTextInfo.maxWidth, baseInfo.paintState);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->StrokeText(fillTextInfo.text, fillTextInfo.x, fillTextInfo.y, fillTextInfo.maxWidth);
    }
}

void CanvasRendererModelNG::SetAntiAlias(const BaseInfo& baseInfo)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetAntiAlias(baseInfo.anti);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->SetAntiAlias(baseInfo.anti);
    }
}

void CanvasRendererModelNG::SetFontWeight(const BaseInfo& baseInfo, const FontWeight& weight)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetFontWeight(weight);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateFontWeight(weight);
    }
}

void CanvasRendererModelNG::SetFontStyle(const BaseInfo& baseInfo, const FontStyle& fontStyle)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetFontStyle(fontStyle);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateFontStyle(fontStyle);
    }
}

void CanvasRendererModelNG::SetFontFamilies(const BaseInfo& baseInfo, const std::vector<std::string>& families)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetFontFamilies(families);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateFontFamilies(families);
    }
}

void CanvasRendererModelNG::SetFontSize(const BaseInfo& baseInfo, const Dimension& size)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetFontSize(size);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateFontSize(size);
    }
}

std::vector<double> CanvasRendererModelNG::GetLineDash(const BaseInfo& baseInfo)
{
    std::vector<double> lineDash {};
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_RETURN(offscreenPattern, lineDash);
        lineDash = offscreenPattern->GetLineDash().lineDash;
    } else if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_RETURN(canvasPattern, lineDash);
        lineDash = canvasPattern->GetLineDash().lineDash;
    }

    return lineDash;
}

void CanvasRendererModelNG::SetFillGradient(const BaseInfo& baseInfo, const Ace::Gradient& gradient)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetFillGradient(gradient);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateFillGradient(gradient);
    }
}

void CanvasRendererModelNG::SetFillPattern(const BaseInfo& baseInfo, const std::shared_ptr<Ace::Pattern>& pattern)
{
    std::weak_ptr<Ace::Pattern> patternWeakptr = pattern;
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetFillPattern(patternWeakptr);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateFillPattern(patternWeakptr);
    }
}

void CanvasRendererModelNG::SetFillColor(const BaseInfo& baseInfo, const Color& color, bool colorFlag)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetFillColor(color);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateFillColor(color);
    }
}

void CanvasRendererModelNG::SetStrokeGradient(const BaseInfo& baseInfo, const Ace::Gradient& gradient)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetStrokeGradient(gradient);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateStrokeGradient(gradient);
    }
}

void CanvasRendererModelNG::SetStrokePattern(const BaseInfo& baseInfo, const std::shared_ptr<Ace::Pattern>& pattern)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetStrokePattern(pattern);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateStrokePattern(pattern);
    }
}

void CanvasRendererModelNG::SetStrokeColor(const BaseInfo& baseInfo, const Color& color, bool colorFlag)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetStrokeColor(color);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateStrokeColor(color);
    }
}

void CanvasRendererModelNG::DrawImage(const BaseInfo& baseInfo, const ImageInfo& imageInfo)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        if (imageInfo.isImage) {
            offscreenPattern->DrawImage(imageInfo.image, imageInfo.imgWidth, imageInfo.imgHeight);
            return;
        }
        offscreenPattern->DrawPixelMap(imageInfo.pixelMap, imageInfo.image);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        if (imageInfo.isImage) {
            canvasPattern->DrawImage(imageInfo.image, imageInfo.imgWidth, imageInfo.imgHeight);
            return;
        }
        canvasPattern->DrawPixelMap(imageInfo.pixelMap, imageInfo.image);
    }
}

void CanvasRendererModelNG::PutImageData(const BaseInfo& baseInfo, const ImageData& imageData)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->PutImageData(imageData);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->PutImageData(imageData);
    }
}

void CanvasRendererModelNG::CloseImageBitmap(const BaseInfo& baseInfo, const std::string& src)
{
    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->CloseImageBitmap(src);
    }
}

std::unique_ptr<ImageData> CanvasRendererModelNG::GetImageData(const BaseInfo& baseInfo, const ImageSize& imageSize)
{
    std::unique_ptr<ImageData> data;
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_RETURN(offscreenPattern, data);
        data = offscreenPattern->GetImageData(imageSize.left, imageSize.top, imageSize.width, imageSize.height);
    } else if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_RETURN(canvasPattern, data);
        data = canvasPattern->GetImageData(imageSize.left, imageSize.top, imageSize.width, imageSize.height);
    }
    return data;
}

void CanvasRendererModelNG::DrawPixelMap(const BaseInfo& baseInfo, const ImageInfo& imageInfo)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->DrawPixelMap(imageInfo.pixelMap, imageInfo.image);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->DrawPixelMap(imageInfo.pixelMap, imageInfo.image);
    }
}

void CanvasRendererModelNG::SetFilterParam(const BaseInfo& baseInfo, const std::string& src)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetFilterParam(src);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->SetFilterParam(src);
    }
}

void CanvasRendererModelNG::SetTextDirection(const BaseInfo& baseInfo, const TextDirection& direction)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetTextDirection(direction);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->SetTextDirection(direction);
    }
}

std::string CanvasRendererModelNG::GetJsonData(const BaseInfo& baseInfo, const std::string& path)
{
    std::string jsonData = "";
    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_RETURN(canvasPattern, jsonData);
        jsonData = canvasPattern->GetJsonData(path);
    }
    return jsonData;
}

std::string CanvasRendererModelNG::ToDataURL(const BaseInfo& baseInfo, const std::string& dataUrl, double quality)
{
    std::string result = "";
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_RETURN(offscreenPattern, result);
        result = offscreenPattern->ToDataURL(dataUrl, quality);
    } else if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_RETURN(canvasPattern, result);
        result = canvasPattern->ToDataURL(dataUrl + "," + std::to_string(quality));
    }
    return result;
}

void CanvasRendererModelNG::SetLineCap(const BaseInfo& baseInfo, const LineCapStyle& lineCap)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetLineCap(lineCap);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateLineCap(lineCap);
    }
}

void CanvasRendererModelNG::SetLineJoin(const BaseInfo& baseInfo, const LineJoinStyle& lineJoin)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetLineJoin(lineJoin);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateLineJoin(lineJoin);
    }
}

void CanvasRendererModelNG::SetMiterLimit(const BaseInfo& baseInfo, double limit)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetMiterLimit(limit);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateMiterLimit(limit);
    }
}

void CanvasRendererModelNG::SetLineWidth(const BaseInfo& baseInfo, double lineWidth)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetLineWidth(lineWidth);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateLineWidth(lineWidth);
    }
}

void CanvasRendererModelNG::SetGlobalAlpha(const BaseInfo& baseInfo, double alpha)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetAlpha(alpha);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateGlobalAlpha(alpha);
    }
}

void CanvasRendererModelNG::SetCompositeType(const BaseInfo& baseInfo, const CompositeOperation& type)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetCompositeType(type);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateCompositeOperation(type);
    }
}

void CanvasRendererModelNG::SetLineDashOffset(const BaseInfo& baseInfo, double lineDashOffset)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetLineDashOffset(lineDashOffset);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateLineDashOffset(lineDashOffset);
    }
}

void CanvasRendererModelNG::SetShadowBlur(const BaseInfo& baseInfo, double blur)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetShadowBlur(blur);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateShadowBlur(blur);
    }
}

void CanvasRendererModelNG::SetShadowColor(const BaseInfo& baseInfo, const Color& color)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetShadowColor(color);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateShadowColor(color);
    }
}

void CanvasRendererModelNG::SetShadowOffsetX(const BaseInfo& baseInfo, double offsetX)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetShadowOffsetX(offsetX);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateShadowOffsetX(offsetX);
    }
}

void CanvasRendererModelNG::SetShadowOffsetY(const BaseInfo& baseInfo, double offsetY)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetShadowOffsetY(offsetY);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateShadowOffsetY(offsetY);
    }
}

void CanvasRendererModelNG::SetSmoothingEnabled(const BaseInfo& baseInfo, bool enabled)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetSmoothingEnabled(enabled);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateSmoothingEnabled(enabled);
    }
}

void CanvasRendererModelNG::SetSmoothingQuality(const BaseInfo& baseInfo, const std::string& quality)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetSmoothingQuality(quality);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateSmoothingQuality(quality);
    }
}

void CanvasRendererModelNG::MoveTo(const BaseInfo& baseInfo, double x, double y)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->MoveTo(x, y);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->MoveTo(x, y);
    }
}

void CanvasRendererModelNG::LineTo(const BaseInfo& baseInfo, double x, double y)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->LineTo(x, y);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->LineTo(x, y);
    }
}

void CanvasRendererModelNG::BezierCurveTo(const BaseInfo& baseInfo, const BezierCurveParam& param)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->BezierCurveTo(param);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->BezierCurveTo(param);
    }
}

void CanvasRendererModelNG::QuadraticCurveTo(const BaseInfo& baseInfo, const QuadraticCurveParam& param)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->QuadraticCurveTo(param);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->QuadraticCurveTo(param);
    }
}

void CanvasRendererModelNG::ArcTo(const BaseInfo& baseInfo, const ArcToParam& param)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->ArcTo(param);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->ArcTo(param);
    }
}

void CanvasRendererModelNG::Arc(const BaseInfo& baseInfo, const ArcParam& param)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->Arc(param);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->Arc(param);
    }
}

void CanvasRendererModelNG::Ellipse(const BaseInfo& baseInfo, const EllipseParam& param)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->Ellipse(param);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->Ellipse(param);
    }
}

void CanvasRendererModelNG::SetFillRuleForPath(const BaseInfo& baseInfo, const CanvasFillRule& fillRule)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetFillRuleForPath(fillRule);
        offscreenPattern->Fill();
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateFillRuleForPath(fillRule);
        canvasPattern->Fill();
    }
}

void CanvasRendererModelNG::SetFillRuleForPath2D(const BaseInfo& baseInfo, const CanvasFillRule& fillRule,
    const RefPtr<CanvasPath2D>& path)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetFillRuleForPath2D(fillRule);
        offscreenPattern->Fill(path);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateFillRuleForPath2D(fillRule);
        canvasPattern->Fill(path);
    }
}

void CanvasRendererModelNG::SetStrokeRuleForPath2D(const BaseInfo& baseInfo, const CanvasFillRule& fillRule,
    const RefPtr<CanvasPath2D>& path)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetFillRuleForPath(fillRule);
        offscreenPattern->Stroke(path);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateFillRuleForPath2D(fillRule);
        canvasPattern->Stroke(path);
    }
}

void CanvasRendererModelNG::SetStrokeRuleForPath(const BaseInfo& baseInfo, const CanvasFillRule& fillRule)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetFillRuleForPath(fillRule);
        offscreenPattern->Stroke();
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateFillRuleForPath(fillRule);
        canvasPattern->Stroke();
    }
}

void CanvasRendererModelNG::SetClipRuleForPath(const BaseInfo& baseInfo, const CanvasFillRule& fillRule)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetFillRuleForPath(fillRule);
        offscreenPattern->Clip();
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateFillRuleForPath(fillRule);
        canvasPattern->Clip();
    }
}

void CanvasRendererModelNG::SetClipRuleForPath2D(const BaseInfo& baseInfo, const CanvasFillRule& fillRule,
    const RefPtr<CanvasPath2D>& path)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetFillRuleForPath2D(fillRule);
        offscreenPattern->Clip(path);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateFillRuleForPath2D(fillRule);
        canvasPattern->Clip(path);
    }
}

void CanvasRendererModelNG::AddRect(const BaseInfo& baseInfo, const Rect& rect)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->AddRect(rect);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->AddRect(rect);
    }
}

void CanvasRendererModelNG::BeginPath(const BaseInfo& baseInfo)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->BeginPath();
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->BeginPath();
    }
}

void CanvasRendererModelNG::ClosePath(const BaseInfo& baseInfo)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->ClosePath();
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->ClosePath();
    }
}

void CanvasRendererModelNG::Restore(const BaseInfo& baseInfo)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->Restore();
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->Restore();
    }
}

void CanvasRendererModelNG::CanvasRendererSave(const BaseInfo& baseInfo)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->Save();
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->Save();
    }
}

void CanvasRendererModelNG::CanvasRendererRotate(const BaseInfo& baseInfo, double angle)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->Rotate(angle);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->Rotate(angle);
    }
}

void CanvasRendererModelNG::CanvasRendererScale(const BaseInfo& baseInfo, double x, double y)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->Scale(x, y);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->Scale(x, y);
    }
}

void CanvasRendererModelNG::SetTransform(const BaseInfo& baseInfo, TransformParam& param, bool lengthFlag)
{
    auto temp = param.skewX;
    param.skewX = param.skewY;
    param.skewY = temp;
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetTransform(param);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->SetTransform(param);
    }
}

void CanvasRendererModelNG::ResetTransform(const BaseInfo& baseInfo)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->ResetTransform();
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->ResetTransform();
    }
}

void CanvasRendererModelNG::Transform(const BaseInfo& baseInfo, const TransformParam& param)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->Transform(param);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->Transform(param);
    }
}

void CanvasRendererModelNG::Translate(const BaseInfo& baseInfo, double x, double y)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->Translate(x, y);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->Translate(x, y);
    }
}

void CanvasRendererModelNG::SetLineDash(const BaseInfo& baseInfo, const std::vector<double>& lineDash)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetLineDash(lineDash);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateLineDash(lineDash);
    }
}

void CanvasRendererModelNG::SetTextAlign(const BaseInfo& baseInfo, const TextAlign& align)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetTextAlign(align);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateTextAlign(align);
    }
}

void CanvasRendererModelNG::SetTextBaseline(const BaseInfo& baseInfo, const TextBaseline& baseline)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->SetTextBaseline(baseline);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->UpdateTextBaseline(baseline);
    }
}

double CanvasRendererModelNG::GetMeasureTextWidth(const BaseInfo& baseInfo, const std::string& text)
{
    double width = 0.0;
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_RETURN(offscreenPattern, width);
        width = offscreenPattern->MeasureText(text, baseInfo.paintState);
        return width;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_RETURN(canvasPattern, width);
        width = canvasPattern->MeasureText(text, baseInfo.paintState);
    }
    return width;
}

double CanvasRendererModelNG::GetMeasureTextHeight(const BaseInfo& baseInfo, const std::string& text)
{
    double height = 0.0;
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_RETURN(offscreenPattern, height);
        height = offscreenPattern->MeasureTextHeight(text, baseInfo.paintState);
        return height;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_RETURN(canvasPattern, height);
        height = canvasPattern->MeasureTextHeight(text, baseInfo.paintState);
    }
    return height;
}

void CanvasRendererModelNG::FillRect(const BaseInfo& baseInfo, const Rect& rect)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->FillRect(rect);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->FillRect(rect);
    }
}

void CanvasRendererModelNG::StrokeRect(const BaseInfo& baseInfo, const Rect& rect)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->StrokeRect(rect);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->StrokeRect(rect);
    }
}

void CanvasRendererModelNG::ClearRect(const BaseInfo& baseInfo, const Rect& rect)
{
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_VOID(offscreenPattern);
        offscreenPattern->ClearRect(rect);
        return;
    }

    if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_VOID(canvasPattern);
        canvasPattern->ClearRect(rect);
    }
}

TransformParam CanvasRendererModelNG::GetTransform(const BaseInfo& baseInfo)
{
    TransformParam param;
    if (baseInfo.isOffscreen && baseInfo.offscreenPattern) {
        auto offscreenPattern = AceType::DynamicCast<NG::OffscreenCanvasPattern>(baseInfo.offscreenPattern);
        CHECK_NULL_RETURN(offscreenPattern, param);
        param = offscreenPattern->GetTransform();
    } else if (!baseInfo.isOffscreen && baseInfo.canvasPattern) {
        auto canvasPattern = AceType::DynamicCast<NG::CustomPaintPattern>(baseInfo.canvasPattern);
        CHECK_NULL_RETURN(canvasPattern, param);
        param = canvasPattern->GetTransform();
    }
    return param;
}
} // namespace OHOS::Ace::NG