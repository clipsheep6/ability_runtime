/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "form_bms_helper.h"
#include "ability_manager_interface.h"
#include "appexecfwk_errors.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
sptr<IBundleMgr> FormBmsHelper::bundleMgr_ = nullptr;
std::mutex FormBmsHelper::bundleMgrMutex_;

FormBmsHelper::FormBmsHelper()
{}

FormBmsHelper::~FormBmsHelper()
{}

/**
 * @brief Acquire a bundle manager, if it not existed.
 * @return returns the bundle manager ipc object, or nullptr for failed.
 */
sptr<IBundleMgr> FormBmsHelper::GetBundleMgr()
{
    if (bundleMgr_ == nullptr) {
        std::lock_guard<std::mutex> lock(bundleMgrMutex_);
        if (bundleMgr_ == nullptr) {
            auto systemAbilityManager = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (systemAbilityManager == nullptr) {
                HILOG_ERROR("GetBundleMgr GetSystemAbilityManager is null");
                return nullptr;
            }
            auto bundleMgrSa = systemAbilityManager->GetSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
            if (bundleMgrSa == nullptr) {
                HILOG_ERROR("GetBundleMgr GetSystemAbility is null");
                return nullptr;
            }
            auto bundleMgr = OHOS::iface_cast<IBundleMgr>(bundleMgrSa);
            if (bundleMgr == nullptr) {
                HILOG_ERROR("GetBundleMgr iface_cast get null");
            }
            bundleMgr_ = bundleMgr;
        }
    }
    return bundleMgr_;
}

/**
 * @brief Add the bundle manager instance for debug.
 * @param bundleManager the bundle manager ipc object.
 */
void FormBmsHelper::SetBundleManager(const sptr<IBundleMgr> &bundleManager)
{
    HILOG_INFO("%{public}s called.", __func__);
    bundleMgr_ = bundleManager;
}
/**
 * @brief Notify module removable.
 * @param bundleName Provider ability bundleName.
 * @param moduleName Provider ability moduleName.
 */
void FormBmsHelper::NotifyModuleRemovable(const std::string &bundleName, const std::string &moduleName)
{
    HILOG_INFO("%{public}s, bundleName:%{public}s, moduleName:%{public}s",
        __func__, bundleName.c_str(), moduleName.c_str());
    if (bundleName.empty() || moduleName.empty()) {
        return;
    }

    std::string key = GenerateModuleKey(bundleName, moduleName);
    HILOG_INFO("%{public}s, begin to notify %{public}s removable", __func__, key.c_str());
    sptr<IBundleMgr> iBundleMgr = GetBundleMgr();
    if (iBundleMgr == nullptr) {
        HILOG_ERROR("%{public}s, failed to get IBundleMgr.", __func__);
        return;
    }

    std::string originId = IPCSkeleton::ResetCallingIdentity();

    IPCSkeleton::SetCallingIdentity(originId);
}
/**
 * @brief Notify module not removable.
 * @param bundleName Provider ability bundleName.
 * @param moduleName Provider ability moduleName.
 */
void FormBmsHelper::NotifyModuleNotRemovable(const std::string &bundleName, const std::string &moduleName) const
{
    std::string key = GenerateModuleKey(bundleName, moduleName);
    HILOG_INFO("%{public}s, begin to notify %{public}s not removable", __func__, key.c_str());
}

std::string FormBmsHelper::GenerateModuleKey(const std::string &bundleName, const std::string &moduleName) const
{
    return bundleName + "#" + moduleName;
}
}  // namespace AppExecFwk
}  // namespace OHOS
