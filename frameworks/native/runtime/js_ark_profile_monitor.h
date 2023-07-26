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

#ifndef OHOS_ABILITY_RUNTIME_JS_ARK_PROFILE_MONITOR_H
#define OHOS_ABILITY_RUNTIME_JS_ARK_PROFILE_MONITOR_H

#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "js_runtime.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
class ArkProfileMonitor : public EventFwk::CommonEventSubscriber {
public:
    explicit ArkProfileMonitor(const EventFwk::CommonEventSubscribeInfo &info) : EventFwk::CommonEventSubscriber(info) {};
    void OnReceiveEvent(const EventFwk::CommonEventData &data) override
    {
        HILOG_ERROR("###ZZB### ArkProfileMonitor::OnReceiveEvent: code: %{public}d, data: %{public}s, want: %{public}s",
                    data.GetCode(), data.GetData().c_str(), data.GetWant().ToString().c_str());
        panda::JSNApi::SavePGOProfiler();
        // jsRuntime_->GetEcmaVm()->SavePGOProfiler();
    }
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif