/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "mission_listener_controller.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string THREAD_NAME = "MissionListener";
}
using Cmd = IMissionListener::MissionListenerCmd;

MissionListenerController::MissionListenerController()
{
}

MissionListenerController::~MissionListenerController()
{}

void MissionListenerController::Init()
{
    if (!handler_) {
        handler_ = TaskHandlerWrap::CreateQueueHandler("mission_listener_task_queue");
    }
}

int MissionListenerController::AddMissionListener(const sptr<IMissionListener> &listener)
{
    if (!listener) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "listener is invalid");
        return -1;
    }

    std::lock_guard<ffrt::mutex> guard(listenerLock_);
    auto it = std::find_if(missionListeners_.begin(), missionListeners_.end(),
        [&listener](const sptr<IMissionListener> &item) {
            return (item && item->AsObject() == listener->AsObject());
        }
    );
    if (it != missionListeners_.end()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "listener was already added, do not add again");
        return 0;
    }

    if (!listenerDeathRecipient_) {
        std::weak_ptr<MissionListenerController> thisWeakPtr(shared_from_this());
        listenerDeathRecipient_ =
            new ListenerDeathRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto controller = thisWeakPtr.lock();
                if (controller) {
                    controller->OnListenerDied(remote);
                }
            });
    }
    auto listenerObject = listener->AsObject();
    if (listenerObject) {
        listenerObject->AddDeathRecipient(listenerDeathRecipient_);
    }
    missionListeners_.emplace_back(listener);

    return 0;
}

void MissionListenerController::DelMissionListener(const sptr<IMissionListener> &listener)
{
    if (!listener) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "listener is invalid");
        return;
    }

    std::lock_guard<ffrt::mutex> guard(listenerLock_);
    auto it = std::find_if(missionListeners_.begin(), missionListeners_.end(),
        [&listener](const sptr<IMissionListener> item) {
            return (item && item->AsObject() == listener->AsObject());
        }
    );
    if (it != missionListeners_.end()) {
        missionListeners_.erase(it);
    }
}

void MissionListenerController::NotifyMissionCreated(int32_t missionId)
{
    if (!handler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "handler not init");
        return;
    }
    auto task = [weak = weak_from_this(), missionId]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "self is nullptr, NotifyMissionCreated failed.");
            return;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "notify listeners mission is created, missionId:%{public}d.", missionId);
        self->CallListeners(&IMissionListener::OnMissionCreated, missionId);
    };
    handler_->SubmitTask(task);
}

void MissionListenerController::NotifyMissionDestroyed(int32_t missionId)
{
    if (!handler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "handler not init");
        return;
    }
    auto task = [weak = weak_from_this(), missionId]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "self is nullptr, NotifyMissionDestroyed failed.");
            return;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "notify listeners mission is destroyed, missionId:%{public}d.", missionId);
        self->CallListeners(&IMissionListener::OnMissionDestroyed, missionId);
    };
    handler_->SubmitTask(task);
}

void MissionListenerController::HandleUnInstallApp(const std::list<int32_t> &missions)
{
    if (!handler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "handler not init");
        return;
    }

    if (missions.empty()) {
        return;
    }

    auto task = [weak = weak_from_this(), missions]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "self is nullptr, NotifyMissionDestroyed failed.");
            return;
        }
        for (auto id : missions) {
            self->CallListeners(&IMissionListener::OnMissionDestroyed, id);
        }
    };
    handler_->SubmitTask(task);
}

void MissionListenerController::NotifyMissionSnapshotChanged(int32_t missionId)
{
    if (!handler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "handler not init.");
        return;
    }

    auto task = [weak = weak_from_this(), missionId]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "self is nullptr, NotifyMissionSnapshotChanged failed.");
            return;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "notify listeners mission snapshot changed, missionId:%{public}d.", missionId);
        self->CallListeners(&IMissionListener::OnMissionSnapshotChanged, missionId);
    };
    handler_->SubmitTask(task);
}

