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

#ifndef OHOS_ABILITY_ABILITY_RECOVERY_H
#define OHOS_ABILITY_ABILITY_RECOVERY_H

#include <memory>
#include <mutex>

#include "ability.h"
#include "ability_info.h"
#include "application_info.h"
#include "event_handler.h"
#include "iremote_object.h"
#include "parcel.h"
#include "recovery_param.h"
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityRecovery {
public:
    AbilityRecovery();
    virtual ~AbilityRecovery();
    void EnableAbilityRecovery(uint16_t restartFlag, uint16_t saveFlag, uint16_t saveMode);
    bool InitAbilityInfo(const std::shared_ptr<Ability>& ability,
        const std::shared_ptr<AbilityInfo>& abilityInfo, const sptr<IRemoteObject>& token);
    bool ScheduleSaveAbilityState(StateReason reason);
    bool ScheduleRecoverAbility(StateReason reason, const Want *want = nullptr);
    bool ScheduleRestoreAbilityState(StateReason reason, const Want &want);
    bool CallOnRestoreAbilityState(StateReason reason);
    bool PersistState();
    bool IsSameAbility(uintptr_t ability);
    void SetJsAbility(uintptr_t ability);
    std::string GetSavedPageStack(StateReason reason);
    uint16_t GetRestartFlag() const;
    uint16_t GetSaveOccasionFlag() const;
    uint16_t GetSaveModeFlag() const;

    wptr<IRemoteObject> GetToken() const
    {
        return token_;
    }
private:
    bool SaveAbilityState();
    bool SerializeDataToFile(int32_t savedStateId, AAFwk::WantParams& params);
    bool ReadSerializeDataFromFile(int32_t savedStateId, AAFwk::WantParams& params);
    bool LoadSavedState(StateReason reason);
    bool IsSaveAbilityState(StateReason reason);

    bool isEnable_;
    uint16_t restartFlag_;
    uint16_t saveOccasion_;
    uint16_t saveMode_;
    int32_t missionId_;
    std::weak_ptr<AppExecFwk::Ability> ability_;
    std::weak_ptr<AppExecFwk::AbilityInfo> abilityInfo_;
    uintptr_t jsAbilityPtr_;
    wptr<IRemoteObject> token_;
    std::string pageStack_;
    WantParams params_;
    Parcel parcel_;
    bool hasTryLoad_ = false;
    bool hasLoaded_ = false;
    std::mutex lock_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_ABILITY_RECOVERY_H