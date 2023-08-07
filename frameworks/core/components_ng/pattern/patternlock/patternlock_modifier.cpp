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

#include "core/components_ng/pattern/patternlock/patternlock_modifier.h"

#include <algorithm>
#include <string>
#include <vector>

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/log/log_wrapper.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/base/modifier.h"
#include "core/components_ng/pattern/patternlock/patternlock_paint_property.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t PATTERN_LOCK_COL_COUNT = 3;
constexpr int32_t PATTERN_LOCK_POINT_COUNT = 9;
constexpr int32_t RADIUS_TO_DIAMETER = 2;
constexpr float FLASH_POINT_OPACITY = 0.5f;
constexpr int32_t ACTIVE_RADIUS_ANIMATION_DURATION = 200;
constexpr int32_t LIGHT_RING_RADIUS_ANIMATION_DURATION = 500;
constexpr int32_t LIGHT_RING_ALPHAF_ANIMATION_DURATION_FIRST = 200;
constexpr int32_t LIGHT_RING_ALPHAF_ANIMATION_DURATION_SECOND = 300;
constexpr int32_t CONNECT_ANIMATION_DURATION_FIRST = 100;
constexpr int32_t WRONG_ANIMATION_DURATION_DIMMING = 150;
constexpr int32_t WRONG_ANIMATION_DURATION_BRIGHTENING = 200;
constexpr int32_t WRONG_ANIMATION_DURATION_FLASH_ONCE =
    WRONG_ANIMATION_DURATION_DIMMING + WRONG_ANIMATION_DURATION_BRIGHTENING;
constexpr int32_t WRONG_ANIMATION_DURATION_FLASH_TWICE =
    WRONG_ANIMATION_DURATION_FLASH_ONCE + WRONG_ANIMATION_DURATION_FLASH_ONCE;
constexpr float BACKGROUND_RADIUS_SPRING_RESPONSE = 0.347f;
constexpr float BACKGROUND_RADIUS_SPRING_DAMPING = 0.55f;
constexpr Dimension LIGHT_RING_LINE_WIDTH = 2.5_vp;
constexpr Dimension LIGHT_RING_MASK_RADIUS = 10.0_vp;
constexpr float LIGHT_RING_ALPHAF_START = 0.0f;
constexpr float LIGHT_RING_ALPHAF_END = 0.5f;
constexpr float CONNECTED_LINE_SPRING_RESPONSE = 0.22f;
constexpr float CONNECTED_LINE_SPRING_DAMPING = 0.88f;
constexpr float CANCELED_LINE_SPRING_RESPONSE = 0.22f;
constexpr float CANCELED_LINE_SPRING_DAMPING = 0.88f;
constexpr int32_t ANIMATABLE_POINT_COUNT = 2;
} // namespace

PatternLockCell::PatternLockCell(int32_t column, int32_t row)
{
    column_ = column;
    row_ = row;
    code_ = PATTERN_LOCK_COL_COUNT * (row - 1) + (column - 1);
}

void PatternLockModifier::CreateProperties()
{
    sideLength_ = AceType::MakeRefPtr<PropertyFloat>(0.0f);
    circleRadius_ = AceType::MakeRefPtr<PropertyFloat>(0.0f);
    regularColor_ = AceType::MakeRefPtr<PropertyColor>(Color::BLACK);
    selectedColor_ = AceType::MakeRefPtr<PropertyColor>(Color::BLACK);
    activeColor_ = AceType::MakeRefPtr<PropertyColor>(Color::BLACK);
    hoverColor_ = AceType::MakeRefPtr<PropertyColor>(Color::BLACK);
    wrongColor_ = AceType::MakeRefPtr<PropertyColor>(Color::RED);
    correctColor_ = AceType::MakeRefPtr<PropertyColor>(Color::BLUE);
    pressColor_ = AceType::MakeRefPtr<PropertyColor>(Color::BLACK);
    pathColor_ = AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor(Color::BLUE));
    pointAnimateColor_ = AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor(Color::BLACK));
    pathStrokeWidth_ = AceType::MakeRefPtr<PropertyFloat>(0.0f);
    offset_ = AceType::MakeRefPtr<PropertyOffsetF>(OffsetF());
    cellCenter_ = AceType::MakeRefPtr<PropertyOffsetF>(OffsetF());
    isMoveEventValid_ = AceType::MakeRefPtr<PropertyBool>(false);
    isHover_ = AceType::MakeRefPtr<PropertyBool>(false);
    hoverIndex_ = AceType::MakeRefPtr<PropertyInt>(-1);
    connectedLineTailPoint_ = AceType::MakeRefPtr<AnimatablePropertyOffsetF>(OffsetF());
    canceledLineTailPoint_ = AceType::MakeRefPtr<AnimatablePropertyOffsetF>(OffsetF());
}

