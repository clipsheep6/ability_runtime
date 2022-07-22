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
#include "application_controll_utils.h"

namespace OHOS {
namespace AAFwk {
namespace ApplicationControllUtils {

int InterceptCrowdtestExpired(const Want &want, RequestCode requestCode, int32_t userId)
{
    if (IsCrowdtestExpired(want)) {
#ifdef SUPPORT_GRAPHICS
        Want newWant;
        newWant.SetBundleName(CROWDTEST_EXPEIRD_IMPLICIT_BUNDLE_NAME);
        newWant.SetAction(CROWDTEST_EXPEIRD_IMPLICIT_ACTION_NAME);
        return AbilityManagerService::StartAbility(newWant, userId, requestCode);
#endif
        return ERR_NOT_OK;
    }
    return ERR_OK;
}

int InterceptCrowdtestExpired(const Want& want)
{
    if (IsCrowdtestExpired(want)) {
        return ERR_NOT_OK;
    }
    return ERR_OK;
}

}
}
}

