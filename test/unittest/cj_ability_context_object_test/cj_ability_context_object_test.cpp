/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
 
#include <gtest/gtest.h>

#include "cj_ability_context_object.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class CjAbilityContextProxyTest : public testing::Test {
};

int g_result = 0;

HWTEST_F(CjAbilityContextProxyTest, RegisterCJAbilityCallbacks_0100, TestSize.Level1)
{
    RegisterCJAbilityCallbacks(nullptr);
    void (*registerFunc)(CJAbilityCallbacks*) = [](CJAbilityCallbacks* cjAbilityCallbacks)
    {
        if (cjAbilityCallbacks != nullptr) g_result += 1;
        cjAbilityCallbacks->invokeAbilityResultCallback = [](int64_t id, int32_t error,
                                                              CJAbilityResult* cjAbilityResult) {};
        cjAbilityCallbacks->invokePermissionRequestResultCallback =
            [](int64_t id, int32_t error, CJPermissionRequestResult* cjPermissionRequestResult) {};
        cjAbilityCallbacks->invokeDialogRequestResultCallback = [](int64_t id, int32_t error,
                                                                    CJDialogRequestResult* cjDialogRequestResult) {};
    };
    RegisterCJAbilityCallbacks(registerFunc);
    RegisterCJAbilityCallbacks(registerFunc);
    EXPECT_EQ(g_result, 1);
}
}
}