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

#include "core/components/progress/flutter_render_loading_progress.h"

#include "include/core/SkMaskFilter.h"

#include "base/log/log.h"
#include "base/utils/utils.h"
#include "core/animation/curves.h"
#include "core/pipeline/base/flutter_render_context.h"
#include "core/pipeline/base/scoped_canvas_state.h"
#include "core/pipeline/layers/offset_layer.h"

namespace OHOS::Ace {
namespace {

constexpr double BG_START = 90.0;
constexpr double BG_END = 270.0;
constexpr float CAMERA_ROTATE_X = -23.3f;
constexpr float CAMERA_ROTATE_Y = -90.0f;
constexpr uint8_t RING_ALPHA = 200;
constexpr uint8_t RING_BG_BLUR_ALPHA = 150;

} // namespace

void FlutterRenderLoadingProgress::Paint(RenderContext& context, const Offset& offset)
{
    if (cometParams_.empty()) {
        LOGE("no comet need to paint.");
        return;
    }

    const auto renderContext = static_cast<FlutterRenderContext*>(&context);
    flutter::Canvas* canvas = renderContext->GetCanvas();
    if (canvas == nullptr) {
        LOGE("canvas is nullptr");
        return;
    }

    // Paint the Ring first when ring in front.
    if (cometParams_[0].angular > BG_START && cometParams_[0].angular < BG_END) {
        PaintComet(canvas, offset + center_ + ringOffset_);
        PaintRing(canvas, offset + center_ + ringOffset_);
    } else {
        PaintRing(canvas, offset + center_ + ringOffset_);
        PaintComet(canvas, offset + center_ + ringOffset_);
    }
}

void FlutterRenderLoadingProgress::PaintRing(flutter::Canvas* canvas, const Offset& offset)
{
    flutter::Paint ringPaint;
    flutter::PaintData paintData;
    uint8_t ringAlpha = RING_ALPHA;
    // Need to Paint Ring Blur effect when Dark Mode.
    if (progressColor_ == PROGRESS_COLOR_DARK) {
        ringPaint.paint()->setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle,
            ringBlurRadius_ * scale_));
        flutter::Paint ringBgPaint;
        flutter::PaintData paintBgData;
        ringBgPaint.paint()->setStyle(SkPaint::Style::kStroke_Style);
        ringBgPaint.paint()->setStrokeWidth(ringBgWidth_ * scale_);
        ringBgPaint.paint()->setColor(progressColor_.GetValue());
        ringBgPaint.paint()->setAlpha(RING_BG_BLUR_ALPHA * dragAlpha_ * exitAlpha_);
        ringBgPaint.paint()->setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle,
            ringBgBlurRadius_ * scale_));
        canvas->drawCircle(offset.GetX(), offset.GetY(),
            ringRadius_ * scale_ * dragScale_ * exitScale_, ringBgPaint, paintBgData);
        ringAlpha = UINT8_MAX;
    }
    ringAlpha = ringAlpha * progressColor_.GetAlpha() / UINT8_MAX;
    ringPaint.paint()->setAntiAlias(true);
    ringPaint.paint()->setStyle(SkPaint::Style::kStroke_Style);
    ringPaint.paint()->setStrokeWidth(ringWidth_ * scale_);
    ringPaint.paint()->setColor(progressColor_.GetValue());
    ringPaint.paint()->setAlpha(ringAlpha * dragAlpha_ * exitAlpha_);
    canvas->drawCircle(offset.GetX(), offset.GetY(),
        ringRadius_ * scale_ * dragScale_ * exitScale_, ringPaint, paintData);
}

void FlutterRenderLoadingProgress::PaintComet(flutter::Canvas* canvas, const Offset& offset)
{
    // Init the Camera Degree and Location when first paint.
    if (!camera_) {
        camera_ = new Sk3DView();
        camera_->rotateY(CAMERA_ROTATE_Y);
        camera_->rotateX(CAMERA_ROTATE_X);
        camera_->save();
        camera_->setCameraLocation(camera_->getCameraLocationX(), 1.0f, camera_->getCameraLocationZ());
    }

    // Scale changed will effect the locationOffsetY_.
    if (!NearEqual(usedScale_, scale_)) {
        usedScale_ = scale_;
        cometCenter_.SetX(orbitRadius_);
        Offset locationOffset;
        RotateCalculate(cometCenter_ * usedScale_, locationOffset, 0.0f);
        locationOffsetY_ = locationOffset.GetY();
    }

    // Paint Comet Head and Tail.
    Offset rotatePos;
    SkPaint tailPaint;
    tailPaint.setAntiAlias(true);
    tailPaint.setStyle(SkPaint::Style::kFill_Style);
    tailPaint.setColor(progressColor_.GetValue());
    double colorAlpha = 1.0 * progressColor_.GetAlpha() / UINT8_MAX;
    for (CometParam para : cometParams_) {
        if (para.alpha == 0) {
            return;
        }
        tailPaint.setAlpha(para.alpha * exitAlpha_ * colorAlpha);
        RotateCalculate(cometCenter_ * scale_, rotatePos, para.angular);
        canvas->canvas()->drawCircle(offset.GetX() + rotatePos.GetX(),
            offset.GetY() + rotatePos.GetY() - locationOffsetY_,
            cometRadius_ * para.scale * scale_ * exitScale_, tailPaint);
    }
}

RenderLayer FlutterRenderLoadingProgress::GetRenderLayer()
{
    if (!layer_) {
        layer_ = AceType::MakeRefPtr<Flutter::OffsetLayer>();
    }
    return AceType::RawPtr(layer_);
}

// Convert 2D position to 3D position according to the angle of rotation.
void FlutterRenderLoadingProgress::RotateCalculate(const Offset& src, Offset& dst, float degrees)
{
    camera_->rotateY(degrees);
    SkPoint point;
    point.set(src.GetX(), src.GetY());
    SkMatrix matrix;
    camera_->getMatrix(&matrix);
    matrix.mapPoints(&point, &point, 1);
    dst.SetX(point.fX);
    dst.SetY(point.fY);
    camera_->restore();
    camera_->save();
}

} // namespace OHOS::Ace
