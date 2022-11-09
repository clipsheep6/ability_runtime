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

#include "core/components_ng/pattern/tabs/tab_bar_layout_property.h"

#include "core/components_ng/base/frame_node.h"

namespace OHOS::Ace::NG {

RectF TabBarLayoutProperty::GetIndicatorRect(int32_t index)
{
    auto node = GetHost();
    CHECK_NULL_RETURN(node, RectF());
    auto childColumn = DynamicCast<FrameNode>(node->GetChildAtIndex(index));
    CHECK_NULL_RETURN(childColumn, RectF());
    LOGE("childColumn tag %s", childColumn->GetTag().c_str());
    auto grandChildren = DynamicCast<FrameNode>(childColumn->GetChildren().back());
    CHECK_NULL_RETURN(grandChildren, RectF());
    LOGE("grandChildren tag %s", grandChildren->GetTag().c_str());
    auto geometryNode = grandChildren->GetGeometryNode();
    RectF indicator = geometryNode->GetFrameRect();

    /* Set indicator at the bottom of columnNode's last child */
    auto childColumnRect = childColumn->GetGeometryNode()->GetFrameRect();
    indicator.SetLeft(indicator.GetX() + childColumnRect.GetX());
    indicator.SetTop(indicator.Bottom() + childColumnRect.GetY());
    return indicator;
}

} // namespace OHOS::Ace::NG
