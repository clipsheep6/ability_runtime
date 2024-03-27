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

#ifndef OHOS_ABILITY_RUNTIME_ECOLOGICAL_RULE_INTERCEPTOR_H
#define OHOS_ABILITY_RUNTIME_ECOLOGICAL_RULE_INTERCEPTOR_H

#include "ability_interceptor_interface.h"

namespace OHOS {
namespace EcologicalRuleMgrService {
struct AbilityCallerInfo;
struct AbilityExperienceRule;
}
namespace AAFwk {
using namespace OHOS::EcologicalRuleMgrService;
using ErmsCallerInfo = OHOS::EcologicalRuleMgrService::AbilityCallerInfo;
using ExperienceRule = OHOS::EcologicalRuleMgrService::AbilityExperienceRule;

class EcologicalRuleInterceptor : public IAbilityInterceptor {
public:
    EcologicalRuleInterceptor() = default;
    ~EcologicalRuleInterceptor() = default;
    ErrCode DoProcess(AbilityInterceptorParam param) override;
    virtual void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) override
    {
        return;
    };
private:
    void GetEcologicalCallerInfo(const Want &want, ErmsCallerInfo &callerInfo, int32_t userId);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ECOLOGICAL_RULE_INTERCEPTOR_H