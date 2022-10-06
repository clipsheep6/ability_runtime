/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_QUICK_FIX_MANAGER_STUB_H
#define OHOS_ABILITY_RUNTIME_MOCK_QUICK_FIX_MANAGER_STUB_H

#include "gmock/gmock.h"
#include "quick_fix_manager_stub.h"

namespace OHOS {
namespace AAFwk {
class MockQuickFixManagerStub : public QuickFixManagerStub {
public:
    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));
    MOCK_METHOD1(ApplyQuickFix, int32_t(const std::vector<std::string> &quickFixFiles));
    MOCK_METHOD2(GetApplyedQuickFixInfo, int32_t(const std::string &bundleName, ApplicationQuickFixInfo &quickFixInfo));

    int InvokeSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return 0;
    }

private:
    int code_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MOCK_QUICK_FIX_MANAGER_STUB_H
