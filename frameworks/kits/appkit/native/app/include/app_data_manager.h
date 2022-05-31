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

#ifndef OHOS_APPEXECFWK_APP_DATA_MANAGER_H
#define OHOS_APPEXECFWK_APP_DATA_MANAGER_H

#include <list>
#include <mutex>
#include <string>
#include <unordered_map>

#include "ierror_observer.h"
#include "singleton.h"

namespace OHOS {
namespace AppExecFwk {
class AppDataManager : public std::enable_shared_from_this<AppDataManager> {
    DECLARE_DELAYED_SINGLETON(AppDataManager)
public:
    bool AddErrorObservers(int64_t observerId, const std::shared_ptr<IErrorObserver> &observer);
    bool RemoveErrorObservers(int64_t observerId);
    void NotifyObserversUnhandledException(const std::string &errMsg);

private:
    bool ContainsObserver(const std::shared_ptr<IErrorObserver> &observer);

private:
    std::unordered_map<int64_t, std::shared_ptr<IErrorObserver>> errorObservers_;
    mutable std::recursive_mutex observerMutex_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APPEXECFWK_APP_DATA_MANAGER_H