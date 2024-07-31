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

#include "acquire_share_data_callback_proxy.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool AcquireShareDataCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AcquireShareDataCallbackProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface tokern failed.");
        return false;
    }
    return true;
}

int32_t AcquireShareDataCallbackProxy::AcquireShareDataDone(int32_t resultCode, WantParams &wantParam)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AcquireShareDataDone start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed");
        return INNER_ERR;
    }

    if (!data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "resultCode write failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&wantParam)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wantParam write failed.");
        return INNER_ERR;
    }
    auto remote = Remote();
    if (!remote) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remote object is nullptr.");
        return INNER_ERR;
    }
    int32_t ret = remote->SendRequest(IAcquireShareDataCallback::ACQUIRE_SHARE_DATA_DONE, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AcquireShareDataDone fail to Send request, err: %{public}d.", ret);
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AcquireShareDataDone end.");
    return ret;
}
}
}