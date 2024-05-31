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

#include "interceptor/extension_control_interceptor.h"

#include "ability_info.h"
#include "ability_util.h"
#include "extension_config.h"
#include "hilog_tag_wrapper.h"
#include "start_ability_utils.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr char STRICT_MODE[] = "strictMode";
}

ErrCode ExtensionControlInterceptor::DoProcess(AbilityInterceptorParam param)
{
    TAG_LOGE(AAFwkTag::ABILITYMGR, "call.");
    if (!param.want.GetBoolParam(STRICT_MODE, false)) {
        return ERR_OK;
    }
    AppExecFwk::AbilityInfo callerAbilityInfo;
    if (StartAbilityUtils::GetCallerAbilityInfo(param.callerToken, callerAbilityInfo)) {
        if (callerAbilityInfo.type != AppExecFwk::AbilityType::EXTENSION ||
            callerAbilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE ||
            callerAbilityInfo.bundleName == param.want.GetElement().GetBundleName()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "not other extension.");
            return ERR_OK;
        }
        // get target application info
        AppExecFwk::AbilityInfo targetAbilityInfo;
        if (StartAbilityUtils::startAbilityInfo != nullptr) {
            targetAbilityInfo = StartAbilityUtils::startAbilityInfo->abilityInfo;
        } else {
            auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
            if (bundleMgrHelper == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "The bundleMgrHelper is nullptr.");
                return ERR_OK;
            }
            IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->QueryAbilityInfo(param.want,
                AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, param.userId, targetAbilityInfo));
        }
        // check blocked list
        if (!targetAbilityInfo.applicationInfo.isSystemApp &&
            !DelayedSingleton<ExtensionConfig>::GetInstance()->IsExtensionStartThirdPartyAppEnable(
                callerAbilityInfo.extensionTypeName)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "The extension start has been blocked by third party app flag.");
            return EXTENSION_BLOCKED_BY_THIRD_PARTY_APP_FLAG;
        }
        if (targetAbilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE &&
            !DelayedSingleton<ExtensionConfig>::GetInstance()->IsExtensionStartServiceEnable(
                callerAbilityInfo.extensionTypeName, param.want.GetElement().GetURI())) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "The extension start has been blocked by service list.");
            return EXTENSION_BLOCKED_BY_SERVICE_LIST;
        }
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "other ok.");
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS