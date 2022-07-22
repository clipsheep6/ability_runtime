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
#include "bundlemgr/bundle_mgr_interface.h"
#include "bundle_constants.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AAFWK {
namespace ApplicationControllUtils {
using Want = OHOS::AAFwk::Want;
const std::string CROWDTEST_EXPEIRD_IMPLICIT_ACTION_NAME = "ohos.action.crowdtestDead";
const std::string CROWDTEST_EXPEIRD_IMPLICIT_BUNDLE_NAME = "com.demo.crowdtest";

[[maybe_unused]] static bool IsCrowdtestExpired(const Want &want)
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

[[maybe_unused]] static sptr<AppExecFwk::IBundleMgr> GetBundleManager()
{
    auto bundleObj =
        OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleObj == nullptr) {
        HILOG_ERROR("failed to get bundle manager service");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
}

int InterceptCrowdtestExpired(const Want &want, RequestCode requestCode, int32_t userId);

int InterceptCrowdtestExpired(const Want& want);
}  // namespace ApplicationControllUtils
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPLICATION_CONTROLL_UTIL_H