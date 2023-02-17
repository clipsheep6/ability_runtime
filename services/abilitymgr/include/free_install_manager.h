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

#ifndef OHOS_ABILITY_RUNTIME_FREE_INSTALL_MANAGER_H
#define OHOS_ABILITY_RUNTIME_FREE_INSTALL_MANAGER_H

#include <future>

#include <iremote_object.h>
#include <iremote_stub.h>

#include "ability_info.h"
#include "free_install_observer_manager.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerService;

/**
 * Store StartAbility info 
 */
struct FreeInstallInfo {
    Want want;
    int32_t userId = -1;
    int32_t requestCode = -1;
    bool isInstalled = false;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject> dmsCallback = nullptr;
};

/**
 * @class FreeInstallManager
 * FreeInstallManager.
 */
class FreeInstallManager : public std::enable_shared_from_this<FreeInstallManager> {
public:
    explicit FreeInstallManager(const std::weak_ptr<AbilityManagerService> &server);
    virtual ~FreeInstallManager() = default;

    /**
     * OnInstallFinished, StartFreeInstall is complete.
     *
     * @param resultCode, ERR_OK on success, others on failure.
     * @param want, installed ability.
     * @param userId, user`s id.
     */
    void OnInstallFinished(int resultCode, const Want &want, int32_t userId);

    /**
     * OnRemoteInstallFinished, DMS has finished.
     *
     * @param resultCode, ERR_OK on success, others on failure.
     * @param want, installed ability.
     * @param userId, user`s id.
     */
    void OnRemoteInstallFinished(int resultCode, const Want &want, int32_t userId);

    /**
     * Start to free install.
     *
     * @param want, the want of the ability to free install.
     * @param userId, designation User ID.
     * @param requestCode, ability request code.
     * @param callerToken, caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartFreeInstall(const Want &want, int32_t userId, int requestCode, const sptr<IRemoteObject> &callerToken);

    /**
     * Start to remote free install.
     *
     * @param want, the want of the ability to free install.
     * @param requestCode, ability request code.
     * @param validUserId, designation User ID.
     * @param callerToken, caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartRemoteFreeInstall(const Want &want, int requestCode, int32_t validUserId,
        const sptr<IRemoteObject> &callerToken);

    /**
     * Start to free install from another devices.
     * The request is send from DMS.
     *
     * @param want, the want of the ability to free install.
     * @param callback, used to notify caller the result of free install.
     * @param userId, designation User ID.
     * @param requestCode, ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    int FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
        int32_t userId, int requestCode);

    /**
     * Connect if the request is free install.
     * @param want, the want of the ability to free install.
     * @param userId, designation User ID.
     * @param callerToken, caller ability token.
     * @param localDeviceId, the device id of local.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ConnectFreeInstall(const Want &want, int32_t userId, const sptr<IRemoteObject> &callerToken,
        const std::string& localDeviceId);

    void PostUpgradeAtomicServiceTask(int resultCode, const Want &want, int32_t userId);

    int AddFreeInstallObserver(const sptr<AbilityRuntime::IFreeInstallObserver> &observer);

    void OnRemoveTimeoutTask(const Want &want);

private:
    std::weak_ptr<AbilityManagerService> server_;
    std::vector<FreeInstallInfo> freeInstallList_;
    std::vector<FreeInstallInfo> dmsFreeInstallCbs_;
    std::map<std::string, std::time_t> timeStampMap_;
    std::mutex distributedFreeInstallLock_;
    std::mutex freeInstallListLock_;

    std::mutex freeInstallObserverLock_;
    /**
     * Start remote free install.
     *
     * @param want, the want of the ability to remote free install.
     * @param userId, designation User ID.
     * @param requestCode, ability request code.
     * @param callerToken, caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int RemoteFreeInstall(const Want &want, int32_t userId, int requestCode, const sptr<IRemoteObject> &callerToken);

    int NotifyDmsCallback(const Want &want, int resultCode);
    bool IsTopAbility(const sptr<IRemoteObject> &callerToken);
    void NotifyFreeInstallResult(const Want &want, int resultCode);
    FreeInstallInfo BuildFreeInstallInfo(const Want &want, int32_t userId, int requestCode,
        const sptr<IRemoteObject> &callerToken);
    std::time_t GetTimeStamp();

    void PostTimeoutTask(const Want &want);
    void HandleTimeoutTask(const std::string bundleName, const std::string abilityName, const std::string startTime);
    void RemoveTimeoutTask(const std::string bundleName, const std::string abilityName, const std::string startTime);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_FREE_INSTALL_MANAGER_H
