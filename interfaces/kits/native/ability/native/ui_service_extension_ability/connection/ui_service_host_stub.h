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
#ifndef OHOS_ABILITY_RUNTIME_UI_SERVICE_HOST_STUB_H
#define OHOS_ABILITY_RUNTIME_UI_SERVICE_HOST_STUB_H

#include <map>

#include <iremote_object.h>
#include <iremote_stub.h>
#include "ipc_types.h"
#include "message_parcel.h"
#include "ui_service_host_interface.h"

namespace OHOS {
namespace AAFwk {

class UIServiceHostStub : public IRemoteStub<IUIServiceHost> {
public:
    UIServiceHostStub();
    virtual ~UIServiceHostStub();

    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

    int32_t OnSendData(MessageParcel& data, MessageParcel& reply);

protected:
    using RequestFuncType = int32_t (UIServiceHostStub::*)(MessageParcel& data, MessageParcel& reply);
    std::map<uint32_t, RequestFuncType> requestFuncMap_;
};

} // namespace AAFwk
} // namespace OHOS
#endif //OHOS_ABILITY_RUNTIME_UI_SERVICE_HOST_STUB_H