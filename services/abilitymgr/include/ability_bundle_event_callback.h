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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_BUNDLE_EVENT_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_ABILITY_BUNDLE_EVENT_CALLBACK_H

#include "bundle_event_callback_host.h"
#include "common_event_support.h"
#include "ability_event_handler.h"
#include "ability_event_util.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
/**
 * @brief This class is a callback class that will be registered to BundleManager.
 * This class will be called by BundleManager when install, uninstall, updates of haps happens,
 * and executes corresponding functionalities of ability manager.
 */
class AbilityBundleEventCallback : public AppExecFwk::BundleEventCallbackHost {
public:
    AbilityBundleEventCallback();
    explicit AbilityBundleEventCallback(std::shared_ptr<AbilityEventHandler> eventHandler);

    ~AbilityBundleEventCallback() = default;

    /**
     * @brief The main callback function that will be called by BundleManager
     * when install, uninstall, updates of haps happens to notify AbilityManger.
     * @param eventData the data passed from BundleManager that includes bundleName, change type of haps
     * etc. More can be found from BundleCommonEventMgr::NotifyBundleStatus()
     */
    void OnReceiveEvent(const EventFwk::CommonEventData eventData) override;

private:
    DISALLOW_COPY_AND_MOVE(AbilityBundleEventCallback);
    AbilityEventUtil abilityEventHelper_;
    std::shared_ptr<AbilityEventHandler> eventHandler_ = nullptr;
};
} // namespace OHOS
} // namespace AAFwk
#endif // OHOS_ABILITY_RUNTIME_ABILITY_BUNDLE_EVENT_CALLBACK_H