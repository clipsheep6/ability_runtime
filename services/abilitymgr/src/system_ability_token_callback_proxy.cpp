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

#include "system_ability_token_callback_proxy.h"

#include "ability_manager_errors.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
const std::u16string SYSTEM_ABILITY_TOKEN_CALLBACK_INTERFACE_TOKEN = u"ohos.aafwk.ISystemAbilityTokenCallback";

int32_t SystemAbilityTokenCallbackProxy::SendResult(OHOS::AAFwk::Want& want, int32_t callerUid,
    int32_t requestCode, uint32_t accessToken, int32_t resultCode)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(SYSTEM_ABILITY_TOKEN_CALLBACK_INTERFACE_TOKEN)) {
        HILOG_ERROR("SendResult Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("fail to WriteParcelable");
        return INNER_ERR;
    }
    data.WriteInt32(callerUid);
    data.WriteInt32(requestCode);
    data.WriteUint32(accessToken);
    data.WriteInt32(resultCode);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    int result = Remote()->SendRequest(ISystemAbilityTokenCallback::SEND_RESULT, data, reply, option);
    if (result != NO_ERROR) {
        HILOG_ERROR("SendResult SendRequest fail, error: %{public}d", result);
        return result;
    }
    return reply.ReadInt32();
}
}  // namespace AAFwk
}  // namespace OHOS
