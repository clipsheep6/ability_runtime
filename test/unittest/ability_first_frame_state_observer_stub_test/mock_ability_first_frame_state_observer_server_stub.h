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

#ifndef UNITTEST_OHOS_MOCK_ABILITY_FIRST_FRAME_STATE_OBSERVER_SERVER_STUB_H
#define UNITTEST_OHOS_MOCK_ABILITY_FIRST_FRAME_STATE_OBSERVER_SERVER_STUB_H

#define private public
#define protected public
#include "ability_first_frame_state_observer_stub.h"
#include "ability_first_frame_state_observer_interface.h"
#undef private
#undef protected

namespace OHOS {
namespace AppExecFwk {
class MockAbilityFirstFrameStateObserverServerStub : public AbilityFirstFrameStateObserverStub {
public:
    MockAbilityFirstFrameStateObserverServerStub() = default;
    virtual ~MockAbilityFirstFrameStateObserverServerStub() = default;
    void OnAbilityFirstFrameState(const AbilityFirstFrameStateData &abilityFirstFrameStateData) override {}

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }
};
} // namespace AAFwk
} // namespace OHOS
#endif // UNITTEST_OHOS_MOCK_ABILITY_FIRST_FRAME_STATE_OBSERVER_SERVER_STUB_H
