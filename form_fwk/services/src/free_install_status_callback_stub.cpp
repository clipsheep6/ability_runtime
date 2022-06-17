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
FreeInstallStatusCallBackStub::FreeInstallStatusCallBackStub()
{
    vecMemberFunc_.resize(IFreeInstallStatusCallBackCmd::CMD_MAX);
    vecMemberFunc_[IFreeInstallStatusCallBackCmd::ON_FREE_INSTALL_DONE] =
        &FreeInstallStatusCallBackStub::OnInstallFinishedInner;
}

int FreeInstallStatusCallBackStub::OnInstallFinishedInner(MessageParcel &data, MessageParcel &reply)
{
    auto resultCode = data.ReadInt32();
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOG_ERROR("FreeInstallStatusCallBackStub want is nullptr");
        return ERR_INVALID_VALUE;
    }

    auto userId = data.ReadInt32();
    OnInstallFinished(resultCode, *want, userId);
    return NO_ERROR;
}

int FreeInstallStatusCallBackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_INFO("OnRemoteRequest, code: %{public}d", code);
    std::u16string descriptor = FreeInstallStatusCallBackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("Local descriptor is not equal to remote, descriptor: %{public}s, remoteDescriptor: %{public}s",
            Str16ToStr8(descriptor).c_str(), Str16ToStr8(remoteDescriptor).c_str());
        return ERR_INVALID_STATE;
    }

    if (code < IFreeInstallStatusCallBackCmd::CMD_MAX && code >= 0) {
        auto memberFunc = vecMemberFunc_[code];
        return (this->*memberFunc)(data, reply);
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace AppExecFwk
}  // namespace OHOS
