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

#include "quick_fix_util.h"

#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
sptr<AppExecFwk::IAppMgr> QuickFixUtil::GetAppManagerProxy()
{
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        HILOG_ERROR("Failed to get SystemAbilityManager.");
        return nullptr;
    }

    auto remoteObj = systemAbilityMgr->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (remoteObj == nullptr) {
        HILOG_ERROR("Remote object is nullptr.");
        return nullptr;
    }

    return iface_cast<AppExecFwk::IAppMgr>(remoteObj);
}

sptr<AppExecFwk::IBundleMgr> QuickFixUtil::GetBundleManagerProxy()
{
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        HILOG_ERROR("Failed to get SystemAbilityManager.");
        return nullptr;
    }

    auto remoteObj = systemAbilityMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        HILOG_ERROR("Remote object is nullptr.");
        return nullptr;
    }

    return iface_cast<AppExecFwk::IBundleMgr>(remoteObj);
}

sptr<AppExecFwk::IQuickFixManager> QuickFixUtil::GetBundleQuickFixMgrProxy()
{
    HILOG_INFO("%{public}s is called.", __func__);
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        HILOG_ERROR("Failed to get SystemAbilityManager.");
        return nullptr;
    }

    auto remoteObj = systemAbilityMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        HILOG_ERROR("Remote object is nullptr.");
        return nullptr;
    }

    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObj);
    if (bundleMgr == nullptr) {
        HILOG_ERROR("Failed to get bms.");
        return nullptr;
    }

    auto bundleQuickFixMgr = bundleMgr->GetQuickFixManagerProxy();
    if (bundleQuickFixMgr == nullptr) {
        HILOG_ERROR("Failed to get bundle quick fix manager.");
        return nullptr;
    }

    HILOG_INFO("%{public}s is finished.", __func__);
    return bundleQuickFixMgr;
}
} // namespace AAFwk
} // namespace OHOS
