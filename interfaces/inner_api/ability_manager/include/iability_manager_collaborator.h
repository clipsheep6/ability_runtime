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

#ifndef OHOS_ABILITY_RUNTIME_IABILITY_MANAGER_COLLABORATOR_H
#define OHOS_ABILITY_RUNTIME_IABILITY_MANAGER_COLLABORATOR_H

#include "ability_info.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {

class IAbilityManagerCollaborator : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.IAbilityManagerCollaborator");

    /**
     * @brief Notify collaborator to StartAbility.
     * @param AbilityInfo ability info from bms
     * @param userId userId.
     * @param want targert info, will modify by collaborator.
     * @return 0 or else.
    */
    virtual int32_t NotifyStartAbility(const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t userId, Want &want, uint64_t accessTokenIDEx)
    {
        return 0;
    }
    
    /**
     * @brief Notify when mission is created.
     * @param missionId missionId.
     * @param want target info.
     * @return 0 or else.
    */
    virtual int32_t NotifyMissionCreated(int32_t missionId, const Want &want)
    {
        return 0;
    }

    /**
     * @brief Notify when start loading ability record.
     * @param AbilityInfo ability info from bms.
     * @param missionId missionId.
     * @param want target info.
     * @return 0 or else.
    */
    virtual int32_t NotifyLoadAbility(
        const AppExecFwk::AbilityInfo &abilityInfo, int32_t missionId, const Want &want)
    {
        return 0;
    }

    /**
     * @brief Notify when notify app to background.
     * @param missionId missionId.
     * @return 0 or else.
    */
    virtual int32_t NotifyMoveMissionToBackground(int32_t missionId)
    {
        return 0;
    }

    /**
     * @brief Notify when notify app to foreground.
     * @param missionId missionId.
     * @return 0 or else.
    */
    virtual int32_t NotifyMoveMissionToForeground(int32_t missionId)
    {
        return 0;
    }

    /**
     * @brief Notify when notify ability is terminated, but mission is not cleared.
     * @param missionId missionId.
     * @return 0 or else.
    */
    virtual int32_t NotifyTerminateMission(int32_t missionId)
    {
        return 0;
    }

    /**
     * @brief Notify to broker when clear mission.
     * @param missionId missionId.
     * @return 0 or else.
    */
    virtual int32_t NotifyClearMission(int32_t missionId)
    {
        return 0;
    }

    /**
     * @brief Notify to broker when clear mission.
     * @param pid pid of shell process.
     * @param type died type.
     * @param reason addational message for died reason.
     * @return 0 or else.
    */
    virtual int32_t NotifyRemoveShellProcess(int32_t pid, int32_t type, const std::string &reason)
    {
        return 0;
    }

    // virtual int32_t UpdateMissionInfo(InnerMissionInfo &info)
    // {
    //     return 0;
    // }

    enum {
        NOTIFY_START_ABILITY = 1,
        NOTIFY_MISSION_CREATED,
        NOTIFY_LOAD_ABILITY,
        NOTIFY_MOVE_MISSION_TO_BACKGROUND,
        NOTIFY_MOVE_MISSION_TO_FOREGROUND,
        NOTIFY_TERMINATE_MISSION,
        NOTIFY_CLEAR_MISSION,
        NOTIFY_REMOVE_SHELL_PROCESS,
    };
};
}
}
#endif // OHOS_ABILITY_RUNTIME_IABILITY_MANAGER_COLLABORATOR_H