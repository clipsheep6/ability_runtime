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

#ifndef OHOS_CONTAINER_MANAGER_CLIENT_H
#define OHOS_CONTAINER_MANAGER_CLIENT_H

#include <mutex>

#include "icontainer_manager.h"

#include "iremote_object.h"
#include "system_memory_attr.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class ContainerManagerClient
 * ContainerManagerClient is used to access container manager services.
 */
class ContainerManagerClient {
public:
    ContainerManagerClient();
    virtual ~ContainerManagerClient();
    static std::shared_ptr<ContainerManagerClient> GetInstance();

    /**
     * Notify the state of boot
     *
     * @param state, state of boot.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode NotifyBootComplete(int state);
private:
    /**
     * Connect ability manager service.
     *
     * @return Returns ERR_Ok on success, others on failure.
     */
    ErrCode Connect();

    static std::mutex mutex_;
    static std::shared_ptr<ContainerManagerClient> instance_;
    sptr<IRemoteObject> remoteObject_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_CONTAINER_MANAGER_CLIENT_H