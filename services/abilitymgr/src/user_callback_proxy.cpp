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

#include "user_callback_proxy.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
void UserCallbackProxy::OnStopUserDone(int userId, int errcode)
{
    SendRequestCommon(userId, errcode, IUserCallback::UserCallbackCmd::ON_STOP_USER_DONE);
}

void UserCallbackProxy::OnStartUserDone(int userId, int errcode)
{
    SendRequestCommon(userId, errcode, IUserCallback::UserCallbackCmd::ON_START_USER_DONE);
}

void UserCallbackProxy::SendRequestCommon(int userId, int errcode, IUserCallback::UserCallbackCmd cmd)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "UserCallbackProxy, sendrequest, cmd:%{public}d, userId:%{public}d, errcode:%{public}d", cmd, userId, errcode);
    if (!data.WriteInterfaceToken(IUserCallback::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return;
    }

    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write userId error.");
        return;
    }

    if (!data.WriteInt32(errcode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write errcode error.");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remote object is nullptr.");
        return;
    }

    int error = remote->SendRequest(cmd, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest fail, error: %{public}d", error);
        return;
    }
}
}  // namespace AAFwk
}  // namespace OHOS
