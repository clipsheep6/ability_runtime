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
#include "want.h"

namespace OHOS {
namespace AAFwk {
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
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_H