void PatternLockModifier::AttachProperties()
{
    AttachProperty(sideLength_);
    AttachProperty(circleRadius_);
    AttachProperty(regularColor_);
    AttachProperty(selectedColor_);
    AttachProperty(activeColor_);
    AttachProperty(hoverColor_);
    AttachProperty(wrongColor_);
    AttachProperty(correctColor_);
    AttachProperty(pressColor_);
    AttachProperty(pathColor_);
    AttachProperty(pointAnimateColor_);
    AttachProperty(pathStrokeWidth_);
    AttachProperty(cellCenter_);
    AttachProperty(offset_);
    AttachProperty(isMoveEventValid_);
    AttachProperty(isHover_);
    AttachProperty(hoverIndex_);
    AttachProperty(connectedLineTailPoint_);
    AttachProperty(canceledLineTailPoint_);
}

PatternLockModifier::PatternLockModifier()
{
    CreateProperties();
    AttachProperties();
    for (size_t count = 0; count < PATTERN_LOCK_POINT_COUNT; count++) {
        auto backgroundRadius = AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f);
        AttachProperty(backgroundRadius);
        backgroundCircleRadius_.emplace_back(backgroundRadius);

        auto activeRadius = AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f);
        AttachProperty(activeRadius);
        activeCircleRadius_.emplace_back(activeRadius);

        auto lightRingRadius = AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f);
        AttachProperty(lightRingRadius);
        lightRingRadius_.emplace_back(lightRingRadius);

        auto lightRingAlphaF = AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f);
        AttachProperty(lightRingAlphaF);
        lightRingAlphaF_.emplace_back(lightRingAlphaF);
    }
}

void PatternLockModifier::onDraw(DrawingContext& context)
{
    auto& canvas = context.canvas;
    PaintLockLine(canvas, offset_->Get());
    canvas.Save();
    for (int i = 0; i < PATTERN_LOCK_COL_COUNT; i++) {
        for (int j = 0; j < PATTERN_LOCK_COL_COUNT; j++) {
            PaintLockCircle(canvas, offset_->Get(), i + 1, j + 1);
        }
    }
    canvas.Restore();
}

void PatternLockModifier::PaintLockLine(RSCanvas& canvas, const OffsetF& offset)
{
    size_t count = choosePoint_.size();
    if (count < 1) {
        return;
    }

    float sideLength = sideLength_->Get();
    float pathStrokeWidth = pathStrokeWidth_->Get();
    if (LessOrEqual(pathStrokeWidth, 0.0)) {
        return;
    }
    float handleStrokeWidth = std::min(pathStrokeWidth, sideLength / PATTERN_LOCK_COL_COUNT);
    pathStrokeWidth = std::max(handleStrokeWidth, 0.0f);

    auto pathColor = pathColor_->Get();
    RSPen pen;
    pen.SetAntiAlias(true);
    pen.SetWidth(pathStrokeWidth);
    pen.SetCapStyle(RSPen::CapStyle::ROUND_CAP);

    pen.SetColor(pathColor.GetValue());
    canvas.Save();
    canvas.AttachPen(pen);
    if (count > ANIMATABLE_POINT_COUNT) {
        for (size_t i = 0; i < count - ANIMATABLE_POINT_COUNT; i++) {
            OffsetF pointBegin = GetCircleCenterByXY(offset, choosePoint_[i].GetColumn(), choosePoint_[i].GetRow());
            OffsetF pointEnd =
                GetCircleCenterByXY(offset, choosePoint_[i + 1].GetColumn(), choosePoint_[i + 1].GetRow());
            canvas.DrawLine(RSPoint(pointBegin.GetX(), pointBegin.GetY()), RSPoint(pointEnd.GetX(), pointEnd.GetY()));
        }
    }
    PaintConnectedLineTail(canvas, offset);
    if (isMoveEventValid_->Get()) {
        OffsetF pointBegin =
            GetCircleCenterByXY(offset, choosePoint_[count - 1].GetColumn(), choosePoint_[count - 1].GetRow());
        OffsetF pointEnd = GetPointEndByCellCenter();
        if (pointEnd != pointBegin) {
            canvas.DrawLine(RSPoint(pointBegin.GetX(), pointBegin.GetY()), RSPoint(pointEnd.GetX(), pointEnd.GetY()));
        }
    }
    PaintCanceledLineTail(canvas, offset);
    canvas.DetachPen();
    canvas.Restore();
}

