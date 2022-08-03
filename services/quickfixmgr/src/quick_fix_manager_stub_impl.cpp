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

#include "uri_permission_manager_stub_impl.h"

#include "accesstoken_kit.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "os_account_manager_wrapper.h"
#include "singleton.h"
#include "system_ability_definition.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
void ApplyQuickFix(const std::vector<std::string> &quickFixFiles)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    auto bms = ConnectBundleManager();
    if (bms == nullptr) {
        HILOG_ERROR("Bundle manager is nullptr.");
        return;
    }

    // call bms DeployQuickFix

    // call bms SwitchQuickFix

    // call bms DeleteQuickFix

    HILOG_DEBUG("%{public}s is finished.", __func__);
    return;
}

sptr<AppExecFwk::IBundleMgr> QuickFixManagerStubImpl::ConnectBundleManager()
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    std::lock_guard<std::mutex> lock(bmsMutex_);
    if (bundleManager_ == nullptr) {
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityMgr) {
            HILOG_ERROR("Failed to get SystemAbilityManager.");
            return nullptr;
        }

        auto remoteObj = systemAbilityMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (!remoteObj || (bundleManager_ = iface_cast<AppExecFwk::IBundleMgr>(remoteObj)) == nullptr) {
            HILOG_ERROR("Failed to get bms.");
            return nullptr;
        }
        auto self = weak_from_this();
        const auto& onClearProxyCallback = [self](const wptr<IRemoteObject>& remote) {
            auto impl = self.lock();
            if (impl && impl->bundleManager_ == remote) {
                impl->ClearProxy();
            }
        };
        sptr<BMSDeathRecipient> recipient(new BMSDeathRecipient(onClearProxyCallback));
        bundleManager_->AsObject()->AddDeathRecipient(recipient);
    }
    HILOG_DEBUG("%{public}s end.", __func__);
    return bundleManager_;
}

void QuickFixManagerStubImpl::ClearProxy()
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    std::lock_guard<std::mutex> lock(bmsMutex_);
    bundleManager_ = nullptr;
}

void QuickFixManagerStubImpl::BMSDeathRecipient::OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject>& remote)
{
    if (proxy_) {
        HILOG_DEBUG("%{public}s, bms stub died.", __func__);
        proxy_(remote);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
