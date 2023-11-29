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

#include "acquire_share_data_callback_stub.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {

AcquireShareDataCallbackStub::AcquireShareDataCallbackStub()
{
    vecMemberFunc_.resize(IAcquireShareDataCallback::CODE_MAX);
    vecMemberFunc_[ACQUIRE_SHARE_DATA_DONE] = &AcquireShareDataCallbackStub::AcquireShareDataDoneInner;
}

AcquireShareDataCallbackStub::~AcquireShareDataCallbackStub()
{
    HILOG_INFO("~AcquireShareDataCallbackStub.");
}

int32_t AcquireShareDataCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = AcquireShareDataCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_INFO("local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    if (code < IAcquireShareDataCallback::CODE_MAX) {
        auto memberFunc = vecMemberFunc_[code];
        return (this->*memberFunc)(data, reply);
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AcquireShareDataCallbackStub::AcquireShareDataDoneInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t resultCode = data.ReadInt32();
    std::shared_ptr<WantParams> wantParam(data.ReadParcelable<WantParams>());
    if (wantParam == nullptr) {
        HILOG_ERROR("wantParam is nullptr");
        return ERR_INVALID_VALUE;
    }
    return AcquireShareDataDone(resultCode, *wantParam);
}

int32_t AcquireShareDataCallbackStub::AcquireShareDataDone(int32_t resultCode, WantParams &wantParam)
{
    HILOG_INFO("resultCode:%{public}d, wantParam size:%{public}d", resultCode, wantParam.Size());
    if (resultCode || wantParam.IsEmpty()) {
        HILOG_INFO("invaild param.");
    }
    auto task = [resultCode, wantParam, shareRuntimeTask = shareRuntimeTask_]() {
        if (shareRuntimeTask) {
            shareRuntimeTask(resultCode, wantParam);
        }
    };
    HILOG_INFO("AcquireShareDataDone shareRuntimeTask start.");
    if (!handler_) {
        HILOG_ERROR("handler_ object is nullptr.");
        return OBJECT_NULL;
    }
    handler_->PostTask(task, "AcquieShareDataDone.");
    HILOG_INFO("AcquireShareDataDone shareRuntimeTask end.");
    return NO_ERROR;
}

void AcquireShareDataCallbackStub::SetHandler(std::shared_ptr<AppExecFwk::EventHandler> handler)
{
    handler_ = handler;
}

void AcquireShareDataCallbackStub::SetShareRuntimeTask(ShareRuntimeTask &shareRuntimeTask)
{
    shareRuntimeTask_ = shareRuntimeTask;
}
}
}