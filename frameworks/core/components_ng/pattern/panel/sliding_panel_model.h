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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_PANEL_SLIDING_PANEL_MODEL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_PANEL_SLIDING_PANEL_MODEL_H

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/memory/referenced.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/components_ng/pattern/panel/sliding_panel_event_hub.h"
#include "core/components_ng/pattern/panel/sliding_panel_node.h"

namespace OHOS::Ace {

class SlidingPanelModel {
public:
    static SlidingPanelModel* GetInstance();
    virtual ~SlidingPanelModel() = default;

    virtual void Create(bool isShow) = 0;
    virtual void SetPanelType(PanelType type) = 0;
    virtual void SetPanelMode(PanelMode mode) = 0;
    virtual void SetHasDragBar(bool hasDragBar) = 0;
    virtual void SetMiniHeight(const Dimension& miniHeight) = 0;
    virtual void SetHalfHeight(const Dimension& halfHeight) = 0;
    virtual void SetFullHeight(const Dimension& fullHeight) = 0;
    virtual void SetIsShow(bool isShow) = 0;
    virtual void SetBackgroundMask(const Color& backgroundMask) = 0;
    virtual void SetBackgroundColor(const Color& backgroundColor) = 0;
    virtual void SetOnSizeChange(std::function<void(const BaseEventInfo*)>&& changeEvent) = 0;
    virtual void SetOnHeightChange(std::function<void(const float)>&& onHeightChange) = 0;
    virtual void Pop();
    virtual void SetBorderColor(const Color& borderColor);
    virtual void SetBorderWidth(const Dimension& borderWidth);
    virtual void SetBorderStyle(const BorderStyle& borderStyle);
    virtual void SetBorder(const BorderStyle& borderStyle, const Dimension& borderWidth);

private:
    static std::unique_ptr<SlidingPanelModel> instance_;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_PANEL_SLIDING_PANEL_MODEL_H