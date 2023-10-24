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

#ifndef MOCK_ABILITY_MANAGER_COLLABORATOR_H
#define MOCK_ABILITY_MANAGER_COLLABORATOR_H

#include <gmock/gmock.h>
#include "iability_manager_collaborator.h"

namespace OHOS {
namespace AAFwk {
class MockAbilityManagerCollaborator : public IAbilityManagerCollaborator {
public:
    MOCK_METHOD4(NotifyStartAbility, int32_t(const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t userId, Want &want, uint64_t accessTokenIDEx));
    MOCK_METHOD1(NotifyPreloadAbility, int32_t(const std::string &bundleName));
    MOCK_METHOD2(NotifyMissionCreated, int32_t(int32_t missionId, const Want &want));
    MOCK_METHOD1(NotifyMissionCreated, int32_t(const sptr<SessionInfo> &sessionInfo));
    MOCK_METHOD3(NotifyLoadAbility, int32_t(const AppExecFwk::AbilityInfo &abilityInfo, int32_t missionId,
        const Want &want));
    MOCK_METHOD2(NotifyLoadAbility, int32_t(const AppExecFwk::AbilityInfo &abilityInfo,
        const sptr<SessionInfo> &sessionInfo));
    MOCK_METHOD1(NotifyMoveMissionToBackground, int32_t(int32_t missionId));
    MOCK_METHOD1(NotifyMoveMissionToForeground, int32_t(int32_t missionId));
    MOCK_METHOD1(NotifyTerminateMission, int32_t(int32_t missionId));
    MOCK_METHOD1(NotifyClearMission, int32_t(int32_t missionId));
    MOCK_METHOD3(NotifyRemoveShellProcess, int32_t(int32_t pid, int32_t type, const std::string &reason));
    MOCK_METHOD1(UpdateMissionInfo, void(InnerMissionInfoDto &info));
    MOCK_METHOD1(UpdateMissionInfo, void(sptr<SessionInfo> &sessionInfo));
    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
};
} // AAFwk
} // OHOS
#endif // MOCK_ABILITY_MANAGER_COLLABORATOR_H