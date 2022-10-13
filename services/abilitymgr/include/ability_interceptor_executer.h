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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_EXECUTER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_EXECUTER_H

#include <vector>
#include "ability_interceptor.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class AbilityInterceptorExecuter
 * AbilityInterceptorExecuter excute the interceptors.
 */
class AbilityInterceptorExecuter {
public:
    /**
     * Add Interceptor to Executer.
     *
     * @param interceptor, interceptor handle the interception processing.
     */
    void AddInterceptor(const std::shared_ptr<AbilityInterceptor> &interceptor);

    /**
     * Excute the DoProcess of the interceptors.
     */
    ErrCode DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground);
private:
    std::vector<std::shared_ptr<AbilityInterceptor>> interceptorList_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_EXECUTER_H
