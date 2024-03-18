/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_ABILITY_DEBUG_RESPONSE_STUB_H
#define OHOS_ABILITY_RUNTIME_MOCK_ABILITY_DEBUG_RESPONSE_STUB_H

#include "gmock/gmock.h"
#define private public
#include "ability_debug_response_stub.h"
#undef private

namespace OHOS {
namespace AppExecFwk {
class MockAbilityDebugResponseStub : public AbilityDebugResponseStub {
public:
    MockAbilityDebugResponseStub() {}
    virtual ~ MockAbilityDebugResponseStub() {}
    MOCK_METHOD1(OnAbilitysDebugStarted, void(const std::vector<sptr<IRemoteObject>> &tokens));
    MOCK_METHOD1(OnAbilitysDebugStoped, void(const std::vector<sptr<IRemoteObject>> &tokens));
    MOCK_METHOD2(OnAbilitysAssertDebugChange, void(const std::vector<sptr<IRemoteObject>> &, bool));
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MOCK_ABILITY_DEBUG_RESPONSE_STUB_H
