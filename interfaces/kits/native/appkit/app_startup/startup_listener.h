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

#ifndef OHOS_ABILITY_RUNTIME_STARTUP_LISTENER_H
#define OHOS_ABILITY_RUNTIME_STARTUP_LISTENER_H

#include <functional>

#include "ability_manager_errors.h"
#include "startup_task_result.h"

namespace OHOS {
namespace AbilityRuntime {
using OnCompletedCallback = std::function<void(const StartupTaskResult &)>;

class StartupListener {
public:
    explicit StartupListener(const OnCompletedCallback &callback);

    ~StartupListener();

    void OnCompleted(const StartupTaskResult &result);

private:
    OnCompletedCallback onCompletedCallback_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STARTUP_LISTENER_H
