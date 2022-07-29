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

#ifndef RESOURCE_OHOS_ABILITY_RUNTIME_TEST_ABILITY_CONNECTION_H
#define RESOURCE_OHOS_ABILITY_RUNTIME_TEST_ABILITY_CONNECTION_H

#include "test_ability_connect_callback_stub.h"

namespace OHOS {
namespace AppExecFwk {
class TestAbilityConnection : public TestAbilityConnectionStub {
public:
    /**
     * @brief Default constructor used to create a AbilityThread instance.
     */
    TestAbilityConnection();
    ~TestAbilityConnection();

    /**
     * @description:  Provide operating system ConnectAbility information to the observer
     * @param  want Indicates the structure containing connect information about the ability.
     */
    virtual int32_t ScheduleAdd(int32_t a, int32_t b) override;
    virtual int32_t ScheduleSub(int32_t a, int32_t b) override;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // RESOURCE_OHOS_ABILITY_RUNTIME_TEST_ABILITY_CONNECTION_H
