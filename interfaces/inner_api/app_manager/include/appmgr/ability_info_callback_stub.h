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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_INFO_CALLBACK_STUB_H
#define OHOS_ABILITY_RUNTIME_ABILITY_INFO_CALLBACK_STUB_H

#include "iability_info_callback.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Transfer abilityInfo to the initiator.
 */
class AbilityInfoCallbackStub : public IRemoteStub<IAbilityInfoCallback> {
public:
    AbilityInfoCallbackStub();
    virtual ~AbilityInfoCallbackStub();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

   /**
     * Notify the initiator of the ability token.
     *
     * @param token The token of ability.
     * @param want The want of ability to start.
     */
    virtual void NotifyAbilityToken(const sptr<IRemoteObject> token, const Want &want) override;

private:
    int32_t HandleNotifyAbilityToken(MessageParcel &data, MessageParcel &reply);

    using AbilityInfoCallbackFunc = int32_t (AbilityInfoCallbackStub::*)(MessageParcel &data,
        MessageParcel &reply);
    std::map<uint32_t, AbilityInfoCallbackFunc> memberFuncMap_;

    DISALLOW_COPY_AND_MOVE(AbilityInfoCallbackStub);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_INFO_CALLBACK_STUB_H
