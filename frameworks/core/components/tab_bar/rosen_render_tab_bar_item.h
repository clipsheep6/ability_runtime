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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TAB_BAR_ROSEN_RENDER_TAB_BAR_ITEM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TAB_BAR_ROSEN_RENDER_TAB_BAR_ITEM_H

#include "core/components/common/painter/debug_boundary_painter.h"
#include "core/components/tab_bar/render_tab_bar_item.h"

namespace OHOS::Ace {

class RosenRenderTabBarItem : public RenderTabBarItem {
    DECLARE_ACE_TYPE(RosenRenderTabBarItem, RenderTabBarItem);

public:
    void Paint(RenderContext& context, const Offset& offset) override;
    void RenderTabBarItemBoundary(RenderContext& context, const Offset& offset,
        double width, double height);
private:
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TAB_BAR_ROSEN_RENDER_TAB_BAR_ITEM_H
