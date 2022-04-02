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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FREE_INSTALL_STATUS_CALLBACK_STUB_H
#define FOUNDATION_APPEXECFWK_OHOS_FREE_INSTALL_STATUS_CALLBACK_STUB_H

#include <map>

#include "free_install_status_callback_interface.h"

#include "iremote_object.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FreeInstallStatusCallbackStub
 * FreeInstallStatusCallbackStub.
 */
class FreeInstallStatusCallbackStub : public IRemoteStub<AAFwk::IFreeInstallStatusCallback> {
public:
    FreeInstallStatusCallbackStub();
    virtual ~FreeInstallStatusCallbackStub() = default;

    int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int OnInstallFinishedInner(MessageParcel &data, MessageParcel &reply);
    using FreeInstallStatusCallbackFunc = int (FreeInstallStatusCallbackStub::*)(MessageParcel &data,
        MessageParcel &reply);
    std::map<uint32_t, FreeInstallStatusCallbackFunc> memberFuncMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FREE_INSTALL_STATUS_CALLBACK_STUB_H
