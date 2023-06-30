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

#ifndef MOCK_SYSTEM_ABILITY_MANAGER_H
#define MOCK_SYSTEM_ABILITY_MANAGER_H

#include "refbase.h"
#include "storage_manager_service_mock.h"

namespace OHOS {
namespace AAFwk {

class MockSystemAbilityManager : public RefBase {
public:
    static bool isNullptr;
    MockSystemAbilityManager() = default;
    ~MockSystemAbilityManager() = default;

    sptr<IRemoteObject> GetSystemAbility(int serviceId);
};

bool MockSystemAbilityManager::isNullptr = false;
sptr<IRemoteObject> MockSystemAbilityManager::GetSystemAbility(int serviceId)
{
    if (isNullptr) {
        return nullptr;
    }
    return new StorageManager::StorageManagerServiceMock();
}
}  // namespace AAFwk
}  // namespace OHOS
#endif // MOCK_SYSTEM_ABILITY_MANAGER_H