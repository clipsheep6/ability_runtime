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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_FOREGROUND_STATE_OBSERVER_STUB_H
#define OHOS_ABILITY_RUNTIME_ABILITY_FOREGROUND_STATE_OBSERVER_STUB_H

#include <map>
#include <mutex>

#include "ability_foreground_state_observer_interface.h"
#include "ability_state_data.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityForegroundStateObserverStub : public IRemoteStub<IAbilityForegroundStateObserver> {
public:
    AbilityForegroundStateObserverStub();
    virtual ~AbilityForegroundStateObserverStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t HandleOnAbilityStateChanged(MessageParcel &data, MessageParcel &reply);

    using AbilityForegroundStateObserverFunc = int32_t (AbilityForegroundStateObserverStub::*)(
        MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, AbilityForegroundStateObserverFunc> memberFuncMap_;
    static std::mutex callbackMutex_;

    DISALLOW_COPY_AND_MOVE(AbilityForegroundStateObserverStub);
};

/**
 * @class AppForegroundStateObserverRecipient
 * AppForegroundStateObserverRecipient notices IRemoteBroker died.
 */
class AbilityForegroundStateObserverRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    explicit AbilityForegroundStateObserverRecipient(RemoteDiedHandler handler);
    virtual ~AbilityForegroundStateObserverRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

private:
    RemoteDiedHandler handler_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_FOREGROUND_STATE_OBSERVER_STUB_H
