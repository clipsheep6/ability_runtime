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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_PROGRESS_PROGRESS_PAINT_METHOD_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_PROGRESS_PROGRESS_PAINT_METHOD_H

#include <optional>

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/components/common/properties/color.h"
#include "core/components/progress/progress_theme.h"
#include "core/components_ng/pattern/progress/progress_date.h"
#include "core/components_ng/pattern/progress/progress_paint_property.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/components_ng/render/node_paint_method.h"
#include "core/components_ng/pattern/progress/progress_modifier.h"

namespace OHOS::Ace::NG {
constexpr double DEFAULT_BORDER_WIDTH = 1.0;

class ACE_EXPORT ProgressPaintMethod : public NodePaintMethod {
    DECLARE_ACE_TYPE(ProgressPaintMethod, NodePaintMethod)
public:
    explicit ProgressPaintMethod(ProgressType progressType, float strokeWidth,
        const RefPtr<ProgressModifier>& progressModifier)
        : strokeWidth_(strokeWidth), progressType_(progressType),
          progressModifier_(progressModifier)
    {
        progressModifier_->SetProgressType(progressType_);
    }
    ~ProgressPaintMethod() override = default;

    RefPtr<Modifier> GetContentModifier(PaintWrapper* paintWrapper) override
    {
        CHECK_NULL_RETURN(progressModifier_, nullptr);
        return progressModifier_;
    }

    void UpdateContentModifier(PaintWrapper* paintWrapper) override
    {
        CHECK_NULL_VOID(progressModifier_);
        GetThemeDate();
        auto paintProperty = DynamicCast<ProgressPaintProperty>(paintWrapper->GetPaintProperty());
        CHECK_NULL_VOID(paintProperty);
        color_ = paintProperty->GetColor().value_or(color_);
        bgColor_ = paintProperty->GetBackgroundColor().value_or(bgColor_);
        borderColor_ = paintProperty->GetBorderColor().value_or(borderColor_);
        maxValue_ = paintProperty->GetMaxValue().value_or(maxValue_);
        value_ = paintProperty->GetValue().value_or(value_);
        scaleCount_ = paintProperty->GetScaleCount().value_or(scaleCount_);

        progressModifier_->SetContentOffset(paintWrapper->GetContentOffset());
        progressModifier_->SetContentSize(paintWrapper->GetContentSize());
        CalculateStrokeWidth(paintWrapper->GetContentSize());
        progressModifier_->SetStrokeWidth(strokeWidth_);
        progressModifier_->SetBorderWidth(capsuleBorderWidth_);
        progressModifier_->SetColor(LinearColor(color_));
        progressModifier_->SetBackgroundColor(LinearColor(bgColor_));
        progressModifier_->SetBorderColor(LinearColor(borderColor_));
        progressModifier_->SetProgressType(progressType_);
        progressModifier_->SetMaxValue(maxValue_);
        progressModifier_->SetValue(value_);
        progressModifier_->SetScaleWidth(scaleWidth_);
        progressModifier_->SetScaleCount(scaleCount_);
    }

    void GetThemeDate();
    void CalculateStrokeWidth(const SizeF& contentSize);
private:
    Color color_ = Color::BLUE;
    Color bgColor_ = Color::GRAY;
    Color borderColor_ = Color::GRAY;
    float strokeWidth_ = 2.0f;
    float scaleWidth_ = 10.0f;
    int32_t scaleCount_ = 100;
    float maxValue_ = 100.0f;
    float value_ = 0.0f;
    Dimension capsuleBorderWidth_ = Dimension(DEFAULT_BORDER_WIDTH,  DimensionUnit::VP);

    ProgressType progressType_ = ProgressType::LINEAR;
    RefPtr<ProgressModifier> progressModifier_;

    ACE_DISALLOW_COPY_AND_MOVE(ProgressPaintMethod);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_PROGRESS_PROGRESS_PAINT_METHOD_H
