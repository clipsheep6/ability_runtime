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
#include "erms_mgr_param.h"
#include "erms_mgr_interface.h"
#include "want.h"
#include "in_process_call_wrapper.h"

namespace OHOS {
namespace AAFwk {
using ErmsCallerInfo = OHOS::AppExecFwk::ErmsParams::CallerInfo;
using ExperienceRule = OHOS::AppExecFwk::ErmsParams::ExperienceRule;

class AbilityInterceptor {
public:
    virtual ~AbilityInterceptor();

    /**
     * Excute interception processing.
     */
    virtual ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground) = 0;
};

// start ability interceptor
class CrowdTestInterceptor : public AbilityInterceptor {
public:
    CrowdTestInterceptor();
    ~CrowdTestInterceptor();
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground) override;
private:
    bool CheckCrowdtest(const Want &want, int32_t userId);
};

class ControlInterceptor : public AbilityInterceptor {
public:
    ControlInterceptor();
    ~ControlInterceptor();
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground) override;
private:
    bool CheckControl(const Want &want, int32_t userId, AppExecFwk::AppRunningControlRuleResult &controlRule);
};

class EcologicalRuleInterceptor : public AbilityInterceptor {
public:
    EcologicalRuleInterceptor();
    ~EcologicalRuleInterceptor();
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground) override;
private:
    bool CheckRule(const Want &want, ErmsCallerInfo &callerInfo, ExperienceRule &rule);
};

// ability jump interceptor
class AbilityJumpInterceptor : public AbilityInterceptor {
public:
    AbilityJumpInterceptor();
    ~AbilityJumpInterceptor();
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground) override;
    
private:
    bool CheckControl(sptr<AppExecFwk::IBundleMgr> &bms, const Want &want, int32_t userId,
        AppExecFwk::AppJumpControlRule &controlRule);
    bool CheckIsJumpFromOrToSystemOrExemptApp(sptr<AppExecFwk::IBundleMgr> &bms,
        AppExecFwk::AppJumpControlRule &controlRule, int32_t userId);
    bool VerifyPermissionForBundleName(sptr<AppExecFwk::IBundleMgr> &bms, const std::string &bundleName,
        const std::string &permission, int32_t userId);
    bool LoadAppLabelInfo(sptr<AppExecFwk::IBundleMgr> &bms, Want &want, AppExecFwk::AppJumpControlRule &controlRule,
        int32_t userId);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_H
