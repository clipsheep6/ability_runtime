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

#ifndef OHOS_ABILITY_RUNTIME_LOAD_ABILITY_PARAM_H
#define OHOS_ABILITY_RUNTIME_LOAD_ABILITY_PARAM_H

#include <memory>
#include "ability_info.h"
#include "application_info.h"
#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
struct LoadAbilityParam {
    int32_t abilityRecordId = -1; // The unique identification to start the ability.
    sptr<IRemoteObject> token; // This will be obsolete. The unique identification to start the ability.
    int32_t preRecordId = -1; // The unique identification to call the ability.
    sptr<IRemoteObject> preToken; // This will be obsolete. The unique identification to call the ability.
    std::shared_ptr<AbilityInfo> abilityInfo; // The ability information.
    std::shared_ptr<ApplicationInfo> appInfo; // The app information.
    std::shared_ptr<AAFwk::Want> want; // Request param from the caller.
};
} // AppExecFwk
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_LOAD_ABILITY_PARAM_H