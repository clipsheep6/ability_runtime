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

#include "auto_startup_callback_stub.h"

#include "ability_manager_ipc_interface_code.h"
#include "auto_startup_info.h"
#include "event_handler.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;
AutoStartupCallBackStub::AutoStartupCallBackStub()
{
    Init();
}

AutoStartupCallBackStub::~AutoStartupCallBackStub()
{
    requestFuncMap_.clear();
}

void AutoStartupCallBackStub::Init()
{
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ON_AUTO_STARTUP_ON)] =
        &AutoStartupCallBackStub::OnAutoStartupOnInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ON_AUTO_STARTUP_OFF)] =
        &AutoStartupCallBackStub::OnAutoStartupOffInner;
}

int AutoStartupCallBackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string autoStartUpCallBackDescriptor = AutoStartupCallBackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (autoStartUpCallBackDescriptor != remoteDescriptor) {
        HILOG_ERROR("Local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    HILOG_WARN("Default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AutoStartupCallBackStub::OnAutoStartupOnInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AutoStartupInfo> info = data.ReadParcelable<AutoStartupInfo>();
    if (info == nullptr) {
        HILOG_ERROR("Failed to read parcelable.");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<AppExecFwk::EventHandler> handler =
        std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    wptr<AutoStartupCallBackStub> weak = this;
    handler->PostSyncTask([weak, info]() {
        auto autoStartUpCallBackStub = weak.promote();
        if (autoStartUpCallBackStub == nullptr) {
            HILOG_ERROR("autoStartUpCallBackStub is nullptr.");
            return;
        }
        autoStartUpCallBackStub->OnAutoStartupOn(*info);
    });

    return NO_ERROR;
}

int32_t AutoStartupCallBackStub::OnAutoStartupOffInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AutoStartupInfo> info = data.ReadParcelable<AutoStartupInfo>();
    if (info == nullptr) {
        HILOG_ERROR("Failed to read parcelable.");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<AppExecFwk::EventHandler> handler =
        std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    wptr<AutoStartupCallBackStub> weak = this;
    handler->PostSyncTask([weak, info]() {
        auto autoStartUpCallBackStub = weak.promote();
        if (autoStartUpCallBackStub == nullptr) {
            HILOG_ERROR("autoStartUpCallBackStub is nullptr.");
            return;
        }
        autoStartUpCallBackStub->OnAutoStartupOff(*info);
    });

    return NO_ERROR;
}
} // namespace AbilityRuntime
} // namespace OHOS
