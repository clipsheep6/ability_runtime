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

#include "ability_bundle_event_callback.h"

#include "ability_manager_service.h"
#include "uri_permission_manager_client.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string KEY_TOKEN = "accessTokenId";
}
AbilityBundleEventCallback::AbilityBundleEventCallback(std::shared_ptr<TaskHandlerWrap> taskHandler)
    : taskHandler_(taskHandler) {}

void AbilityBundleEventCallback::OnReceiveEvent(const EventFwk::CommonEventData eventData)
{
    // env check
    if (taskHandler_ == nullptr) {
        HILOG_ERROR("OnReceiveEvent failed, taskHandler is nullptr");
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
        HILOG_ERROR("OnReceiveEvent failed, empty action/bundleName");
        return;
    }
    HILOG_DEBUG("OnReceiveEvent, action:%{public}s.", action.c_str());

    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        // uninstall bundle
        HandleRemoveUriPermission(tokenId);
        HandleUpdatedModuleInfo(bundleName, uid);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED) {
        // install or uninstall module/bundle
        HandleUpdatedModuleInfo(bundleName, uid);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED) {
        HandleUpdatedModuleInfo(bundleName, uid);
        HandleAppUpgradeCompleted(bundleName, uid);
    }
}

void AbilityBundleEventCallback::HandleRemoveUriPermission(uint32_t tokenId)
{
    HILOG_DEBUG("HandleRemoveUriPermission: %{public}i", tokenId);
    auto ret = IN_PROCESS_CALL(AAFwk::UriPermissionManagerClient::GetInstance().RevokeAllUriPermissions(tokenId));
    if (!ret) {
        HILOG_ERROR("Revoke all uri permissions failed.");
    }
}

void AbilityBundleEventCallback::HandleUpdatedModuleInfo(const std::string &bundleName, int32_t uid)
{
    wptr<AbilityBundleEventCallback> weakThis = this;
    auto task = [weakThis, bundleName, uid]() {
        sptr<AbilityBundleEventCallback> sharedThis = weakThis.promote();
        if (sharedThis == nullptr) {
            HILOG_ERROR("sharedThis is nullptr.");
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
            HILOG_ERROR("sharedThis is nullptr.");
            return;
        }

        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        if (abilityMgr == nullptr) {
            HILOG_ERROR("abilityMgr is nullptr.");
            return;
        }
        abilityMgr->AppUpgradeCompleted(bundleName, uid);
    };
    taskHandler_->SubmitTask(task);
}
} // namespace AAFwk
} // namespace OHOS