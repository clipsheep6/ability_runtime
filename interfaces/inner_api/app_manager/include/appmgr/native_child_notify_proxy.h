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

#ifndef OHOS_ABILITY_RUNTIME_NATIVE_CHILD_NOTIFY_PROXY_H
#define OHOS_ABILITY_RUNTIME_NATIVE_CHILD_NOTIFY_PROXY_H

#include "native_child_notify_interface.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {

class NativeChildNotifyProxy : public IRemoteProxy<INativeChildNotify> {
public:
    explicit NativeChildNotifyProxy(const sptr<IRemoteObject> &impl);
    virtual ~NativeChildNotifyProxy() = default;

    void OnNativeChildStarted(const sptr<IRemoteObject> &nativeChild) override;
    void OnError(int32_t errCode) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    int32_t SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption& option);

    static inline BrokerDelegator<NativeChildNotifyProxy> delegator_;
};

} // OHOS
} // AppExecFwk

#endif // OHOS_ABILITY_RUNTIME_NATIVE_CHILD_NOTIFY_PROXY_H