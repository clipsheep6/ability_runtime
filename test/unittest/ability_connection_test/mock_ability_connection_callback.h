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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_ABILITY_CONNECTION_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_MOCK_ABILITY_CONNECTION_CALLBACK_H

#include "gmock/gmock.h"
#include "ability_connect_callback.h"

namespace OHOS {
namespace AbilityRuntime {

class MockAbilityConnectCallback : public AbilityConnectCallback {
public:
    MockAbilityConnectCallback() = default;
    ~MockAbilityConnectCallback() = default;

    MOCK_METHOD3(OnAbilityConnectDone,
        void(const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode));
    MOCK_METHOD2(OnAbilityDisconnectDone, void(const AppExecFwk::ElementName& element, int resultCode));
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_CALLBACK_H