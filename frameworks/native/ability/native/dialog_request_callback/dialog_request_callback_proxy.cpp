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

#include "dialog_request_callback_proxy.h"

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AbilityRuntime {
void DialogRequestCallbackProxy::SendResult(int32_t resultCode, const AAFwk::Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    TAG_LOGI(AAFwkTag::DIALOG, "send result");
    if (!data.WriteInterfaceToken(IDialogRequestCallback::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::DIALOG, "Write interface token failed.");
        return;
    }

    if (!data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::DIALOG, "Write resultCode error.");
        return;
    }

    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::DIALOG, "want write failed.");
        return;
    }

    auto remote = Remote();
    if (remote) {
        auto errCode = remote->SendRequest(IDialogRequestCallback::CODE_SEND_RESULT, data, reply, option);
        TAG_LOGI(AAFwkTag::DIALOG, "SendRequest result, error code:%{public}d", errCode);
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS
