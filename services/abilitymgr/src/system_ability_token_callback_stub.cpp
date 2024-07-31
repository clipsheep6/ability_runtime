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

#include "system_ability_token_callback_stub.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::u16string SYSTEM_ABILITY_TOKEN_CALLBACK_INTERFACE_TOKEN = u"ohos.aafwk.ISystemAbilityTokenCallback";
}
int32_t SystemAbilityTokenCallbackStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != SYSTEM_ABILITY_TOKEN_CALLBACK_INTERFACE_TOKEN) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnRemoteRequest::ReadInterfaceToken error");
        return ERR_PERMISSION_DENIED;
    }
    switch (code) {
        case SEND_RESULT: {
            std::shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
            if (want == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "SEND_RESULT want readParcelable failed!");
                return ERR_NULL_OBJECT;
            }
            int32_t callerUid = data.ReadInt32();
            int32_t requestCode = data.ReadInt32();
            uint32_t accessToken = data.ReadUint32();
            int32_t resultCode = data.ReadInt32();
            return SendResult(*want, callerUid, requestCode, accessToken, resultCode);
        }
        default: {
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}
}  // namespace AAFwk
}  // namespace OHOS
