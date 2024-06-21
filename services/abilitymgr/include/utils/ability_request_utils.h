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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_REQUEST_UTILS_H
#define OHOS_ABILITY_RUNTIME_ABILITY_REQUEST_UTILS_H

#include "iremote_object.h"
#include "ability_record.h"
#include "start_ability_utils.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AbilityRequestUtils final {
public:
    static int GenerateAbilityRequest(const Want &want, int requestCode, AbilityRequest &request,
        sptr<IRemoteObject> callerToken, int32_t userId, bool isNeedSetDebugApp = true);

    static int GenerateExtensionAbilityRequest(const Want &want, AbilityRequest &request,
        sptr<IRemoteObject> callerToken, int32_t userId);

    static int GenerateEmbeddableUIAbilityRequest(const Want &want,
        AbilityRequest &request, sptr<IRemoteObject> callerToken, int32_t userId);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_REQUEST_UTILS_H
