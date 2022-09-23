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

#include "authorization_result.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
void AuthorizationResult::GrantResultsCallback(const std::vector<std::string> &permissions,
    const std::vector<int> &grantResults)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    if (task_) {
        HILOG_DEBUG("%{public}s callback client function.", __func__);
        task_(permissions, grantResults);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS