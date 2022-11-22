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

#ifndef OHOS_ABILITY_RUNTIME_QUICK_FIX_CALLBACK_PROXY_H
#define OHOS_ABILITY_RUNTIME_QUICK_FIX_CALLBACK_PROXY_H

#include "iquick_fix_callback.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
class QuickFixCallbackProxy : public IRemoteProxy<IQuickFixCallback> {
public:
    explicit QuickFixCallbackProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IQuickFixCallback>(impl) {};
    virtual ~QuickFixCallbackProxy() = default;

    void OnLoadPatchDone(int32_t resultCode) override;
    void OnUnloadPatchDone(int32_t resultCode) override;
    void OnReloadPageDone(int32_t resultCode) override;

private:
    bool SendRequestWithCmd(uint32_t code, MessageParcel &data, MessageParcel &reply);

    static inline BrokerDelegator<QuickFixCallbackProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_QUICK_FIX_CALLBACK_PROXY_H
