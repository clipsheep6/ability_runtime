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
const std::string CROWDTEST_EXPEIRD_IMPLICIT_ACTION_NAME = "ohos.action.crowdtestDead";
const std::string CROWDTEST_EXPEIRD_IMPLICIT_BUNDLE_NAME = "com.demo.crowdtest";
const int32_t CROWDTEST_EXPEIRD_IMPLICIT_START_FAILED = 1;
const int32_t CROWDTEST_EXPEIRD_REFUSED = -1;

int InterceptCrowdtestExpired(const Want &want, RequestCode requestCode, int32_t userId)
{
    if (IsCrowdtestExpired(want)) {
#ifdef SUPPORT_GRAPHICS
        Want newWant;
        newWant.SetBundleName(CROWDTEST_EXPEIRD_IMPLICIT_BUNDLE_NAME);
        newWant.SetAction(CROWDTEST_EXPEIRD_IMPLICIT_ACTION_NAME);
        int result = AbilityManagerService::StartAbility(newWant, userId, requestCode);
        if (result != 0) {
            return CROWDTEST_EXPEIRD_IMPLICIT_START_FAILED;
        }
#endif
        return CROWDTEST_EXPEIRD_REFUSED;
    }
    return ERR_OK;
}

int InterceptCrowdtestExpired(const Want& want)
{
    if (IsCrowdtestExpired(want)) {
        return CROWDTEST_EXPEIRD_REFUSED;
    }
    return ERR_OK;
}

bool IsCrowdtestExpired(const Want &want)
{
    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, ERR_INVALID_VALUE);
    std::string bundleName = want.GetBundle();
    AppExecFwk::Application callerAppInfo;
    bool result = IN_PROCESS_CALL(
        bms->GetApplicationInfo(bundleName, AppExecFwk::BundleFlags::GET_BUNDLE_DEFAULT,，
            GetUserId(), callerAppInfo)
    );
    if (!result) {
        HILOG_ERROR("%{public}s GetApplicaionInfo from bms failed.", __func__);
        return ERR_INVALID_VALUE;
    }

    auto appDistributionType = callerAppInfo.appDistributionType;
    auto appCrowdtestDeadline = callerAppInfo.crowdtestDeadline;
    if (appDistributionType == AppExecFwk::Constants::APP_DISTRIBUTION_TYPE_CROWDTESTING &&
        appCrowdtestDeadline <= 0) {
        return true;
    }
    return false;
}

}
}
}

