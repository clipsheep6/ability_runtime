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

#include "dialog_request_callback_impl.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
void DialogRequestCallbackImpl::SendResult(int32_t resultCode)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (task_) {
        HILOG_DEBUG("result code:%{public}d.", resultCode);
        task_(resultCode);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS