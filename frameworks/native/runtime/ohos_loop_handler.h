/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_OHOS_LOOP_HANDLER_H
#define OHOS_ABILITY_RUNTIME_OHOS_LOOP_HANDLER_H

#include <sys/epoll.h>
#include "event_handler.h"
#include "hilog_wrapper.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr char TIMER_TASK[] = "uv_timer_task";

class OHOSLoopHandler : public AppExecFwk::FileDescriptorListener,
    public std::enable_shared_from_this<OHOSLoopHandler> {
public:
    explicit OHOSLoopHandler(uv_loop_t* uvLoop) : uvLoop_(uvLoop) {}
    void OnReadable(int32_t) override;
    void OnWritable(int32_t) override;

private:
    void OnTriggered();

private:
    uv_loop_t* uvLoop_ = nullptr;
    int64_t lastTimeStamp_ = 0;
    bool haveTimerTask_ = false;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_OHOS_LOOP_HANDLER_H
