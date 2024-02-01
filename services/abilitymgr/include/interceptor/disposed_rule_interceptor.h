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

#ifndef OHOS_ABILITY_RUNTIME_DISPOSED_RULE_INTERCEPTOR_H
#define OHOS_ABILITY_RUNTIME_DISPOSED_RULE_INTERCEPTOR_H

#include "ability_interceptor_interface.h"

#include "app_mgr_interface.h"
#include "disposed_observer.h"
#include "task_utils_wrap.h"

namespace OHOS {
namespace AppExecFwk {
struct DisposedRule;
}
namespace AAFwk {
class DisposedRuleInterceptor : public IAbilityInterceptor,
                                public std::enable_shared_from_this<DisposedRuleInterceptor> {
public:
    DisposedRuleInterceptor() = default;
    ~DisposedRuleInterceptor() = default;
    ErrCode DoProcess(AbilityInterceptorParam param) override;
    void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) override
    {
        taskHandler_ = taskHandler;
    };
    void UnregisterObserver(const std::string &bundleName);
private:
    bool CheckControl(const Want &want, int32_t userId, AppExecFwk::DisposedRule &disposedRule);
    bool CheckDisposedRule(const Want &want, AppExecFwk::DisposedRule &disposedRule);
    ErrCode StartNonBlockRule(const Want &want, AppExecFwk::DisposedRule &disposedRule);
    sptr<AppExecFwk::IAppMgr> GetAppMgr();
    ErrCode CreateModalUIExtension(const Want &want, const sptr<IRemoteObject> &callerToken);
private:
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::map<std::string, sptr<DisposedObserver>> disposedObserverMap_;
    ffrt::mutex observerLock_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DISPOSED_RULE_INTERCEPTOR_H
