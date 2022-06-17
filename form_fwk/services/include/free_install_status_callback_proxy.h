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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FREE_INSTALL_STATUS_CALLBACK_PROXY_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FREE_INSTALL_STATUS_CALLBACK_PROXY_H

// #include <string>
// #include "iremote_broker.h"
// #include "iremote_object.h"
#include "iremote_proxy.h"
// #include "ability_info.h"
#include "i_free_install_status_callback.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FreeInstallStatusCallBackProxy
 * FreeInstallStatusCallBackProxy.
 */
class FreeInstallStatusCallBackProxy : public IRemoteProxy<IFreeInstallStatusCallBack> {
public:
    explicit FreeInstallStatusCallBackProxy(const sptr<IRemoteObject>& impl);

    /**
     * OnInstallFinished, FreeInstall is complete.
     *
     * @param resultCode, ERR_OK on success, others on failure.
     * @param want, installed ability.
     * @param userId, user`s id.
     */
    void OnInstallFinished(int resultCode, const Want &want, int32_t userId) override;

private:
    void SendRequestCommon(int errcode, IFreeInstallStatusCallBack::IFreeInstallStatusCallBackCmd cmd);
    static inline BrokerDelegator<FreeInstallStatusCallBackProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FREE_INSTALL_STATUS_CALLBACK_PROXY_H
