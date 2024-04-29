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

#ifndef OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_SERVICE_ABILITY_H
#define OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_SERVICE_ABILITY_H

#include "quick_fix_manager_service.h"
#include "system_ability.h"

namespace OHOS {
namespace AAFwk {
class QuickFixManagerServiceAbility final : public SystemAbility {
public:
    DISALLOW_COPY_AND_MOVE(QuickFixManagerServiceAbility);
    DECLARE_SYSTEM_ABILITY(QuickFixManagerServiceAbility);

    QuickFixManagerServiceAbility(const int32_t systemAbilityId, bool runOnCreate);
    ~QuickFixManagerServiceAbility();

private:
    void OnStart() override;
    void OnStop() override;

    sptr<QuickFixManagerService> service_ = nullptr;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_SERVICE_ABILITY_H
