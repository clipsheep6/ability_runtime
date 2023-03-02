/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "core/common/window.h"

#include "base/log/log.h"
#include "base/utils/macros.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"

namespace OHOS::Ace {

Window::Window(std::unique_ptr<PlatformWindow> platformWindow) : platformWindow_(std::move(platformWindow))
{
    CHECK_NULL_VOID(platformWindow_);
    auto&& callback = [this](uint64_t nanoTimestamp, uint32_t frameCount) {
        OnVsync(nanoTimestamp, frameCount);
    };
    platformWindow_->RegisterVsyncCallback(callback);
    LOGI("Window Created success.");
}

void Window::RequestFrame()
{
    if (!onShow_) {
        LOGD("window is not show, stop request frame");
        return;
    }
    if (!isRequestVsync_ && platformWindow_ != nullptr) {
        LOGD("request next vsync");
        platformWindow_->RequestFrame();
        isRequestVsync_ = true;
    }
}

void Window::SetRootRenderNode(const RefPtr<RenderNode>& root)
{
    CHECK_NULL_VOID(platformWindow_);
    platformWindow_->SetRootRenderNode(root);
}

void Window::OnVsync(uint64_t nanoTimestamp, uint32_t frameCount)
{
    isRequestVsync_ = false;

    for (auto& callback : callbacks_) {
        if (callback.callback_ == nullptr) {
            continue;
        }
#if !defined(PREVIEW)
        callback.callback_(nanoTimestamp, frameCount);
#else
        auto task = [nanoTimestamp, frameCount, callback = callback.callback_] {
            callback(nanoTimestamp, frameCount);
        };

        ContainerScope scope(callback.containerId_);
        auto executor = Container::CurrentTaskExecutor();
        if (executor != nullptr) {
            executor->PostTask(task, TaskExecutor::TaskType::UI);
        }
#endif
    }
}

void Window::SetVsyncCallback(AceVsyncCallback&& callback)
{
    callbacks_.push_back({
        .callback_ = std::move(callback),
        .containerId_ = Container::CurrentId(),
    });
}


} // namespace OHOS::Ace
