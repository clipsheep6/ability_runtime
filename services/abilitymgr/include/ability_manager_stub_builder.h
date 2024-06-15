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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_STUB_BUILDER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_STUB_BUILDER_H

#include "ability_manager_stub.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerStubBuilder {
    friend AbilityManagerStubBuilder;
public:
    AbilityManagerStubBuilder() = default;
    ~AbilityManagerStubBuilder() = default;
    static void BuildMessageMap(sptr<AbilityManagerStub> stub);

protected:
    static void FirstStepInit(sptr<AbilityManagerStub> &stub);
    static void SecondStepInit(sptr<AbilityManagerStub> &stub);
    static void ThirdStepInit(sptr<AbilityManagerStub> &stub);
    static void FourthStepInit(sptr<AbilityManagerStub> &stub);
    static void FifthStepInit(sptr<AbilityManagerStub> &stub);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_STUB_BUILDER_H