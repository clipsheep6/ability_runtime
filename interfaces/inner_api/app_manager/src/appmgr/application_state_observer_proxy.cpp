/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"


namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t ERR_INVALID_STUB = 32;
}
ApplicationStateObserverProxy::ApplicationStateObserverProxy(
    const sptr<IRemoteObject> &impl) : IRemoteProxy<IApplicationStateObserver>(impl)
{}

bool ApplicationStateObserverProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(ApplicationStateObserverProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write interface token failed");
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
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IApplicationStateObserver::Message::TRANSACT_ON_FOREGROUND_APPLICATION_CHANGED),
        data, reply, option);
    if (ret != NO_ERROR && ret != ERR_INVALID_STUB) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d, bundleName: %{public}s.",
            ret, appStateData.bundleName.c_str());
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
    data.WriteParcelable(&abilityStateData);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IApplicationStateObserver::Message::TRANSACT_ON_ABILITY_STATE_CHANGED),
        data, reply, option);
    if (ret != NO_ERROR && ret != ERR_INVALID_STUB) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is wrong, error code: %{public}d, bundleName: %{public}s.",
            ret, abilityStateData.bundleName.c_str());
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
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IApplicationStateObserver::Message::TRANSACT_ON_EXTENSION_STATE_CHANGED),
        data, reply, option);
    if (ret != NO_ERROR && ret != ERR_INVALID_STUB) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is wrong, error code: %{public}d, bundleName:%{public}s.",
            ret, abilityStateData.bundleName.c_str());
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
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_CREATED),
        data, reply, option);
    if (ret != NO_ERROR && ret != ERR_INVALID_STUB) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is wrong, error code: %{public}d, bundleName:%{public}s.",
            ret, processData.bundleName.c_str());
    }
}

void ApplicationStateObserverProxy::OnProcessReused(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&processData);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_REUSED),
        data, reply, option);
    if (ret != NO_ERROR && ret != ERR_INVALID_STUB) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is wrong, error code: %{public}d, bundleName:%{public}s.",
            ret, processData.bundleName.c_str());
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
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_STATE_CHANGED),
        data, reply, option);
    if (ret != NO_ERROR && ret != ERR_INVALID_STUB) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is wrong, error code: %{public}d, bundleName:%{public}s.",
            ret, processData.bundleName.c_str());
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
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
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_DIED),
        data, reply, option);
    if (ret != NO_ERROR && ret != ERR_INVALID_STUB) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is wrong, error code: %{public}d, bundleName:%{public}s.",
            ret, processData.bundleName.c_str());
    }
}

void ApplicationStateObserverProxy::OnApplicationStateChanged(const AppStateData &appStateData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }
    data.WriteParcelable(&appStateData);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IApplicationStateObserver::Message::TRANSACT_ON_APPLICATION_STATE_CHANGED),
        data, reply, option);
    if (ret != NO_ERROR && ret != ERR_INVALID_STUB) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d, bundleName: %{public}s.",
            ret, appStateData.bundleName.c_str());
    }
}

void ApplicationStateObserverProxy::OnAppStateChanged(const AppStateData &appStateData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }
    data.WriteParcelable(&appStateData);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IApplicationStateObserver::Message::TRANSACT_ON_APP_STATE_CHANGED),
        data, reply, option);
    if (ret != NO_ERROR && ret != ERR_INVALID_STUB) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d, , bundleName: %{public}s",
            ret, appStateData.bundleName.c_str());
    }
}

void ApplicationStateObserverProxy::OnAppStarted(const AppStateData &appStateData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }
    data.WriteParcelable(&appStateData);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IApplicationStateObserver::Message::TRANSACT_ON_APP_STARTED),
        data, reply, option);
    if (ret != NO_ERROR && ret != ERR_INVALID_STUB) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d, bundleName: %{public}s.",
            ret, appStateData.bundleName.c_str());
    }
}

void ApplicationStateObserverProxy::OnAppStopped(const AppStateData &appStateData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "OnAppStopped, WriteInterfaceToken failed");
        return;
    }
    data.WriteParcelable(&appStateData);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IApplicationStateObserver::Message::TRANSACT_ON_APP_STOPPED),
        data, reply, option);
    if (ret != NO_ERROR && ret != ERR_INVALID_STUB) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d, bundleName: %{public}s.",
            ret, appStateData.bundleName.c_str());
    }
}

void ApplicationStateObserverProxy::OnPageShow(const PageStateData &pageStateData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }
    data.WriteParcelable(&pageStateData);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IApplicationStateObserver::Message::TRANSACT_ON_PAGE_SHOW),
        data, reply, option);
    if (ret != NO_ERROR && ret != ERR_INVALID_STUB) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d, bundleName: %{public}s",
            ret, pageStateData.bundleName.c_str());
    }
}

void ApplicationStateObserverProxy::OnPageHide(const PageStateData &pageStateData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }
    data.WriteParcelable(&pageStateData);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IApplicationStateObserver::Message::TRANSACT_ON_PAGE_HIDE),
        data, reply, option);
    if (ret != NO_ERROR && ret != ERR_INVALID_STUB) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d, bundleName: %{public}s",
            ret, pageStateData.bundleName.c_str());
    }
}

void ApplicationStateObserverProxy::OnAppCacheStateChanged(const AppStateData &appStateData)
{
}

int32_t ApplicationStateObserverProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    return remote->SendRequest(code, data, reply, option);
}
}  // namespace AppExecFwk
}  // namespace OHOS
