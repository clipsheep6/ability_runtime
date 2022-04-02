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

#ifndef OHOS_AAFWK_FREE_INSTALL_STATUS_CALLBACK_INTERFACE_H
#define OHOS_AAFWK_FREE_INSTALL_STATUS_CALLBACK_INTERFACE_H

#include <iremote_broker.h>

namespace OHOS {
namespace AAFwk {
/**
 * @class IFreeInstallStatusCallback
 * IFreeInstallStatusCallback is used to notify caller ability that free install is complete.
 */
class IFreeInstallStatusCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.IFreeInstallStatusCallback");

    /**
     * OnInstallFinished, FreeInstall is complete.
     *
     * @param resultCode, ERR_OK on success, others on failure.
     */
    virtual void OnInstallFinished(int resultCode) = 0;

protected:
    enum IFreeInstallStatusCallbackCmd {
        ON_FREE_INSTALL_DONE = 0,
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_FREE_INSTALL_STATUS_CALLBACK_INTERFACE_H
