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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CIRCLE_PAINT_METHOD_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CIRCLE_PAINT_METHOD_H

#include "base/geometry/ng/size_t.h"
#include "core/components_ng/base/geometry_node.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_content_modifier.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_modifier.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_property.h"
#include "core/components_ng/pattern/shape/shape_paint_property.h"
#include "core/components_ng/render/circle_painter.h"
#include "core/components_ng/render/node_paint_method.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT SelectOverlayPaintMethod : public NodePaintMethod {
    DECLARE_ACE_TYPE(SelectOverlayPaintMethod, NodePaintMethod)
public:
    SelectOverlayPaintMethod(const RefPtr<SelectOverlayModifier>& selectOverlayModifier,
        const RefPtr<SelectOverlayContentModifier>& selectOverlayContentModifier, SelectOverlayInfo info,
        const OffsetF& offset, bool hasExtensionMenu, bool hasShowAnimation, bool isCreated)
        : selectOverlayModifier_(selectOverlayModifier), selectOverlayContentModifier_(selectOverlayContentModifier),
          info_(std::move(info)), defaultMenuEndOffset_(offset), hasExtensionMenu_(hasExtensionMenu),
          hasShowAnimation_(hasShowAnimation), isCreated_(isCreated)
    {}
    ~SelectOverlayPaintMethod() override = default;

    RefPtr<Modifier> GetOverlayModifier(PaintWrapper* paintWrapper) override
    {
        CHECK_NULL_RETURN(selectOverlayModifier_, nullptr);
        return selectOverlayModifier_;
    }

    RefPtr<Modifier> GetContentModifier(PaintWrapper* paintWrapper) override
    {
        CHECK_NULL_RETURN(selectOverlayContentModifier_, nullptr);
        return selectOverlayContentModifier_;
    }

    void UpdateOverlayModifier(PaintWrapper* paintWrapper) override;

    void UpdateContentModifier(PaintWrapper* paintWrapper) override;

private:
    void CheckCirclesAndBackArrowIsShown();

    void CheckHasExtensionMenu();

    void CheckHandleIsShown();

    RefPtr<SelectOverlayModifier> selectOverlayModifier_;

    RefPtr<SelectOverlayContentModifier> selectOverlayContentModifier_;

    SelectOverlayInfo info_;

    OffsetF defaultMenuEndOffset_;

    bool hasExtensionMenu_ = false;

    bool hasShowAnimation_ = false;

    bool isCreated_ = false;

    bool handleIsShown_ = false;

    bool circlesAndBackArrowIsShown_ = false;

    ACE_DISALLOW_COPY_AND_MOVE(SelectOverlayPaintMethod);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CIRCLE_PAINT_METHOD_H