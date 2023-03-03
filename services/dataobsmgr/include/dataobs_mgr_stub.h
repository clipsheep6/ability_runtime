/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_DATAOBS_MGR_STUB_H
#define OHOS_ABILITY_RUNTIME_DATAOBS_MGR_STUB_H

#include "dataobs_mgr_interface.h"

#include <iremote_object.h>
#include <iremote_stub.h>
#include <map>

#include "hilog_wrapper.h"
#include "dataobs_mgr_errors.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class DataObsManagerStub
 * DataObsManagerStub.
 */
class DataObsManagerStub : public IRemoteStub<IDataObsMgr> {
public:
    DataObsManagerStub();
    ~DataObsManagerStub();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t RegisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyChangeInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterObserverExtInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterObserverExtInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterObserverExtALLInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyChangeExtInner(MessageParcel &data, MessageParcel &reply);

    using RequestFuncType = int32_t (DataObsManagerStub::*)(MessageParcel &data, MessageParcel &reply);
    static const RequestFuncType HANDLES[TRANS_BUTT];
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DATAOBS_MGR_STUB_H
