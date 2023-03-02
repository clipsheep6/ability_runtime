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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_EVENT_HUB_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_EVENT_HUB_H

#include <functional>

#include "base/memory/ace_type.h"
#include "core/components_ng/event/event_hub.h"

namespace OHOS::Ace::NG {
using OnErrorCallback = std::function<void()>;

class UIExtensionEventHub : public EventHub {
    DECLARE_ACE_TYPE(UIExtensionEventHub, EventHub)

public:
    UIExtensionEventHub() = default;
    ~UIExtensionEventHub() override = default;

    void SetOnError(OnErrorCallback&& onError)
    {
        onError_ = std::move(onError);
    }
    void FireErrorEvent()
    {
        if (onError_) {
            onError_();
        }
    }

private:
    OnErrorCallback onError_;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_EVENT_HUB_H
