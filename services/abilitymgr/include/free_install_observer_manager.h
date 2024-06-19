/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_FREE_INSTALL_OBSERVER_MANAGER_H
#define OHOS_ABILITY_RUNTIME_FREE_INSTALL_OBSERVER_MANAGER_H

#include <map>
#include <mutex>
#include <vector>

#include "free_install_observer_interface.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::AbilityRuntime;
class FreeInstallObserverManager : public std::enable_shared_from_this<FreeInstallObserverManager> {
    DECLARE_DELAYED_SINGLETON(FreeInstallObserverManager)
public:
    int32_t AddObserver(const sptr<IFreeInstallObserver> &observer);

    int32_t RemoveObserver(const sptr<IFreeInstallObserver> &observer);

    void OnInstallFinished(const std::string &bundleName, const std::string &abilityName,
        const std::string &startTime, const int &resultCode);

private:
    bool ObserverExistLocked(const sptr<IFreeInstallObserver> &observer);

    void OnObserverDied(const wptr<IRemoteObject> &remote);

    void HandleOnInstallFinished(const std::string &bundleName, const std::string &abilityName,
        const std::string &startTime, const int &resultCode);

    std::mutex observerLock_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    std::vector<sptr<IFreeInstallObserver>> observerList_;
};

class FreeInstallObserverRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    explicit FreeInstallObserverRecipient(RemoteDiedHandler handler);
    virtual ~FreeInstallObserverRecipient();
    virtual void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    RemoteDiedHandler handler_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_FREE_INSTALL_OBSERVER_MANAGER_H