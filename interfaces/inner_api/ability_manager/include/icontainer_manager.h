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

#ifndef OHOS_CONTAINER_MANAGER_H
#define OHOS_CONTAINER_MANAGER_H

#include <ipc_types.h>
#include <iremote_broker.h>

namespace OHOS {
namespace AAFwk {
constexpr const char* CONTAINER_MANAGER_ABILITY_NAME = "ContainerManagerAbility";
/**
 * @class IContainerManager
 * IContainerManager interface is used to access container manager services.
 */
class IContainerManager : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.IContainerManager")

    enum : int32_t {
        ERROR_DEF = -1,
    };

    /**
     * Notify the state of boot.
     *
     * @param state, state of boot.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyBootComplete(int32_t state = 0) = 0;

    enum {
        // ipc id 1-1000 for kit
        // ipc id for notify boot complete (1)
        NOTIFY_BOOT_COMPLETE = 1,
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_CONTAINER_MANAGER_H