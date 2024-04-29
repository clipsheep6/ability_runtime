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

#ifndef MOCK_ABILITY_INFO_CALLBACK_STUB_H
#define MOCK_ABILITY_INFO_CALLBACK_STUB_H

#include "ability_info_callback_stub.h"

namespace OHOS::AAFwk {
class MockAbilityInfoCallbackStub : public AppExecFwk::AbilityInfoCallbackStub {
public:
    void NotifyAbilityToken(const sptr<IRemoteObject> token, const Want &want) override {}
    void NotifyStartSpecifiedAbility(const sptr<IRemoteObject> &callerToken, const Want &want, int requestCode,
        sptr<Want> &extraParam) override {}
    void NotifyRestartSpecifiedAbility(const sptr<IRemoteObject> &token) override {}
    void NotifyStartAbilityResult(const Want &want, int result) override {}
};
} // namespace OHOS::AAFwk
#endif  // MOCK_ABILITY_INFO_CALLBACK_STUB_H
