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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_CONTROLL_UTIL
#define OHOS_ABILITY_RUNTIME_APPLICATION_CONTROLL_UTIL

#include <string>

#include "want.h"
#include "ability_manager_service.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "bundle_constants.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "ability_util.h"

namespace OHOS {
namespace AAFWK {
namespace ApplicationControllUtils {
using Want = OHOS::AAFwk::Want;
const std::string CROWDTEST_EXPEIRD_IMPLICIT_ACTION_NAME = "ohos.action.crowdtestDead";
const std::string CROWDTEST_EXPEIRD_IMPLICIT_BUNDLE_NAME = "com.demo.crowdtest";
const int32_t CROWDTEST_EXPEIRD_IMPLICIT_START_FAILED = 1;
const int32_t CROWDTEST_EXPEIRD_REFUSED = -1;

static int InterceptCrowdtestExpired(const Want &want, RequestCode requestCode, int32_t userId)
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

static int InterceptCrowdtestExpired(const Want& want)
{
    if (IsCrowdtestExpired(want)) {
        return CROWDTEST_EXPEIRD_REFUSED;
    }
    return ERR_OK;
}

static bool IsCrowdtestExpired(const Want &want)
{
    auto bms = AbilityUtil::GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, ERR_INVALID_VALUE);
    std::string bundleName = want.GetBundle();
    AppExecFwk::Application callerAppInfo;
    bool result = IN_PROCESS_CALL(
        bms->GetApplicationInfo(bundleName, AppExecFwk::BundleFlags::GET_BUNDLE_DEFAULT,
            GetUserId(), callerAppInfo)
    );
    if (!result) {
        HILOG_ERROR("%{public}s GetApplicaionInfo from bms failed.", __func__);
        return false;
    }

    auto appDistributionType = callerAppInfo.appDistributionType;
    auto appCrowdtestDeadline = callerAppInfo.crowdtestDeadline;
    if (appDistributionType == AppExecFwk::Constants::APP_DISTRIBUTION_TYPE_CROWDTESTING &&
        appCrowdtestDeadline <= 0) {
        return true;
    }
    return false;
}
}  // namespace ApplicationControllUtils
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPLICATION_CONTROLL_UTIL_H