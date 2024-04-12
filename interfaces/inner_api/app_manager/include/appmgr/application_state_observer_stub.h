/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_STATE_OBSERVER_STUB_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_STATE_OBSERVER_STUB_H

#include <map>
#include <mutex>

#include "iremote_stub.h"
#include "nocopyable.h"
#include "string_ex.h"
#include "app_mgr_constants.h"
#include "iapplication_state_observer.h"

namespace OHOS {
namespace AppExecFwk {
class ApplicationStateObserverStub : public IRemoteStub<IApplicationStateObserver> {
public:
    ApplicationStateObserverStub() = default;
    virtual ~ApplicationStateObserverStub() = default;

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    /**
     * Application foreground state changed callback.
     *
     * @param appStateData Application Process data.
     */
    virtual void OnForegroundApplicationChanged(const AppStateData &appStateData) override;

    /**
     * Will be called when the ability state changes.
     *
     * @param abilityStateData Ability state data.
     */
    virtual void OnAbilityStateChanged(const AbilityStateData &abilityStateData) override;

    /**
     * Will be called when the extension state changes.
     *
     * @param abilityStateData Extension state data.
     */
    virtual void OnExtensionStateChanged(const AbilityStateData &abilityStateData) override;

    /**
     * Will be called when the process start.
     *
     * @param processData Process data.
     */
    virtual void OnProcessCreated(const ProcessData &processData) override;

    /**
     * Will be called when the process state change.
     *
     * @param processData Process data.
     */
    virtual void OnProcessStateChanged(const ProcessData &processData) override;

    /**
     * Will be called when the process die.
     *
     * @param processData Process data.
     */
    virtual void OnProcessDied(const ProcessData &processData) override;

    /**
     * Application state changed callback.
     * Only observe APP_STATE_CREATE and APP_STATE_TERMINATED
     *
     * @param appStateData Application state data.
     */
    virtual void OnApplicationStateChanged(const AppStateData &appStateData) override;

    /**
     * Application state changed callback.
     * Only observe APP_STATE_FOREGROUND and APP_STATE_BACKGROUND
     *
     * @param appStateData Application state data.
     */
    virtual void OnAppStateChanged(const AppStateData &appStateData) override;

    virtual void OnProcessReused(const ProcessData &processData) override;

    /**
     * Will be called when the application start.
     *
     * @param appStateData Application state data.
     */
    virtual void OnAppStarted(const AppStateData &appStateData) override;

    /**
     * Will be called when the application stop.
     *
     * @param appStateData Application state data.
     */
    virtual void OnAppStopped(const AppStateData &appStateData) override;

    /**
     * Will be called when page show.
     *
     * @param pageStateData Page state data.
     */
    virtual void OnPageShow(const PageStateData &pageStateData) override;

    /**
     * Will be called whe page hide.
     *
     * @param pageStateData Page state data.
     */
    virtual void OnPageHide(const PageStateData &pageStateData) override;

    /**
     * Will be called when application cache state change.
     *
     * @param appStateData Application state data.
     */
    virtual void OnAppCacheStateChanged(const AppStateData &appStateData) override;

private:
    int32_t HandleOnForegroundApplicationChanged(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnAbilityStateChanged(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnExtensionStateChanged(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnProcessCreated(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnProcessStateChanged(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnProcessDied(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnApplicationStateChanged(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnAppStateChanged(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnProcessReused(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnAppStarted(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnAppStopped(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnPageShow(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnPageHide(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnAppCacheStateChanged(MessageParcel &data, MessageParcel &reply);

    using ApplicationStateObserverFunc = int32_t (ApplicationStateObserverStub::*)(MessageParcel &data,
        MessageParcel &reply);
    const std::map<uint32_t, ApplicationStateObserverFunc> memberFuncMap_ = {
        { static_cast<uint32_t>(Message::TRANSACT_ON_FOREGROUND_APPLICATION_CHANGED),
            &ApplicationStateObserverStub::HandleOnForegroundApplicationChanged },
        { static_cast<uint32_t>(Message::TRANSACT_ON_ABILITY_STATE_CHANGED),
            &ApplicationStateObserverStub::HandleOnAbilityStateChanged },
        { static_cast<uint32_t>(Message::TRANSACT_ON_EXTENSION_STATE_CHANGED),
            &ApplicationStateObserverStub::HandleOnExtensionStateChanged },
        { static_cast<uint32_t>(Message::TRANSACT_ON_PROCESS_CREATED),
            &ApplicationStateObserverStub::HandleOnProcessCreated },
        { static_cast<uint32_t>(Message::TRANSACT_ON_PROCESS_STATE_CHANGED),
            &ApplicationStateObserverStub::HandleOnProcessStateChanged },
        { static_cast<uint32_t>(Message::TRANSACT_ON_PROCESS_DIED),
            &ApplicationStateObserverStub::HandleOnProcessDied },
        { static_cast<uint32_t>(Message::TRANSACT_ON_APPLICATION_STATE_CHANGED),
            &ApplicationStateObserverStub::HandleOnApplicationStateChanged },
        { static_cast<uint32_t>(Message::TRANSACT_ON_APP_STATE_CHANGED),
            &ApplicationStateObserverStub::HandleOnAppStateChanged },
        { static_cast<uint32_t>(Message::TRANSACT_ON_PROCESS_REUSED),
            &ApplicationStateObserverStub::HandleOnProcessReused },
        { static_cast<uint32_t>(Message::TRANSACT_ON_APP_STARTED), &ApplicationStateObserverStub::HandleOnAppStarted },
        { static_cast<uint32_t>(Message::TRANSACT_ON_APP_STOPPED), &ApplicationStateObserverStub::HandleOnAppStopped },
        { static_cast<uint32_t>(Message::TRANSACT_ON_PAGE_SHOW), &ApplicationStateObserverStub::HandleOnPageShow },
        { static_cast<uint32_t>(Message::TRANSACT_ON_PAGE_HIDE), &ApplicationStateObserverStub::HandleOnPageHide },
        { static_cast<uint32_t>(Message::TRANSACT_ON_APP_CACHE_STATE_CHANGED),
            &ApplicationStateObserverStub::HandleOnAppCacheStateChanged },
    };
    static std::mutex callbackMutex_;

    DISALLOW_COPY_AND_MOVE(ApplicationStateObserverStub);
};

/**
 * @class ApplicationStateObserverRecipient
 * ApplicationStateObserverRecipient notices IRemoteBroker died.
 */
class ApplicationStateObserverRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    explicit ApplicationStateObserverRecipient(RemoteDiedHandler handler);
    virtual ~ApplicationStateObserverRecipient();
    virtual void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    RemoteDiedHandler handler_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPLICATION_STATE_OBSERVER_STUB_H
