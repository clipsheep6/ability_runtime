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

#ifndef OHOS_ABILITYRUNTIME_CONNECTION_OBSERVER_STUB_IMPL_H
#define OHOS_ABILITYRUNTIME_CONNECTION_OBSERVER_STUB_IMPL_H

#include "connection_observer_client_impl.h"
#include "connection_observer_stub.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * interface for connection observer.
 */
class ConnectionObserverClientImpl;
class ConnectionObserverStubImpl : public ConnectionObserverStub {
public:
    explicit ConnectionObserverStubImpl(const std::shared_ptr<ConnectionObserverClientImpl>& owner) : owner_(owner) {}
    ~ConnectionObserverStubImpl() = default;

    void OnExtensionConnected(const ConnectionData &data) override;

    void OnExtensionDisconnected(const ConnectionData &data) override;

#ifdef WITH_DLP
    void OnDlpAbilityOpened(const DlpStateData &data) override;

    void OnDlpAbilityClosed(const DlpStateData &data) override;
#endif // WITH_DLP

private:
    std::weak_ptr<ConnectionObserverClientImpl> owner_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITYRUNTIME_CONNECTION_OBSERVER_STUB_IMPL_H
