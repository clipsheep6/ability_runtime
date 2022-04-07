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

#include "free_install_status_callback_stub.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AppExecFwk {
FreeInstallStatusCallbackStub::FreeInstallStatusCallbackStub()
{
    memberFuncMap_[IFreeInstallStatusCallbackCmd::ON_FREE_INSTALL_DONE] =
        &FreeInstallStatusCallbackStub::OnInstallFinishedInner;
}

int FreeInstallStatusCallbackStub::OnInstallFinishedInner(MessageParcel &data, MessageParcel &reply)
{
    auto resultCode = data.ReadInt32();
    OnInstallFinished(resultCode);
    return NO_ERROR;
}

int FreeInstallStatusCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = FreeInstallStatusCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto it = memberFuncMap_.find(code);
    if (it == memberFuncMap_.end()) {
        HILOG_ERROR("Not found");
        return ERR_INVALID_STATE;
    }

    return (this->*(it->second))(data, reply);
}
}  // namespace AppExecFwk
}  // namespace OHOS
