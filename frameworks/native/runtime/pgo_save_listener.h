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

#ifndef OHOS_ABILITY_RUNTIME_PGO_SAVE_LISTENER_H
#define OHOS_ABILITY_RUNTIME_PGO_SAVE_LISTENER_H

#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"

namespace OHOS {
namespace AppExecFwk {
class PgoSaveListener : public EventFwk::CommonEventSubscriber {
public:
    explicit PgoSaveListener(const std::string &bundleName, const EventFwk::CommonEventSubscribeInfo &subscribeInfo);
    virtual ~PgoSaveListener() = default;
    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
private:
    std::string bundleName_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PGO_SAVE_LISTENER_H
