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
#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SELECT_OVERLAY_MODIFIER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SELECT_OVERLAY_MODIFIER_H

#include <optional>
#include <vector>

#include "base/geometry/dimension.h"
#include "base/memory/ace_type.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/base/modifier.h"
#include "core/components_ng/render/node_paint_method.h"

namespace OHOS::Ace::NG {
class SelectOverlayModifier : public OverlayModifier {
    DECLARE_ACE_TYPE(SelectOverlayModifier, OverlayModifier)

public:
    SelectOverlayModifier(const OffsetF& menuOptionOffset);

    ~SelectOverlayModifier() override = default;

    void onDraw(DrawingContext& drawingContext) override;

    void SetOtherPointRadius(const Dimension& radius);

    void SetHeadPointRadius(const Dimension& radius);

    void SetMenuOptionOffset(const OffsetF& offset)
    {
        if (menuOptionOffset_) {
            menuOptionOffset_->Set(offset);
        }
    }

    void SetLineEndOffset(bool isMore);

    void SetHasExtensitonMenu(bool hasExtensitonMenu)
    {
        if (hasExtensitonMenu_) {
            hasExtensitonMenu_->Set(hasExtensitonMenu);
        }
    }

private:
    void SetDefaultCircleAndLineEndOffset();

    void DrawbBackArrow(DrawingContext& context);
    void DrawbCircles(DrawingContext& context);

    RefPtr<PropertyOffsetF> menuOptionOffset_;
    RefPtr<PropertyBool> hasExtensitonMenu_;
    RefPtr<AnimatablePropertyFloat> pointRadius_;
    RefPtr<AnimatablePropertyFloat> headPointRadius_;
    RefPtr<AnimatablePropertyFloat> rotationAngle_;
    std::vector<RefPtr<AnimatablePropertyOffsetF>> circleOffset_;
    std::vector<RefPtr<AnimatablePropertyOffsetF>> lineEndOffset_;

    Color iconColor_ = Color::BLACK;

    ACE_DISALLOW_COPY_AND_MOVE(SelectOverlayModifier);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SELECT_OVERLAY_MODIFIER_H