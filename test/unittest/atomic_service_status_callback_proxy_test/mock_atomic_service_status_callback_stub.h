/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_ATOMIC_SERVICE_STATUS_CALLBACK_STUB_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_ATOMIC_SERVICE_STATUS_CALLBACK_STUB_H

#include <gmock/gmock.h>
#include "atomic_service_status_callback_stub.h"
#include "semaphore_ex.h"

namespace OHOS {
namespace AAFwk {
class MockAtomicServiceStatusCallbackStub : public AtomicServiceStatusCallbackStub {
public:
    MockAtomicServiceStatusCallbackStub()
    {}
    ~MockAtomicServiceStatusCallbackStub()
    {}
    MOCK_METHOD3(OnInstallFinished, void(int resultCode, const Want &want, int32_t userId));
    MOCK_METHOD3(OnRemoteInstallFinished, void(int resultCode, const Want &want, int32_t userId));
    MOCK_METHOD1(OnRemoveTimeoutTask, void(const Want &want));

    void Wait()
    {
        sem_.Wait();
    }

    int Post()
    {
        sem_.Post();
        return 0;
    }

    void PostVoid()
    {
        sem_.Post();
    }

private:
    Semaphore sem_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_ATOMIC_SERVICE_STATUS_CALLBACK_STUB_H
