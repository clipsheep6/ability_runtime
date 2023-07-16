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
#ifndef ABILITY_MANAGER_COLLABORATOR_PROXY_H
#define ABILITY_MANAGER_COLLABORATOR_PROXY_H

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
     * @brief Notify Broker to StartAbility.
     * @param AbilityInfo ability info from bms
     * @param userId userId.
     * @param want targert info, will modify by broker.
     * @param accessTokenIDEx accessToken
     * @return Returns 0 means in container, other return.
    */
    virtual int32_t NotifyStartAbility(const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t userId, Want &want, uint64_t accessTokenIDEx) override;
    
    /**
     * @brief Notify when mission is created.
     * @param missionId missionId.
     * @param want target info.
     * @return 0 or else.
    */
    virtual int32_t NotifyMissionCreated(int32_t missionId, const Want &want) override;

    /**
     * @brief Notify when start loading ability record.
     * @param AbilityInfo ability info from bms.
     * @param missionId missionId.
     * @param want target info.
     * @return 0 or else.
    */
    virtual int32_t NotifyLoadAbility(const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t missionId, const Want &want) override;

    /**
     * @brief Notify when notify app to background.
     * @param missionId missionId.
     * @return 0 or else.
    */
    virtual int32_t NotifyMoveMissionToBackground(int32_t missionId) override;

    /**
     * @brief Notify when notify app to foreground.
     * @param missionId missionId.
     * @return 0 or else.
    */
    virtual int32_t NotifyMoveMissionToForeground(int32_t missionId) override;

    /**
     * @brief Notify when notify ability is terminated, but mission is not cleared.
     * @param missionId missionId.
     * @return 0 or else.
    */
    virtual int32_t NotifyTerminateMission(int32_t missionId) override;

    /**
     * @brief Notify to broker when clear mission.
     * @param missionId missionId.
     * @return 0 or else.
    */
    virtual int32_t NotifyClearMission(int32_t missionId) override;

    /**
     * @brief Notify to broker when clear mission.
     * @param pid pid of shell process.
     * @param type died type.
     * @param reason addational message for died reason.
     * @return 0 or else.
    */
    virtual int32_t NotifyRemoveShellProcess(int32_t pid, int32_t type, const std::string &reason) override;

private:
    static inline BrokerDelegator<AbilityManagerCollaboratorProxy> delegator_;
    bool WriteInterfaceToken(MessageParcel &data);
};
}   // namespace AAFWK
}   // namespace OHOS
#endif // ABILITY_MANAGER_COLLABORATOR_PROXY_H