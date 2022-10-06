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

#include "connection_observer_client.h"

#include "connection_observer_client_impl.h"
#include "connection_observer_errors.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
ConnectionObserverClient::ConnectionObserverClient()
{
    clientImpl_ = std::make_shared<ConnectionObserverClientImpl>();
}

ConnectionObserverClient& ConnectionObserverClient::GetInstance()
{
    static ConnectionObserverClient instance;
    return instance;
}

int32_t ConnectionObserverClient::RegisterObserver(const std::shared_ptr<ConnectionObserver> &observer)
{
    if (!clientImpl_) {
        HILOG_ERROR("ConnectionObserverClient::RegisterObserver impl is nullptr.");
        return ERR_NO_CLIENT_IMPL;
    }

    return clientImpl_->RegisterObserver(observer);
}

int32_t ConnectionObserverClient::UnregisterObserver(const std::shared_ptr<ConnectionObserver> &observer)
{
    if (!clientImpl_) {
        HILOG_ERROR("ConnectionObserverClient::UnregisterObserver impl is nullptr.");
        return ERR_NO_CLIENT_IMPL;
    }

    return clientImpl_->UnregisterObserver(observer);
}

int32_t ConnectionObserverClient::GetDlpConnectionInfos(std::vector<DlpConnectionInfo> &infos)
{
    if (!clientImpl_) {
        HILOG_ERROR("ConnectionObserverClient::GetDlpConnectionInfos impl is nullptr.");
        return ERR_NO_CLIENT_IMPL;
    }

    return clientImpl_->GetDlpConnectionInfos(infos);
}
}
}
