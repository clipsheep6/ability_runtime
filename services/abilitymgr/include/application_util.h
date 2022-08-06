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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_UTIL_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_UTIL_H

#include <chrono>
#include <string>

#include "ability_manager_service.h"
#include "ability_util.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "bundle_constants.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace ApplicationUtil {
using Want = OHOS::AAFwk::Want;

static bool IsCrowdtestExpired(const Want &want, int32_t userId)
{
    auto bms = AbilityUtil::GetBundleManager();
    if (!bms) {
        HILOG_ERROR("%{public}s GetBundleManager failed", __func__);
        return false;
    }
    std::string bundleName = want.GetBundle();
    AppExecFwk::ApplicationInfo callerAppInfo;
    bool result = IN_PROCESS_CALL(
        bms->GetApplicationInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT,
            userId, callerAppInfo)
    );
    if (!result) {
        HILOG_ERROR("%{public}s GetApplicaionInfo from bms failed.", __func__);
        return false;
    }

    auto appDistributionType = callerAppInfo.appDistributionType;
    auto appCrowdtestDeadline = callerAppInfo.crowdtestDeadline;
    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    if (appDistributionType == AppExecFwk::Constants::APP_DISTRIBUTION_TYPE_CROWDTESTING &&
        appCrowdtestDeadline < now) {
        HILOG_INFO("%{public}s The application is expired, expired time is %{public}s",
            __func__, std::to_string(appCrowdtestDeadline).c_str());
        return true;
    }
    return false;
}
}  // namespace ApplicationlUtil
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPLICATION_UTIL_H
