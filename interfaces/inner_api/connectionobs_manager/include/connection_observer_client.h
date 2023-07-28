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

#ifndef ABILITY_RUNTIME_CONNECTION_OBSERVER_CLIENT_H
#define ABILITY_RUNTIME_CONNECTION_OBSERVER_CLIENT_H

#include <memory>

#include "connection_observer.h"
#include "dlp_connection_info.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class ConnectionObserverClient
 * ConnectionObserverClient is used to manage connection observer.
 */
class ConnectionObserverClientImpl;
class ConnectionObserverClient {
public:
    static ConnectionObserverClient& GetInstance();

    /**
     * @brief Destructor.
     *
     */
    virtual ~ConnectionObserverClient() = default;

    /**
     * register connection state observer.
     *
     * @param observer the observer callback.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterObserver(const std::shared_ptr<ConnectionObserver> &observer);

    /**
     * unregister connection state observer.
     *
     * @param observer the observer callback.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterObserver(const std::shared_ptr<ConnectionObserver> &observer);

    /**
     * get exist dlp connection infos.
     *
     * @param infos output dlp connection result.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetDlpConnectionInfos(std::vector<DlpConnectionInfo> &infos);

private:
    ConnectionObserverClient();
    ConnectionObserverClient(const ConnectionObserverClient&) = delete;
    ConnectionObserverClient(ConnectionObserverClient&&) = delete;
    ConnectionObserverClient& operator=(const ConnectionObserverClient&) = delete;
    ConnectionObserverClient& operator=(ConnectionObserverClient&&) = delete;

    std::shared_ptr<ConnectionObserverClientImpl> clientImpl_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // ABILITY_RUNTIME_CONNECTION_OBSERVER_CLIENT_H
