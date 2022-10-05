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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SHAPE_PATH_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SHAPE_PATH_PATTERN_H

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/noncopyable.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/shape/path_paint_method.h"
#include "core/components_ng/pattern/shape/path_paint_property.h"
#include "core/components_ng/pattern/shape/shape_layout_algorithm.h"
#include "core/components_ng/pattern/shape/shape_paint_property.h"
#include "core/components_ng/pattern/shape/shape_pattern.h"

namespace OHOS::Ace::NG {
class PathPattern : public ShapePattern {
    DECLARE_ACE_TYPE(PathPattern, ShapePattern);

public:
    PathPattern() = default;
    ~PathPattern() override = default;

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        return MakeRefPtr<PathPaintMethod>(GetAncestorPaintProperty());
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<PathPaintProperty>();
    }

private:
    ACE_DISALLOW_COPY_AND_MOVE(PathPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SHAPE_PATH_PATTERN_H