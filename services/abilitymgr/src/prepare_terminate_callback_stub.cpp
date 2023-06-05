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

#include "prepare_terminate_callback_stub.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {

PrepareTerminateCallbackStub::PrepareTerminateCallbackStub()
{
    requestFuncMap_[ON_DO_PREPARE_TERMINATE] = &PrepareTerminateCallbackStub::DoPrepareTerminateInner;
}

PrepareTerminateCallbackStub::~PrepareTerminateCallbackStub()
{
    HILOG_INFO("call");
    requestFuncMap_.clear();
}

int32_t PrepareTerminateCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IPrepareTerminateCallback::GetDescriptor()) {
        HILOG_ERROR("InterfaceToken not equal IPrepareTerminateCallback's descriptor.");
        return ERR_INVALID_STATE;
    }

    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    HILOG_WARN("default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t PrepareTerminateCallbackStub::DoPrepareTerminateInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("call");
    DoPrepareTerminate();
    return ERR_OK;
}
}
}
