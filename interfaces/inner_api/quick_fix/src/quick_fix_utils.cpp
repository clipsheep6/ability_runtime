/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "bundle_mgr_helper.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "singleton.h"
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

sptr<AppExecFwk::IQuickFixManager> QuickFixUtil::GetBundleQuickFixMgrProxy()
{
    HILOG_DEBUG("Function called.");
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("The bundleMgrHelper is nullptr.");
        return nullptr;
    }

    auto bundleQuickFixMgr = bundleMgrHelper->GetQuickFixManagerProxy();
    if (bundleQuickFixMgr == nullptr) {
        HILOG_ERROR("The bundleQuickFixMgr is nullptr.");
        return nullptr;
    }

    HILOG_DEBUG("Function finished.");
    return bundleQuickFixMgr;
}
} // namespace AAFwk
} // namespace OHOS