void PatternLockModifier::PaintConnectedLineTail(RSCanvas& canvas, const OffsetF& offset)
{
    size_t count = choosePoint_.size();
    if (count < ANIMATABLE_POINT_COUNT) {
        return;
    }
    OffsetF pointBegin = GetCircleCenterByXY(offset, choosePoint_[count - ANIMATABLE_POINT_COUNT].GetColumn(),
        choosePoint_[count - ANIMATABLE_POINT_COUNT].GetRow());
    OffsetF pointEnd = GetConnectedLineTailPoint();
    if (pointEnd != pointBegin) {
        canvas.DrawLine(RSPoint(pointBegin.GetX(), pointBegin.GetY()), RSPoint(pointEnd.GetX(), pointEnd.GetY()));
    }
}

void PatternLockModifier::PaintCanceledLineTail(RSCanvas& canvas, const OffsetF& offset)
{
    if (!needCanceledLine_) {
        return;
    }
    size_t count = choosePoint_.size();
    OffsetF pointBegin =
        GetCircleCenterByXY(offset, choosePoint_[count - 1].GetColumn(), choosePoint_[count - 1].GetRow());
    OffsetF pointEnd = GetCanceledLineTailPoint();
    if (pointEnd != pointBegin) {
        canvas.DrawLine(RSPoint(pointBegin.GetX(), pointBegin.GetY()), RSPoint(pointEnd.GetX(), pointEnd.GetY()));
    }
}

void PatternLockModifier::PaintLockCircle(RSCanvas& canvas, const OffsetF& offset, int32_t x, int32_t y)
{
    auto activeColor = activeColor_->Get();
    auto regularColor = regularColor_->Get();
    auto selectedColor = selectedColor_->Get();
    auto sideLength = sideLength_->Get();
    auto circleRadius = circleRadius_->Get();
    auto pointAnimateColor = pointAnimateColor_->Get();
    auto pathColor = pathColor_->Get();
    auto pressColor = pressColor_->Get();

    OffsetF cellcenter = GetCircleCenterByXY(offset, x, y);
    float offsetX = cellcenter.GetX();
    float offsetY = cellcenter.GetY();
    const int32_t radiusCount = RADIUS_TO_DIAMETER * PATTERN_LOCK_COL_COUNT;
    float handleCircleRadius = std::min(circleRadius, sideLength / scaleActiveCircleRadius_ / radiusCount);
    circleRadius = std::max(handleCircleRadius, 0.0f);

    auto index = (x - 1) * PATTERN_LOCK_COL_COUNT + y - 1;
    if (CheckChoosePoint(x, y)) {
        PaintCircle(canvas, offsetX, offsetY, GetBackgroundCircleRadius(index), ToRSColor(pathColor));
        PaintLightRing(canvas, offsetX, offsetY, GetLightRingCircleRadius(index), GetLightRingAlphaF(index));
        const int32_t lastIndexFir = 1;
        CheckIsHoverAndPaint(canvas, offsetX, offsetY, GetActiveCircleRadius(index), index);
        if (isMoveEventValid_->Get() && CheckChoosePointIsLastIndex(x, y, lastIndexFir)) {
            PaintCircle(canvas, offsetX, offsetY, circleRadius * pressRadiusScale_, ToRSColor(pressColor));
            PaintCircle(canvas, offsetX, offsetY, GetActiveCircleRadius(index), ToRSColor(activeColor));
        } else {
            if (challengeResult_.has_value()) {
                PaintCircle(canvas, offsetX, offsetY, GetActiveCircleRadius(index), ToRSColor(pointAnimateColor));
            } else {
                PaintCircle(canvas, offsetX, offsetY, GetActiveCircleRadius(index), ToRSColor(selectedColor));
            }
        }
    } else {
        CheckIsHoverAndPaint(canvas, offsetX, offsetY, circleRadius, index);
        PaintCircle(canvas, offsetX, offsetY, circleRadius, ToRSColor(regularColor));
    }
}

