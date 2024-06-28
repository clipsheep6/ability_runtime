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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_AUTO_STARTUP_CALLBACK_STUB_H
#define OHOS_ABILITY_RUNTIME_MOCK_AUTO_STARTUP_CALLBACK_STUB_H

#include "gmock/gmock.h"
#define private public
#include "auto_startup_callback_stub.h"
#undef private

namespace OHOS {
namespace AbilityRuntime {
class MockAutoStartupCallbackStub : public AutoStartupCallBackStub {
public:
    MockAutoStartupCallbackStub() {}
    virtual ~MockAutoStartupCallbackStub() {}
    MOCK_METHOD1(OnAutoStartupOn, void(const AutoStartupInfo &info));
    MOCK_METHOD1(OnAutoStartupOff, void(const AutoStartupInfo &info));
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MOCK_AUTO_STARTUP_CALLBACK_STUB_H
