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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_UTIL_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_UTIL_H

#include "common_event_manager.h"
#include "common_event_support.h"
#include "hilog_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace ApplicationUtil {
using Want = OHOS::AAFwk::Want;

[[maybe_unused]] static void AppFwkBootEventCallback(const char *key, const char *value, void *context)
{
    if (strcmp(key, "bootevent.boot.completed") == 0 && strcmp(value, "true") == 0) {
        HILOG_INFO("%{public}s %{public}s is true", __func__, key);
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED);
        EventFwk::CommonEventData commonData {want};
        EventFwk::CommonEventManager::PublishCommonEvent(commonData);
        HILOG_INFO("%{public}s BootEvent completed", __func__);
    }
}
}  // namespace ApplicationlUtil
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPLICATION_UTIL_H
