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
#include "disposed_observer.h"
#include "ecological_rule/ability_ecological_rule_mgr_service.h"
#include "in_process_call_wrapper.h"
#include "task_handler_wrap.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::EcologicalRuleMgrService;
using ErmsCallerInfo = OHOS::EcologicalRuleMgrService::AbilityCallerInfo;
using ExperienceRule = OHOS::EcologicalRuleMgrService::AbilityExperienceRule;

class AbilityInterceptor {
public:
    virtual ~AbilityInterceptor() = default;

    /**
     * Excute interception processing.
     */
    virtual ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground,
        const sptr<IRemoteObject> &callerToken) = 0;
    virtual void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) {};
};

// start ability interceptor
class CrowdTestInterceptor : public AbilityInterceptor {
public:
    CrowdTestInterceptor() = default;
    ~CrowdTestInterceptor() = default;
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground,
        const sptr<IRemoteObject> &callerToken) override;
    virtual void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) override
    {
        return;
    };
private:
    bool CheckCrowdtest(const Want &want, int32_t userId);
};

class ControlInterceptor : public AbilityInterceptor {
public:
    ControlInterceptor() = default;
    ~ControlInterceptor() = default;
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground,
        const sptr<IRemoteObject> &callerToken) override;
    virtual void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) override
    {
        return;
    };
private:
    bool CheckControl(const Want &want, int32_t userId, AppExecFwk::AppRunningControlRuleResult &controlRule);
};

class DisposedRuleInterceptor : public AbilityInterceptor,
                                public std::enable_shared_from_this<DisposedRuleInterceptor> {
public:
    DisposedRuleInterceptor() = default;
    ~DisposedRuleInterceptor() = default;
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground,
        const sptr<IRemoteObject> &callerToken) override;
    void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) override
    {
        taskHandler_ = taskHandler;
    };
    void UnregisterObserver(const std::string &bundleName);
private:
    bool CheckControl(const Want &want, int32_t userId, AppExecFwk::DisposedRule &disposedRule);
    bool CheckDisposedRule(const Want &want, AppExecFwk::DisposedRule &disposedRule);
    ErrCode StartNonBlockRule(const Want &want, AppExecFwk::DisposedRule &disposedRule);
private:
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::map<std::string, sptr<DisposedObserver>> disposedObserverMap_;
    ffrt::mutex observerLock_;
};

class EcologicalRuleInterceptor : public AbilityInterceptor {
public:
    EcologicalRuleInterceptor() = default;
    ~EcologicalRuleInterceptor() = default;
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground,
        const sptr<IRemoteObject> &callerToken) override;
    virtual void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) override
    {
        return;
    };
private:
    void GetEcologicalCallerInfo(const Want &want, ErmsCallerInfo &callerInfo, int32_t userId);
};

// ability jump interceptor
class AbilityJumpInterceptor : public AbilityInterceptor {
public:
    AbilityJumpInterceptor() = default;
    ~AbilityJumpInterceptor() = default;
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground,
        const sptr<IRemoteObject> &callerToken) override;
    virtual void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) override
    {
        return;
    };

private:
    bool CheckControl(std::shared_ptr<AppExecFwk::BundleMgrHelper> &undleMgrHelper, const Want &want, int32_t userId,
        AppExecFwk::AppJumpControlRule &controlRule);
    bool CheckIfJumpExempt(std::shared_ptr<AppExecFwk::BundleMgrHelper> &undleMgrHelper,
        AppExecFwk::AppJumpControlRule &controlRule, int32_t userId);
    bool CheckIfExemptByBundleName(std::shared_ptr<AppExecFwk::BundleMgrHelper> &undleMgrHelper,
        const std::string &bundleName, const std::string &permission, int32_t userId);
    bool LoadAppLabelInfo(std::shared_ptr<AppExecFwk::BundleMgrHelper> &undleMgrHelper, Want &want,
        AppExecFwk::AppJumpControlRule &controlRule, int32_t userId);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_H
