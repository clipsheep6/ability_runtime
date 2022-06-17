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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FREE_INSTALL_STATUS_CALLBACK_STUB_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FREE_INSTALL_STATUS_CALLBACK_STUB_H

#include <iremote_object.h>
#include <iremote_stub.h>
#include "nocopyable.h"

#include "i_free_install_status_callback.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FreeInstallStatusCallBackStub
 * FreeInstallStatusCallBackStub.
 */
class FreeInstallStatusCallBackStub : public IRemoteStub<IFreeInstallStatusCallBack> {
public:
    FreeInstallStatusCallBackStub();
    virtual ~FreeInstallStatusCallBackStub() = default;

    int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    DISALLOW_COPY_AND_MOVE(FreeInstallStatusCallBackStub);
    int OnInstallFinishedInner(MessageParcel &data, MessageParcel &reply);
    using FreeInstallStatusCallbackFunc = int (FreeInstallStatusCallBackStub::*)(MessageParcel &data,
        MessageParcel &reply);
    std::vector<FreeInstallStatusCallbackFunc> vecMemberFunc_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FREE_INSTALL_STATUS_CALLBACK_STUB_H
