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

#include "ability_interceptor.h"

#include <chrono>
#include <string>

#include "ability_manager_errors.h"
#include "app_running_control_rule_result.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "bundle_constants.h"
#include "erms_mgr_interface.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
using ErmsCallerInfo = OHOS::AppExecFwk::ErmsParams::CallerInfo;
using ExperienceRule = OHOS::AppExecFwk::ErmsParams::ExperienceRule;

const std::string ACTION_MARKET_CROWDTEST = "ohos.want.action.marketCrowdTest";
const std::string ACTION_MARKET_DISPOSED = "ohos.want.action.marketDisposed";
const std::string PERMISSION_MANAGE_DISPOSED_APP_STATUS = "ohos.permission.MANAGE_DISPOSED_APP_STATUS";

AbilityInterceptor::~AbilityInterceptor()
{}

CrowdTestInterceptor::CrowdTestInterceptor()
{}

CrowdTestInterceptor::~CrowdTestInterceptor()
{}

ErrCode CrowdTestInterceptor::DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground)
{
    if (CheckCrowdtest(want, userId)) {
        HILOG_ERROR("Crowdtest expired.");
#ifdef SUPPORT_GRAPHICS
        if (isForeground) {
            int ret = IN_PROCESS_CALL(AbilityUtil::StartAppgallery(requestCode, userId, ACTION_MARKET_CROWDTEST));
            if (ret != ERR_OK) {
                HILOG_ERROR("Crowdtest implicit start appgallery failed.");
                return ret;
            }
        }
#endif
        return ERR_CROWDTEST_EXPIRED;
    }
    return ERR_OK;
}

bool CrowdTestInterceptor::CheckCrowdtest(const Want &want, int32_t userId)
{
    // get bms
    auto bms = AbilityUtil::GetBundleManager();
    if (!bms) {
        HILOG_ERROR("GetBundleManager failed");
        return false;
    }

    // get crowdtest status and time
    std::string bundleName = want.GetBundle();
    AppExecFwk::ApplicationInfo callerAppInfo;
    bool result = IN_PROCESS_CALL(
        bms->GetApplicationInfo(bundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO,
            userId, callerAppInfo)
    );
    if (!result) {
        HILOG_ERROR("GetApplicaionInfo from bms failed.");
        return false;
    }

    auto appDistributionType = callerAppInfo.appDistributionType;
    auto appCrowdtestDeadline = callerAppInfo.crowdtestDeadline;
    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    if (appDistributionType == AppExecFwk::Constants::APP_DISTRIBUTION_TYPE_CROWDTESTING &&
        appCrowdtestDeadline < now) {
        HILOG_INFO("The application is expired, expired time is %{public}s",
            std::to_string(appCrowdtestDeadline).c_str());
        return true;
    }
    return false;
}

ControlInterceptor::ControlInterceptor()
{}

ControlInterceptor::~ControlInterceptor()
{}

ErrCode ControlInterceptor::DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground)
{
    AppExecFwk::AppRunningControlRuleResult controlRule;
    if (CheckControl(want, userId, controlRule)) {
        HILOG_INFO("The target application is intercpted. %{public}s", controlRule.controlMessage.c_str());
#ifdef SUPPORT_GRAPHICS
        if (isForeground && controlRule.controlWant != nullptr) {
            int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(*controlRule.controlWant,
                userId, requestCode));
            if (ret != ERR_OK) {
                HILOG_ERROR("Control implicit start appgallery failed.");
                return ret;
            }
        }
#endif
        return ERR_DISPOSED_STATUS;
    }
    return ERR_OK;
}

bool ControlInterceptor::CheckControl(const Want &want, int32_t userId,
    AppExecFwk::AppRunningControlRuleResult &controlRule)
{
    // get bms
    auto bms = AbilityUtil::GetBundleManager();
    if (!bms) {
        HILOG_ERROR("GetBundleManager failed");
        return false;
    }

    // get disposed status
    std::string bundleName = want.GetBundle();
    auto appControlMgr = bms->GetAppControlProxy();
    if (appControlMgr == nullptr) {
        HILOG_ERROR("Get appControlMgr failed");
        return false;
    }

    auto ret = IN_PROCESS_CALL(appControlMgr->GetAppRunningControlRule(bundleName, userId, controlRule));
    if (ret != ERR_OK) {
        HILOG_INFO("Get No AppRunningControlRule");
        return false;
    }
    return true;
}

EcologicalRuleInterceptor::EcologicalRuleInterceptor()
{}

EcologicalRuleInterceptor::~EcologicalRuleInterceptor()
{}

ErrCode EcologicalRuleInterceptor::DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground)
{
    bool isStartIncludeAtomicService = AbilityUtil::IsStartIncludeAtomicService(want, userId);
    if (!isStartIncludeAtomicService) {
        HILOG_INFO("This startup does not contain atomic service, keep going.");
        return ERR_OK;
    }


    ErmsCallerInfo callerInfo;
    ExperienceRule rule;
    int ret = CheckRule(want, callerInfo, rule);
    if (!ret) {
        HILOG_ERROR("check ecological rule failed, keep going.");
        return ERR_OK;
    }

    HILOG_INFO("check ecological rule success");
    if (rule.isAllow) {
        HILOG_ERROR("ecological rule is allow, keep going.");
        return ERR_OK;
    }
#ifdef SUPPORT_GRAPHICS
    if (isForeground && (rule.replaceWant != nullptr)) {
        int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(*rule.replaceWant,
            userId, requestCode));
        if (ret != ERR_OK) {
            HILOG_ERROR("ecological start replace want failed.");
            return ret;
        }
    }
#endif
    return ERR_ECOLOGICAL_CONTROL_STATUS;
}

bool EcologicalRuleInterceptor::CheckRule(const Want &want, ErmsCallerInfo &callerInfo, ExperienceRule &rule)
{
    auto erms = AbilityUtil::GetEcologicalRuleMgr();
    if (!erms) {
        HILOG_ERROR("GetEcologicalRuleMgr failed.");
        return false;
    }
    int ret = IN_PROCESS_CALL(erms->QueryStartExperience(want, callerInfo, rule));
    if (ret != ERR_OK) {
        HILOG_ERROR("Failed to query start experience from erms.");
        return false;
    }

    return true;
}
} // namespace AAFwk
} // namespace OHOS
