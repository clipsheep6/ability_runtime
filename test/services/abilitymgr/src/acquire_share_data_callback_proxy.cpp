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
#include "acquire_share_data_callback_proxy.h"
#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "message_parcel.h"
#include "peer_holder.h"
#include "want_params.h"

namespace OHOS {
namespace AAFwk {
bool AcquireShareDataCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AcquireShareDataCallbackProxy::GetDescriptor())) {
        HILOG_ERROR("write interface tokern failed.");
        return false;
    }
    return true;
}

int32_t AcquireShareDataCallbackProxy::AcquireShareDataDone(int32_t resultCode, WantParams &wantParam)
{
    HILOG_INFO("AcquireShareDataDone start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface token failed");
        return INNER_ERR;
    }

    if (!data.WriteInt32(resultCode)) {
        HILOG_ERROR("resultCode write failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&wantParam)) {
        HILOG_ERROR("wantParam write failed.");
        return INNER_ERR;
    }
    auto remote = Remote();
    if (!remote) {
        HILOG_ERROR("remote object is nullptr.");
        return INNER_ERR;
    }
    int32_t ret = remote->SendRequest(IAcquireShareDataCallback::ACQUIRE_SHARE_DATA_DONE, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("AcquireShareDataDone fail to Send request, err: %{public}d.", ret);
    }
    HILOG_INFO("AcquireShareDataDone end.");
    return ret;
}
}
}