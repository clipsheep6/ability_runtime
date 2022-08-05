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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_NEW_ABILITY_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_NEW_ABILITY_H

#include <gtest/gtest.h>
#include "ability.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

class MockNewAbility : public Ability {
public:
    MockNewAbility() = default;
    virtual ~MockNewAbility() = default;

    void OnNewWant(const Want &want)
    {
        GTEST_LOG_(INFO) << "MockNewAbility::OnNewWant called";
        onNewWantCalled_ = true;
    }

    void ContinuationRestore(const Want &want)
    {
        GTEST_LOG_(INFO) << "Mock Ability::ContinuationRestore called";
        continueRestoreCalled_ = true;
    }

    bool onNewWantCalled_ = false;
    bool continueRestoreCalled_ = false;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_NEW_ABILITY_H
