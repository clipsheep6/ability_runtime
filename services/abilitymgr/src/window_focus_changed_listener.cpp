/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "window_focus_changed_listener.h"

#include "ability_manager_service.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::Rosen;
void WindowFocusChangedListener::OnFocused(const sptr<FocusChangeInfo> &focusChangeInfo)
{
    if (!focusChangeInfo) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "OnFocused invalid focusChangeInfo.");
        return;
    }

    if (taskHandler_) {
        auto task = [inner = owner_, focusChangeInfo] {
            auto owner = inner.lock();
            if (!owner) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "OnUnfocused failed to get app mgr service inner.");
                return;
            }
            owner->HandleFocused(focusChangeInfo);
        };
        taskHandler_->SubmitTask(task);
    }
}

void WindowFocusChangedListener::OnUnfocused(const sptr<FocusChangeInfo> &focusChangeInfo)
{
    if (!focusChangeInfo) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "OnUnfocused invalid focusChangeInfo.");
        return;
    }

    if (taskHandler_) {
        auto task = [inner = owner_, focusChangeInfo] {
            auto owner = inner.lock();
            if (!owner) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "OnUnfocused failed to get app mgr service inner.");
                return;
            }
            owner->HandleUnfocused(focusChangeInfo);
        };
        taskHandler_->SubmitTask(task);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
