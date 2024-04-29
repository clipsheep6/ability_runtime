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

#include "app_debug_listener_proxy.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t CYCLE_LIMIT_MIN = 0;
constexpr int32_t CYCLE_LIMIT_MAX = 1000;
}
AppDebugListenerProxy::AppDebugListenerProxy(
    const sptr<IRemoteObject> &impl) : IRemoteProxy<IAppDebugListener>(impl)
{}

bool AppDebugListenerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AppDebugListenerProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return false;
    }
    return true;
}

void AppDebugListenerProxy::OnAppDebugStarted(const std::vector<AppDebugInfo> &debugInfos)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    SendRequest(IAppDebugListener::Message::ON_APP_DEBUG_STARTED, debugInfos);
}

void AppDebugListenerProxy::OnAppDebugStoped(const std::vector<AppDebugInfo> &debugInfos)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    SendRequest(IAppDebugListener::Message::ON_APP_DEBUG_STOPED, debugInfos);
}

void AppDebugListenerProxy::SendRequest(
    const IAppDebugListener::Message &message, const std::vector<AppDebugInfo> &debugInfos)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return;
    }

    if (debugInfos.size() <= CYCLE_LIMIT_MIN || debugInfos.size() > CYCLE_LIMIT_MAX ||
        !data.WriteInt32(debugInfos.size())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write debug info size failed.");
        return;
    }
    for (auto &debugInfo : debugInfos) {
        if (!data.WriteParcelable(&debugInfo)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Write debug info failed.");
            return;
        }
    };

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(message), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}
} // namespace AppExecFwk
} // namespace OHOS
