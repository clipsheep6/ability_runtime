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
#include "app_running_status_stub.h"

#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AbilityRuntime {
AppRunningStatusStub::AppRunningStatusStub()
{
    requestFuncMap_[static_cast<uint32_t>(AppRunningStatusListenerInterface::MessageCode::APP_RUNNING_STATUS)] =
        &AppRunningStatusStub::HandleAppRunningStatus;
}

AppRunningStatusStub::~AppRunningStatusStub()
{
    requestFuncMap_.clear();
}

int AppRunningStatusStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called, code = %{public}u, flags= %{public}d.", code, option.GetFlags());
    std::u16string descriptor = AppRunningStatusStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "Local descriptor is not equal to remote.");
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

ErrCode AppRunningStatusStub::HandleAppRunningStatus(MessageParcel &data, MessageParcel &reply)
{
    std::string bundle = data.ReadString();
    int32_t uid = data.ReadInt32();
    RunningStatus runningStatus = static_cast<RunningStatus>(data.ReadInt32());
    NotifyAppRunningStatus(bundle, uid, runningStatus);
    return NO_ERROR;
}
} // namespace AbilityRuntime
} // namespace OHOS
