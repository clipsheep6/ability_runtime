/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "new_ability_impl.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
/**
 * @brief Handling the life cycle switching of NewAbility.
 *
 * @param want Indicates the structure containing information about the ability.
 * @param targetState The life cycle state to switch to.
 *
 */

void NewAbilityImpl::HandleAbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState)
{
    HILOG_INFO("NewAbilityImpl::HandleAbilityTransaction begin sourceState:%{public}d; targetState: %{public}d; "
             "isNewWant: %{public}d, sceneFlag: %{public}d",
        lifecycleState_,
        targetState.state,
        targetState.isNewWant,
        targetState.sceneFlag);
#ifdef SUPPORT_GRAPHICS
    if (ability_ != nullptr) {
        ability_->sceneFlag_ = targetState.sceneFlag;
    }
    if ((lifecycleState_ == targetState.state) && !targetState.isNewWant) {
        if (ability_ != nullptr && targetState.state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
            ability_->RequestFocus(want);
            AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, targetState.state, GetRestoreData());
        }
        HILOG_ERROR("Org lifeCycleState equals to Dst lifeCycleState.");
        return;
    }
#endif
    SetLifeCycleStateInfo(targetState);
    if (ability_ != nullptr) {
        ability_->SetLaunchParam(targetState.launchParam);
        if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL) {
            ability_->SetStartAbilitySetting(targetState.setting);
            Start(want);
            CheckAndRestore();
        }
    }

    bool ret = false;
    ret = AbilityTransaction(want, targetState);
    if (ret) {
        AbilityTransactionCallback(targetState.state);
    }
}

void NewAbilityImpl::AbilityTransactionCallback(const AbilityLifeCycleState &state)
{
    HILOG_INFO("Handle ability transaction done, notify ability manager service.");
    AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, state, GetRestoreData());
}

/**
 * @brief Handling the life cycle switching of NewAbility in switch.
 *
 * @param want Indicates the structure containing information about the ability.
 * @param targetState The life cycle state to switch to.
 *
 * @return return true if need notify ams, otherwise return false.
 *
 */
bool NewAbilityImpl::AbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState)
{
    HILOG_DEBUG("NewAbilityImpl::AbilityTransaction begin");
    bool ret = true;
    switch (targetState.state) {
        case AAFwk::ABILITY_STATE_INITIAL: {
#ifdef SUPPORT_GRAPHICS
            if (lifecycleState_ == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
                Background();
            }
#endif
            bool isAsyncCallback = false;
            Stop(isAsyncCallback);
            if (isAsyncCallback) {
                // AMS will be notified after async callback
                ret = false;
            }
            break;
        }
        case AAFwk::ABILITY_STATE_FOREGROUND_NEW: {
            if (targetState.isNewWant) {
                NewWant(want);
            }
#ifdef SUPPORT_GRAPHICS
            if (lifecycleState_ == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
                if (ability_) {
                    ability_->RequestFocus(want);
                }
            } else {
                {
                    std::lock_guard<std::mutex> lock(notifyForegroundLock_);
                    notifyForegroundByWindow_ = false;
                }
                Foreground(want);
                std::lock_guard<std::mutex> lock(notifyForegroundLock_);
                ret = notifyForegroundByWindow_;
                if (ret) {
                    notifyForegroundByWindow_ = false;
                    notifyForegroundByAbility_ = false;
                }
            }
#endif
            break;
        }
        case AAFwk::ABILITY_STATE_BACKGROUND_NEW: {
            if (lifecycleState_ != ABILITY_STATE_STARTED_NEW) {
                ret = false;
            }
#ifdef SUPPORT_GRAPHICS
            Background();
#endif
            break;
        }
        default: {
            ret = false;
            HILOG_ERROR("NewAbilityImpl::HandleAbilityTransaction state error");
            break;
        }
    }
    HILOG_DEBUG("NewAbilityImpl::AbilityTransaction end: retVal = %{public}d", (int)ret);
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
