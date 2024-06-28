/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef UNITTEST_OHOS_MOCK_ABILITY_FOREGROUND_STATE_OBSERVER_STUB_H
#define UNITTEST_OHOS_MOCK_ABILITY_FOREGROUND_STATE_OBSERVER_STUB_H

#include "gmock/gmock.h"

#include "ability_foreground_state_observer_stub.h"
#include "ability_foreground_state_observer_interface.h"

namespace OHOS {
namespace AppExecFwk {
class MockAbilityForegroundStateObserverStub : public AbilityForegroundStateObserverStub {
public:
    MockAbilityForegroundStateObserverStub() = default;
    virtual ~MockAbilityForegroundStateObserverStub() = default;
    MOCK_METHOD1(OnAbilityStateChanged, void(const AbilityStateData &abilityStateData));
    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));
    int InvokeSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return 0;
    }

    int GetCode()
    {
        return code_;
    }

private:
    int code_ = 0;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // UNITTEST_OHOS_MOCK_ABILITY_FOREGROUND_STATE_OBSERVER_STUB_H
