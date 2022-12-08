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

#include <gmock/gmock.h>

#include "iability_controller.h"

namespace OHOS {
namespace AppExecFwk {
class MockAbilityController : public IAbilityController {
public:
    MockAbilityController() = default;
    virtual ~MockAbilityController() = default;

    MOCK_METHOD2(AllowAbilityStart, bool(const Want &want, const std::string &bundleName));
    MOCK_METHOD1(AllowAbilityBackground, bool(const std::string &bundleName));
    sptr<IRemoteObject> AsObject()
    {
        return {};
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS