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

#ifndef OHOS_ABILITY_RUNTIME_UNLOCK_SCREEN_MANAGER_H
#define OHOS_ABILITY_RUNTIME_UNLOCK_SCREEN_MANAGER_H

#include "nocopyable.h"

#ifdef SUPPORT_POWER
#include "power_mgr_client.h"
#endif

#ifdef SUPPORT_GRAPHICS
#include "unlock_screen_callback.h"
#include "screenlock_manager.h"
#include "screenlock_common.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
class UnlockScreenManager {
public:
    static UnlockScreenManager &GetInstance();
    ~UnlockScreenManager();
    bool UnlockScreen();
private:
    UnlockScreenManager();
    DISALLOW_COPY_AND_MOVE(UnlockScreenManager);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UNLOCK_SCREEN_MANAGER_H