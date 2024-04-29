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

#ifndef OHOS_ABILITY_RUNTIME_PREPARE_TERMINATE_CALLBACK_PROXY_H
#define OHOS_ABILITY_RUNTIME_PREPARE_TERMINATE_CALLBACK_PROXY_H

#include "iremote_proxy.h"
#include "iprepare_terminate_callback_interface.h"

namespace OHOS {
namespace AAFwk {
class PrepareTerminateCallbackProxy : public IRemoteProxy<IPrepareTerminateCallback> {
public:
    explicit PrepareTerminateCallbackProxy
        (const sptr<IRemoteObject> &impl) : IRemoteProxy<IPrepareTerminateCallback>(impl) {}

    virtual ~PrepareTerminateCallbackProxy() {}

    virtual void DoPrepareTerminate() override;

private:
    static inline BrokerDelegator<PrepareTerminateCallbackProxy> delegator_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PREPARE_TERMINATE_CALLBACK_PROXY_H
