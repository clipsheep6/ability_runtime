/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_DEBUG_LISTENER_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_APP_DEBUG_LISTENER_INTERFACE_H

#include "app_debug_info.h"
#include "iremote_broker.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Interface to monitor when debug mode.
 */
class IAppDebugListener : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.AppExecFwk.AppDebugListener");

    /**
     * @brief Notification of application information registered in listening and debugging mode.
     * @param tokens The app info of app running record.
     */
    virtual void OnAppDebugStarted(const std::vector<AppDebugInfo> &debugInfos) = 0;

    /**
     * @brief Notification of application information registered in listening and remove debug mode.
     * @param tokens The app info of app running record.
     */
    virtual void OnAppDebugStoped(const std::vector<AppDebugInfo> &debugInfos) = 0;

    enum class Message {
        ON_APP_DEBUG_STARTED = 0,
        ON_APP_DEBUG_STOPED,
    };
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_DEBUG_LISTENER_INTERFACE_H
