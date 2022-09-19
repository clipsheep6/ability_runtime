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

#include "window_focus_changed_listener.h"

#include "app_mgr_service_inner.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::Rosen;

WindowFocusChangedListener::WindowFocusChangedListener(
    const std::shared_ptr<AppMgrServiceInner> &owner) : owner_(owner) {}

WindowFocusChangedListener::~WindowFocusChangedListener() {}

void WindowFocusChangedListener::OnFocused(const sptr<FocusChangeInfo> &focusChangeInfo)
{
    if (!focusChangeInfo) {
        HILOG_WARN("OnFocused invalid focusChangeInfo.");
        return;
    }

    auto owner = owner_.lock();
    if (!owner) {
        HILOG_WARN("OnFocused failed to get app mgr service inner.");
        return;
    }

    owner->HandleFocused(focusChangeInfo);
}

void WindowFocusChangedListener::OnUnfocused(const sptr<FocusChangeInfo> &focusChangeInfo)
{
    if (!focusChangeInfo) {
        HILOG_WARN("OnUnfocused invalid focusChangeInfo.");
        return;
    }

    auto owner = owner_.lock();
    if (!owner) {
        HILOG_WARN("OnUnfocused failed to get app mgr service inner.");
        return;
    }
    owner->HandleUnfocused(focusChangeInfo);
}
}  // namespace AppExecFwk
}  // namespace OHOS
