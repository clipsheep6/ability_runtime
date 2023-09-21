/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "application_state_observer_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"


namespace OHOS {
namespace AppExecFwk {
ApplicationStateObserverProxy::ApplicationStateObserverProxy(
    const sptr<IRemoteObject> &impl) : IRemoteProxy<IApplicationStateObserver>(impl)
{}

bool ApplicationStateObserverProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(ApplicationStateObserverProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

void ApplicationStateObserverProxy::OnForegroundApplicationChanged(const AppStateData &appStateData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&appStateData);

    IApplicationStateObserver::Message code(IApplicationStateObserver::Message::TRANSACT_ON_FOREGROUND_APPLICATION_CHANGED);
    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void ApplicationStateObserverProxy::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    IApplicationStateObserver::Message code(IApplicationStateObserver::Message::TRANSACT_ON_ABILITY_STATE_CHANGED);
    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void ApplicationStateObserverProxy::OnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&abilityStateData);
    if (!SendRequest(IApplicationStateObserver::Message::TRANSACT_ON_EXTENSION_STATE_CHANGED,
        data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void ApplicationStateObserverProxy::OnProcessCreated(const ProcessData &processData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&processData);
    IApplicationStateObserver::Message code(IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_CREATED);
    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void ApplicationStateObserverProxy::OnProcessReused(const ProcessData &processData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&processData);
    IApplicationStateObserver::Message code(IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_REUSED);
    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void ApplicationStateObserverProxy::OnProcessStateChanged(const ProcessData &processData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&processData);
    IApplicationStateObserver::Message code(IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_STATE_CHANGED);
    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void ApplicationStateObserverProxy::OnProcessDied(const ProcessData &processData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&processData);
    IApplicationStateObserver::Message code(IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_DIED);
    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void ApplicationStateObserverProxy::OnApplicationStateChanged(const AppStateData &appStateData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("OnApplicationStateChanged, WriteInterfaceToken failed");
        return;
    }
    data.WriteParcelable(&appStateData);
    if (!SendRequest(IApplicationStateObserver::Message::TRANSACT_ON_APPLICATION_STATE_CHANGED,
        data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void ApplicationStateObserverProxy::OnAppStateChanged(const AppStateData &appStateData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("OnAppStateChanged, WriteInterfaceToken failed");
        return;
    }
    data.WriteParcelable(&appStateData);
    if (!SendRequest(IApplicationStateObserver::Message::TRANSACT_ON_APP_STATE_CHANGED, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void ApplicationStateObserverProxy::OnAppStarted(const AppStateData &appStateData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("OnAppStarted, WriteInterfaceToken failed");
        return;
    }
    data.WriteParcelable(&appStateData);
    if (!SendRequest(IApplicationStateObserver::Message::TRANSACT_ON_APP_STARTED, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void ApplicationStateObserverProxy::OnAppStopped(const AppStateData &appStateData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("OnAppStopped, WriteInterfaceToken failed");
        return;
    }
    data.WriteParcelable(&appStateData);
    if (!SendRequest(IApplicationStateObserver::Message::TRANSACT_ON_APP_STOPPED, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

bool ApplicationStateObserverProxy::SendRequest(IApplicationStateObserver::Message code, MessageParcel &data,
                                                MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("remote object is nullptr.");
        return false;
    }

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest failed. code is %{public}d, ret is %{public}d.", code, ret);
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
