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

#include "connection_observer_stub_impl.h"

#include "connection_observer_client_impl.h"

namespace OHOS {
namespace AbilityRuntime {
void ConnectionObserverStubImpl::OnExtensionConnected(const ConnectionData &data)
{
    auto owner = owner_.lock();
    if (!owner) {
        return;
    }
    owner->HandleExtensionConnected(data);
}

void ConnectionObserverStubImpl::OnExtensionDisconnected(const ConnectionData &data)
{
    auto owner = owner_.lock();
    if (!owner) {
        return;
    }
    owner->HandleExtensionDisconnected(data);
}

#ifdef WITH_DLP
void ConnectionObserverStubImpl::OnDlpAbilityOpened(const DlpStateData &data)
{
    auto owner = owner_.lock();
    if (!owner) {
        return;
    }
    owner->HandleDlpAbilityOpened(data);
}

void ConnectionObserverStubImpl::OnDlpAbilityClosed(const DlpStateData &data)
{
    auto owner = owner_.lock();
    if (!owner) {
        return;
    }
    owner->HandleDlpAbilityClosed(data);
}
#endif // WITH_DLP
}  // namespace AbilityRuntime
}  // namespace OHOS
