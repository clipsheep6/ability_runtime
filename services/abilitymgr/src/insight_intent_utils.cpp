/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "insight_intent_utils.h"

#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "insight_intent_profile.h"
#include "iservice_registry.h"
#include "module_info.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
sptr<AppExecFwk::IBundleMgr> InsightIntentUtils::GetBundleManagerProxy()
{
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        HILOG_ERROR("Failed to get system ability manager.");
        return nullptr;
    }

    auto remoteObj = systemAbilityMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        HILOG_ERROR("Remote object is nullptr.");
        return nullptr;
    }

    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObj);
    if (bundleMgr == nullptr) {
        HILOG_ERROR("Failed to get bundle manager.");
        return nullptr;
    }

    return bundleMgr;
}

std::string InsightIntentUtils::GetSrcEntry(const std::string &bundleName, const std::string &moduleName,
    const std::string &intentName)
{
    HILOG_DEBUG("Get srcEntry, bundleName: %{public}s, moduleName: %{public}s, intentName: %{public}s.",
        bundleName.c_str(), moduleName.c_str(), intentName.c_str());
    if (bundleName.empty() || moduleName.empty() || intentName.empty()) {
        HILOG_ERROR("Invalid param.");
        return std::string("");
    }

    auto bundleMgr = GetBundleManagerProxy();
    if (bundleMgr == nullptr) {
        return std::string("");
    }

    // Get json profile firstly
    std::string profile;
    auto ret = IN_PROCESS_CALL(bundleMgr->GetJsonProfile(AppExecFwk::INTENT_PROFILE, bundleName, moduleName, profile));
    if (ret != ERR_OK) {
        HILOG_ERROR("Get json profile failed, error code: %{public}d.", ret);
        return std::string("");
    }

    // Transform json string
    std::vector<InsightIntentInfo> infos;
    if (!InsightIntentProfile::TransformTo(profile, infos)) {
        HILOG_ERROR("Transform profile failed.");
        return std::string("");
    }

    // Get srcEntry when intentName matched
    std::string srcEntry("");
    for (const auto &info: infos) {
        if (info.intentName == intentName) {
            srcEntry = info.srcEntry;
        }
    }
    HILOG_DEBUG("srcEntry: %{public}s", srcEntry.c_str());
    return srcEntry;
}
} // namespace AbilityRuntime
} // namespace OHOS
