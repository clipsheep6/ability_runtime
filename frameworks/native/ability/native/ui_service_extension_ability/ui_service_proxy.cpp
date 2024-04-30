/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ui_service_proxy.h"
#include "ipc_types.h"
#include "ability_manager_ipc_interface_code.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {

void UIServiceProxy::SendData(OHOS::AAFwk::WantParams &data)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "UIServiceProxy::SendData");

    MessageParcel parcelData;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!parcelData.WriteInterfaceToken(UIServiceProxy::GetDescriptor())) {
        return;
    }
    if (!parcelData.WriteParcelable(&data)) {
        return;
    }
    sptr<IRemoteObject> remoteObject = Remote();
    if (remoteObject == nullptr) {
        return;
    }
    if (!remoteObject->SendRequest(static_cast<uint32_t>(IUiService::SENDDATA), parcelData, reply, option)) {
        return ;
    }

    TAG_LOGI(AAFwkTag::UISERVC_EXT, "UIServiceProxy::SendData success");
}

}
}
