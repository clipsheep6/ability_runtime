/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MISSION_H
#define OHOS_ABILITY_RUNTIME_MISSION_H

#include <memory>

#include "ability_record.h"
#include "inner_mission_info.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class Mission
 * a mission only contains an AbilityRecord
 */
class Mission : public std::enable_shared_from_this<Mission> {
public:
    Mission(int32_t id, const std::shared_ptr<AbilityRecord> abilityRecord, const std::string &missionName = "",
        int32_t startMethod = 0);
    Mission(const std::shared_ptr<Mission> &mission);
    virtual ~Mission();

    /**
     * set the mission list.
     *
     * @param missionList: the parent mission list
     */
    void SetMissionList(const std::shared_ptr<MissionList> &missionList);

    /**
     * check whether ability contains by this mission is singleton.
     *
     * @return is ability contains by this mission is singleton.
     */
    bool IsSingletonAbility() const;

    /**
     * check whether ability contains by this mission is specified.
     *
     * @return is ability contains by this mission is specified.
     */
    bool IsSpecifiedAbility() const;

    /**
     * check whether ability contains by this mission is standard.
     *
     * @return is ability contains by this mission is standard.
     */
    bool IsStandardAbility() const;

    /**
     * get owner mission list.
     *
     * @return mission list.
     */
    std::shared_ptr<MissionList> GetMissionList();

    /**
     * get name of this mission.
     *
     * @return missionName.
     */
    std::string GetMissionName() const;

    /**
     * @brief Get the Ability Record object
     *
     * @return std::shared_ptr<AbilityRecord>
     */
    std::shared_ptr<AbilityRecord> GetAbilityRecord() const;

    /**
     * @brief Get the mission id
     *
     * @return the mission id
     */
    int32_t GetMissionId() const;

    /**
     * @brief Set the Locked State
     *
     * @param lockedState true/false
     */
    void SetLockedState(bool lockedState);

    /**
     * @brief get the Locked State
     *
     * @return the lockedState
     */
    bool IsLockedState() const;

    /**
     * @brief Set the Moving State
     *
     * @param movingState true/false
     */
    void SetMovingState(bool movingState);

    /**
     * @brief get the Moving State
     *
     * @return the movingState
     */
    bool IsMovingState() const;

    /**
     * @brief Set application not response state true
     */
    void SetANRState(bool state);

    /**
     * @brief Is application not response state
     */
    bool IsANRState() const;

    /**
     * @brief dump mission
     *
     * @param info dump result.
     */
    void Dump(std::vector<std::string> &info);

    /**
     * @brief whether it is a form ByCall start-up
     *
     * @return true form BaCall start-up, false other
     */
    bool IsStartByCall();

    /**
     * @brief update mission id
     *
     * @param id mission id.
     * @param method start method.
     * @return Returns true on success, false on failure
     */
    bool UpdateMissionId(int32_t id, int32_t method);

    /**
     * Set whether to notify Launcher that the mission has been created.
     *
     * @param needNotify Indicates whether the Launcher needs to be notified.
     */
    inline void SetNotifyLabel(bool needNotify)
    {
        needNotify_ = needNotify;
    }

    /**
     * Get whether to notify Launcher that the mission has been created.
     *
     * @param return Whether the Launcher needs to be notified.
     */
    inline bool NeedNotify() const
    {
        return needNotify_;
    }

    /**
     * Set mission specified flag.
     *
     * @param flag specified flag.
     */
    void SetSpecifiedFlag(const std::string &flag);

    /**
     * Get mission specified flag.
     *
     * @return specified flag.
     */
    std::string GetSpecifiedFlag() const;

    inline void SetNeedNotifyUpdateLabel(bool flag)
    {
        needNotifyUpdateLabel_ = flag;
    }

    inline bool NeedNotifyUpdateLabel() const
    {
        return needNotifyUpdateLabel_;
    }

    inline void UpdateMissionTime(const std::string &missionTime)
    {
        missionTime_ = missionTime;
    }

    inline std::string GetMissionTime() const
    {
        return missionTime_;
    }

private:
    int32_t missionId_;
    int32_t startMethod_;
    std::shared_ptr<AbilityRecord> abilityRecord_;
    std::string missionName_;
    std::string specifiedFlag_;
    std::weak_ptr<MissionList> ownerMissionList_;
    bool lockedState_ = false;
    bool isMovingToFront_ = false;
    bool isANRState_ = false;
    bool needNotify_ = true;
    bool needNotifyUpdateLabel_ = false;
    std::string missionTime_ = "0";
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MISSION_H
