/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "sr_samgr_helper.h"

#include "bundle_constants.h"
#include "bundle_mgr_proxy.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#ifdef ACCOUNT_ENABLE
#include "os_account_manager.h"
#endif
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
SrSamgrHelper::SrSamgrHelper()
{}

SrSamgrHelper::~SrSamgrHelper()
{}

sptr<IBundleMgr> SrSamgrHelper::GetBundleMgr()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "GetBundleMgr called.");
    std::lock_guard<std::mutex> lock(bundleMgrMutex_);
    if (iBundleMgr_ == nullptr) {
        ConnectBundleMgrLocked();
    }
    return iBundleMgr_;
}

int32_t SrSamgrHelper::GetCurrentActiveUserId()
{
#ifdef ACCOUNT_ENABLE
    std::vector<int32_t> activeIds;
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeIds);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "QueryActiveOsAccountIds failed ret:%{public}d", ret);
        return Constants::INVALID_USERID;
    }
    if (activeIds.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "QueryActiveOsAccountIds activeIds empty");
        return Constants::INVALID_USERID;
    }
    TAG_LOGE(AAFwkTag::SER_ROUTER, "QueryActiveOsAccountIds activeIds ret:%{public}d", activeIds[0]);
    return activeIds[0];
#else
    TAG_LOGI(AAFwkTag::SER_ROUTER, "ACCOUNT_ENABLE is false");
    return 0;
#endif
}

void SrSamgrHelper::ConnectBundleMgrLocked()
{
    if (iBundleMgr_ != nullptr) {
        return;
    }
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed to get bms saManager.");
        return;
    }

    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed to get bms remoteObj.");
        return;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new (std::nothrow) BmsDeathRecipient());
    if (deathRecipient_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to create BmsDeathRecipient!");
        return;
    }
    if ((remoteObj->IsProxyObject()) && (!remoteObj->AddDeathRecipient(deathRecipient_))) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Add death recipient to bms failed.");
        return;
    }
    iBundleMgr_ = iface_cast<IBundleMgr>(remoteObj);
    if (iBundleMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "iface_cast failed, failed to get bms");
    }
}

void SrSamgrHelper::ResetProxy(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::mutex> lock(bundleMgrMutex_);
    if (iBundleMgr_ == nullptr) {
        return;
    }

    auto serviceRemote = iBundleMgr_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "To remove death recipient.");
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        iBundleMgr_ = nullptr;
    }
}

void SrSamgrHelper::BmsDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "BmsDeathRecipient handle remote abilityms died.");
    SrSamgrHelper::GetInstance().ResetProxy(remote);
}
} // namespace AbilityRuntime
} // namespace OHOS
