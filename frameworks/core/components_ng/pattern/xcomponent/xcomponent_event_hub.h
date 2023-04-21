/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_XCOMPONENT_XCOMPONENT_EVENT_HUB_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_XCOMPONENT_XCOMPONENT_EVENT_HUB_H

#include "base/memory/ace_type.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/event/gesture_event_hub.h"

namespace OHOS::Ace::NG {
using LoadEvent = std::function<void(const std::string&)>;
using DestroyEvent = std::function<void()>;
using ExternalEvent = std::function<void(const std::string&, const uint32_t, const bool)>;

class XComponentEventHub : public EventHub {
    DECLARE_ACE_TYPE(XComponentEventHub, EventHub)

public:
    XComponentEventHub() = default;
    ~XComponentEventHub() override = default;

    void SetOnLoad(LoadEvent&& loadEvent)
    {
        loadEvent_ = std::move(loadEvent);
    }

    void FireLoadEvent(const std::string& xcomponentId) const
    {
        if (loadEvent_) {
            loadEvent_(xcomponentId);
        }
    }

    void SetOnDestroy(DestroyEvent&& destroyEvent)
    {
        destroyEvent_ = std::move(destroyEvent);
    }

    void FireDestroyEvent() const
    {
        if (destroyEvent_) {
            destroyEvent_();
        }
    }

    void SetOnSurfaceInitEvent(ExternalEvent&& surfaceInitEvent)
    {
        surfaceInitEvent_ = std::move(surfaceInitEvent);
    }

    void FireSurfaceInitEvent(const std::string& componentId, const uint32_t nodeId) const
    {
        if (surfaceInitEvent_) {
            surfaceInitEvent_(componentId, nodeId, false);
        }
    }

private:
    LoadEvent loadEvent_;
    DestroyEvent destroyEvent_;
    ExternalEvent surfaceInitEvent_;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_XCOMPONENT_XCOMPONENT_EVENT_HUB_H
