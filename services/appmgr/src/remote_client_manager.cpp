/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "remote_client_manager.h"

#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "singleton.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
RemoteClientManager::RemoteClientManager()
    : appSpawnClient_(std::make_shared<AppSpawnClient>()), nwebSpawnClient_(std::make_shared<AppSpawnClient>(true))
{}

RemoteClientManager::~RemoteClientManager()
{}

std::shared_ptr<AppSpawnClient> RemoteClientManager::GetSpawnClient()
{
    if (appSpawnClient_) {
        return appSpawnClient_;
    }
    return nullptr;
}

void RemoteClientManager::SetSpawnClient(const std::shared_ptr<AppSpawnClient> &appSpawnClient)
{
    appSpawnClient_ = appSpawnClient;
}

std::shared_ptr<AppExecFwk::BundleMgrClient> RemoteClientManager::GetBundleManagerClient()
{
    if (bundleManagerClient_ == nullptr) {
        bundleManagerClient_  = DelayedSingleton<AppExecFwk::BundleMgrClient>::GetInstance();
    }
    return bundleManagerClient_;
}

void RemoteClientManager::SetBundleManagerClient(const std::shared_ptr<AppExecFwk::BundleMgrClient> &bundleManagerClient)
{
    bundleManagerClient_ = bundleManagerClient;
}

std::shared_ptr<AppSpawnClient> RemoteClientManager::GetNWebSpawnClient()
{
    return nwebSpawnClient_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
