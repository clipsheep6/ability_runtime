/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "render_state_observer_manager.h"

#include "hilog_wrapper.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
RenderStateObserverManager::RenderStateObserverManager()
{}

RenderStateObserverManager::~RenderStateObserverManager()
{}

void RenderStateObserverManager::Init()
{
    if (!handler_) {
        handler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("render_state_task_queue");
    }
}

int32_t RenderStateObserverManager::RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    HILOG_DEBUG("begin.");
    if (observer == nullptr) {
        HILOG_ERROR("the observer is nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto task = [weak = weak_from_this(), observer]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr.");
            return;
        }
        self->HandleRegisterRenderStateObserver(observer);
    };
    handler_->SubmitTask(task);
    return ERR_OK;
}

void RenderStateObserverManager::HandleRegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    if (deathRecipient_ == nullptr) {
        std::weak_ptr<RenderStateObserverManager> thisWeakPtr(shared_from_this());
        deathRecipient_ =
            new (std::nothrow) RenderStateObserverRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto renderStateObserverManager = thisWeakPtr.lock();
                if (renderStateObserverManager) {
                    renderStateObserverManager->OnObserverDied(remote);
                }
            });
        if (deathRecipient_ == nullptr) {
            HILOG_ERROR("New RenderStateObserverManager failed.");
            return;
        }
    }

    observerList_.emplace_back(observer);
    HILOG_DEBUG("observerList_ size:%{public}zu", observerList_.size());
}

int32_t RenderStateObserverManager::UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    HILOG_DEBUG("begin.");
    if (observer == nullptr) {
        HILOG_ERROR("the observer is nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto task = [weak = weak_from_this(), observer]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr.");
            return;
        }
        self->HandleUnregisterRenderStateObserver(observer);
    };
    handler_->SubmitTask(task);
    return ERR_OK;
}

void RenderStateObserverManager::HandleUnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    auto it = std::find_if(observerList_.begin(), observerList_.end(),
        [&observer](const sptr<IRenderStateObserver> &item) {
        return (item && item->AsObject() == observer->AsObject());
    });
    if (it != observerList_.end()) {
        observerList_.erase(it);
        HILOG_INFO("observerList_ size:%{public}zu", observerList_.size());
        return;
    }
    HILOG_ERROR("Observer not exist or has been removed.");
}

int32_t RenderStateObserverManager::OnRenderStateChanged(pid_t renderPid, int32_t state)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto task = [weak = weak_from_this(), renderPid, state]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr.");
            return;
        }
        self->HandleOnRenderStateChanged(renderPid, state);
    };
    handler_->SubmitTask(task);
    return ERR_OK;
}

void RenderStateObserverManager::HandleOnRenderStateChanged(pid_t renderPid, int32_t state)
{
    for (auto it = observerList_.begin(); it != observerList_.end(); ++it) {
        if ((*it) == nullptr) {
            continue;
        }
        (*it)->OnRenderStateChanged(renderPid, state);
    }
}

void RenderStateObserverManager::OnObserverDied(const wptr<IRemoteObject> &remote)
{
    HILOG_INFO("OnObserverDied begin.");
    auto remoteObj = remote.promote();
    if (remoteObj == nullptr) {
        HILOG_ERROR("observer is nullptr.");
        return;
    }
    remoteObj->RemoveDeathRecipient(deathRecipient_);

    sptr<IRenderStateObserver> observer = iface_cast<IRenderStateObserver>(remoteObj);
    UnregisterRenderStateObserver(observer);
}

RenderStateObserverRecipient::RenderStateObserverRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

RenderStateObserverRecipient::~RenderStateObserverRecipient()
{}

void RenderStateObserverRecipient::OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote)
{
    HILOG_ERROR("RenderStateObserverRecipient On remote died.");
    if (handler_) {
        handler_(remote);
    }
}
} // namespace AppExecFwk
} // namespace OHOS