void PatternLockModifier::CheckIsHoverAndPaint(
    RSCanvas& canvas, float offsetX, float offsetY, float radius, int32_t index)
{
    if (isHover_->Get() && hoverIndex_->Get() == index) {
        PaintCircle(canvas, offsetX, offsetY, radius * hoverRadiusScale_, ToRSColor(hoverColor_->Get()));
    }
}

void PatternLockModifier::PaintCircle(
    RSCanvas& canvas, float offsetX, float offsetY, float radius, const RSColor& circleColor)
{
    RSBrush brush;
    brush.SetAntiAlias(true);
    brush.SetColor(circleColor);
    canvas.AttachBrush(brush);
    canvas.DrawCircle(RSPoint(offsetX, offsetY), radius);
    canvas.DetachBrush();
}

void PatternLockModifier::PaintLightRing(RSCanvas& canvas, float offsetX, float offsetY, float radius, float alphaF)
{
    if (NearZero(alphaF)) {
        return;
    }
    RSPen pen;
    pen.SetWidth(LIGHT_RING_LINE_WIDTH.ConvertToPx());
    pen.SetAntiAlias(true);
    pen.SetColor(ToRSColor(activeColor_->Get()));
    pen.SetAlphaF(alphaF);
    RSFilter filter;
    filter.SetMaskFilter(RSMaskFilter::CreateBlurMaskFilter(
        RSBlurType::NORMAL, RSDrawing::ConvertRadiusToSigma(LIGHT_RING_MASK_RADIUS.ConvertToPx())));
    pen.SetFilter(filter);

    canvas.AttachPen(pen);
    RSPath path;
    path.AddCircle(offsetX, offsetY, radius);
    canvas.DrawPath(path);
    canvas.DetachPen();
}

bool PatternLockModifier::CheckChoosePoint(int32_t x, int32_t y) const
{
    for (auto it : choosePoint_) {
        if (it.GetColumn() == x && it.GetRow() == y) {
            return true;
        }
    }
    return false;
}

bool PatternLockModifier::CheckChoosePointIsLastIndex(int32_t x, int32_t y, int32_t index) const
{
    if (!choosePoint_.empty() && static_cast<int32_t>(choosePoint_.size()) >= index) {
        if (choosePoint_.at(choosePoint_.size() - static_cast<uint32_t>(index)).GetColumn() == x &&
            choosePoint_.at(choosePoint_.size() - static_cast<uint32_t>(index)).GetRow() == y) {
            return true;
        }
    }
    return false;
}

OffsetF PatternLockModifier::GetCircleCenterByXY(const OffsetF& offset, int32_t x, int32_t y)
{
    float sideLength = sideLength_->Get();
    OffsetF cellCenter;
    int32_t scale = RADIUS_TO_DIAMETER;
    cellCenter.SetX(offset.GetX() + sideLength / PATTERN_LOCK_COL_COUNT / scale * (x * scale - 1));
    cellCenter.SetY(offset.GetY() + sideLength / PATTERN_LOCK_COL_COUNT / scale * (y * scale - 1));
    return cellCenter;
}

