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

#include "interceptor/control_interceptor.h"

#include "ability_util.h"
#include "app_running_control_rule_result.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string INTERCEPT_PARAMETERS = "intercept_parammeters";
const std::string INTERCEPT_BUNDLE_NAME = "intercept_bundleName";
const std::string INTERCEPT_ABILITY_NAME = "intercept_abilityName";
const std::string INTERCEPT_MODULE_NAME = "intercept_moduleName";
const std::string IS_FROM_PARENTCONTROL = "ohos.ability.isFromParentControl";
}

ErrCode ControlInterceptor::DoProcess(AbilityInterceptorParam param)
{
    AppExecFwk::AppRunningControlRuleResult controlRule;
    if (CheckControl(param.want, param.userId, controlRule)) {
        HILOG_INFO("The target application is intercpted. %{public}s", controlRule.controlMessage.c_str());
#ifdef SUPPORT_GRAPHICS
        if (!param.isWithUI || controlRule.controlWant == nullptr) {
            HILOG_ERROR("Can not start control want");
            return AbilityUtil::EdmErrorType(controlRule.isEdm);
        }
        if (controlRule.controlWant->GetBoolParam(IS_FROM_PARENTCONTROL, false)) {
            auto controlWant = controlRule.controlWant;
            auto controlParam = controlWant->GetParams();
            sptr<AAFwk::IWantParams> interceptParam = WantParamWrapper::Box(param.want.GetParams());
            if (interceptParam != nullptr) {
                controlParam.SetParam(INTERCEPT_PARAMETERS, interceptParam);
            }
            controlWant->SetParams(controlParam);
            controlWant->SetParam(INTERCEPT_BUNDLE_NAME, param.want.GetElement().GetBundleName());
            controlWant->SetParam(INTERCEPT_ABILITY_NAME, param.want.GetElement().GetAbilityName());
            controlWant->SetParam(INTERCEPT_MODULE_NAME, param.want.GetElement().GetModuleName());
            controlRule.controlWant = controlWant;
        }
        int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(*controlRule.controlWant,
            param.requestCode, param.userId));
        if (ret != ERR_OK) {
            HILOG_ERROR("Control implicit start appgallery failed.");
            return ret;
        }
#endif
        return AbilityUtil::EdmErrorType(controlRule.isEdm);
    }
    return ERR_OK;
}

bool ControlInterceptor::CheckControl(const Want &want, int32_t userId,
    AppExecFwk::AppRunningControlRuleResult &controlRule)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // get bms
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("The bundleMgrHelper is nullptr.");
        return false;
    }

    // get disposed status
    std::string bundleName = want.GetBundle();
    auto appControlMgr = bundleMgrHelper->GetAppControlProxy();
    if (appControlMgr == nullptr) {
        HILOG_ERROR("The appControlMgr is nullptr.");
        return false;
    }

    auto ret = IN_PROCESS_CALL(appControlMgr->GetAppRunningControlRule(bundleName, userId, controlRule));
    if (ret != ERR_OK) {
        HILOG_DEBUG("Get No AppRunningControlRule.");
        return false;
    }
    return true;
}
} // namespace AAFwk
} // namespace OHOS