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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_BUNDLE_EVENT_CALLBACK_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_BUNDLE_EVENT_CALLBACK_H

#include "ability_auto_startup_service.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AAFwk {
/**
 * @brief This class is a callback class that will be registered to BundleManager.
 * This class will be called by BundleManager when install, uninstall, updates of haps happens,
 * and executes corresponding functionalities of ability manager.
 */
class AbilityBundleEventCallback {
public:
    AbilityBundleEventCallback(std::shared_ptr<TaskHandlerWrap> taskHandler,
        std::shared_ptr<AbilityRuntime::AbilityAutoStartupService> abilityAutoStartupService) {}

    ~AbilityBundleEventCallback() = default;
};
} // namespace OHOS
} // namespace AAFwk
#endif // MOCK_OHOS_ABILITY_RUNTIME_ABILITY_BUNDLE_EVENT_CALLBACK_H