void MissionListenerController::NotifyMissionMovedToFront(int32_t missionId)
{
    if (!handler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "handler not init!");
        return;
    }

    auto task = [weak = weak_from_this(), missionId]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "self is nullptr, NotifyMissionSnapshotChanged failed!");
            return;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "notify listeners mission is moved to front, missionId:%{public}d.", missionId);
        self->CallListeners(&IMissionListener::OnMissionMovedToFront, missionId);
    };
    handler_->SubmitTask(task);
}

void MissionListenerController::NotifyMissionFocused(int32_t missionId)
{
    if (missionId == -1) {
        return;
    }

    if (!handler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "handler is null.");
        return;
    }
    auto task = [weak = weak_from_this(), missionId]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "self is nullptr, NotifyMissionFocused failed.");
            return;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "NotifyMissionFocused, missionId:%{public}d.", missionId);
        self->CallListeners(&IMissionListener::OnMissionFocused, missionId);
    };
    handler_->SubmitTask(task);
}

void MissionListenerController::NotifyMissionUnfocused(int32_t missionId)
{
    if (missionId == -1) {
        return;
    }

    if (!handler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "handler is null!");
        return;
    }
    auto task = [weak = weak_from_this(), missionId]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "self is nullptr, NotifyMissionUnfocused failed!");
            return;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "NotifyMissionUnfocused, missionId:%{public}d.", missionId);
        self->CallListeners(&IMissionListener::OnMissionUnfocused, missionId);
    };
    handler_->SubmitTask(task);
}

#ifdef SUPPORT_GRAPHICS
void MissionListenerController::NotifyMissionIconChanged(int32_t missionId,
    const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
    if (!handler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "handler not init when notify mission icon changed");
        return;
    }

    auto task = [weak = weak_from_this(), missionId, icon]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "self is nullptr, NotifyMissionIconChanged failed.");
            return;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "notify listeners mission icon has changed, missionId:%{public}d.", missionId);
        self->CallListeners(&IMissionListener::OnMissionIconUpdated, missionId, icon);
    };
    handler_->SubmitTask(task);
}
#endif

void MissionListenerController::NotifyMissionClosed(int32_t missionId)
{
    if (!handler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "handler not init");
        return;
    }
    auto task = [weak = weak_from_this(), missionId]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "self is nullptr, NotifyMissionClosed failed.");
            return;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "NotifyMissionClosed, missionId:%{public}d.", missionId);
        self->CallListeners(&IMissionListener::OnMissionClosed, missionId);
    };
    handler_->SubmitTask(task);
}

void MissionListenerController::NotifyMissionLabelUpdated(int32_t missionId)
{
    if (!handler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyMissionLabelUpdated, handler not init");
        return;
    }
    auto task = [weak = weak_from_this(), missionId]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "self is nullptr, NotifyMissionLabelUpdated failed.");
            return;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "notify listeners mission label has updated, missionId:%{public}d.", missionId);
        self->CallListeners(&IMissionListener::OnMissionLabelUpdated, missionId);
    };
    handler_->SubmitTask(task);
}

void MissionListenerController::OnListenerDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "On mission listener died.");
    auto remoteObj = remote.promote();
    if (!remoteObj) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "invalid remote object.");
        return;
    }
    remoteObj->RemoveDeathRecipient(listenerDeathRecipient_);

    std::lock_guard<ffrt::mutex> guard(listenerLock_);
    auto it = std::find_if(missionListeners_.begin(), missionListeners_.end(),
        [&remoteObj](const sptr<IMissionListener> item) {
            return (item && item->AsObject() == remoteObj);
        }
    );
    if (it != missionListeners_.end()) {
        missionListeners_.erase(it);
    }
}

MissionListenerController::ListenerDeathRecipient::ListenerDeathRecipient(ListenerDiedHandler handler)
    : diedHandler_(handler)
{}

void MissionListenerController::ListenerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (diedHandler_) {
        diedHandler_(remote);
    }
}
} // namespace AAFwk
} // namespace OHOS
