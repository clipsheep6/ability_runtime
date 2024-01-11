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

#include "child_scheduler_stub.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
ChildSchedulerStub::ChildSchedulerStub()
{
    memberFuncMap_[static_cast<uint32_t>(IChildScheduler::Message::SCHEDULE_LOAD_JS)] =
        &ChildSchedulerStub::HandleScheduleLoadJs;
    memberFuncMap_[static_cast<uint32_t>(IChildScheduler::Message::SCHEDULE_EXIT_PROCESS_SAFELY)] =
        &ChildSchedulerStub::HandleScheduleExitProcessSafely;
}

ChildSchedulerStub::~ChildSchedulerStub()
{
    memberFuncMap_.clear();
}

int32_t ChildSchedulerStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    HILOG_INFO("ChildSchedulerStub::OnReceived, code = %{public}u, flags= %{public}d.", code, option.GetFlags());
    std::u16string descriptor = ChildSchedulerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("A local descriptor is not equivalent to a remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HILOG_INFO("ChildSchedulerStub::OnRemoteRequest end");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t ChildSchedulerStub::HandleScheduleLoadJs(MessageParcel &data, MessageParcel &reply)
{
    ScheduleLoadJs();
    return ERR_NONE;
}

int32_t ChildSchedulerStub::HandleScheduleExitProcessSafely(MessageParcel &data, MessageParcel &reply)
{
    ScheduleExitProcessSafely();
    return ERR_NONE;
}
}  // namespace AppExecFwk
}  // namespace OHOS