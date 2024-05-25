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
#include "in_process_call_wrapper.h"
#include "parameters.h"
#include "permission_verification.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
}

UnlockScreenManager::~UnlockScreenManager() {}

UnlockScreenManager::UnlockScreenManager() {}

UnlockScreenManager &UnlockScreenManager::GetInstance()
{
    static UnlockScreenManager instance;
    return instance;
}

bool UnlockScreenManager::UnlockScreen()
{
    bool isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    bool isDeveloperMode = system::GetBoolParameter(DEVELOPER_MODE_STATE, false);
    if (!isShellCall) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not aa start call, just start ability");
        return true;
    }
    if (!isDeveloperMode) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not devlop mode, just start ability");
        return true;
    }

#ifdef SUPPORT_GRAPHICS
    bool isScreenLocked = OHOS::ScreenLock::ScreenLockManager::GetInstance()->IsScreenLocked();
    bool isScreenSecured = OHOS::ScreenLock::ScreenLockManager::GetInstance()->GetSecure();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "isScreenLocked: %{public}d, isScreenSecured: %{public}d",
        isScreenLocked, isScreenSecured);
    if (isScreenLocked && isScreenSecured) {
        return false;
    }
    if (isScreenLocked) {
        sptr<UnlockScreenCallback> listener = sptr<UnlockScreenCallback>(new (std::nothrow) UnlockScreenCallback());
        IN_PROCESS_CALL(OHOS::ScreenLock::ScreenLockManager::GetInstance()->Unlock(
            OHOS::ScreenLock::Action::UNLOCKSCREEN, listener));
    }
#endif

#ifdef SUPPORT_POWER
    bool isScreenOn = PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "isScreenOn: %{public}d", isScreenOn);
    if (!isScreenOn) {
        PowerMgr::PowerMgrClient::GetInstance().WakeupDevice();
    }
#endif
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS