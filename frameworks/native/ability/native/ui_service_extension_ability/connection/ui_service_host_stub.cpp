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

#include "ui_service_host_stub.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

UIServiceHostStub::UIServiceHostStub()
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "called");
    requestFuncMap_[SEND_DATA] = &UIServiceHostStub::OnSendData;
}

UIServiceHostStub::~UIServiceHostStub()
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "called");
    requestFuncMap_.clear();
}

int UIServiceHostStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    std::u16string descriptor = UIServiceHostStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        return ERR_INVALID_STATE;
    }
    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t UIServiceHostStub::OnSendData(MessageParcel& data, MessageParcel& reply)
{
    std::unique_ptr<AAFwk::WantParams> wantParams(data.ReadParcelable<AAFwk::WantParams>());
    if (wantParams == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "UIServiceHostStub::OnSendData, read WantParams failed");
        return ERR_INVALID_VALUE;
    }
    int32_t result = SendData(*wantParams);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "UIServiceHostStub::OnSendData, write result failed.");
        return IPC_STUB_ERR;
    }
    return NO_ERROR;
}
}
}
