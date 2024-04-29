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

#ifndef OHOS_ABILITY_RUNTIME_SYSTEM_ABILITY_TOKEN_CALLBACK_STUB_H
#define OHOS_ABILITY_RUNTIME_SYSTEM_ABILITY_TOKEN_CALLBACK_STUB_H

#include "iremote_object.h"
#include "iremote_stub.h"
#include "system_ability_token_callback.h"

namespace OHOS {
namespace AAFwk {
class SystemAbilityTokenCallbackStub : public IRemoteStub<ISystemAbilityTokenCallback> {
public:
    SystemAbilityTokenCallbackStub() = default;
    ~SystemAbilityTokenCallbackStub() = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
private:
    DISALLOW_COPY_AND_MOVE(SystemAbilityTokenCallbackStub);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_SYSTEM_ABILITY_TOKEN_CALLBACK_STUB_H
