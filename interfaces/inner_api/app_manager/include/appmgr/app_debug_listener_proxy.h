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

#ifndef OHOS_ABILITY_RUNTIME_APP_DEBUG_LISTENER_PROXY_H
#define OHOS_ABILITY_RUNTIME_APP_DEBUG_LISTENER_PROXY_H

#include "app_debug_info.h"
#include "app_debug_listener_interface.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
class AppDebugListenerProxy : public IRemoteProxy<IAppDebugListener> {
public:
    explicit AppDebugListenerProxy(const sptr<IRemoteObject> &impl);
    virtual ~AppDebugListenerProxy() = default;

    /**
     * @brief Notification of application information registered in listening and debugging mode.
     * @param tokens The app info of app running record.
     */
    void OnAppDebugStarted(const std::vector<AppDebugInfo> &debugInfos) override;

    /**
     * @brief Notification of application information registered in listening and remove debug mode.
     * @param tokens The app info of app running record.
     */
    void OnAppDebugStoped(const std::vector<AppDebugInfo> &debugInfos) override;
   
private:
    bool WriteInterfaceToken(MessageParcel &data);
    void SendRequest(const IAppDebugListener::Message &message, const std::vector<AppDebugInfo> &debugInfos);
    static inline BrokerDelegator<AppDebugListenerProxy> delegator_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_DEBUG_LISTENER_PROXY_H
