/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_STATUS_BAR_DELEGATE_PROXY_H
#define OHOS_ABILITY_RUNTIME_STATUS_BAR_DELEGATE_PROXY_H

#include "iremote_proxy.h"
#include "status_bar_delegate_interface.h"

namespace OHOS {
namespace AbilityRuntime {
class StatusBarDelegateProxy : public IRemoteProxy<IStatusBarDelegate> {
public:
    explicit StatusBarDelegateProxy(const sptr<IRemoteObject> &impl);
    virtual ~StatusBarDelegateProxy() = default;

    virtual int32_t CheckIfStatusBarItemExists(uint32_t accessTokenId, bool& isExist);
    virtual int32_t AttachPidToStatusBarItem(uint32_t accessTokenId, int32_t pid);

private:
    int32_t SendRequest(StatusBarDelegateCmd code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    static inline BrokerDelegator<StatusBarDelegateProxy> delegator_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STATUS_BAR_DELEGATE_PROXY_H