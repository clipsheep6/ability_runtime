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

#ifndef OHOS_ABILITY_RUNTIME_STARTUP_CONFIG_H
#define OHOS_ABILITY_RUNTIME_STARTUP_CONFIG_H

#include <memory>

#include "startup_listener.h"
#include "startup_task_result.h"
#include "startup_utils.h"

namespace OHOS {
namespace AbilityRuntime {
class StartupConfig {
public:
    StartupConfig();

    virtual ~StartupConfig();

    explicit StartupConfig(int32_t awaitTimeoutMs);

    explicit StartupConfig(const std::shared_ptr<StartupListener> &listener);

    explicit StartupConfig(int32_t awaitTimeoutMs, const std::shared_ptr<StartupListener> &listener);

    virtual int32_t Init();

    int32_t GetAwaitTimeoutMs() const;

    int32_t ListenerOnCompleted(const std::shared_ptr<StartupTaskResult> &result);

protected:
    static constexpr int32_t DEFAULT_AWAIT_TIMEOUT_MS = 10000; // 10s
    int32_t awaitTimeoutMs_ = DEFAULT_AWAIT_TIMEOUT_MS;
    std::shared_ptr<StartupListener> listener_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STARTUP_CONFIG_H
