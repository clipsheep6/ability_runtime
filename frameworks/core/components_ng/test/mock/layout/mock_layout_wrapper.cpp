/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/swiper_indicator/indicator_common/swiper_indicator_pattern.h"

namespace OHOS::Ace::NG {

const RefPtr<LayoutAlgorithmWrapper>& GetLayoutAlgorithm(bool needReset = false)
{
    return nullptr;
}

void Measure(const std::optional<LayoutConstraintF>& parentConstraint) {}

void Layout() {}

int32_t GetTotalChildCount()
{
    return 0;
}

const RefPtr<GeometryNode>& GetGeometryNode()
{
    return nullptr;
}

const RefPtr<LayoutProperty>& GetLayoutProperty()
{
    return nullptr;
}

RefPtr<LayoutWrapper> GetOrCreateChildByIndex(uint32_t index, bool addToRenderTree = true)
{
    if (index >= 2) {
        auto indicatorNode = FrameNode::GetOrCreateFrameNode(V2::SWIPER_INDICATOR_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<SwiperIndicatorPattern>(); });
        auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
        auto layoutWrapper =
            AceType::MakeRefPtr<LayoutWrapperNode>(indicatorNode, geometryNode, indicatorNode->GetLayoutProperty());
        return layoutWrapper;
    } else if (index == 1) {
        auto indicatorNode = FrameNode::GetOrCreateFrameNode(V2::SWIPER_INDICATOR_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<SwiperIndicatorPattern>(); });
        auto layoutWrapper =
            AceType::MakeRefPtr<LayoutWrapperNode>(indicatorNode, nullptr, indicatorNode->GetLayoutProperty());
        return layoutWrapper;
    }
    return nullptr;
}

const std::list<RefPtr<LayoutWrapper>>& GetAllChildrenWithBuild(bool addToRenderTree = true)
{
    return {};
}

void RemoveChildInRenderTree(uint32_t index) {}

void RemoveAllChildInRenderTree() {}

RefPtr<FrameNode> GetHostNode()
{
    return nullptr;
}

const std::string& GetHostTag()
{
    return V2::SWIPER_ETS_TAG;
}

bool IsActive()
{
    return false;
}

void SetActive(bool active = true) {}

bool SkipMeasureContent()
{
    return false;
}

void SetCacheCount(int32_t cacheCount = 0, const std::optional<LayoutConstraintF>& itemConstraint = std::nullopt) {}

float GetBaselineDistance()
{
    return 0.0f;
}

bool CheckNeedForceMeasureAndLayout()
{
    return false;
}

void AddNodeFlexLayouts() {}

void AddNodeLayoutTime(int64_t time) {}

void RestoreGeoState() {}
} // namespace OHOS::Ace::NG
