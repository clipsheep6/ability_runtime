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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TABS_TAB_BAR_LAYOUT_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TABS_TAB_BAR_LAYOUT_PROPERTY_H

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/utils/macros.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/property/property.h"

namespace OHOS::Ace::NG {

struct TabBarProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TabBarMode, TabBarMode);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TabBarWidth, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TabBarHeight, Dimension);
};

class ACE_EXPORT TabBarLayoutProperty : public LayoutProperty {
    DECLARE_ACE_TYPE(TabBarLayoutProperty, LayoutProperty);

public:
    TabBarLayoutProperty() = default;
    ~TabBarLayoutProperty() override = default;

    RefPtr<LayoutProperty> Clone() const override
    {
        auto value = MakeRefPtr<TabBarLayoutProperty>();
        value->LayoutProperty::UpdateLayoutProperty(AceType::DynamicCast<LayoutProperty>(this));
        value->propTabBarProperty_ = CloneTabBarProperty();
        value->propAxis_ = CloneAxis();
        value->propIndicator_ = CloneIndicator();
        value->propIsBuilder_ = CloneIsBuilder();
        return value;
    }

    void Reset() override
    {
        LayoutProperty::Reset();
        ResetTabBarProperty();
        ResetAxis();
        ResetIndicator();
        ResetIsBuilder();
    }

    RectF GetIndicatorRect(int32_t index);

    ACE_DEFINE_PROPERTY_GROUP(TabBarProperty, TabBarProperty);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(TabBarProperty, TabBarMode, TabBarMode, PROPERTY_UPDATE_LAYOUT);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(TabBarProperty, TabBarWidth, Dimension, PROPERTY_UPDATE_LAYOUT);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(TabBarProperty, TabBarHeight, Dimension, PROPERTY_UPDATE_LAYOUT);

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(Axis, Axis, PROPERTY_UPDATE_LAYOUT);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(Indicator, int32_t, PROPERTY_UPDATE_NORMAL);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(IsBuilder, bool, PROPERTY_UPDATE_LAYOUT);
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TABS_TAB_BAR_LAYOUT_PROPERTY_H
