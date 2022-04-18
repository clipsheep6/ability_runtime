/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "sys_mgr_client.h"

#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
OHOS::sptr<IRemoteObject> SysMrgClient::remoteObject_ = nullptr;
std::mutex SysMrgClient::remoteObjectMutex_;

SysMrgClient::SysMrgClient()
{}

SysMrgClient::~SysMrgClient()
{}

/**
 *
 * Get the systemAbility by ID.
 *
 * @param systemAbilityId The ID of systemAbility which want to get.
 */
sptr<IRemoteObject> SysMrgClient::GetSystemAbility(const int32_t systemAbilityId)
{
    if (remoteObject_ == nullptr) {
        std::lock_guard<std::mutex> lock(remoteObjectMutex_);
        if (remoteObject_ == nullptr) {
            auto systemAbilityManager = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (systemAbilityManager == nullptr) {
                HILOG_ERROR("GetSystemAbilityManager for %{public}d failed.", systemAbilityId);
                return nullptr;
            }
            remoteObject_ = systemAbilityManager->GetSystemAbility(systemAbilityId);
            if (remoteObject_ == nullptr) {
                HILOG_ERROR("GetSystemAbility for %{public}d failed.", systemAbilityId);
                return nullptr;
            }
        }
    }
    return remoteObject_;
}

/**
 *
 * Register the systemAbility by ID.
 *
 * @param systemAbilityId The ID of systemAbility which want to register.
 * @param broker The systemAbility which want to be registered.
 */
void SysMrgClient::RegisterSystemAbility(
    const int32_t __attribute__((unused)) systemAbilityId, sptr<IRemoteObject> __attribute__((unused)) broker)
{
    (void)servicesMap_;
}

/**
 *
 * Unregister the systemAbility by ID.
 *
 * @param systemAbilityId The ID of systemAbility which want to unregister.
 */
void SysMrgClient::UnregisterSystemAbility(const int32_t systemAbilityId)
{}
}  // namespace AppExecFwk
}  // namespace OHOS