void PatternLockModifier::SetSideLength(float sideLength)
{
    CHECK_NULL_VOID(sideLength_);
    if (!NearEqual(sideLength_->Get(), sideLength)) {
        sideLength_->Set(sideLength);
        size_t count = choosePoint_.size();
        if (count > 0) {
            OffsetF lastPoint = GetCircleCenterByXY(
                offset_->Get(), choosePoint_[count - 1].GetColumn(), choosePoint_[count - 1].GetRow());
            connectedLineTailPoint_->Set(lastPoint);
            canceledLineTailPoint_->Set(lastPoint);
        }
    }
}

void PatternLockModifier::SetCircleRadius(float circleRadius)
{
    CHECK_NULL_VOID(circleRadius_);
    if (!NearEqual(circleRadius_->Get(), circleRadius)) {
        circleRadius_->Set(circleRadius);
        for (const auto& cell : choosePoint_) {
            auto index = (cell.GetColumn() - 1) * PATTERN_LOCK_COL_COUNT + cell.GetRow() - 1;
            if (index < PATTERN_LOCK_POINT_COUNT && index >= 0) {
                backgroundCircleRadius_.at(index)->Set(circleRadius * scaleBackgroundCircleRadius_);
                activeCircleRadius_.at(index)->Set(circleRadius * scaleActiveCircleRadius_);
                lightRingRadius_.at(index)->Set(circleRadius * scaleLightRingRadiusStart_);
            }
        }
    }
}

void PatternLockModifier::SetRegularColor(const Color& regularColor)
{
    CHECK_NULL_VOID(regularColor_);
    regularColor_->Set(regularColor);
}

void PatternLockModifier::SetSelectColor(const Color& selectedColor)
{
    CHECK_NULL_VOID(selectedColor_);
    if (selectedColor_->Get() != selectedColor) {
        selectedColor_->Set(selectedColor);
    }
}

void PatternLockModifier::SetActiveColor(const Color& activeColor)
{
    CHECK_NULL_VOID(activeColor_);
    activeColor_->Set(activeColor);
}

void PatternLockModifier::SetPathColor(const LinearColor& pathColor)
{
    CHECK_NULL_VOID(pathColor_);
    pathColor_->Set(pathColor);
}

void PatternLockModifier::SetHoverColor(const Color& hoverColor)
{
    CHECK_NULL_VOID(hoverColor_);
    if (hoverColor_->Get() != hoverColor) {
        hoverColor_->Set(hoverColor);
    }
}

void PatternLockModifier::SetWrongColor(const Color& wrongColor)
{
    CHECK_NULL_VOID(wrongColor_);
    if (wrongColor_->Get() != wrongColor) {
        wrongColor_->Set(wrongColor);
    }
}

void PatternLockModifier::SetCorrectColor(const Color& correctColor)
{
    CHECK_NULL_VOID(correctColor_);
    correctColor_->Set(correctColor);
}

void PatternLockModifier::SetPressColor(const Color& pressColor)
{
    CHECK_NULL_VOID(pressColor_);
    if (pressColor_->Get() != pressColor) {
        pressColor_->Set(pressColor);
    }
}

void PatternLockModifier::SetPathStrokeWidth(float pathStrokeWidth)
{
    CHECK_NULL_VOID(pathStrokeWidth_);
    pathStrokeWidth_->Set(pathStrokeWidth);
}

void PatternLockModifier::SetContentOffset(const OffsetF& offset)
{
    CHECK_NULL_VOID(offset_);
    offset_->Set(offset);
}

void PatternLockModifier::SetIsMoveEventValid(bool isMoveEventValid)
{
    isMoveEventValid_->Set(isMoveEventValid);
}

void PatternLockModifier::SetIsHover(bool isHover)
{
    isHover_->Set(isHover);
}

void PatternLockModifier::SetHoverIndex(int32_t hoverIndex)
{
    hoverIndex_->Set(hoverIndex);
}

