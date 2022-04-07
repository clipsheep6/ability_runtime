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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FREE_INSTALL_STATUS_CALLBACK_PROXY_H
#define FOUNDATION_APPEXECFWK_OHOS_FREE_INSTALL_STATUS_CALLBACK_PROXY_H

#include <string>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

#include "free_install_status_callback_interface.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * interface for FreeInstallStatusCallbackProxy.
 */
class FreeInstallStatusCallbackProxy : public IRemoteProxy<AAFwk::IFreeInstallStatusCallback> {
public:
    explicit FreeInstallStatusCallbackProxy(const sptr<IRemoteObject>& impl);

    /**
     * OnInstallFinished, FreeInstall is complete.
     *
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void OnInstallFinished(int resultCode) override;

private:
    void SendRequestCommon(int errcode, IFreeInstallStatusCallback::IFreeInstallStatusCallbackCmd cmd);
    static inline BrokerDelegator<FreeInstallStatusCallbackProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FREE_INSTALL_STATUS_CALLBACK_PROXY_H
