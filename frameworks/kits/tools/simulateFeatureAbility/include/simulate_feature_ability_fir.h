/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef _SIMULATE_FEATURE_ABILITY_FIR_
#define _SIMULATE_FEATURE_ABILITY_FIR_
#include "ability_loader.h"
namespace OHOS {
namespace AppExecFwk {
class SimulateFeatureAbilityFir : public Ability {
protected:
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  //_SIMULATE_FEATURE_ABILITY_FIR_