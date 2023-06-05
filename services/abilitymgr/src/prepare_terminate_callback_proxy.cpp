/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ability_manager_errors.h"
#include "prepare_terminate_callback_proxy.h"
#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "message_parcel.h"
#include "peer_holder.h"
#include "want_params.h"

namespace OHOS {
namespace AAFwk {
void PrepareTerminateCallbackProxy::DoPrepareTerminate()
{
    HILOG_DEBUG("call");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IPrepareTerminateCallback::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int error = remote->SendRequest(ON_DO_PREPARE_TERMINATE, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
    }
}
}
}
