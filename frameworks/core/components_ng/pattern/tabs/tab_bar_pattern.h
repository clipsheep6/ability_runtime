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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TAB_BAT_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TAB_BAT_PATTERN_H

#include <optional>

#include "base/geometry/axis.h"
#include "base/memory/referenced.h"
#include "core/components/common/layout/constants.h"
#include "core/components/swiper/swiper_controller.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/tabs/tab_bar_layout_algorithm.h"
#include "core/components_ng/pattern/tabs/tab_bar_layout_property.h"

namespace OHOS::Ace::NG {

class TabBarPattern : public Pattern {
    DECLARE_ACE_TYPE(TabBarPattern, Pattern);

public:
    explicit TabBarPattern(const RefPtr<SwiperController>& swiperController) : swiperController_(swiperController) {};
    ~TabBarPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<TabBarLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<TabBarLayoutAlgorithm>();
    }

private:
    void OnModifyDone() override;
    void OnAttachToFrameNode() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, bool skipMeasure, bool skipLayout) override;

    void HandleClick(const GestureEvent& info) const;

    RefPtr<SwiperController> swiperController_;
    std::vector<OffsetF> tabItemOffsets_;
    OffsetF scrollableOffset_;
    bool isRTL_ = false; // TODO Adapt RTL.
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TAB_BAT_PATTERN_H