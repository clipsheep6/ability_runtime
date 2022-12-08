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

#ifndef OHOS_ABILITY_RUNTIME_TEST_OBSERVER_STUB_H
#define OHOS_ABILITY_RUNTIME_TEST_OBSERVER_STUB_H

#include "itest_observer.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AAFwk {
class TestObserverStub : public IRemoteStub<ITestObserver> {
public:
    /**
     * Default constructor.
     */
    TestObserverStub();

    /**
     * Deconstructor used to deconstruct.
     */
    virtual ~TestObserverStub() override;

    /**
     * Handles ipc request.
     *
     * @param code, Indicates the ipc id.
     * @param data, Indicates the input data.
     * @param reply, Indicates the reply data.
     * @param option, Indicates the option.
     * @return the result of execution.
     */
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_TEST_OBSERVER_STUB_H
