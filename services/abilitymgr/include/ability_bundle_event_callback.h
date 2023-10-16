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

#include "ability_auto_startup_service.h"
#include "bundle_event_callback_host.h"
#include "common_event_support.h"
#include "task_handler_wrap.h"
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
    explicit AbilityBundleEventCallback(std::shared_ptr<TaskHandlerWrap> taskHandler,
        std::shared_ptr<AbilityRuntime::AbilityAutoStartupService> abilityAutoStartupService);

    ~AbilityBundleEventCallback() = default;

    /**
     * @brief The main callback function that will be called by BundleManager
     * when install, uninstall, updates of haps happens to notify AbilityManger.
     * @param eventData the data passed from BundleManager that includes bundleName, change type of haps
     * etc. More can be found from BundleCommonEventMgr::NotifyBundleStatus()
     */
    void OnReceiveEvent(const EventFwk::CommonEventData eventData) override;

private:
    void HandleUpdatedModuleInfo(const std::string &bundleName, int32_t uid);
    void HandleAppUpgradeCompleted(const std::string &bundleName, int32_t uid);
    void HandleRemoveUriPermission(uint32_t tokenId);

    DISALLOW_COPY_AND_MOVE(AbilityBundleEventCallback);
    AbilityEventUtil abilityEventHelper_;
    std::shared_ptr<TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AbilityRuntime::AbilityAutoStartupService> abilityAutoStartupService_;
};
} // namespace OHOS
} // namespace AAFwk
#endif // OHOS_ABILITY_RUNTIME_ABILITY_BUNDLE_EVENT_CALLBACK_H