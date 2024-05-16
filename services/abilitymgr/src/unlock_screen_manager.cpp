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

#include "unlock_screen_manager.h"

#include "hilog_tag_wrapper.h"
#include "parameters.h"
#include "permission_verification.h"
#include "screenlock_common.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
}

UnlockScreenCallback::~UnlockScreenCallback() {}

UnlockScreenCallback::UnlockScreenCallback() {}

void UnlockScreenCallback::OnCallBack(const int32_t screenLockResult) {
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Unlock Screen result is %{public}d", screenLockResult);
}

UnlockScreenManager::~UnlockScreenManager() {}

UnlockScreenManager::UnlockScreenManager() {}

UnlockScreenManager &UnlockScreenManager::GetInstance()
{
    static UnlockScreenManager instance;
    return instance;
}

void UnlockScreenManager::UnlockScreen()
{
    bool isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    bool isDeveloperMode = system::GetBoolParameter(DEVELOPER_MODE_STATE, false);
    if (!isShellCall || !isDeveloperMode) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not aa start call or not devlop mode, just start ability");
        return;
    }
    bool isScreenLocked = OHOS::ScreenLock::ScreenLockManager::GetInstance()->IsScreenLocked();
    bool isScreenOn = PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "isScreenLocked: %{public}d, isScreenOn: %{public}d",
        isScreenLocked, isScreenOn);
    if (!isScreenOn) {
        PowerMgr::PowerMgrClient::GetInstance().WakeupDevice();
    }
    if (isScreenLocked) {
        sptr<UnlockScreenCallback> listener = sptr<UnlockScreenCallback>(new (std::nothrow) UnlockScreenCallback());
        OHOS::ScreenLock::ScreenLockManager::GetInstance()->Unlock(OHOS::ScreenLock::Action::UNLOCKSCREEN, listener);
    }
    return;
}
} // namespace AbilityRuntime
} // namespace OHOS