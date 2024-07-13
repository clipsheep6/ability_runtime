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

#ifndef ABILITY_RUNTIME_SERVICE_PROXY_ADAPTER_H
#define ABILITY_RUNTIME_SERVICE_PROXY_ADAPTER_H

#ifdef WITH_DLP
#include "dlp_connection_info.h"
#endif // WITH_DLP

#include "iconnection_observer.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class ServiceProxyAdapter
 * ServiceProxyAdapter is used to send request to ability manager service.
 */
class ServiceProxyAdapter : public std::enable_shared_from_this<ServiceProxyAdapter> {
public:
    explicit ServiceProxyAdapter(const sptr<IRemoteObject> remoteObj) : remoteObj_(remoteObj) {}
    virtual ~ServiceProxyAdapter() = default;

    ServiceProxyAdapter(const ServiceProxyAdapter&) = default;
    ServiceProxyAdapter(ServiceProxyAdapter&&) = default;
    ServiceProxyAdapter& operator=(const ServiceProxyAdapter&) = default;
    ServiceProxyAdapter& operator=(ServiceProxyAdapter&&) = default;

    int32_t RegisterObserver(const sptr<IConnectionObserver> &observer);

    int32_t UnregisterObserver(const sptr<IConnectionObserver> &observer);

#ifdef WITH_DLP
    int32_t GetDlpConnectionInfos(std::vector<DlpConnectionInfo> &infos);
#endif // WITH_DLP

    int32_t GetConnectionData(std::vector<ConnectionData> &infos);

    sptr<IRemoteObject> GetProxyObject() const;

private:
    sptr<IRemoteObject> remoteObj_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // ABILITY_RUNTIME_CONNECTION_OBSERVER_CLIENT_IMPL_H
