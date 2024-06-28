/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifdef SUPPORT_GRAPHICS
#include "unlock_screen_callback.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
UnlockScreenCallback::~UnlockScreenCallback() {}

UnlockScreenCallback::UnlockScreenCallback() {}

void UnlockScreenCallback::OnCallBack(const int32_t screenLockResult)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Unlock Screen result is %{public}d", screenLockResult);
}
} // namespace AbilityRuntime
} // namespace OHOS
#endif // SUPPORT_GRAPHICS

