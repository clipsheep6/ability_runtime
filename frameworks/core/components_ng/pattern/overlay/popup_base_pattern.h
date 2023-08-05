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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_OVERLAY_POPUP_PATTERN_BASE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_OVERLAY_POPUP_PATTERN_BASE_H

#include "base/utils/noncopyable.h"
#include "core/components_ng/pattern/pattern.h"
namespace OHOS::Ace::NG {
// base class of all Popup patterns (Dialog, Toast, Menu, Bubble, etc.)
class PopupBasePattern : virtual public Pattern {
    DECLARE_ACE_TYPE(PopupBasePattern, Pattern);
public:
    PopupBasePattern() = default;
    ~PopupBasePattern() override = default;

    void BeforeCreateLayoutWrapper() override;

    void SetContainerId(int32_t containerId)
    {
        containerId_ = containerId;
    }

    int32_t GetContainerId() const
    {
        return containerId_;
    }

private:
    // record node in which container.
    int32_t containerId_ = -1;

    ACE_DISALLOW_COPY_AND_MOVE(PopupBasePattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_OVERLAY_POPUP_PATTERN_BASE_H
