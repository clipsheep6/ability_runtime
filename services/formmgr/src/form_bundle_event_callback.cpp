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

#include "form_bundle_event_callback.h"

namespace OHOS {
namespace AppExecFwk {
void FormBundleEventCallback::OnReceiveEvent(const EventFwk::CommonEventData eventData)
{
    // env check
    if (eventHandler_ == nullptr) {
        HILOG_ERROR("%{public}s failed, eventHandler_ is nullptr", __func__);
        return;
    }
    const AAFwk::Want& want = eventData.GetWant();
    // action contains the change type of haps.
    std::string action = want.GetAction();
    std::string bundleName = want.GetElement().GetBundleName();
    int userId = want.GetIntParam(KEY_USER_ID, 0);
    // verify data
    if (action.empty() || bundleName.empty()) {
        HILOG_ERROR("%{public}s failed, empty action/bundleName", __func__);
        return;
    }
    if (eventHandler_ == nullptr) {
        HILOG_ERROR("%{public}s fail, invalid event handler.", __func__);
        return;
    }
    HILOG_INFO("%{public}s, action:%{public}s.", __func__, action.c_str());

    wptr<FormBundleEventCallback> weakThis = this;
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED ||
        action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED) {
        // install or update
        auto task = [weakThis, bundleName, userId]() {
            HILOG_INFO("%{public}s, bundle changed, bundleName: %{public}s", __func__, bundleName.c_str());
            sptr<FormBundleEventCallback> sharedThis = weakThis.promote();
            if (sharedThis) {
                sharedThis->formEventHelper_.HandleBundleFormInfoChanged(bundleName, userId);
                sharedThis->formEventHelper_.HandleProviderUpdated(bundleName, userId);
            }
        };
        eventHandler_->PostSyncTask(task);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        // uninstall module/bundle
        auto task = [weakThis, bundleName, userId]() {
            HILOG_INFO("%{public}s, bundle removed, bundleName: %{public}s", __func__, bundleName.c_str());
            sptr<FormBundleEventCallback> sharedThis = weakThis.promote();
            if (sharedThis) {
                sharedThis->formEventHelper_.HandleBundleFormInfoRemoved(bundleName, userId);
                sharedThis->formEventHelper_.HandleProviderRemoved(bundleName, userId);
            }
        };
        eventHandler_->PostSyncTask(task);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED) {
        // bundle data cleared
        auto task = [weakThis, bundleName, userId]() {
            sptr<FormBundleEventCallback> sharedThis = weakThis.promote();
            if (sharedThis) {
                sharedThis->formEventHelper_.HandleBundleDataCleared(bundleName, userId);
            }
        };
        eventHandler_->PostSyncTask(task);
    }
}

} // namespace AppExecFwk
} // namespace OHOS