/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "application_state_observer_stub.h"
#include "appexecfwk_errors.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
ApplicationStateObserverStub::ApplicationStateObserverStub()
{
    memberFuncMap_[static_cast<uint32_t>(
        IApplicationStateObserver::Message::TRANSACT_ON_FOREGROUND_APPLICATION_CHANGED)] =
        &ApplicationStateObserverStub::HandleOnForegroundApplicationChanged;
    memberFuncMap_[static_cast<uint32_t>(
        IApplicationStateObserver::Message::TRANSACT_ON_ABILITY_STATE_CHANGED)] =
        &ApplicationStateObserverStub::HandleOnAbilityStateChanged;
    memberFuncMap_[static_cast<uint32_t>(
        IApplicationStateObserver::Message::TRANSACT_ON_EXTENSION_STATE_CHANGED)] =
        &ApplicationStateObserverStub::HandleOnExtensionStateChanged;
    memberFuncMap_[static_cast<uint32_t>(
        IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_CREATED)] =
        &ApplicationStateObserverStub::HandleOnProcessCreated;
    memberFuncMap_[static_cast<uint32_t>(
        IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_STATE_CHANGED)] =
        &ApplicationStateObserverStub::HandleOnProcessStateChanged;
    memberFuncMap_[static_cast<uint32_t>(
        IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_DIED)] =
        &ApplicationStateObserverStub::HandleOnProcessDied;
    memberFuncMap_[static_cast<uint32_t>(
        IApplicationStateObserver::Message::TRANSACT_ON_APPLICATION_STATE_CHANGED)] =
        &ApplicationStateObserverStub::HandleOnApplicationStateChanged;
    memberFuncMap_[static_cast<uint32_t>(
        IApplicationStateObserver::Message::TRANSACT_ON_APP_STATE_CHANGED)] =
        &ApplicationStateObserverStub::HandleOnAppStateChanged;
    memberFuncMap_[static_cast<uint32_t>(
        IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_REUSED)] =
        &ApplicationStateObserverStub::HandleOnProcessReused;
    memberFuncMap_[static_cast<uint32_t>(
        IApplicationStateObserver::Message::TRANSACT_ON_APP_STARTED)] =
        &ApplicationStateObserverStub::HandleOnAppStarted;
    memberFuncMap_[static_cast<uint32_t>(
        IApplicationStateObserver::Message::TRANSACT_ON_APP_STOPPED)] =
        &ApplicationStateObserverStub::HandleOnAppStopped;
    memberFuncMap_[static_cast<uint32_t>(
        IApplicationStateObserver::Message::TRANSACT_ON_PAGE_SHOW)] =
        &ApplicationStateObserverStub::HandleOnPageShow;
    memberFuncMap_[static_cast<uint32_t>(
        IApplicationStateObserver::Message::TRANSACT_ON_PAGE_HIDE)] =
        &ApplicationStateObserverStub::HandleOnPageHide;
}

ApplicationStateObserverStub::~ApplicationStateObserverStub()
{
    memberFuncMap_.clear();
}

int ApplicationStateObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_DEBUG("ApplicationStateObserverStub::OnReceived, code = %{public}u, flags= %{public}d.",
        code, option.GetFlags());
    std::u16string descriptor = ApplicationStateObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HILOG_DEBUG("ApplicationStateObserverStub::OnRemoteRequest end");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

void ApplicationStateObserverStub::OnForegroundApplicationChanged(const AppStateData &appStateData)
{}

void ApplicationStateObserverStub::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{}

void ApplicationStateObserverStub::OnExtensionStateChanged(const AbilityStateData &abilityStateData)
{}

void ApplicationStateObserverStub::OnProcessCreated(const ProcessData &processData)
{}

void ApplicationStateObserverStub::OnProcessStateChanged(const ProcessData &processData)
{}

void ApplicationStateObserverStub::OnProcessDied(const ProcessData &processData)
{}

