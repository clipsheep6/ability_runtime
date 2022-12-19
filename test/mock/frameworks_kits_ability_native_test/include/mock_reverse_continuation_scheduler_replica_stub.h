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

#ifndef MOCK_REVERSE_CONTINUATION_SCHEDULER_REPLICA_STUB_H
#define MOCK_REVERSE_CONTINUATION_SCHEDULER_REPLICA_STUB_H
#include <gmock/gmock.h>
#include "reverse_continuation_scheduler_replica_stub.h"

namespace OHOS {
namespace AppExecFwk {
class MockReverseContinuationSchedulerReplicaStub : public ReverseContinuationSchedulerReplicaStub {
public:
    MOCK_METHOD1(PassPrimary, void(const sptr<IRemoteObject> &primary));
    MOCK_METHOD0(ReverseContinuation, bool());
    MOCK_METHOD1(NotifyReverseResult, void(int reverseResult));
    MOCK_METHOD4(OnRemoteRequest, int(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // MOCK_REVERSE_CONTINUATION_SCHEDULER_REPLICA_STUB_H