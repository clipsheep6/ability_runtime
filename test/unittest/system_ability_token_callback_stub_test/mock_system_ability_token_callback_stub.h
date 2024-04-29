/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_SYSTEM_ABILITY_TOKEN_CALLBACK_STUB_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_SYSTEM_ABILITY_TOKEN_CALLBACK_STUB_H

#include <gmock/gmock.h>
#include "system_ability_token_callback_stub.h"

namespace OHOS {
namespace AAFwk {
class MockSystemAbilityTokenCallbackStub : public SystemAbilityTokenCallbackStub {
public:
    MockSystemAbilityTokenCallbackStub() = default;
    virtual ~MockSystemAbilityTokenCallbackStub() = default;

    MOCK_METHOD5(SendResult, int32_t(OHOS::AAFwk::Want& want, int32_t callerUid, int32_t requestCode,
        uint32_t accessToken, int32_t resultCode));
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_SYSTEM_ABILITY_TOKEN_CALLBACK_STUB_H
