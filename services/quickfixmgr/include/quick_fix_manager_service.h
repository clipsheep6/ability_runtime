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

#ifndef OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_SERVICE_H
#define OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_SERVICE_H

#include "singleton.h"
#include "system_ability.h"
#include "quick_fix_manager_stub_impl.h"

namespace OHOS {
namespace AAFwk {
class QuickFixManagerService : public SystemAbility {
    DECLARE_DELAYED_SINGLETON(QuickFixManagerService);
    DECLARE_SYSTEM_ABILITY(QuickFixManagerService);

public:
    void OnStart() override;
    void OnStop() override;

private:
    DISALLOW_COPY_AND_MOVE(QuickFixManagerService);

    bool Init();

    bool running_ = false;
    sptr<QuickFixManagerStubImpl> quickFixStubImpl_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_SERVICE_H
