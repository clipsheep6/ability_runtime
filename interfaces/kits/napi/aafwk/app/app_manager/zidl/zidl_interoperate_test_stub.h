/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITYRUNTIME_ZIDLINTEROPERATETESTSTUB_H
#define OHOS_ABILITYRUNTIME_ZIDLINTEROPERATETESTSTUB_H

#include "izidl_interoperate_test.h"
#include <iremote_stub.h>

namespace OHOS {
namespace AbilityRuntime {
class ZidlInteroperateTestStub : public IRemoteStub<IZidlInteroperateTest> {
public:
    int OnRemoteRequest(
        /* [in] */ uint32_t code,
        /* [in] */ MessageParcel& data,
        /* [out] */ MessageParcel& reply,
        /* [in] */ MessageOption& option) override;

private:
    static constexpr int COMMAND_VOID_VOID = MIN_TRANSACTION_ID + 0;
    static constexpr int COMMAND_BOOLEAN_VOID = MIN_TRANSACTION_ID + 1;
    static constexpr int COMMAND_BYTE_VOID = MIN_TRANSACTION_ID + 2;
    static constexpr int COMMAND_SHORT_VOID = MIN_TRANSACTION_ID + 3;
    static constexpr int COMMAND_INT_VOID = MIN_TRANSACTION_ID + 4;
    static constexpr int COMMAND_LONG_VOID = MIN_TRANSACTION_ID + 5;
    static constexpr int COMMAND_FLOAT_VOID = MIN_TRANSACTION_ID + 6;
    static constexpr int COMMAND_DOUBLE_VOID = MIN_TRANSACTION_ID + 7;
    static constexpr int COMMAND_STRING_VOID = MIN_TRANSACTION_ID + 8;
    static constexpr int COMMAND_MAP_STRING_STRING_VOID = MIN_TRANSACTION_ID + 9;
    static constexpr int COMMAND_DOUBLE_ARRAY_VOID = MIN_TRANSACTION_ID + 10;
    static constexpr int COMMAND_VOID_IN_MAP_STRING_INT = MIN_TRANSACTION_ID + 11;
    static constexpr int COMMAND_VOID_IN_STRING_ARRAY = MIN_TRANSACTION_ID + 12;
    static constexpr int COMMAND_VOID_OUT_MAP_STRING_INT = MIN_TRANSACTION_ID + 13;
    static constexpr int COMMAND_VOID_OUT_STRING_ARRAY = MIN_TRANSACTION_ID + 14;
    static constexpr int COMMAND_VOID_IN_INT = MIN_TRANSACTION_ID + 15;
    static constexpr int COMMAND_VOID_IN_STRING = MIN_TRANSACTION_ID + 16;
    static constexpr int COMMAND_VOID_OUT_INT = MIN_TRANSACTION_ID + 17;
    static constexpr int COMMAND_VOID_OUT_STRING = MIN_TRANSACTION_ID + 18;
    static constexpr int COMMAND_VOID_PARAMETER_TYPE_BOOLEAN = MIN_TRANSACTION_ID + 19;
    static constexpr int COMMAND_VOID_PARAMETER_TYPE_BYTE = MIN_TRANSACTION_ID + 20;
    static constexpr int COMMAND_VOID_PARAMETER_TYPE_SHORT = MIN_TRANSACTION_ID + 21;
    static constexpr int COMMAND_VOID_PARAMETER_TYPE_INT = MIN_TRANSACTION_ID + 22;
    static constexpr int COMMAND_VOID_PARAMETER_TYPE_LONG = MIN_TRANSACTION_ID + 23;
    static constexpr int COMMAND_VOID_PARAMETER_TYPE_FLOAT = MIN_TRANSACTION_ID + 24;
    static constexpr int COMMAND_VOID_PARAMETER_TYPE_DOUBLE = MIN_TRANSACTION_ID + 25;
    static constexpr int COMMAND_VOID_PARAMETER_TYPE_STRING = MIN_TRANSACTION_ID + 26;
    static constexpr int COMMAND_VOID_PARAMETER_TYPE_MAP_STRING_STRING = MIN_TRANSACTION_ID + 27;
    static constexpr int COMMAND_VOID_PARAMETER_TYPE_DOUBLE_ARRAY = MIN_TRANSACTION_ID + 28;
    static constexpr int COMMAND_VOID_ORDER_OUT_IN = MIN_TRANSACTION_ID + 29;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITYRUNTIME_ZIDLINTEROPERATETESTSTUB_H

