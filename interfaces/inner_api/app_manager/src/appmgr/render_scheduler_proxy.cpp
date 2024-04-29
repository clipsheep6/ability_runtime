/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "render_scheduler_proxy.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"


namespace OHOS {
namespace AppExecFwk {
RenderSchedulerProxy::RenderSchedulerProxy(
    const sptr<IRemoteObject> &impl) : IRemoteProxy<IRenderScheduler>(impl)
{}

bool RenderSchedulerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(RenderSchedulerProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write interface token failed");
        return false;
    }
    return true;
}

void RenderSchedulerProxy::NotifyBrowserFd(int32_t ipcFd, int32_t sharedFd,
                                           int32_t crashFd)
{
    TAG_LOGD(AAFwkTag::APPMGR, "NotifyBrowserFd start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteFileDescriptor(ipcFd) || !data.WriteFileDescriptor(sharedFd) ||
        !data.WriteFileDescriptor(crashFd)) {
        TAG_LOGE(AAFwkTag::APPMGR, "want fd failed, ipcFd:%{public}d, sharedFd:%{public}d, "
            "crashFd:%{public}d", ipcFd, sharedFd, crashFd);
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IRenderScheduler::Message::NOTIFY_BROWSER_FD), data,
        reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "NotifyBrowserFd end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
