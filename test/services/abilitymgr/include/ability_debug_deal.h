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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_DEBUG_DEAL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_DEBUG_DEAL_H

#include "ability_debug_response_interface.h"
#include "ability_debug_response_stub.h"

namespace OHOS {
namespace AAFwk {
class AbilityDebugDeal : public std::enable_shared_from_this<AbilityDebugDeal>  {
public:
    AbilityDebugDeal() = default;
    ~AbilityDebugDeal() = default;

    /**
     * @brief Set ability attach debug flag to ability manager service.
     * @param tokens The token of ability records.
     */
    void OnAbilitysDebugStarted(const std::vector<sptr<IRemoteObject>> &tokens);

    /**
     * @brief Cancel ability attach debug flag to ability manager service.
     * @param tokens The token of ability records.
     */
    void OnAbilitysDebugStoped(const std::vector<sptr<IRemoteObject>> &tokens);

    /**
     * @brief Register debug response in attach mode.
     */
    void RegisterAbilityDebugResponse();

private:
    sptr<AppExecFwk::IAbilityDebugResponse> abilityDebugResponse_;
};

class AbilityDebugResponse : public AppExecFwk::AbilityDebugResponseStub {
public:
    explicit AbilityDebugResponse(const std::weak_ptr<AbilityDebugDeal> &deal) : abilityDebugDeal_(deal) {}
    virtual ~AbilityDebugResponse() = default;

private:
    void OnAbilitysDebugStarted(const std::vector<sptr<IRemoteObject>> &tokens) override;
    void OnAbilitysDebugStoped(const std::vector<sptr<IRemoteObject>> &tokens) override;

    std::weak_ptr<AbilityDebugDeal> abilityDebugDeal_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_DEBUG_DEAL_H
