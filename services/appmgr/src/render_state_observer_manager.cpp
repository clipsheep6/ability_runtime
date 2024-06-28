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

#include "hilog_tag_wrapper.h"
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
    TAG_LOGD(AAFwkTag::APPMGR, "begin.");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "the observer is nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "handler is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto task = [weak = weak_from_this(), observer]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "self is nullptr.");
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
            TAG_LOGE(AAFwkTag::APPMGR, "New RenderStateObserverManager failed.");
        }
    }

    auto observerObj = observer->AsObject();
    if (!observerObj || !observerObj->AddDeathRecipient(deathRecipient_)) {
        TAG_LOGE(AAFwkTag::APPMGR, "AddDeathRecipient failed.");
    }

    observerList_.emplace_back(observer);
    TAG_LOGD(AAFwkTag::APPMGR, "observerList_ size:%{public}zu", observerList_.size());
}

int32_t RenderStateObserverManager::UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin.");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "the observer is nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "handler is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto task = [weak = weak_from_this(), observer]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "self is nullptr.");
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
        TAG_LOGI(AAFwkTag::APPMGR, "observerList_ size:%{public}zu", observerList_.size());
        return;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "Observer not exist or has been removed.");
}

int32_t RenderStateObserverManager::OnRenderStateChanged(const std::shared_ptr<RenderRecord> &renderRecord,
    int32_t state)
{
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "handler is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto task = [weak = weak_from_this(), renderRecord, state]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "self is nullptr.");
            return;
        }
        self->HandleOnRenderStateChanged(renderRecord, state);
    };
    handler_->SubmitTask(task);
    return ERR_OK;
}

void RenderStateObserverManager::HandleOnRenderStateChanged(const std::shared_ptr<RenderRecord> &renderRecord,
    int32_t state)
{
    if (renderRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "renderRecord is nullptr");
        return;
    }
    RenderStateData data = WrapRenderStateData(renderRecord, state);
    TAG_LOGD(AAFwkTag::APPMGR,
        "pid:%{public}d, hostPid:%{public}d, uid:%{public}d, hostUid:%{public}d, state:%{public}d",
        data.pid, data.hostPid, data.uid, data.hostUid, data.state);
    for (auto it = observerList_.begin(); it != observerList_.end(); ++it) {
        if ((*it) == nullptr) {
            continue;
        }
        (*it)->OnRenderStateChanged(data);
    }
}

RenderStateData RenderStateObserverManager::WrapRenderStateData(const std::shared_ptr<RenderRecord> &renderRecord,
    int32_t state)
{
    RenderStateData renderStateData;
    renderStateData.pid = renderRecord->GetPid();
    renderStateData.uid = renderRecord->GetUid();
    renderStateData.hostPid = renderRecord->GetHostPid();
    renderStateData.hostUid = renderRecord->GetHostUid();
    renderStateData.state = state;
    return renderStateData;
}

void RenderStateObserverManager::OnObserverDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGI(AAFwkTag::APPMGR, "OnObserverDied begin.");
    auto remoteObj = remote.promote();
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer is nullptr.");
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
    TAG_LOGE(AAFwkTag::APPMGR, "RenderStateObserverRecipient On remote died.");
    if (handler_) {
        handler_(remote);
    }
}
} // namespace AppExecFwk
} // namespace OHOS