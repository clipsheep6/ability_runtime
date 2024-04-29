/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILIYT_DEBUG_RESPONSE_STUB_H
#define OHOS_ABILITY_RUNTIME_ABILIYT_DEBUG_RESPONSE_STUB_H

#include <map>

#include "ability_debug_response_interface.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityDebugResponseStub : public IRemoteStub<IAbilityDebugResponse> {
public:
    AbilityDebugResponseStub();
    virtual ~AbilityDebugResponseStub();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t HandleOnAbilitysDebugStarted(MessageParcel &data, MessageParcel &reply);
    int32_t HandleOnAbilitysDebugStoped(MessageParcel &data, MessageParcel &reply);
    int32_t HandleOnAbilitysAssertDebugChange(MessageParcel &data, MessageParcel &reply);

    using AbilityDebugResponseFunc = int32_t (AbilityDebugResponseStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, AbilityDebugResponseFunc> responseFuncMap_;

    DISALLOW_COPY_AND_MOVE(AbilityDebugResponseStub);
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILIYT_DEBUG_RESPONSE_STUB_H
