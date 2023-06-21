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

#ifndef OHOS_ABILITY_RUNTIME_WINDOW_FOCUS_CHANGE_LISTENER_H
#define OHOS_ABILITY_RUNTIME_WINDOW_FOCUS_CHANGE_LISTENER_H


#include "window_manager.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerService;
class AbilityEventHandler;
class WindowFocusChangedListener : public OHOS::Rosen::IFocusChangedListener {
public:
    WindowFocusChangedListener(const std::shared_ptr<AbilityManagerService>& owner,
        const std::shared_ptr<AbilityEventHandler>& handler) : owner_(owner), eventHandler_(handler) {}
    virtual ~WindowFocusChangedListener() = default;

    void OnFocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo) override;
    void OnUnfocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo) override;

private:
    std::weak_ptr<AbilityManagerService> owner_;
    std::shared_ptr<AbilityEventHandler> eventHandler_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_WINDOW_FOCUS_CHANGE_LISTENER_H
