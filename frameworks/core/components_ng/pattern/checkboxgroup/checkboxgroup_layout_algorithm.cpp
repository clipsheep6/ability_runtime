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

#include "core/components_ng/pattern/checkboxgroup/checkboxgroup_layout_algorithm.h"

#include <algorithm>

#include "base/geometry/ng/size_t.h"
#include "base/utils/utils.h"
#include "core/components/checkable/checkable_theme.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {

std::optional<SizeF> CheckBoxGroupLayoutAlgorithm::MeasureContent(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* /*layoutWrapper*/)
{
    InitializeParam();
    // Case 1: Width and height are set in the front end.
    if (contentConstraint.selfIdealSize.Width().has_value() && contentConstraint.selfIdealSize.Height().has_value() &&
        contentConstraint.selfIdealSize.IsNonNegative()) {
        auto height = contentConstraint.selfIdealSize.Height().value();
        auto width = contentConstraint.selfIdealSize.Width().value();
        auto length = std::min(width, height);
        return SizeF(length, length);
    }
    // Case 2: The front end only sets either width or height
    // 2.1 Width is set to a value
    if (contentConstraint.selfIdealSize.Width().has_value() &&
        NonNegative(contentConstraint.selfIdealSize.Width().value())) {
        auto width = contentConstraint.selfIdealSize.Width().value();
        return SizeF(width, width);
    }
    // 2.2 Height is set to a value
    if (contentConstraint.selfIdealSize.Height().has_value() &&
        NonNegative(contentConstraint.selfIdealSize.Height().value())) {
        auto height = contentConstraint.selfIdealSize.Height().value();
        return SizeF(height, height);
    }
    return SizeF();
}

void CheckBoxGroupLayoutAlgorithm::InitializeParam()
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto checkBoxTheme = pipeline->GetTheme<CheckboxTheme>();
    CHECK_NULL_VOID(checkBoxTheme);
    defaultWidth_ = checkBoxTheme->GetDefaultWidth().ConvertToPx();
    defaultHeight_ = checkBoxTheme->GetDefaultHeight().ConvertToPx();
    horizontalPadding_ = checkBoxTheme->GetHotZoneHorizontalPadding().ConvertToPx();
    verticalPadding_ = checkBoxTheme->GetHotZoneVerticalPadding().ConvertToPx();
}

} // namespace OHOS::Ace::NG
