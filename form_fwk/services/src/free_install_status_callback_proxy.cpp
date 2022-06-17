/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "free_install_status_callback_proxy.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AppExecFwk {
FreeInstallStatusCallBackProxy::FreeInstallStatusCallBackProxy(const sptr<IRemoteObject>& impl)
    : IRemoteProxy<IFreeInstallStatusCallBack>(impl)
{
}

void FreeInstallStatusCallBackProxy::OnInstallFinished(int resultCode, const Want &want, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(IFreeInstallStatusCallBack::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteInt32(resultCode)) {
        HILOG_ERROR("Write resultCode error.");
        return;
    }

    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("Write want error.");
        return;
    }

    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("Write userId error.");
        return;
    }

    int error = Remote()->SendRequest(IFreeInstallStatusCallBackCmd::ON_FREE_INSTALL_DONE, data,
        reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("OnFinished fail, error: %{public}d", error);
        return;
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
