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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONNECT_MODULE_TEST_CONNECTION_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONNECT_MODULE_TEST_CONNECTION_H

#include "ability_connect_callback_stub.h"

namespace OHOS {
namespace AAFwk {
class UIExtensionConnectModuleTestConnection : public AbilityConnectionStub {
public:
    UIExtensionConnectModuleTestConnection() = default;
    ~UIExtensionConnectModuleTestConnection() = default;

    std::condition_variable connectCondation_;
    std::mutex connectMutex_;
    bool connectFinished_ = false;
    bool disConnectFinished_ = false;

private:
    void OnAbilityConnectDone(const AppExecFwk::ElementName& element,
        const sptr<IRemoteObject>& remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONNECT_MODULE_TEST_CONNECTION_H
