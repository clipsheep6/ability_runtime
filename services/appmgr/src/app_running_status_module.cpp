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

#include "app_running_status_module.h"

#include <algorithm>

#include "app_running_status_proxy.h"
#include "cpp/mutex.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
int32_t AppRunningStatusModule::RegisterListener(const sptr<AppRunningStatusListenerInterface> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (listener == nullptr || listener->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Listener is null.");
        return ERR_INVALID_OPERATION;
    }

    std::lock_guard<std::mutex> lock(listenerMutex_);
    auto findTask =
        [listener](
            const std::pair<sptr<AppRunningStatusListenerInterface>, sptr<IRemoteObject::DeathRecipient>> &item) {
            return listener->AsObject() == item.first->AsObject();
        };
    auto itemFind = std::find_if(listeners_.begin(), listeners_.end(), findTask);
    if (itemFind != listeners_.end()) {
        TAG_LOGD(AAFwkTag::APPMGR, "Listener is already exist.");
        return ERR_OK;
    }

    sptr<ClientDeathRecipient> deathRecipient = new (std::nothrow) ClientDeathRecipient(shared_from_this());
    if (deathRecipient == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Death recipient is null.");
        return ERR_NO_MEMORY;
    }

    listener->AsObject()->AddDeathRecipient(deathRecipient);
    listeners_.emplace(listener, deathRecipient);
    return ERR_OK;
}

int32_t AppRunningStatusModule::UnregisterListener(const sptr<AppRunningStatusListenerInterface> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (listener == nullptr || listener->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Input param invalid.");
        return ERR_INVALID_VALUE;
    }

    return RemoveListenerAndDeathRecipient(listener->AsObject());
}

void AppRunningStatusModule::NotifyAppRunningStatusEvent(
    const std::string &bundle, int32_t uid, RunningStatus runningStatus)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    std::lock_guard<std::mutex> lock(listenerMutex_);
    for (const auto &item : listeners_) {
        if (item.first == nullptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "Invalid listener.");
            continue;
        }

        item.first->NotifyAppRunningStatus(bundle, uid, runningStatus);
    }
}

AppRunningStatusModule::ClientDeathRecipient::ClientDeathRecipient(const std::weak_ptr<AppRunningStatusModule> &weakPtr)
{
    weakPtr_ = weakPtr;
}

void AppRunningStatusModule::ClientDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    auto appRunningStatus = weakPtr_.lock();
    if (appRunningStatus == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningStatus is nullptr.");
        return;
    }
    appRunningStatus->RemoveListenerAndDeathRecipient(remote);
}

int32_t AppRunningStatusModule::RemoveListenerAndDeathRecipient(const wptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    auto listener = remote.promote();
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote object is nullptr.");
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<std::mutex> lock(listenerMutex_);
    auto findTask =
        [listener](
            const std::pair<sptr<AppRunningStatusListenerInterface>, sptr<IRemoteObject::DeathRecipient>> &item) {
            return listener == item.first->AsObject();
        };
    auto itemFind = std::find_if(listeners_.begin(), listeners_.end(), findTask);
    if (itemFind == listeners_.end()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Listener is not exist.");
        return ERR_INVALID_OPERATION;
    }
    auto storedListener = itemFind->first;
    auto deathRecipient = itemFind->second;
    listeners_.erase(itemFind);

    if (storedListener == nullptr || storedListener->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid listener.");
        return ERR_INVALID_OPERATION;
    }

    storedListener->AsObject()->RemoveDeathRecipient(deathRecipient);
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
