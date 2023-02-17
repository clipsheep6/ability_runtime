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

#include "free_install_observer_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AbilityRuntime {
FreeInstallObserverProxy::FreeInstallObserverProxy(
    const sptr<IRemoteObject> &impl) : IRemoteProxy<IFreeInstallObserver>(impl)
{}

bool FreeInstallObserverProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(FreeInstallObserverProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed.");
        return false;
    }
    return true;
}

void FreeInstallObserverProxy::OnInstallFinished(const std::string bundleName, const std::string abilityName, const std::string startTime, int resultCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteString(bundleName) || !data.WriteString(abilityName) || !data.WriteString(startTime) ||
        !data.WriteInt32(resultCode)) {
        HILOG_ERROR("params is wrong");
        return;
    }

    sptr<IRemoteObject> remote = Remote();  
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        IFreeInstallObserver::ON_INSTALL_FINISHED,
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
        return;
    }
}
}
}