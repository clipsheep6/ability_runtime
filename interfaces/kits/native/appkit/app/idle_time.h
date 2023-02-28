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

#ifndef OHOS_ABILITY_RUNTIME_IDLE_TIME_H
#define OHOS_ABILITY_RUNTIME_IDLE_TIME_H

#include "event_handler.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AppExecFwk {
using IdleTimeCallback = std::function<void(int32_t)>;
class IdleTime : public std::enable_shared_from_this<IdleTime> {
public:
    IdleTime(const std::shared_ptr<EventHandler> &eventHandler, IdleTimeCallback idleTimeCallback);
    ~IdleTime() = default;
    void Start();

private:
    int64_t GetSysTimeNs();
    void OnVSync(int64_t nanoTimestamp, void* client);
    void RequestVSync();
    void EventTask();
    void PostTask();

    int64_t firstVSyncTime_ = 0;
    int64_t period_ = 0;
    int32_t continueFailCount_ = 0;
    int32_t successCount_ = 0;
    std::shared_ptr<EventHandler> eventHandler_;
    IdleTimeCallback callback_ = nullptr;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_IDLE_TIME_H