void PatternLockModifier::StartChallengeResultAnimate()
{
    if (!challengeResult_.has_value()) {
        return;
    }
    if (challengeResult_.value() == NG::PatternLockChallengeResult::CORRECT) {
        pointAnimateColor_->Set(LinearColor(selectedColor_->Get()));
        AnimationOption option = AnimationOption();
        option.SetDuration(CONNECT_ANIMATION_DURATION_FIRST);
        option.SetCurve(Curves::SHARP);
        AnimationUtils::Animate(option, [&]() { pointAnimateColor_->Set(LinearColor(correctColor_->Get())); });
    } else if (challengeResult_.value() == NG::PatternLockChallengeResult::WRONG) {
        pointAnimateColor_->Set(LinearColor(wrongColor_->Get()));
        auto pathColor = pathColor_->Get();
        AnimationOption option = AnimationOption();
        option.SetDuration(WRONG_ANIMATION_DURATION_FLASH_TWICE);
        option.SetCurve(Curves::SHARP);
        auto dimmingAnimation = [weak = WeakClaim(this)]() {
            auto modifier = weak.Upgrade();
            CHECK_NULL_VOID(modifier);
            modifier->pointAnimateColor_->Set(
                LinearColor(modifier->wrongColor_->Get().BlendOpacity(FLASH_POINT_OPACITY)));
            modifier->SetPathColor(LinearColor(modifier->pathColor_->Get().BlendOpacity(FLASH_POINT_OPACITY)));
        };
        auto brighteningAnimation = [weak = WeakClaim(this), pathColor]() {
            auto modifier = weak.Upgrade();
            CHECK_NULL_VOID(modifier);
            modifier->pointAnimateColor_->Set(LinearColor(modifier->wrongColor_->Get()));
            modifier->SetPathColor(pathColor);
        };
        AnimationUtils::OpenImplicitAnimation(option, Curves::SHARP, nullptr);
        AnimationUtils::AddKeyFrame(((float)WRONG_ANIMATION_DURATION_DIMMING / WRONG_ANIMATION_DURATION_FLASH_TWICE),
            Curves::SHARP, dimmingAnimation);
        AnimationUtils::AddKeyFrame(((float)WRONG_ANIMATION_DURATION_FLASH_ONCE / WRONG_ANIMATION_DURATION_FLASH_TWICE),
            Curves::SHARP, brighteningAnimation);
        AnimationUtils::AddKeyFrame(((float)(WRONG_ANIMATION_DURATION_FLASH_ONCE + WRONG_ANIMATION_DURATION_DIMMING) /
                                        WRONG_ANIMATION_DURATION_FLASH_TWICE),
            Curves::SHARP, dimmingAnimation);
        AnimationUtils::AddKeyFrame(1.0f, Curves::SHARP, brighteningAnimation);
        AnimationUtils::CloseImplicitAnimation();
    }
}

void PatternLockModifier::SetChallengeResult(std::optional<NG::PatternLockChallengeResult>& challengeResult)
{
    if (challengeResult_ != challengeResult) {
        challengeResult_ = challengeResult;
        StartChallengeResultAnimate();
    }
}

void PatternLockModifier::SetCellCenterOffset(const OffsetF& cellcenter)
{
    CHECK_NULL_VOID(cellCenter_);
    cellCenter_->Set(cellcenter);
}

void PatternLockModifier::SetChoosePoint(const std::vector<PatternLockCell>& choosePoint)
{
    choosePoint_ = choosePoint;
}

void PatternLockModifier::SetActiveCircleRadiusScale(float scale)
{
    scaleActiveCircleRadius_ = scale;
}

void PatternLockModifier::SetBackgroundCircleRadiusScale(float scale)
{
    scaleBackgroundCircleRadius_ = scale;
}

void PatternLockModifier::SetLightRingRadiusStartScale(float scale)
{
    scaleLightRingRadiusStart_ = scale;
}

void PatternLockModifier::SetLightRingRadiusEndScale(float scale)
{
    scaleLightRingRadiusEnd_ = scale;
}

void PatternLockModifier::SetPressRadiusScale(float scale)
{
    pressRadiusScale_ = scale;
}

void PatternLockModifier::SetHoverRadiusScale(float scale)
{
    hoverRadiusScale_ = scale;
}

