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

#include "startup_util.h"

#include "ability_info.h"
#include "server_constant.h"
#include "want.h"

namespace OHOS::AbilityRuntime {
int32_t StartupUtil::GetAppTwinIndex(const AAFwk::Want &want)
{
    int32_t appTwinIndex = want.GetIntParam(ServerConstant::APP_TWIN_INDEX, 0);
    if (appTwinIndex == 0) {
        appTwinIndex = want.GetIntParam(ServerConstant::DLP_INDEX, 0);
    }
    return appTwinIndex;
}

int32_t StartupUtil::BuildAbilityInfoFlag()
{
    return AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_METADATA;
}
}  // namespace OHOS::AbilityRuntime
