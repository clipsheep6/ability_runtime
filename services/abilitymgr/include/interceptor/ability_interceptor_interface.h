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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_INTERFACE_H

#include "ability_info.h"
#include "ability_manager_errors.h"
#include "want.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AAFwk {
struct AbilityInterceptorParam {
    AbilityInterceptorParam(const Want &want, int requestCode, int32_t userId, bool isWithUI,
        const sptr<IRemoteObject> &callerToken) : want(want), requestCode(requestCode), userId(userId),
        isWithUI(isWithUI), callerToken(callerToken){};
    AbilityInterceptorParam(const Want &want, int requestCode, int32_t userId, bool isWithUI,
        const sptr<IRemoteObject> &callerToken,
        const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo, bool isStartAsCaller = false) : want(want),
        requestCode(requestCode), userId(userId), isWithUI(isWithUI),
        callerToken(callerToken), abilityInfo(abilityInfo), isStartAsCaller(isStartAsCaller){};
    const Want &want;
    int32_t requestCode;
    int32_t userId;
    bool isWithUI = false;
    const sptr<IRemoteObject> &callerToken;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo;  // target abilityInfo get in afterCheckExecuter_
    bool isStartAsCaller = false;
};

/**
 * @class IAbilityInterceptor
 * IAbilityInterceptor is used to intercept a different type of start request.
 */
class IAbilityInterceptor {
public:
    virtual ~IAbilityInterceptor() = default;

    /**
     * Excute interception processing.
     */
    virtual ErrCode DoProcess(AbilityInterceptorParam param) = 0;

    /**
     * Set handler for async task executing.
     */
    virtual void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) {};
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_INTERFACE_H