void PatternLockModifier::SetBackgroundCircleRadius(int32_t index)
{
    if (index < static_cast<int32_t>(backgroundCircleRadius_.size()) && index >= 0) {
        backgroundCircleRadius_.at(index)->Set(0.0f);
        AnimationOption option = AnimationOption();
        auto curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(
            BACKGROUND_RADIUS_SPRING_RESPONSE, BACKGROUND_RADIUS_SPRING_DAMPING);
        option.SetCurve(curve);
        AnimationUtils::Animate(option,
            [&]() { backgroundCircleRadius_.at(index)->Set(circleRadius_->Get() * scaleBackgroundCircleRadius_); });
    }
}

float PatternLockModifier::GetBackgroundCircleRadius(int32_t index) const
{
    if ((index >= PATTERN_LOCK_POINT_COUNT || index < 0)) {
        return 0;
    }
    return backgroundCircleRadius_.at(index)->Get();
}

void PatternLockModifier::SetActiveCircleRadius(int32_t index)
{
    if (index < static_cast<int32_t>(activeCircleRadius_.size()) && index >= 0) {
        activeCircleRadius_.at(index)->Set(circleRadius_->Get());
        AnimationOption option = AnimationOption();
        option.SetDuration(ACTIVE_RADIUS_ANIMATION_DURATION);
        option.SetCurve(Curves::FRICTION);
        AnimationUtils::Animate(
            option, [&]() { activeCircleRadius_.at(index)->Set(circleRadius_->Get() * scaleActiveCircleRadius_); });
    }
}

float PatternLockModifier::GetActiveCircleRadius(int32_t index) const
{
    if ((index >= PATTERN_LOCK_POINT_COUNT || index < 0)) {
        return 0;
    }
    return activeCircleRadius_.at(index)->Get();
}

void PatternLockModifier::SetLightRingCircleRadius(int32_t index)
{
    if (index < static_cast<int32_t>(lightRingRadius_.size()) && index >= 0) {
        auto circleRadius = circleRadius_->Get();
        lightRingRadius_.at(index)->Set(circleRadius * scaleLightRingRadiusStart_);
        AnimationOption option = AnimationOption();
        option.SetDuration(LIGHT_RING_RADIUS_ANIMATION_DURATION);
        option.SetCurve(Curves::LINEAR);
        AnimationUtils::Animate(
            option, [&]() { lightRingRadius_.at(index)->Set(circleRadius * scaleLightRingRadiusEnd_); });
    }
}

float PatternLockModifier::GetLightRingCircleRadius(int32_t index) const
{
    if ((index >= PATTERN_LOCK_POINT_COUNT || index < 0)) {
        return 0;
    }
    return lightRingRadius_.at(index)->Get();
}

void PatternLockModifier::SetLightRingAlphaF(int32_t index)
{
    if (index < static_cast<int32_t>(lightRingAlphaF_.size()) && index >= 0) {
        auto singleLightRingAlphaF = lightRingAlphaF_.at(index);
        singleLightRingAlphaF->Set(LIGHT_RING_ALPHAF_START);
        AnimationOption optionFirst = AnimationOption();
        optionFirst.SetDuration(LIGHT_RING_ALPHAF_ANIMATION_DURATION_FIRST);
        optionFirst.SetCurve(Curves::SHARP);
        optionFirst.SetOnFinishEvent([=] {
            AnimationOption optionSecond = AnimationOption();
            optionSecond.SetDuration(LIGHT_RING_ALPHAF_ANIMATION_DURATION_SECOND);
            optionSecond.SetCurve(Curves::SHARP);
            AnimationUtils::Animate(optionSecond, [=]() { singleLightRingAlphaF->Set(LIGHT_RING_ALPHAF_START); });
        });
        AnimationUtils::Animate(
            optionFirst, [&]() { singleLightRingAlphaF->Set(LIGHT_RING_ALPHAF_END); }, optionFirst.GetOnFinishEvent());
    }
}

float PatternLockModifier::GetLightRingAlphaF(int32_t index) const
{
    if ((index >= PATTERN_LOCK_POINT_COUNT || index < 0)) {
        return 0;
    }
    return lightRingAlphaF_.at(index)->Get();
}

