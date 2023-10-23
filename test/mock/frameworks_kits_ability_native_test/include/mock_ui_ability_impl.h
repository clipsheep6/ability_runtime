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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_UI_ABILITY_IMPL_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_UI_ABILITY_IMPL_H

#include <gtest/gtest.h>

#include "ui_ability_impl.h"

namespace OHOS {
namespace AbilityRuntime {
using Want = OHOS::AAFwk::Want;

class MockUIAbilityimpl : public UIAbilityImpl {
public:
    MockUIAbilityimpl() = default;
    virtual ~MockUIAbilityimpl() = default;

    void ImplStart(const Want &want)
    {
        this->Start(want);
    }

    void ImplStop()
    {
        this->Stop();
    }

    void SetlifecycleState(int state)
    {
        this->lifecycleState_ = state;
    }

    int GetCurrentState()
    {
        return lifecycleState_;
    }

    sptr<IRemoteObject> GetToken()
    {
        return token_;
    }

    std::shared_ptr<UIAbility> GetAbility()
    {
        return ability_;
    }

    bool CheckAndRestore()
    {
        return UIAbilityImpl::CheckAndRestore();
    }

#ifdef SUPPORT_GRAPHICS
    void ImplForeground(const Want &want)
    {
        this->Foreground(want);
    }

    void ImplBackground()
    {
        this->Background();
    }
#endif

private:
    UIAbilityImpl AbilityImpl_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // MOCK_OHOS_ABILITY_RUNTIME_MOCK_UI_ABILITY_IMPL_H
