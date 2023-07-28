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

#ifndef OHOS_ABILITY_RUNTIME_AUTHORIZATION_RESULT_H
#define OHOS_ABILITY_RUNTIME_AUTHORIZATION_RESULT_H

#include "ability_context.h"
#include "security_access_token/interfaces/innerkits/token_callback/include/token_callback_stub.h"

namespace OHOS {
namespace AbilityRuntime {
class AuthorizationResult : public Security::AccessToken::TokenCallbackStub {
public:
    explicit AuthorizationResult(PermissionRequestTask&& task) : task_(task) {}
    virtual ~AuthorizationResult() = default;

    virtual void GrantResultsCallback(const std::vector<std::string> &permissions,
        const std::vector<int> &grantResults) override;

private:
    PermissionRequestTask task_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTHORIZATION_RESULT_H
