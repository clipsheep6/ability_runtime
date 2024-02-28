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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_SERVICE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_SERVICE_H

#include <map>
#include <mutex>
#include <vector>

#include "auto_startup_info.h"
#include "bundle_mgr_client.h"
#include "iremote_object.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityAutoStartupService : public std::enable_shared_from_this<AbilityAutoStartupService> {
public:
    explicit AbilityAutoStartupService();

    virtual ~AbilityAutoStartupService();

    /**
     * @brief Register auto start up callback for system api.
     * @param callback The point of JsAbilityAutoStartupCallBack.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback);

    /**
     * @brief Unregister auto start up callback for system api.
     * @param callback The point of JsAbilityAutoStartupCallBack.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback);

    /**
     * @brief Set every application auto start up state.
     * @param info The auto startup info,include bundle name, module name, ability name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetApplicationAutoStartup(const AutoStartupInfo &info);

    /**
     * @brief Cancel every application auto start up .
     * @param info The auto startup info,include bundle name, module name, ability name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t CancelApplicationAutoStartup(const AutoStartupInfo &info);

    /**
     * @brief Query auto startup state all application.
     * @param infoList Output parameters, return auto startup info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList);

    /**
     * @brief Query auto startup state all application without permission.
     * @param infoList Output parameters, return auto startup info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t QueryAllAutoStartupApplicationsWithoutPermission(std::vector<AutoStartupInfo> &infoList);

    /**
     * @brief Delete current bundleName auto start up data.
     * @param bundleName The current bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t DeleteAutoStartupData(const std::string &bundleName);

    /**
     * @brief Check current bundleName auto start up data.
     * @param bundleName The current bundleName.
     * @param uid The uid.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t CheckAutoStartupData(const std::string &bundleName, int32_t uid);

    /**
     * @brief Set application auto start up state by EDM.
     * @param info The auto startup info, include bundle name, module name, ability name.
     * @param flag Indicate whether to allow the application to change the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag);

    /**
     * @brief Cancel application auto start up state by EDM.
     * @param info The auto startup info, include bundle name, module name, ability name.
     * @param flag Indicate whether to allow the application to change the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag);

    /**
     * @brief Set applications auto start up state by EDM.
     * @param infoList The auto startup info vector list.
     * @param flag Indicate whether to allow the application to change the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetApplicationAutoStartupByEDM(const std::vector<AutoStartupInfo> &infoList, bool flag);

    /**
     * @brief Cancel applications auto start up state by EDM.
     * @param infoList The auto startup info vector list.
     * @param flag Indicate whether to allow the application to change the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t CancelApplicationAutoStartupByEDM(const std::vector<AutoStartupInfo> &infoList, bool flag);

    /**
     * @class ClientDeathRecipient
     * notices IRemoteBroker died.
     */
    class ClientDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        /**
         * @brief Constructor
         */
        explicit ClientDeathRecipient(const std::weak_ptr<AbilityAutoStartupService> &weakPtr);
        virtual ~ClientDeathRecipient() = default;
        /**
         * @brief handle remote object died event.
         * @param remote remote object.
         */
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        std::weak_ptr<AbilityAutoStartupService> weakPtr_;
    };

private:
    int32_t InnerSetApplicationAutoStartup(const AutoStartupInfo &info);
    int32_t InnerCancelApplicationAutoStartup(const AutoStartupInfo &info);
    void ExecuteCallbacks(bool isCallOn, const AutoStartupInfo &info);
    void SetDeathRecipient(
        const sptr<IRemoteObject> &callback, const sptr<IRemoteObject::DeathRecipient> &deathRecipient);
    void CleanResource(const wptr<IRemoteObject> &remote);
    std::string GetSelfApplicationBundleName();
    bool CheckSelfApplication(const std::string &bundleName);
    bool GetBundleInfo(const std::string &bundleName, AppExecFwk::BundleInfo &bundleInfo, int32_t uid = -1);
    bool GetAbilityData(const AutoStartupInfo &info, bool &isVisible, std::string &abilityTypeName);
    std::string GetAbilityTypeName(AppExecFwk::AbilityInfo abilityInfo);
    std::string GetExtensionTypeName(AppExecFwk::ExtensionAbilityInfo extensionInfo);
    std::shared_ptr<AppExecFwk::BundleMgrClient> GetBundleMgrClient();
    int32_t CheckPermissionForSystem();
    int32_t CheckPermissionForSelf(const std::string &bundleName);
    int32_t CheckPermissionForEDM();
    int32_t InnerApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool isSet, bool flag);
    int32_t GetAbilityInfo(const AutoStartupInfo &info, std::string &abilityTypeName);

    mutable std::mutex autoStartUpMutex_;
    mutable std::mutex deathRecipientsMutex_;
    std::vector<sptr<IRemoteObject>> callbackVector_;
    std::map<std::string, sptr<IRemoteObject>> callbackMaps_;
    std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>> deathRecipients_;
    std::shared_ptr<AppExecFwk::BundleMgrClient> bundleMgrClient_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_SERVICE_H