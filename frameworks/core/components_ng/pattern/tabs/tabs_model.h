/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TABS_TABS_MODEL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TABS_TABS_MODEL_H

#include <mutex>

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/image/pixel_map.h"
#include "base/memory/referenced.h"
#include "core/components/common/layout/constants.h"
#include "core/components/swiper/swiper_controller.h"
#include "core/components/tab_bar/tab_controller.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components/tab_bar/tabs_event.h"
#include "core/event/ace_events.h"
#include "core/pipeline/pipeline_context.h"

namespace OHOS::Ace {

struct TabsItemDivider final {
    Dimension strokeWidth = 0.0_vp;
    Dimension startMargin = 0.0_vp;
    Dimension endMargin = 0.0_vp;
    Color color = Color::BLACK;
    bool isNull = false;
    TabsItemDivider()
    {
        auto pipelineContext = PipelineContext::GetCurrentContext();
        if (!pipelineContext) {
            return;
        }
        auto tabTheme = pipelineContext->GetTheme<TabTheme>();
        if (!tabTheme) {
            return;
        }
        color = tabTheme->GetDividerColor();
    }
    bool operator==(const TabsItemDivider& itemDivider) const
    {
        return (strokeWidth == itemDivider.strokeWidth) && (startMargin == itemDivider.startMargin) &&
               (endMargin == itemDivider.endMargin) && (color == itemDivider.color) && (isNull == itemDivider.isNull);
    }
};

class TabsModel {
public:
    static TabsModel* GetInstance();
    virtual ~TabsModel() = default;

    virtual void Create(BarPosition barPosition, int32_t index, const RefPtr<TabController>& tabController,
        const RefPtr<SwiperController>& swiperController) = 0;
    virtual void Pop() = 0;
    virtual void SetIndex(int32_t index) = 0;
    virtual void SetTabBarPosition(BarPosition tabBarPosition) = 0;
    virtual void SetTabBarMode(TabBarMode tabBarMode) = 0;
    virtual void SetTabBarWidth(const Dimension& tabBarWidth) = 0;
    virtual void SetTabBarHeight(const Dimension& tabBarHeight) = 0;
    virtual void SetIsVertical(bool isVertical) = 0;
    virtual void SetScrollable(bool scrollable) = 0;
    virtual void SetAnimationDuration(float duration) = 0;
    virtual void SetOnChange(std::function<void(const BaseEventInfo*)>&& onChange) = 0;
    virtual void SetDivider(const TabsItemDivider& divider) = 0;
    virtual void SetFadingEdge(bool fadingEdge) = 0;
    virtual void SetBarOverlap(bool barOverlap) = 0;
    virtual void SetOnChangeEvent(std::function<void(const BaseEventInfo*)>&& onChangeEvent) = 0;
    virtual void SetBarBackgroundColor(const Color& backgroundColor) = 0;
    virtual void SetClipEdge(bool clipEdge) = 0;

private:
    static std::unique_ptr<TabsModel> instance_;
    static std::mutex mutex_;
};

} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TABS_TABS_MODEL_H
