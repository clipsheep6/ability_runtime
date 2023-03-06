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

#ifndef OHOS_FORM_FWK_MOCK_ECOLOGICAL_RULE_MANAGER_H
#define OHOS_FORM_FWK_MOCK_ECOLOGICAL_RULE_MANAGER_H

#include <vector>

#include "erms_mgr_interface.h"
#include "erms_mgr_param.h"
#include "gmock/gmock.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

class MockEcologicalRuleMgrService : public OHOS::IRemoteStub<IEcologicalRuleManager> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IEcologicalRuleManagerMock");

    MockEcologicalRuleMgrService()
    {}

    virtual ~MockEcologicalRuleMgrService()
    {}

    virtual int32_t QueryFreeInstallExperience(const Want &want,
        const ErmsParams::CallerInfo &callerInfo, ErmsParams::ExperienceRule &rule)
    {
        return 0;
    }

    virtual int32_t EvaluateResolveInfos(const Want &want, const ErmsParams::CallerInfo &callerInfo, int32_t type,
                                         std::vector<AbilityInfo> &abilityInfos, std::vector<ExtensionAbilityInfo> extensionInfos)
    {
        return 0;
    }

    virtual int32_t QueryStartExperience(const Want &want,
                                         const ErmsParams::CallerInfo &callerInfo, ErmsParams::ExperienceRule &rule)
    {
        GTEST_LOG_(INFO) << "MockEcologicalRuleMgrService::QueryStartExperience";
        std::string abilityName = want.GetElement().GetAbilityName();
        if (abilityName == "com.test.pass")
        {
            GTEST_LOG_(INFO) << "MockEcologicalRuleMgrService::pass";
            rule.isAllow = true;
        } else if (abilityName == "com.test.deny")
        {
            GTEST_LOG_(INFO) << "MockEcologicalRuleMgrService::deny";
            rule.isAllow = false;
            rule.replaceWant = nullptr;
        } else if (abilityName == "com.test.jump")
        {
            GTEST_LOG_(INFO) << "MockEcologicalRuleMgrService::jump";
            rule.isAllow = false;
            rule.replaceWant = std::make_shared<Want>();
        } else
        {
            GTEST_LOG_(INFO) << "MockEcologicalRuleMgrService::invalid";
            return -1;
        }
        return 0;
    }

    virtual int32_t QueryPublishFormExperience(const Want &want, ErmsParams::ExperienceRule &rule)
    {
        return 0;
    }

    virtual int32_t IsSupportPublishForm(const Want &want, const ErmsParams::CallerInfo &callerInfo,
                                         ErmsParams::ExperienceRule &rule)
    {
        return 0;
    }

    virtual long QueryLastSyncTime()
    {
        return 0;
    }
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_FORM_FWK_MOCK_ECOLOGICAL_RULE_MANAGER_H
