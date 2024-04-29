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
#include "app_running_status_proxy.h"

#include "app_running_status_listener_interface.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AbilityRuntime {
AppRunningStatusProxy::AppRunningStatusProxy(
    const sptr<IRemoteObject> &impl) : IRemoteProxy<AppRunningStatusListenerInterface>(impl)
{}

void AppRunningStatusProxy::NotifyAppRunningStatus(const std::string &bundle, int32_t uid, RunningStatus runningStatus)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AppRunningStatusProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return;
    }
    if (!data.WriteString(bundle) || !data.WriteInt32(uid) || !data.WriteInt32(static_cast<int32_t>(runningStatus))) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write data failed.");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(AppRunningStatusListenerInterface::MessageCode::APP_RUNNING_STATUS), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS