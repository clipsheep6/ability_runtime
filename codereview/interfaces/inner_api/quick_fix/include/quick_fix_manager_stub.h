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

#ifndef OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_STUB_H
#define OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_STUB_H

#include <map>

#include "iremote_stub.h"
#include "message_parcel.h"
#include "nocopyable.h"
#include "quick_fix_manager_interface.h"

namespace OHOS {
namespace AAFwk {
class QuickFixManagerStub : public IRemoteStub<AAFwk::IQuickFixManager> {
public:
    QuickFixManagerStub();
    virtual ~QuickFixManagerStub();

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t ApplyQuickFixInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetApplyedQuickFixInfoInner(MessageParcel &data, MessageParcel &reply);

    using RequestFuncType = int32_t (QuickFixManagerStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, RequestFuncType> requestFuncMap_;

    DISALLOW_COPY_AND_MOVE(QuickFixManagerStub);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_STUB_H
