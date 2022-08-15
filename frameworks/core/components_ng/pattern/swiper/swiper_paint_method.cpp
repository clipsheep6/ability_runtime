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

#include "core/components_ng/pattern/swiper/swiper_paint_method.h"
#include "core/components_ng/pattern/swiper/swiper_paint_property.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::NG {

CanvasDrawFunction SwiperPaintMethod::GetForegroundDrawFunction(PaintWrapper* paintWrapper)
{
    auto paintFunc = [weak = WeakClaim(this), paintWrapper](const RefPtr<Canvas>& canvas) {
        auto swiper = weak.Upgrade();
        if (swiper) {
            swiper->PaintFade(canvas, paintWrapper);
        }
    };

    return paintFunc;
}

void SwiperPaintMethod::PaintFade(const RefPtr<Canvas>& canvas, PaintWrapper* paintWrapper) const
{
    if (NearZero(mainDelta_)) {
        return;
    }

    CHECK_NULL_VOID(paintWrapper);
    auto paintProperty = DynamicCast<SwiperPaintProperty>(paintWrapper->GetPaintProperty());
    CHECK_NULL_VOID(paintProperty);

    // TODO use theme.
    constexpr float FADE_MAX_DISTANCE = 2000.0f;
    constexpr float FADE_MAX_TRANSLATE = 40.0f;
    constexpr float FADE_MAX_RADIUS = 2.0f;
    constexpr float FADE_ALPHA = 0.45f;
    constexpr float FADE_SCALE_RATE = 0.2f;

    bool isVertical = (axis_ == Axis::VERTICAL);
    auto frameSize = paintWrapper->GetGeometryNode()->GetFrameSize();
    float width = frameSize.Width();
    float height = frameSize.Height();
    float centerX = 0.0;
    float centerY = 0.0;
    float fadeTranslate = mainDelta_ * FADE_SCALE_RATE;
    float radius = 0.0;
    if (GreatNotEqual(mainDelta_, 0.0)) {
        fadeTranslate = fadeTranslate > FADE_MAX_TRANSLATE ? FADE_MAX_TRANSLATE : fadeTranslate;
        if (isVertical) {
            centerY = -FADE_MAX_DISTANCE + mainDelta_ / FADE_SCALE_RATE;
            if (centerY > (-width * FADE_MAX_RADIUS)) {
                centerY = -width * FADE_MAX_RADIUS;
            }
            centerX = width / 2;
        } else {
            centerX = -FADE_MAX_DISTANCE + mainDelta_ / FADE_SCALE_RATE;
            if (centerX > (-FADE_MAX_RADIUS * height)) {
                centerX = (-FADE_MAX_RADIUS * height);
            }
            centerY = height / 2;
        }
        radius = sqrt(pow(centerX, 2) + pow(centerY, 2));
    } else {
        fadeTranslate = fadeTranslate > -FADE_MAX_TRANSLATE ? fadeTranslate : -FADE_MAX_TRANSLATE;
        if (isVertical) {
            centerY = height + FADE_MAX_DISTANCE + mainDelta_ / FADE_SCALE_RATE;
            if (centerY < (height + width * FADE_MAX_RADIUS)) {
                centerY = height + width * FADE_MAX_RADIUS;
            }
            centerX = width / 2;
            radius = sqrt(pow(centerY - height, 2) + pow(centerX, 2));
        } else {
            centerX = width + FADE_MAX_DISTANCE + mainDelta_ / FADE_SCALE_RATE;
            if (centerX < (width + FADE_MAX_RADIUS * height)) {
                centerX = width + FADE_MAX_RADIUS * height;
            }
            centerY = height / 2;
            radius = sqrt(pow(centerX - width, 2) + pow(centerY, 2));
        }
    }

    Offset center = Offset(centerX, centerY);
    RefPtr<Paint> painter = Paint::Create();
    painter->SetColor(paintProperty->GetFadeColor().value_or(Color::GRAY));
    painter->SetAlphaf(FADE_ALPHA);
    painter->SetBlendMode(BlendMode::SRC_OVER);
    if (isVertical) {
        canvas->DrawCircle(center.GetX(), center.GetY() + fadeTranslate, radius, painter);
    } else {
        canvas->DrawCircle(center.GetX() + fadeTranslate, center.GetY(), radius, painter);
    }
}

} // namespace OHOS::Ace::NG
