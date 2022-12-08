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

#include "quick_fix_utils.h"

#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
sptr<IRemoteObject> QuickFixUtil::GetRemoteObjectOfSystemAbility(const int32_t systemAbilityId)
{
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        HILOG_ERROR("Failed to get SystemAbilityManager.");
        return nullptr;
    }

    auto remoteObj = systemAbilityMgr->GetSystemAbility(systemAbilityId);
    if (remoteObj == nullptr) {
        HILOG_ERROR("Remote object is nullptr.");
        return nullptr;
    }

    return remoteObj;
}

sptr<AppExecFwk::IAppMgr> QuickFixUtil::GetAppManagerProxy()
{
    return iface_cast<AppExecFwk::IAppMgr>(GetRemoteObjectOfSystemAbility(APP_MGR_SERVICE_ID));
}

sptr<AppExecFwk::IBundleMgr> QuickFixUtil::GetBundleManagerProxy()
{
    return iface_cast<AppExecFwk::IBundleMgr>(GetRemoteObjectOfSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID));
}

sptr<AppExecFwk::IQuickFixManager> QuickFixUtil::GetBundleQuickFixMgrProxy()
{
    HILOG_DEBUG("function called.");
    auto bundleMgr = GetBundleManagerProxy();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("Failed to get bms.");
        return nullptr;
    }

    auto bundleQuickFixMgr = bundleMgr->GetQuickFixManagerProxy();
    if (bundleQuickFixMgr == nullptr) {
        HILOG_ERROR("Failed to get bundle quick fix manager.");
        return nullptr;
    }

    HILOG_DEBUG("function finished.");
    return bundleQuickFixMgr;
}
} // namespace AAFwk
} // namespace OHOS
