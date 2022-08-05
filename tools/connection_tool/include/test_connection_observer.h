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

#ifndef OHOS_ABILITY_RUNTIME_TEST_CONNECTION_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_TEST_CONNECTION_OBSERVER_H

#include "connection_observer.h"

namespace OHOS {
namespace AbilityRuntime {
class TestConnectionObserver : public ConnectionObserver {
public:
    TestConnectionObserver() = default;
    virtual ~TestConnectionObserver() = default;

    virtual void OnExtensionConnected(const ConnectionData& data) override;

    virtual void OnExtensionDisconnected(const ConnectionData& data) override;

    virtual void OnDlpAbilityOpened(const DlpStateData& data) override;

    virtual void OnDlpAbilityClosed(const DlpStateData& data) override;

private:
    std::string GenerateString(const ConnectionData& data);
    std::string GenerateString(const DlpStateData& data);
};
}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_TEST_CONNECTION_OBSERVER_H
