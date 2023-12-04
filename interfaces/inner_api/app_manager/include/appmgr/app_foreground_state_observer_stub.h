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

#ifndef OHOS_ABILITY_RUNTIME_APP_FOREGROUND_STATE_OBSERVER_STUB_H
#define OHOS_ABILITY_RUNTIME_APP_FOREGROUND_STATE_OBSERVER_STUB_H

#include <map>

#include "app_foreground_state_observer_interface.h"
#include "iremote_stub.h"
#include "nocopyable.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
class AppForegroundStateObserverStub : public IRemoteStub<IAppForegroundStateObserver> {
public:
    AppForegroundStateObserverStub();
    virtual ~AppForegroundStateObserverStub();
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t HandleOnAppStateChanged(MessageParcel &data, MessageParcel &reply);

    using AppForegroundStateObserverFunc = int32_t (AppForegroundStateObserverStub::*)(
        MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, AppForegroundStateObserverFunc> memberFuncMap_;

    DISALLOW_COPY_AND_MOVE(AppForegroundStateObserverStub);
};

/**
 * @class AppForegroundStateObserverRecipient
 * AppForegroundStateObserverRecipient notices IRemoteBroker died.
 */
class AppForegroundStateObserverRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    explicit AppForegroundStateObserverRecipient(RemoteDiedHandler handler);
    virtual ~AppForegroundStateObserverRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

private:
    RemoteDiedHandler handler_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_FOREGROUND_STATE_OBSERVER_STUB_H
