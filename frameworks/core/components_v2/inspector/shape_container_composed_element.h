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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_INSPECTOR_SHAPE_CONTAINER_COMPOSED_ELEMENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_INSPECTOR_SHAPE_CONTAINER_COMPOSED_ELEMENT_H

#include "core/components/shape/render_shape_container.h"
#include "core/components/shape/shape_container_component.h"
#include "core/components/shape/shape_container_element.h"
#include "core/components_v2/inspector/inspector_composed_element.h"
#include "core/pipeline/base/composed_element.h"

namespace OHOS::Ace::V2 {

class ACE_EXPORT ShapeContainerComposedElement : public InspectorComposedElement {
    DECLARE_ACE_TYPE(ShapeContainerComposedElement, InspectorComposedElement)

public:
    static std::string LineCapStyleToString(LineCapStyle lineCapStyle);
    static std::string LineJoinStyleToString(LineJoinStyle lineJoinStyle);

    explicit ShapeContainerComposedElement(const ComposeId& id) : InspectorComposedElement(id) {}
    ~ShapeContainerComposedElement() override = default;
    void Dump() override;
    std::unique_ptr<JsonValue> ToJsonObject() const override;

    std::string GetAntiAlias() const;
    std::string GetFill() const;
    std::string GetFillOpacity() const;
    std::string GetStroke() const;
    std::string GetStrokeDashOffset() const;
    std::string GetStrokeLineCap() const;
    std::string GetStrokeLineJoin() const;
    std::string GetStrokeMiterLimit() const;
    std::string GetStrokeOpacity() const;
    std::string GetStrokeWidth() const;
    std::unique_ptr<JsonValue> GetViewBox() const;
    std::unique_ptr<JsonValue> GetStrokeDashArray() const;
    std::unique_ptr<JsonValue> GetMesh() const;

    AceType::IdType GetTargetTypeId() const override
    {
        return ShapeContainerElement::TypeId();
    }
};

} // namespace OHOS::Ace::V2

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_INSPECTOR_SHAPE_CONTAINER_COMPOSED_ELEMENT_H