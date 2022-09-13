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

namespace OHOS {
namespace AAFwk{
class AbilityInterceptorExecuter {
public:
    void AddInterceptor(sptr<Interceptor> Interceptor);
    ErrCode DoProcess();
private:
    std::vector<sptr<Interceptor>> interceptorList_;
}

// 使用示例

// sptr<AbilityInterceptorExcuter> excuter = std::make_shared<AbilityInterceptorExcuter>();

// excuter->AddInterceptor(std::make_shared<Interceptor>());

// auto result = excuter->DoProces();
// if (result != ERR_OK) {
//     return result;
// }
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_EXECUTER_H
