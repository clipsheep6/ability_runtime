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

#ifndef OHOS_ABILITY_RUNTIME_ACQUIRE_SHARE_DATA_CALLBACK_STUB_H
#define OHOS_ABILITY_RUNTIME_ACQUIRE_SHARE_DATA_CALLBACK_STUB_H

#include "event_handler.h"
#include "iacquire_share_data_callback_interface.h"
#include <iremote_stub.h>
#include "want_params.h"

namespace OHOS {
namespace AAFwk {

using ShareRuntimeTask = std::function<void(int32_t, const WantParams&)>;

class AcquireShareDataCallbackStub : public IRemoteStub<IAcquireShareDataCallback> {
public:
    AcquireShareDataCallbackStub();
    virtual ~AcquireShareDataCallbackStub();
    virtual int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    virtual int32_t AcquireShareDataDoneInner(MessageParcel &data, MessageParcel &reply);
    virtual int32_t AcquireShareDataDone(int32_t resultCode, WantParams &wantParam) override;
    void SetHandler(std::shared_ptr<AppExecFwk::EventHandler> handler);
    void SetShareRuntimeTask(ShareRuntimeTask &shareRuntimeTask);

private:
    DISALLOW_COPY_AND_MOVE(AcquireShareDataCallbackStub);
    
    ShareRuntimeTask shareRuntimeTask_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    using StubFunc = int32_t (AcquireShareDataCallbackStub::*)(MessageParcel &data, MessageParcel &reply);
    std::vector<StubFunc> vecMemberFunc_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ACQUIRE_SHARE_DATA_CALLBACK_STUB_H