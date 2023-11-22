/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_H
#define OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_H

#include "ability_util.h"
#ifdef SUPPORT_ERMS
#include "ecological_rule_mgr_service_client.h"
#else
#include "erms_mgr_param.h"
#include "erms_mgr_interface.h"
#endif
#include "in_process_call_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
#ifdef SUPPORT_ERMS
using ErmsCallerInfo = OHOS::EcologicalRuleMgrService::CallerInfo;
using ExperienceRule = OHOS::EcologicalRuleMgrService::ExperienceRule;
#else
using ErmsCallerInfo = OHOS::AppExecFwk::ErmsParams::CallerInfo;
using ExperienceRule = OHOS::AppExecFwk::ErmsParams::ExperienceRule;
#endif

class AbilityInterceptor {
public:
    virtual ~AbilityInterceptor() = default;

    /**
     * Excute interception processing.
     */
    virtual ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground) = 0;
};

// start ability interceptor
class CrowdTestInterceptor : public AbilityInterceptor {
public:
    CrowdTestInterceptor() = default;
    ~CrowdTestInterceptor() = default;
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground) override;
private:
    bool CheckCrowdtest(const Want &want, int32_t userId);
};

class ControlInterceptor : public AbilityInterceptor {
public:
    ControlInterceptor() = default;
    ~ControlInterceptor() = default;
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground) override;
private:
    bool CheckControl(const Want &want, int32_t userId, AppExecFwk::AppRunningControlRuleResult &controlRule);
};

class DisposedRuleInterceptor : public AbilityInterceptor {
public:
    DisposedRuleInterceptor() = default;
    ~DisposedRuleInterceptor() = default;
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground) override;
private:
    bool CheckControl(const Want &want, int32_t userId, AppExecFwk::DisposedRule &disposedRule);
    bool CheckDisposedRule(const Want &want, AppExecFwk::DisposedRule &disposedRule);
};

class EcologicalRuleInterceptor : public AbilityInterceptor {
public:
    EcologicalRuleInterceptor() = default;
    ~EcologicalRuleInterceptor() = default;
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground) override;
private:
    void GetEcologicalCallerInfo(const Want &want, ErmsCallerInfo &callerInfo, int32_t userId);
    bool CheckRule(const Want &want, ErmsCallerInfo &callerInfo, ExperienceRule &rule);
};

// ability jump interceptor
class AbilityJumpInterceptor : public AbilityInterceptor {
public:
    AbilityJumpInterceptor() = default;
    ~AbilityJumpInterceptor() = default;
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground) override;

private:
    bool CheckControl(sptr<AppExecFwk::IBundleMgr> &bms, const Want &want, int32_t userId,
        AppExecFwk::AppJumpControlRule &controlRule);
    bool CheckIfJumpExempt(sptr<AppExecFwk::IBundleMgr> &bms, AppExecFwk::AppJumpControlRule &controlRule,
        int32_t userId);
    bool CheckIfExemptByBundleName(sptr<AppExecFwk::IBundleMgr> &bms, const std::string &bundleName,
        const std::string &permission, int32_t userId);
    bool LoadAppLabelInfo(sptr<AppExecFwk::IBundleMgr> &bms, Want &want, AppExecFwk::AppJumpControlRule &controlRule,
        int32_t userId);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_H
