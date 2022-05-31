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

#include "app_data_manager.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AppDataManager::AppDataManager() {}

AppDataManager::~AppDataManager() {}

bool AppDataManager::AddErrorObservers(int64_t observerId, const std::shared_ptr<IErrorObserver> &observer)
{
    HILOG_DEBUG("Add error observers come.");
    if (observer == nullptr) {
        HILOG_ERROR("Observer is nullptr.");
        return false;
    }

    std::lock_guard<std::recursive_mutex> lock(observerMutex_);
    if (ContainsObserver(observer)) {
        HILOG_ERROR("Observer is already exists.");
        return false;
    }
    errorObservers_[observerId] = observer;
    return true;
}

bool AppDataManager::RemoveErrorObservers(int64_t observerId)
{
    HILOG_DEBUG("Remove error observers come.");
    std::lock_guard<std::recursive_mutex> lock(observerMutex_);
    return errorObservers_.erase(observerId) == 1;
}

void AppDataManager::NotifyObserversUnhandledException(const std::string &errMsg)
{
    HILOG_DEBUG("Notify error observers come.");
    std::lock_guard<std::recursive_mutex> lock(observerMutex_);
    for (auto it = errorObservers_.begin(); it != errorObservers_.end(); it++) {
        auto observer = it->second;
        if (observer) {
            observer->OnUnhandledException(errMsg);
        }
    }
}

bool AppDataManager::ContainsObserver(const std::shared_ptr<IErrorObserver> &observerParam)
{
    for (auto it = errorObservers_.begin(); it != errorObservers_.end(); it++) {
        auto observer = it->second;
        if (observer && observer == observerParam) {
            return true;
        }
    }

    return false;
}
}  // namespace AppExecFwk
}  // namespace OHOS
