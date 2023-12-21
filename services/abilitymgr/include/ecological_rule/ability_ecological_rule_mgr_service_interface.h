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

#ifndef ABILITY_SERVICES_INCLUDE_ECOLOGICALRULEMANAGERSERVICE_INTERFACE_H
#define ABILITY_SERVICES_INCLUDE_ECOLOGICALRULEMANAGERSERVICE_INTERFACE_H

#include <string>
#include "iremote_broker.h"
#include "ability_ecological_rule_mgr_service_param.h"
#include "want.h"
#include "ability_info.h"

namespace OHOS {
namespace EcologicalRuleMgrService {
class IAbilityEcologicalRuleMgrService : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.cloud.ecologicalrulemgrservice.IEcologicalRuleMgrService");

    using Want = OHOS::AAFwk::Want;

    using AbilityInfo = OHOS::AppExecFwk::AbilityInfo;

    virtual int32_t QueryStartExperience(const Want &want, const AbilityCallerInfo &callerInfo, AbilityExperienceRule &rule) = 0;
    virtual int32_t EvaluateResolveInfos(const Want &want, const AbilityCallerInfo &callerInfo, int32_t type,
        std::vector<AbilityInfo> &abilityInfos) = 0;

    enum {
        QUERY_START_EXPERIENCE_CMD = 1,
        EVALUATE_RESOLVE_INFO_CMD = 2
    };

    enum ErrCode {
        ERR_BASE = (-99),
        ERR_FAILED = (-1),
        ERR_PERMISSION_DENIED = (-2),
        ERR_OK = 0,
    };
};
} // namespace EcologicalRuleMgrService
} // namespace OHOS

#endif // ABILITY_SERVICES_INCLUDE_ECOLOGICALRULEMGRSERVICE_INTERFACE_H