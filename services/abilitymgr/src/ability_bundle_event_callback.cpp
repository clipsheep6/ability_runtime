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

#include "ability_bundle_event_callback.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "parameters.h"
#include "uri_permission_manager_client.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* KEY_TOKEN = "accessTokenId";
constexpr const char* KEY_UID = "uid";
constexpr const char* WEB_BUNDLE_NAME = "com.ohos.nweb";
constexpr const char* ARKWEB_CORE_PACKAGE_NAME = "persist.arkwebcore.package_name";

}
AbilityBundleEventCallback::AbilityBundleEventCallback(
    std::shared_ptr<TaskHandlerWrap> taskHandler, std::shared_ptr<AbilityAutoStartupService> abilityAutoStartupService)
    : taskHandler_(taskHandler), abilityAutoStartupService_(abilityAutoStartupService) {}

void AbilityBundleEventCallback::OnReceiveEvent(const EventFwk::CommonEventData eventData)
{
    // env check
    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnReceiveEvent failed, taskHandler is nullptr");
        return;
    }
    const Want& want = eventData.GetWant();
    // action contains the change type of haps.
    std::string action = want.GetAction();
    std::string bundleName = want.GetElement().GetBundleName();
    auto tokenId = static_cast<uint32_t>(want.GetIntParam(KEY_TOKEN, 0));
    int uid = want.GetIntParam(KEY_UID, 0);
    // verify data
    if (action.empty() || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnReceiveEvent failed, empty action/bundleName");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OnReceiveEvent, action:%{public}s.", action.c_str());

    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        // uninstall bundle
        HandleRemoveUriPermission(tokenId);
        HandleUpdatedModuleInfo(bundleName, uid);
        if (abilityAutoStartupService_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "OnReceiveEvent failed, abilityAutoStartupService is nullptr");
            return;
        }
        abilityAutoStartupService_->DeleteAutoStartupData(bundleName, tokenId);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED) {
        // install or uninstall module/bundle
        HandleUpdatedModuleInfo(bundleName, uid);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED) {
        if (bundleName == WEB_BUNDLE_NAME ||
            bundleName == system::GetParameter(ARKWEB_CORE_PACKAGE_NAME, "false")) {
            HandleRestartResidentProcessDependedOnWeb();
        }
        HandleUpdatedModuleInfo(bundleName, uid);
        HandleAppUpgradeCompleted(bundleName, uid);
        if (abilityAutoStartupService_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "OnReceiveEvent failed, abilityAutoStartupService is nullptr");
            return;
        }
        abilityAutoStartupService_->CheckAutoStartupData(bundleName, uid);
    }
}

void AbilityBundleEventCallback::HandleRemoveUriPermission(uint32_t tokenId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "HandleRemoveUriPermission: %{public}i", tokenId);
    auto ret = IN_PROCESS_CALL(AAFwk::UriPermissionManagerClient::GetInstance().RevokeAllUriPermissions(tokenId));
    if (!ret) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Revoke all uri permissions failed.");
    }
}

void AbilityBundleEventCallback::HandleUpdatedModuleInfo(const std::string &bundleName, int32_t uid)
{
    wptr<AbilityBundleEventCallback> weakThis = this;
    auto task = [weakThis, bundleName, uid]() {
        sptr<AbilityBundleEventCallback> sharedThis = weakThis.promote();
        if (sharedThis == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "sharedThis is nullptr.");
            return;
        }
        sharedThis->abilityEventHelper_.HandleModuleInfoUpdated(bundleName, uid);
    };
    taskHandler_->SubmitTask(task);
}

void AbilityBundleEventCallback::HandleAppUpgradeCompleted(const std::string &bundleName, int32_t uid)
{
    wptr<AbilityBundleEventCallback> weakThis = this;
    auto task = [weakThis, bundleName, uid]() {
        sptr<AbilityBundleEventCallback> sharedThis = weakThis.promote();
        if (sharedThis == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "sharedThis is nullptr.");
            return;
        }

        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        if (abilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityMgr is nullptr.");
            return;
        }
        abilityMgr->AppUpgradeCompleted(bundleName, uid);
    };
    taskHandler_->SubmitTask(task);
}

void AbilityBundleEventCallback::HandleRestartResidentProcessDependedOnWeb()
{
    auto task = []() {
        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        if (abilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityMgr is nullptr.");
            return;
        }
        abilityMgr->HandleRestartResidentProcessDependedOnWeb();
    };
    taskHandler_->SubmitTask(task);
}
} // namespace AAFwk
} // namespace OHOS