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

#include "acquire_share_data_callback_stub.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

AcquireShareDataCallbackStub::AcquireShareDataCallbackStub() {}

AcquireShareDataCallbackStub::~AcquireShareDataCallbackStub()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "~AcquireShareDataCallbackStub.");
}

int32_t AcquireShareDataCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = AcquireShareDataCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    if (code < IAcquireShareDataCallback::CODE_MAX) {
        if (code == ACQUIRE_SHARE_DATA_DONE) {
            return AcquireShareDataDoneInner(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AcquireShareDataCallbackStub::AcquireShareDataDoneInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t resultCode = data.ReadInt32();
    std::shared_ptr<WantParams> wantParam(data.ReadParcelable<WantParams>());
    if (wantParam == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wantParam is nullptr");
        return ERR_INVALID_VALUE;
    }
    return AcquireShareDataDone(resultCode, *wantParam);
}

int32_t AcquireShareDataCallbackStub::AcquireShareDataDone(int32_t resultCode, WantParams &wantParam)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "resultCode:%{public}d, wantParam size:%{public}d", resultCode, wantParam.Size());
    if (resultCode || wantParam.IsEmpty()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "invaild param.");
    }
    auto task = [resultCode, wantParam, shareRuntimeTask = shareRuntimeTask_]() {
        if (shareRuntimeTask) {
            shareRuntimeTask(resultCode, wantParam);
        }
    };
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AcquireShareDataDone shareRuntimeTask start.");
    if (!handler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "handler_ object is nullptr.");
        return OBJECT_NULL;
    }
    handler_->PostTask(task, "AcquieShareDataDone.");
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AcquireShareDataDone shareRuntimeTask end.");
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