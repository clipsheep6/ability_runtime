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

#ifndef _AMS_ST_ABILITY_R1_
#define _AMS_ST_ABILITY_R1_
#include "stpageabilityevent.h"
#include <string>
#include <thread>
#include "ability_loader.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
class AmsStAbilityR1 : public Ability {
protected:
    virtual void Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
        const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;
    virtual void OnForeground(const Want &want) override;
    virtual void OnNewWant(const Want &want) override;

private:
    void Clear();
    void GetWantInfo(const Want &want);
    std::string Split(std::string &str, std::string delim);

    std::string shouldReturn;
    std::string targetBundle;
    std::string targetAbility;
    STPageAbilityEvent pageAbilityEvent;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // _AMS_ST_ABILITY_R1_