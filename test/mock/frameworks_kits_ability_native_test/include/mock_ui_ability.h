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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_UI_ABILITY_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_UI_ABILITY_H

#include <gtest/gtest.h>

#include "ui_ability.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS;
using Want = OHOS::AAFwk::Want;

class MockUIAbility : public AbilityRuntime::UIAbility {
public:
    MockUIAbility() = default;
    virtual ~MockUIAbility() = default;

    enum Event { ON_ACTIVE = 0, ON_BACKGROUND, ON_FOREGROUND, ON_INACTIVE, ON_START, ON_STOP, UNDEFINED };

    void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData)
    {
        GTEST_LOG_(INFO) << "MockUIAbility::OnAbilityResult called";
        state_ = ON_ACTIVE;
    }

    void OnNewWant(const Want &want)
    {
        onNewWantCalled_ = true;
        GTEST_LOG_(INFO) << "MockUIAbility::OnNewWant called";
    }

    void OnStart(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
    {
        GTEST_LOG_(INFO) << "MockUIAbility::OnStart called";
        state_ = ON_START;
    }

    void OnStop()
    {
        GTEST_LOG_(INFO) << "MockUIAbility::OnStop called";
        state_ = ON_STOP;
    }

    void OnForeground(const Want &want)
    {
        GTEST_LOG_(INFO) << "MockUIAbility::OnForeground called";
        state_ = ON_FOREGROUND;
    }

    void OnBackground()
    {
        GTEST_LOG_(INFO) << "MockUIAbility::OnBackground called";
        state_ = ON_BACKGROUND;
    }

    void OnRestoreAbilityState(const PacMap &inState)
    {
        GTEST_LOG_(INFO) << "Mock UIAbility::OnRestoreAbilityState called";
    }

    void OnConfigurationUpdated(const Configuration &config)
    {
        GTEST_LOG_(INFO) << "Mock UIAbility::OnConfigurationUpdated called";
        OnConfigurationUpdated_++;
    }

    void ContinuationRestore(const Want &want)
    {
        GTEST_LOG_(INFO) << "Mock UIAbility::ContinuationRestore called";
        continueRestoreCalled_ = true;
    }

    MockUIAbility::Event state_ = UNDEFINED;
    bool onNewWantCalled_ = false;
    bool continueRestoreCalled_ = false;
    int OnConfigurationUpdated_ = 0;
    std::vector<std::string> value;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // MOCK_OHOS_ABILITY_RUNTIME_MOCK_UI_ABILITY_H
