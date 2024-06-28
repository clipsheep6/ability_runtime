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
#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_COLLABORATOR_PROXY_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_COLLABORATOR_PROXY_H

#include "iability_manager_collaborator.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerCollaboratorProxy : public IRemoteProxy<IAbilityManagerCollaborator> {
public:
    explicit AbilityManagerCollaboratorProxy(const sptr<IRemoteObject> &impl)
        : IRemoteProxy<IAbilityManagerCollaborator>(impl)
    {}
    virtual ~AbilityManagerCollaboratorProxy() = default;

    /**
     * @brief Notify collaborator to StartAbility.
     * @param AbilityInfo ability info from bms
     * @param userId userId.
     * @param want targert info, will modify by collaborator.
     * @param accessTokenIDEx accessToken
     * @return 0 when notify start ability success or else failed.
     */
    virtual int32_t NotifyStartAbility(const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t userId, Want &want, uint64_t accessTokenIDEx) override;

    /**
     * @brief Notify collaborator to app preload.
     * @param bundleName bundlName.
     * @return 0 means success or else failed.
     */
    virtual int32_t NotifyPreloadAbility(const std::string &bundleName) override;

    /**
     * @brief Notify when mission is created.
     * @param missionId missionId.
     * @param want target info.
     * @return 0 when notify mission created success or else failed.
     */
    virtual int32_t NotifyMissionCreated(int32_t missionId, const Want &want) override;

    /**
     * @brief Notify when mission is created.
     * @param sessionInfo sessionInfo.
     * @return 0 when notify mission created success or else failed.
     */
    virtual int32_t NotifyMissionCreated(const sptr<SessionInfo> &sessionInfo) override;

    /**
     * @brief Notify when start loading ability record.
     * @param AbilityInfo ability info from bms.
     * @param missionId missionId.
     * @param want target info.
     * @return 0 when notify load ability success or else failed.
     */
    virtual int32_t NotifyLoadAbility(const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t missionId, const Want &want) override;

    /**
     * @brief Notify when start loading ability record.
     * @param AbilityInfo ability info from bms.
     * @param sessionInfo sessionInfo.
     * @return 0 when notify load ability success or else failed.
    */
    virtual int32_t NotifyLoadAbility(
        const AppExecFwk::AbilityInfo &abilityInfo, const sptr<SessionInfo> &sessionInfo) override;

    /**
     * @brief Notify when notify app to background.
     * @param missionId missionId.
     * @return 0 when notify move mission to background success or else failed.
     */
    virtual int32_t NotifyMoveMissionToBackground(int32_t missionId) override;

    /**
     * @brief Notify when notify app to foreground.
     * @param missionId missionId.
     * @return 0 when notify move mission to foreground success or else failed.
     */
    virtual int32_t NotifyMoveMissionToForeground(int32_t missionId) override;

    /**
     * @brief Notify when notify ability is terminated, but mission is not cleared.
     * @param missionId missionId.
     * @return 0 when notify terminate mission success or else failed.
     */
    virtual int32_t NotifyTerminateMission(int32_t missionId) override;

    /**
     * @brief Notify to broker when clear mission.
     * @param missionId missionId.
     * @return 0 when notify clear mission success or else failed.
     */
    virtual int32_t NotifyClearMission(int32_t missionId) override;

    /**
     * @brief Notify to broker when clear mission.
     * @param pid pid of shell process.
     * @param type died type.
     * @param reason addational message for died reason.
     * @return 0 when notify remove shell process success or else failed.
     */
    virtual int32_t NotifyRemoveShellProcess(int32_t pid, int32_t type, const std::string &reason) override;

    /**
     * @brief Update mission info to real element by broker.
     * @param info info of mission.
     */
    virtual void UpdateMissionInfo(InnerMissionInfoDto &info) override;

    /**
     * @brief Update mission info to real element by broker.
     * @param sessionInfo sessionInfo.
     */
    virtual void UpdateMissionInfo(sptr<SessionInfo> &sessionInfo) override;

    /**
     * @brief Check the call permission from shell assistant.
     * @param want target info.
     * @return 0 when check permission success or else failed.
     */
    virtual int32_t CheckCallAbilityPermission(const Want &want) override;

    /**
     * @brief Notify application update system environment changes.
     * @param config System environment change parameters.
     * @param userId userId Designation User ID.
     * @return Return true to notify changes successfully, or false to failed.
     */
    virtual bool UpdateConfiguration(const AppExecFwk::Configuration &config, int32_t userId) override;

    /**
     * @brief Open file by uri.
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @return int The file descriptor.
     */
    virtual int OpenFile(const Uri& uri, uint32_t flag) override;

    virtual void NotifyMissionBindPid(int32_t missionId, int32_t pid) override;
private:
    static inline BrokerDelegator<AbilityManagerCollaboratorProxy> delegator_;
    int32_t SendTransactCmd(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
};
}   // namespace AAFWK
}   // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_COLLABORATOR_PROXY_H
