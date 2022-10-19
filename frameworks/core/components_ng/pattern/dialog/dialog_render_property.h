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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_DIALOG_DIALOG_RENDER_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_DIALOG_DIALOG_RENDER_PROPERTY_H

#include "base/geometry/dimension.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/placement.h"
#include "core/components_ng/render/paint_property.h"

namespace OHOS::Ace::NG {

// PaintProperty are used to set render properties.
class DialogRenderProperty : public PaintProperty {
    DECLARE_ACE_TYPE(DialogRenderProperty, PaintProperty)
public:
    DialogRenderProperty() = default;
    ~DialogRenderProperty() override = default;
    RefPtr<PaintProperty> Clone() const override
    {
        auto paintProperty = MakeRefPtr<DialogRenderProperty>();
        paintProperty->UpdatePaintProperty(this);
        paintProperty->propUseCustomStyle_ = CloneUseCustomStyle();
        paintProperty->propAutoCancel_ = CloneAutoCancel();
        return paintProperty;
    }

    void Reset() override
    {
        PaintProperty::Reset();
        ResetUseCustomStyle();
        ResetAutoCancel();
    }

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(UseCustomStyle, bool, PROPERTY_UPDATE_RENDER);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(AutoCancel, bool, PROPERTY_UPDATE_RENDER);

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override
    {
        PaintProperty::ToJsonValue(json);
        json->Put("customStyle", propUseCustomStyle_.value_or(false) ? "true" : "false");
        json->Put("autoCancel", propAutoCancel_.value_or(true) ? "true" : "false");
    }

    ACE_DISALLOW_COPY_AND_MOVE(DialogRenderProperty);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_DIALOG_DIALOG_RENDER_PROPERTY_H