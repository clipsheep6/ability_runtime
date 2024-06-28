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

#include "app_debug_manager.h"

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
int32_t AppDebugManager::RegisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Listener is nullptr.");
        return ERR_INVALID_DATA;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    const auto &finder =
        std::find_if(listeners_.begin(), listeners_.end(), [&listener](const sptr<IAppDebugListener> &element) {
            if (element != nullptr && listener != nullptr) {
                return element->AsObject() == listener->AsObject();
            }
            return false;
        });
    if (finder == listeners_.end()) {
        listeners_.emplace(listener);
    }

    if (!debugInfos_.empty()) {
        listener->OnAppDebugStarted(debugInfos_);
    }
    return ERR_OK;
}

int32_t AppDebugManager::UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Listener is nullptr.");
        return ERR_INVALID_DATA;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    const auto &finder =
        std::find_if(listeners_.begin(), listeners_.end(), [&listener](const sptr<IAppDebugListener> &element) {
            if (element != nullptr && listener != nullptr) {
                return element->AsObject() == listener->AsObject();
            }
            return false;
        });
    if (finder != listeners_.end()) {
        listeners_.erase(finder);
    }
    return ERR_OK;
}

void AppDebugManager::StartDebug(const std::vector<AppDebugInfo> &infos)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<AppDebugInfo> incrementInfos;
    GetIncrementAppDebugInfos(infos, incrementInfos);
    if (incrementInfos.empty()) {
        return;
    }

    for (const auto &listener : listeners_) {
        if (listener == nullptr) {
            continue;
        }
        listener->OnAppDebugStarted(incrementInfos);
    }
}

void AppDebugManager::StopDebug(const std::vector<AppDebugInfo> &infos)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<AppDebugInfo> debugInfos;
    for (auto &it : infos) {
        auto isExist = [this, it](const AppDebugInfo &info) {
            return (info.bundleName == it.bundleName && info.pid == it.pid &&
                info.uid == it.uid && info.isDebugStart == it.isDebugStart);
        };

        auto finder = std::find_if(debugInfos_.begin(), debugInfos_.end(), isExist);
        if (finder != debugInfos_.end()) {
            debugInfos_.erase(finder);
            debugInfos.emplace_back(it);
        }
    }

    if (!debugInfos.empty()) {
        for (const auto &listener : listeners_) {
            if (listener == nullptr) {
                continue;
            }
            listener->OnAppDebugStoped(debugInfos);
        }
    }
}

bool AppDebugManager::IsAttachDebug(const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &iter : debugInfos_) {
        if (iter.bundleName == bundleName && !iter.isDebugStart) {
            return true;
        }
    }
    return false;
}

void AppDebugManager::GetIncrementAppDebugInfos(
    const std::vector<AppDebugInfo> &infos, std::vector<AppDebugInfo> &incrementInfos)
{
    for (auto &it : infos) {
        auto isExist = [this, it](const AppDebugInfo &info) {
            return (info.bundleName == it.bundleName && info.pid == it.pid && info.uid == it.uid);
        };

        auto finder = std::find_if(debugInfos_.begin(), debugInfos_.end(), isExist);
        if (finder == debugInfos_.end()) {
            incrementInfos.emplace_back(it);
        } else {
            if (!finder->isDebugStart && it.isDebugStart) {
                finder->isDebugStart = it.isDebugStart;
            }
        }
    }

    debugInfos_.insert(debugInfos_.end(), incrementInfos.begin(), incrementInfos.end());
}

void AppDebugManager::RemoveAppDebugInfo(const AppDebugInfo &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    std::lock_guard<std::mutex> lock(mutex_);
    auto isExist = [this, info](const AppDebugInfo &debugInfo) {
        return (debugInfo.bundleName == info.bundleName && debugInfo.pid == info.pid &&
                debugInfo.uid == info.uid && debugInfo.isDebugStart == info.isDebugStart);
    };

    auto finder = std::find_if(debugInfos_.begin(), debugInfos_.end(), isExist);
    if (finder == debugInfos_.end()) {
        return;
    }
    debugInfos_.erase(finder);

    std::vector<AppDebugInfo> debugInfos;
    debugInfos.emplace_back(info);
    for (const auto &listener : listeners_) {
        if (listener == nullptr) {
            continue;
        }
        listener->OnAppDebugStoped(debugInfos);
    }
}
} // namespace AppExecFwk
} // namespace OHOS