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

#include "window_visibility_changed_listener.h"

#include "app_mgr_service_inner.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::Rosen;
WindowVisibilityChangedListener::WindowVisibilityChangedListener(
    const std::weak_ptr<AppMgrServiceInner> &appInner, const std::shared_ptr<AAFwk::TaskHandlerWrap> &handler)
    : appServiceInner_(appInner), taskHandler_(handler)
{}

void WindowVisibilityChangedListener::OnWindowVisibilityChanged(
    const std::vector<sptr<WindowVisibilityInfo>> &windowVisibilityInfos)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (windowVisibilityInfos.empty()) {
        TAG_LOGW(AAFwkTag::APPMGR, "Window visibility info is empty.");
        return;
    }

    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Task handler is nullptr.");
        return;
    }

    auto task = [inner = appServiceInner_, windowVisibilityInfos] {
        auto serviceInner = inner.lock();
        if (serviceInner == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to get app mgr service inner.");
            return;
        }
        serviceInner->HandleWindowVisibilityChanged(windowVisibilityInfos);
    };
    taskHandler_->SubmitTask(task);
}
} // namespace AppExecFwk
} // namespace OHOS
