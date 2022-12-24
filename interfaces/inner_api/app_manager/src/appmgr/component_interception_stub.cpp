/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "component_interception_stub.h"
#include "appexecfwk_errors.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
ComponentInterceptionStub::ComponentInterceptionStub()
{
    requestFuncMap_[static_cast<uint32_t>(
        IComponentInterception::Message::TRANSACT_ON_ALLOW_COMPONENT_START)] =
            &ComponentInterceptionStub::HandleAllowComponentStart;
}

ComponentInterceptionStub::~ComponentInterceptionStub()
{
    requestFuncMap_.clear();
}

int ComponentInterceptionStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_INFO("ComponentInterceptionStub::OnReceived, code = %{public}u, flags= %{public}d.", code, option.GetFlags());
    std::u16string descriptor = ComponentInterceptionStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

bool ComponentInterceptionStub::AllowComponentStart(const Want &want, const sptr<IRemoteObject> &callerToken,
    int requestCode, int componentStatus, sptr<Want> &extraParam)
{
    return true;
}

int32_t ComponentInterceptionStub::HandleAllowComponentStart(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("HandleAllowComponentStart");
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        HILOG_ERROR("ReadParcelable<Want> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    sptr<IRemoteObject> token = nullptr;
    bool hasRemoteToken = data.ReadBool();
    if (hasRemoteToken) {
        token = data.ReadRemoteObject();
    }

    int requestCode = data.ReadInt32();
    int componentStatus = data.ReadInt32();
    
    sptr<Want> extraParam = new (std::nothrow) Want();
    bool result = AllowComponentStart(*want, token, requestCode, componentStatus, extraParam);
    if (want->GetRemoteObject(Want::PARAM_RESV_ABILITY_INFO_CALLBACK)) {
        reply.WriteBool(true);
        reply.WriteParcelable(extraParam);
    } else {
        reply.WriteBool(false);
    }
    reply.WriteBool(result);
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
