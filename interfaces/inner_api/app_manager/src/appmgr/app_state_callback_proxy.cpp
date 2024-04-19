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

#include "app_state_callback_proxy.h"

#include "configuration.h"
#include "ipc_types.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AppStateCallbackProxy::AppStateCallbackProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAppStateCallback>(impl)
{}

bool AppStateCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AppStateCallbackProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write interface token failed");
        return false;
    }
    return true;
}

void AppStateCallbackProxy::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const AbilityState state)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (token) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(token.GetRefPtr())) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag and token");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag");
            return;
        }
    }

    int32_t abilityState = static_cast<int32_t>(state);
    data.WriteInt32(abilityState);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_ABILITY_REQUEST_DONE), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AppStateCallbackProxy::OnAppStateChanged(const AppProcessData &appProcessData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&appProcessData);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_APP_STATE_CHANGED), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AppStateCallbackProxy::NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return;
    }
    if (!data.WriteParcelable(&config)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write config failed.");
        return;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write usr failed.");
        return;
    }
    auto error = SendTransactCmd(
        static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_CONFIG_CHANGE), data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send config error: %{public}d", error);
    }
}

void AppStateCallbackProxy::NotifyStartResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(bundleInfos.size())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write bundle info size failed.");
        return;
    }

    for (auto &bundleInfo : bundleInfos) {
        if (!data.WriteParcelable(&bundleInfo)) {
            TAG_LOGE(AAFwkTag::APPMGR, "write bundle info failed");
            return;
        }
    }
    if (!SendTransactCmd(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_START_RESIDENT_PROCESS, data, reply)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendTransactCmd failed");
    }
}

int32_t AppStateCallbackProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    auto ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error code: %{public}d", ret);
        return ret;
    }
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
