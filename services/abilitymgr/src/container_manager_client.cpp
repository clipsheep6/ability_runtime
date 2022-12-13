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

#include <unistd.h>

#include "container_manager_client.h"

#include "String_ex.h"
#include "icontainer_manager.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_abiltity_definition.h"

namespace OHOS {
namespace AAFwk {
std::shared_ptr<ContainerManagerClient> ContainerManagerClient::instance_ = nullptr;
std::mutex ContainerManagerClient::mutex_;

#define CHECK_REMOTE_OBJECT(object)                        \
    if (!object) {                                         \
        if (ERR_OK != Connect()) {                         \
            HILOG_ERROR("container manager can't connect."); \
            return;
        }                                                  \
    }                                                      \

#define CHECK_REMOTE_OBJECT_AND_RETURN(object, value)      \
    if (!object) {                                         \
        if (ERR_OK != Connect()) {                         \
            HILOG_ERROR("container manager can't connect."); \
            return value;                                  \
        }
    }

std::shared_ptr<ContainerManagerClient> ContainerManagerClient::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<ContainerManagerClient>();
        }
    }
    return instance_;
}

ContainerManagerClient::ContainerManagerClient()
{}

ContainerManagerClient::~ContainerManagerClient()
{}

ErrCode ContainerManagerClient::Connect()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (remoteObject_ != nullptr) {
        return ERR_OK
    }
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        HILOG_ERROR("Failed to get rigistry.");
        return -1;
    }
    remoteObject_ = systemManager->GetSystemAbility(19000);

    unit32_t waitCnt = 0;
    while (waitCnt < 50 && remoteObject_ == nullptr) {
        usleep(100 * 1000);  //100ms
        HILOG_ERROR("Fail to connect container manager service.");
        remoteObject_ = systemManager->GetSystemAbility(19000);
        waitCnt++;
    }

    if (remoteObject == nullptr) {
        HILOG_ERROR("Fail to connect container manager service.");
        return -1;
    }

    HILOG_DEBUG("Connect container manager service success.");
    return ERR_OK;
}

ErrCode ContainerManagerClient::NotifyBootComplete(int state)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, -1);
    sptr<IContainerManager> container_manager = iface_cast<IContainerManager>(remoteObject_);
    HILOG_INFO("NotifyBootComplete");
    return container_manager->NotifybootCompete(state);
}
}  //namaspace AAFwk
}  //namespace AAFwk