void ApplicationStateObserverStub::OnApplicationStateChanged(const AppStateData &appStateData)
{}

void ApplicationStateObserverStub::OnAppStateChanged(const AppStateData &appStateData)
{}

void ApplicationStateObserverStub::OnProcessReused(const ProcessData &processData)
{}

void ApplicationStateObserverStub::OnAppStarted(const AppStateData &appStateData)
{}

void ApplicationStateObserverStub::OnAppStopped(const AppStateData &appStateData)
{}

void ApplicationStateObserverStub::OnPageShow(const PageStateData &pageStateData)
{}

void ApplicationStateObserverStub::OnPageHide(const PageStateData &pageStateData)
{}

int32_t ApplicationStateObserverStub::HandleOnForegroundApplicationChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppStateData> processData(data.ReadParcelable<AppStateData>());
    if (!processData) {
        HILOG_ERROR("ReadParcelable<AppStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnForegroundApplicationChanged(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnAbilityStateChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AbilityStateData> abilityStateData(data.ReadParcelable<AbilityStateData>());
    if (!abilityStateData) {
        HILOG_ERROR("ReadParcelable<AbilityStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    OnAbilityStateChanged(*abilityStateData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnExtensionStateChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AbilityStateData> abilityStateData(data.ReadParcelable<AbilityStateData>());
    if (!abilityStateData) {
        HILOG_ERROR("ReadParcelable<AbilityStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    OnExtensionStateChanged(*abilityStateData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnProcessCreated(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ProcessData> processData(data.ReadParcelable<ProcessData>());
    if (!processData) {
        HILOG_ERROR("ReadParcelable<ProcessData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnProcessCreated(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnProcessStateChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ProcessData> processData(data.ReadParcelable<ProcessData>());
    if (!processData) {
        HILOG_ERROR("ReadParcelable<ProcessData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnProcessStateChanged(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnProcessDied(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ProcessData> processData(data.ReadParcelable<ProcessData>());
    if (!processData) {
        HILOG_ERROR("ReadParcelable<ProcessData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnProcessDied(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnApplicationStateChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppStateData> processData(data.ReadParcelable<AppStateData>());
    if (!processData) {
        HILOG_ERROR("ReadParcelable<AppStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnApplicationStateChanged(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnAppStateChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppStateData> processData(data.ReadParcelable<AppStateData>());
    if (!processData) {
        HILOG_ERROR("ReadParcelable<AppStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnAppStateChanged(*processData);
    return NO_ERROR;
}

void ApplicationStateObserverRecipient::OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote)
{
    HILOG_DEBUG("called");
    if (handler_) {
        handler_(remote);
    }
}

int32_t ApplicationStateObserverStub::HandleOnProcessReused(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ProcessData> processData(data.ReadParcelable<ProcessData>());
    if (!processData) {
        HILOG_ERROR("ReadParcelable<ProcessData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnProcessReused(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnAppStarted(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppStateData> processData(data.ReadParcelable<AppStateData>());
    if (!processData) {
        HILOG_ERROR("ReadParcelable<AppStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnAppStarted(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnAppStopped(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppStateData> processData(data.ReadParcelable<AppStateData>());
    if (!processData) {
        HILOG_ERROR("ReadParcelable<AppStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnAppStopped(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnPageShow(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<PageStateData> pageStateData(data.ReadParcelable<PageStateData>());
    if (!pageStateData) {
        HILOG_ERROR("ReadParcelable<pageStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnPageShow(*pageStateData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnPageHide(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<PageStateData> pageStateData(data.ReadParcelable<PageStateData>());
    if (!pageStateData) {
        HILOG_ERROR("ReadParcelable<pageStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnPageHide(*pageStateData);
    return NO_ERROR;
}

ApplicationStateObserverRecipient::ApplicationStateObserverRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

ApplicationStateObserverRecipient::~ApplicationStateObserverRecipient()
{}
}  // namespace AppExecFwk
}  // namespace OHOS