void PatternLockModifier::SetConnectedLineTailPoint(int32_t x, int32_t y)
{
    size_t count = choosePoint_.size();
    if (count < 1) {
        return;
    }
    OffsetF lastPoint = GetCircleCenterByXY(offset_->Get(), x, y);
    if (isTouchDown_ && isMoveEventValid_) {
        connectedLineTailPoint_->Set(cellCenter_->Get());
        AnimationOption option = AnimationOption();
        auto curve =
            AceType::MakeRefPtr<ResponsiveSpringMotion>(CONNECTED_LINE_SPRING_RESPONSE, CONNECTED_LINE_SPRING_DAMPING);
        option.SetCurve(curve);
        AnimationUtils::Animate(option, [&]() { connectedLineTailPoint_->Set(lastPoint); });
    } else {
        connectedLineTailPoint_->Set(lastPoint);
    }
}

OffsetF PatternLockModifier::GetConnectedLineTailPoint() const
{
    return connectedLineTailPoint_->Get();
}

void PatternLockModifier::SetCanceledLineTailPoint()
{
    size_t count = choosePoint_.size();
    if (count < 1) {
        return;
    }

    canceledLineTailPoint_->Set(GetPointEndByCellCenter());
    OffsetF pointEnd =
        GetCircleCenterByXY(offset_->Get(), choosePoint_[count - 1].GetColumn(), choosePoint_[count - 1].GetRow());
    AnimationOption option = AnimationOption();
    auto curve =
        AceType::MakeRefPtr<ResponsiveSpringMotion>(CANCELED_LINE_SPRING_RESPONSE, CANCELED_LINE_SPRING_DAMPING);
    option.SetCurve(curve);
    AnimationUtils::Animate(option, [&]() { canceledLineTailPoint_->Set(pointEnd); });
}

OffsetF PatternLockModifier::GetCanceledLineTailPoint() const
{
    return canceledLineTailPoint_->Get();
}

void PatternLockModifier::StartConnectedCircleAnimate(int32_t x, int32_t y)
{
    auto index = (x - 1) * PATTERN_LOCK_COL_COUNT + y - 1;
    SetBackgroundCircleRadius(index);
    SetActiveCircleRadius(index);
    SetLightRingCircleRadius(index);
    SetLightRingAlphaF(index);
}

void PatternLockModifier::StartConnectedLineAnimate(int32_t x, int32_t y)
{
    SetConnectedLineTailPoint(x, y);
}

void PatternLockModifier::StartCanceledAnimate()
{
    SetCanceledLineTailPoint();
}

void PatternLockModifier::Reset()
{
    SetIsTouchDown(false);
    needCanceledLine_ = false;
    challengeResult_.reset();
    connectedLineTailPoint_->Set(OffsetF());
    canceledLineTailPoint_->Set(OffsetF());
    for (auto& radius : backgroundCircleRadius_) {
        radius->Set(0.0f);
    }
    for (auto& radius : activeCircleRadius_) {
        radius->Set(circleRadius_->Get());
    }
    for (auto& radius : lightRingRadius_) {
        radius->Set(circleRadius_->Get() * scaleLightRingRadiusStart_);
    }
    for (auto& alphaF : lightRingAlphaF_) {
        alphaF->Set(LIGHT_RING_ALPHAF_START);
    }
}

void PatternLockModifier::SetIsTouchDown(bool isTouchDown)
{
    if (isTouchDown_ && (!isTouchDown)) {
        needCanceledLine_ = true;
    } else if ((!isTouchDown_) && isTouchDown) {
        needCanceledLine_ = false;
    }
    isTouchDown_ = isTouchDown;
}

OffsetF PatternLockModifier::GetPointEndByCellCenter() const
{
    auto offset = offset_->Get();
    float sideLength = sideLength_->Get();
    auto cellCenter = cellCenter_->Get();
    float x = cellCenter.GetX();
    float y = cellCenter.GetY();
    x = x > offset.GetX() + sideLength ? offset.GetX() + sideLength : x;
    x = x < offset.GetX() ? offset.GetX() : x;
    y = y > offset.GetY() + sideLength ? offset.GetY() + sideLength : y;
    y = y < offset.GetY() ? offset.GetY() : y;
    return OffsetF(x, y);
}
} // namespace OHOS::Ace::NG
