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

#ifndef SERVICE_REGISTRY_INCLUDE_H
#define SERVICE_REGISTRY_INCLUDE_H

#include "mock_system_ability_manager.h"

namespace OHOS {
namespace AAFwk {

class SystemAbilityManagerClient {
public:
    static bool nullptrFlag;

    static SystemAbilityManagerClient& GetInstance();

    /**
     * get system ability manager.
     */
    sptr<MockSystemAbilityManager> GetSystemAbilityManager();

private:
    SystemAbilityManagerClient() = default;
    ~SystemAbilityManagerClient() = default;

    sptr<MockSystemAbilityManager> systemAbilityManager_;
    std::mutex systemAbilityManagerLock_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // SERVICE_REGISTRY_INCLUDE_H