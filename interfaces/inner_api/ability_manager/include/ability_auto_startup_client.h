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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_CLIENT_H
#define OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_CLIENT_H

#include <mutex>

#include "ability_manager_errors.h"
#include "ability_manager_interface.h"
#include "auto_startup_info.h"
#include "iremote_object.h"

namespace OHOS {
namespace AAFwk {
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
/**
 * @class AbilityAutoStartupClient
 * AbilityAutoStartupClient is used to access ability manager services.
 */
class AbilityAutoStartupClient {
public:
    AbilityAutoStartupClient();
    virtual ~AbilityAutoStartupClient();
    static std::shared_ptr<AbilityAutoStartupClient> GetInstance();

    /**
     * Connect ability manager service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode Connect();
    /**
     * @brief Set application auto start up state by EDM.
     * @param info The auto startup info, include bundle name, module name, ability name.
     * @param flag Indicate whether to allow the application to change the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag);

    /**
     * @brief Cancel application auto start up state by EDM.
     * @param info The auto startup info, include bundle name, module name, ability name.
     * @param flag Indicate whether to allow the application to change the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag);

    /**
     * @brief Query all auto startup state applications.
     * @param infoList Output parameters, return auto startup info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList);

private:
    class AbilityMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        AbilityMgrDeathRecipient() = default;
        ~AbilityMgrDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    private:
        DISALLOW_COPY_AND_MOVE(AbilityMgrDeathRecipient);
    };

    sptr<IAbilityManager> GetAbilityManager();
    void ResetProxy(wptr<IRemoteObject> remote);

    static std::recursive_mutex mutex_;
    static std::shared_ptr<AbilityAutoStartupClient> instance_;
    sptr<IAbilityManager> proxy_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_CLIENT_H
