/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_UTIL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_UTIL_H

#include <memory>

#include "iremote_object.h"
#include "nocopyable.h"

namespace OHOS {
namespace AppExecFwk {
struct AbilityInfo;
struct RunningProcessInfo;
struct Skill;
}
namespace AAFwk {
class SubManagersHelper;
struct AbilityRequest;
class Want;
class AbilityPermissionUtil {
public:
    static AbilityPermissionUtil &GetInstance();

    void Init(std::shared_ptr<SubManagersHelper> subManagersHelper);
    
    // check caller is scb
    bool IsCallerSceneBoard();

    // check caller is delegator
    bool IsDelegatorCall(const AppExecFwk::RunningProcessInfo &processInfo, const AbilityRequest &abilityRequest) const;

    // check dominate screen
    bool IsDominateScreen(const Want &want, const sptr<IRemoteObject> &callerToken, bool isForegroundToRestartApp);

    // check Caller-Application is in background state
    // isBackgroundCall: TRUE indicates the Caller-Application is not in focus and not in foreground state.
    // backgroundJudgeFlag, background judge flag.
    bool IsCallFromBackground(const AbilityRequest &abilityRequest,
        bool &isBackgroundCall, bool backgroundJudgeFlag, bool isData = false);
    bool SetBackgroundCall(const AppExecFwk::RunningProcessInfo &processInfo,
        const AbilityRequest &abilityRequest, bool &isBackgroundCall, bool backgroundJudgeFlag) const;

    // check static cfg permission
    bool CheckOneSkillPermission(const AppExecFwk::Skill &skill, uint32_t tokenId);
    int CheckStaticCfgPermissionForSkill(const AbilityRequest &abilityRequest, uint32_t tokenId);
    int CheckStaticCfgPermissionForAbility(const AppExecFwk::AbilityInfo &abilityInfo, uint32_t tokenId);
    int CheckStaticCfgPermission(const AbilityRequest &abilityRequest, bool isStartAsCaller,
        uint32_t callerTokenId, bool isData = false, bool isSaCall = false, bool isImplicit = false);

private:
    AbilityPermissionUtil() = default;
    ~AbilityPermissionUtil() = default;

    std::shared_ptr<SubManagersHelper> subManagersHelper_;

    DISALLOW_COPY_AND_MOVE(AbilityPermissionUtil);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_UTIL_H