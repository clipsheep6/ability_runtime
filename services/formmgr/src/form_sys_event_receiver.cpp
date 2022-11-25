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

#include <cinttypes>

#include "appexecfwk_errors.h"
#include "bundle_info.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "form_constants.h"
#include "form_data_mgr.h"
#include "form_db_cache.h"
#include "form_info_mgr.h"
#include "form_sys_event_receiver.h"
#include "form_timer_mgr.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Receiver Constructor.
 * @param subscriberInfo Subscriber info.
 */
FormSysEventReceiver::FormSysEventReceiver(const EventFwk::CommonEventSubscribeInfo &subscriberInfo)
    : EventFwk::CommonEventSubscriber(subscriberInfo)
{}
/**
 * @brief Receive common event.
 * @param eventData Common event data.
 */
void FormSysEventReceiver::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    std::string bundleName = want.GetElement().GetBundleName();
    if (action.empty() || (action != EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED && bundleName.empty())) {
        HILOG_ERROR("%{public}s failed, invalid param, action: %{public}s, bundleName: %{public}s",
            __func__, action.c_str(), bundleName.c_str());
        return;
    }
    if (eventHandler_ == nullptr) {
        HILOG_ERROR("%{public}s fail, eventhandler invalidate.", __func__);
        return;
    }
    HILOG_INFO("%{public}s, action:%{public}s.", __func__, action.c_str());
    std::weak_ptr<FormSysEventReceiver> weakThis = shared_from_this();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED) {
        auto task = [weakThis, want, bundleName]() {
            HILOG_INFO("%{public}s, bundle updated, bundleName: %{public}s", __func__, bundleName.c_str());
            std::shared_ptr<FormSysEventReceiver> sharedThis = weakThis.lock();
            if (sharedThis) {
                int userId = want.GetIntParam(KEY_USER_ID, 0);
                sharedThis->formEventHelper_.HandleProviderUpdated(bundleName, userId);
            }
        };
        eventHandler_->PostTask(task);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED) {
        int32_t userId = eventData.GetCode();
        auto task = [weakThis, userId]() {
            std::shared_ptr<FormSysEventReceiver> sharedThis = weakThis.lock();
            if (sharedThis) {
                sharedThis->HandleUserIdRemoved(userId);
            }
        };
        if (userId != -1) {
            eventHandler_->PostTask(task);
        }
    } else {
        HILOG_WARN("%{public}s warnning, invalid action.", __func__);
    }
}

// multiuser
void FormSysEventReceiver::HandleUserIdRemoved(const int32_t userId)
{
    std::vector<int64_t> removedFormIds;
    FormDataMgr::GetInstance().DeleteFormsByUserId(userId, removedFormIds);
    FormDbCache::GetInstance().DeleteDBFormsByUserId(userId);

    // delete form timer
    std::vector<int64_t>::iterator itRemoved;
    for (itRemoved = removedFormIds.begin();itRemoved != removedFormIds.end(); itRemoved++) {
        FormTimerMgr::GetInstance().RemoveFormTimer(*itRemoved);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
