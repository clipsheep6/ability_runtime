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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_MOCK_STEPPER_RENDER_STEPPER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_MOCK_STEPPER_RENDER_STEPPER_H

#include "core/components/stepper/render_stepper.h"

namespace OHOS::Ace {

class MockRenderStepper : public RenderStepper {
public:
    MockRenderStepper() = default;

    void SetCurrentIndex(int32_t index)
    {
        currentIndex_ = index;
    }

    void SetNeedReverse(bool state)
    {
        needReverse_ = state;
    }

    void SetTotalItemCount(int32_t count)
    {
        totalItemCount_ = count;
    }

    void SetLeftButtonData(ControlPanelData data)
    {
        leftButtonData_ = data;
    }

    void SetRightButtonData(ControlPanelData data)
    {
        rightButtonData_ = data;
    }

    void MockHandleClick(const ClickInfo& clickInfo)
    {
        HandleClick(clickInfo);
    }
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_MOCK_STEPPER_RENDER_STEPPER_H
