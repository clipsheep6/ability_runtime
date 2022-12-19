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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_CONTINUATION_ABILITY_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_CONTINUATION_ABILITY_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ability.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

class MockContinuationAbility : public Ability {
public:
    MockContinuationAbility() = default;
    virtual ~MockContinuationAbility() = default;

    MOCK_METHOD0(OnStartContinuation, bool());
    MOCK_METHOD0(TerminateAbility, int());
    MOCK_METHOD0(OnRemoteTerminated, void());
    MOCK_METHOD0(GetContentInfo, std::string());
    MOCK_METHOD1(OnSaveData, bool(WantParams &saveData));
    MOCK_METHOD1(OnRestoreData, bool(WantParams &restoreData));
    MOCK_METHOD1(OnCompleteContinuation, void(int result));
    MOCK_METHOD1(OnContinue, int32_t(WantParams &wantParams));

    const std::shared_ptr<AbilityInfo> GetAbilityInfo()
    {
        return abilityInfo_;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_CONTINUATION_ABILITY_H
