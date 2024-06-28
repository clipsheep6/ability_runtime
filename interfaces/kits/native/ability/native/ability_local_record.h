/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_LOCAL_RECORD_H
#define OHOS_ABILITY_RUNTIME_ABILITY_LOCAL_RECORD_H

#include <string>

#include "iremote_object.h"
#include "event_runner.h"
#include "ability_info.h"
#include "application_info.h"
#include "refbase.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityThread;
class AbilityImpl;
class AbilityLocalRecord {
public:
    /**
     *
     * default constructor
     *
     */
    AbilityLocalRecord(const std::shared_ptr<AbilityInfo> &info, const sptr<IRemoteObject> &token);

    /**
     *
     * @default Destructor
     *
     */
    virtual ~AbilityLocalRecord();

    /**
     * @description: Get an AbilityInfo in an ability.
     *
     * @return Returns a pointer to abilityinfo.
     */
    const std::shared_ptr<AbilityInfo> &GetAbilityInfo();

    /**
     * @description: Get an EventHandler in an ability.
     *
     * @return Returns a pointer to EventHandler
     */
    const std::shared_ptr<EventHandler> &GetEventHandler();

    /**
     * @description: Set an EventHandler in an ability.
     * @param handler EventHandler object
     * @return None.
     */
    void SetEventHandler(const std::shared_ptr<EventHandler> &handler);

    /**
     * @description: Get an EventRunner in an ability.
     *
     * @return Returns a pointer to EventRunner
     */
    const std::shared_ptr<EventRunner> &GetEventRunner();

    /**
     * @description: Set an EventRunner in an ability.
     * @param runner EventHandler object
     * @return None.
     */
    void SetEventRunner(const std::shared_ptr<EventRunner> &runner);

    /**
     * @description: Gets the identity of the ability
     * @return return the identity of the ability.
     */
    const sptr<IRemoteObject> &GetToken();

    int32_t GetAbilityRecordId() const
    {
        return abilityRecordId_;
    }
    void SetAbilityRecordId(int32_t abilityRecordId)
    {
        abilityRecordId_ = abilityRecordId;
    }

    /**
     * @description: Obtains the information based on ability thread.
     * @return return AbilityThread Pointer
     */
    const sptr<AbilityThread> &GetAbilityThread();

    /**
     * @description: Set an AbilityThread in an ability.
     * @param abilityThread AbilityThread object
     * @return None.
     */
    void SetAbilityThread(const sptr<AbilityThread> &abilityThread);

    void SetWant(const std::shared_ptr<AAFwk::Want> &want);

    const std::shared_ptr<AAFwk::Want> &GetWant();
private:
    std::shared_ptr<AbilityInfo> abilityInfo_ = nullptr;
    sptr<IRemoteObject> token_;
    int32_t abilityRecordId_ = 0;
    std::shared_ptr<EventRunner> runner_ = nullptr;
    std::shared_ptr<EventHandler> handler_ = nullptr;
    std::shared_ptr<AbilityImpl> abilityImpl_ = nullptr;  // store abilityImpl
    sptr<AbilityThread> abilityThread_;
    std::shared_ptr<AAFwk::Want> want_ = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_LOCAL_RECORD_H
