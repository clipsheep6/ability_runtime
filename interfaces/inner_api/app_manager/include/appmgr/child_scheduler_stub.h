/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless quired by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_CHILD_SCHEDULER_STUB_H
#define OHOS_ABILITY_RUNTIME_CHILD_SCHEDULER_STUB_H

#include <map>

#include "child_scheduler_interface.h"
#include "iremote_object.h"
#include "iremote_stub.h"
#include "nocopyable.h"

namespace OHOS {
namespace AppExecFwk {
class ChildSchedulerStub : public IRemoteStub<IChildScheduler> {
public:
    ChildSchedulerStub();
    virtual ~ChildSchedulerStub();

    virtual int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t HandleScheduleLoadJs(MessageParcel &data, MessageParcel &reply);
    int32_t HandleScheduleExitProcessSafely(MessageParcel &data, MessageParcel &reply);

    using ChildSchedulerFunc = int32_t (ChildSchedulerStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, ChildSchedulerFunc> memberFuncMap_;

    DISALLOW_COPY_AND_MOVE(ChildSchedulerStub);
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CHILD_SCHEDULER_STUB_H
