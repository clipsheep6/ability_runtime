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

#ifndef OHOS_ABILITY_RUNTIME_DIALOG_REQUEST_CALLBACK_PROXY_H
#define OHOS_ABILITY_RUNTIME_DIALOG_REQUEST_CALLBACK_PROXY_H

#include <string>

#include "idialog_request_callback.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * proxy of dialog request callback.
 */
class DialogRequestCallbackProxy : public IRemoteProxy<IDialogRequestCallback> {
public:
    explicit DialogRequestCallbackProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IDialogRequestCallback>(impl)
    {}

    /**
     * @brief Send dialogRequest result.
     *
     * @param resultCode result code.
     */
    virtual void SendResult(int32_t resultCode) override;

private:
    static inline BrokerDelegator<DialogRequestCallbackProxy> delegator_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DIALOG_REQUEST_CALLBACK_PROXY_H
