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

#ifndef OHOS_ABILITY_RUNTIME_KIA_INTERCEPTOR_PROXY_H
#define OHOS_ABILITY_RUNTIME_KIA_INTERCEPTOR_PROXY_H

#include "kia_interceptor_interface.h"

#include <iremote_proxy.h>

namespace OHOS {
namespace AppExecFwk {
class KiaInterceptorProxy : public IRemoteProxy<IKiaInterceptor> {
public:
    explicit KiaInterceptorProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IKiaInterceptor>(impl)
    {}

    virtual ~KiaInterceptorProxy()
    {}

    virtual int OnIntercept(AAFwk::Want &want) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    static inline BrokerDelegator<KiaInterceptorProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_KIA_INTERCEPTOR_PROXY_H
