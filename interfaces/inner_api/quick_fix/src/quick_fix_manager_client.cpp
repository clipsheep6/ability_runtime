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

#include "quick_fix_manager_client.h"

#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "quick_fix_error_utils.h"
#include "quick_fix_manager_proxy.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
int32_t QuickFixManagerClient::ApplyQuickFix(const std::vector<std::string> &quickFixFiles)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");

    auto quickFixMgr = ConnectQuickFixManagerService();
    if (quickFixMgr == nullptr) {
        HILOG_ERROR("Get quick fix manager service failed.");
        return QUICK_FIX_CONNECT_FAILED;
    }

    return quickFixMgr->ApplyQuickFix(quickFixFiles);
}

int32_t QuickFixManagerClient::GetApplyedQuickFixInfo(const std::string &bundleName,
    ApplicationQuickFixInfo &quickFixInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");

    auto quickFixMgr = ConnectQuickFixManagerService();
    if (quickFixMgr == nullptr) {
        HILOG_ERROR("Get quick fix manager service failed.");
        return QUICK_FIX_CONNECT_FAILED;
    }

    return quickFixMgr->GetApplyedQuickFixInfo(bundleName, quickFixInfo);
}

sptr<IQuickFixManager> QuickFixManagerClient::ConnectQuickFixManagerService()
{
    HILOG_DEBUG("function called.");
    std::lock_guard<std::mutex> lock(mutex_);
    if (quickFixMgr_ == nullptr) {
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityMgr == nullptr) {
            HILOG_ERROR("Failed to get SystemAbilityManager.");
            return nullptr;
        }

        auto remoteObj = systemAbilityMgr->GetSystemAbility(QUICK_FIX_MGR_SERVICE_ID);
        if (remoteObj == nullptr) {
            HILOG_ERROR("Remote object is nullptr.");
            return nullptr;
        }

        quickFixMgr_ = iface_cast<IQuickFixManager>(remoteObj);
        if (quickFixMgr_ == nullptr || quickFixMgr_->AsObject() == nullptr) {
            HILOG_ERROR("Failed to get quick fix manager service.");
            return nullptr;
        }

        auto self = weak_from_this();
        const auto &onClearProxyCallback = [self](const wptr<IRemoteObject> &remote) {
            auto impl = self.lock();
            if (impl && impl->quickFixMgr_ == remote) {
                impl->ClearProxy();
            }
        };

        sptr<QfmsDeathRecipient> recipient(new (std::nothrow) QfmsDeathRecipient(onClearProxyCallback));
        if (recipient == nullptr) {
            HILOG_ERROR("recipient == nullptr.");
            return nullptr;
        }
        quickFixMgr_->AsObject()->AddDeathRecipient(recipient);
    }

    HILOG_DEBUG("function finished.");
    return quickFixMgr_;
}

void QuickFixManagerClient::ClearProxy()
{
    HILOG_DEBUG("function called.");
    std::lock_guard<std::mutex> lock(mutex_);
    quickFixMgr_ = nullptr;
}

void QuickFixManagerClient::QfmsDeathRecipient::OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject> &remote)
{
    if (proxy_ != nullptr) {
        HILOG_ERROR("quick fix manager service died.");
        proxy_(remote);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
