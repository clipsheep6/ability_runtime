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

#ifndef OHOS_ABILITY_RUNTIME_APPFREEZE_STATE_H
#define OHOS_ABILITY_RUNTIME_APPFREEZE_STATE_H

#include <memory>

#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
class AppFreezeState : public DelayedSingleton<AppFreezeState>,
    public std::enable_shared_from_this<AppFreezeState> {
    DISALLOW_COPY_AND_MOVE(AppFreezeState);
public:
    AppFreezeState();
    virtual ~AppFreezeState() = default;

    void SetAppFreezeState(uint32_t flag);
    void CancelAppFreezeState(uint32_t flag);

    enum AppFreezeFlag {
        DEBUG_LAUNCH_MODE = 1,
        ATTACH_DEBUG_MODE = 1 << 1,
        ASSERT_DEBUG_MODE = 1 << 2,
    };
private:
    uint32_t appFreezeStateFlag_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APPFREEZE_STATE_H
