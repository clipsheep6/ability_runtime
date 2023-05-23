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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_PICKER_TEXT_PICKER_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_PICKER_TEXT_PICKER_LAYOUT_ALGORITHM_H

#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_algorithm.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT TextPickerLayoutAlgorithm : public LinearLayoutAlgorithm {
    DECLARE_ACE_TYPE(TextPickerLayoutAlgorithm, LinearLayoutAlgorithm);

public:
    TextPickerLayoutAlgorithm() = default;
    ~TextPickerLayoutAlgorithm() override = default;

    void OnReset() override {}

    void Measure(LayoutWrapper* layoutWrapper) override;
    void Layout(LayoutWrapper* layoutWrapper) override;
    void MeasureText(LayoutWrapper* layoutWrapper, const SizeF& size);
    void ChangeTextStyle(uint32_t index, uint32_t showOptionCount, const SizeF& size,
        const RefPtr<LayoutWrapper>& childLayoutWrapper, LayoutWrapper* layoutWrapper);

    float GetCurrentOffset() const
    {
        return currentOffset_;
    }

    void SetCurrentOffset(float currentOffset)
    {
        currentOffset_ = currentOffset;
    }

    double GetDefaultPickerItemHeight() const
    {
        return defaultPickerItemHeight_;
    }

    void SetDefaultPickerItemHeight(double defaultPickerItemHeight)
    {
        defaultPickerItemHeight_ = defaultPickerItemHeight;
    }

    int32_t GetHalfDisplayCounts() const
    {
        return halfDisplayCounts_;
    }

private:
    float pickerItemHeight_ = 0.0f;
    float currentOffset_ = 0.0f;
    double defaultPickerItemHeight_ = 0.0;
    bool isDefaultPickerItemHeight_ = false;
    int32_t halfDisplayCounts_ = 0;

    ACE_DISALLOW_COPY_AND_MOVE(TextPickerLayoutAlgorithm);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_PICKER_TEXT_PICKER_LAYOUT_ALGORITHM_H
