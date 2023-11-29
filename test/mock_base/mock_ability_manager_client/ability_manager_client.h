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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H

#include <gmock/gmock.h>
#include "mock_base_ability_manager_client.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerClient : public MockBaseAbilityManagerClient {
public:
    static std::shared_ptr<AbilityManagerClient> GetInstance()
    {
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AbilityManagerClient>();
        }
        return instance_;
    }

    MOCK_METHOD(ErrCode, StartAbilityByInsightIntent,
        (const Want &want, const sptr<IRemoteObject> &callerToken, uint64_t intentId), (override));
private:
    inline static std::shared_ptr<AbilityManagerClient> instance_ = nullptr;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H
