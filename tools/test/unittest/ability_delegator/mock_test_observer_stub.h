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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_TEST_OBSERVER_STUB_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_TEST_OBSERVER_STUB_H

#include "gmock/gmock.h"

#include "test_observer_stub.h"

namespace OHOS {
namespace AAFwk {
class MockTestObserverStub : public TestObserverStub {
public:
    MockTestObserverStub() = default;
    ~MockTestObserverStub() override;
    void TestStatus(const std::string& msg, const int64_t& resultCode) override;
    void TestFinished(const std::string& msg, const int64_t& resultCode) override;
    ShellCommandResult ExecuteShellCommand(
        const std::string& cmd, const int64_t timeoutMs) override;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_TEST_OBSERVER_STUB_H
