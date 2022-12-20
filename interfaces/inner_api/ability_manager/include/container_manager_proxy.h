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

#ifndef OHOS_CONTAINER_MANAGER_PROXY_H
#define OHOS_CONTAINER_MANAGER_PROXY_H

#include "icontainer_manager.h"
#include "hilog_wrapper.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class ContainerManagerProxy
 * ContainerManagerProxy.
 */
class ContainerManagerProxy : public IRemoteProxy<IContainerManager> {
public:
    explicit ContainerManagerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IContainerManager>(impl)
    {}

    virtual ~ContainerManagerProxy()
    {}

    /**
     * Notify the state of boot.
     *
     * @param state, state of boot.
     * @return Return ERR_OK on success, others on failure.
     */
    virtual int NotifyBootComplete(int32_t state = 0) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);

private:
    static inline BrokerDelegator<ContainerManagerProxy> delegator_;
};
} // namespace AAFwk
} // namespace OHOS
#endif