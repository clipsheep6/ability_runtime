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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_APP_FOREGROUND_STATE_OBSERVER_PROXY_H
#define OHOS_ABILITY_RUNTIME_MOCK_APP_FOREGROUND_STATE_OBSERVER_PROXY_H

#include <gmock/gmock.h>

#include "app_foreground_state_observer_stub.h"

namespace OHOS {
namespace AppExecFwk {
class MockAppForegroundStateObserverStub : public AppForegroundStateObserverStub {
public:
    MockAppForegroundStateObserverStub() = default;
    virtual ~MockAppForegroundStateObserverStub() = default;

    MOCK_METHOD1(OnAppStateChanged, void(const AppStateData &appStateData));
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_MOCK_APP_FOREGROUND_STATE_OBSERVER_PROXY_H
