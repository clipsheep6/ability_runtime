/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "scene_board_judgement.h"

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AppExecFwk {
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
/**
 * @brief Handling the life cycle switching of NewAbility.
 *
 * @param want Indicates the structure containing information about the ability.
 * @param targetState The life cycle state to switch to.
 *
 */

void NewAbilityImpl::HandleAbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITY,
        "Lifecycle: srcState:%{public}d; targetState: %{public}d; isNewWant: %{public}d, sceneFlag: %{public}d",
        lifecycleState_, targetState.state, targetState.isNewWant, targetState.sceneFlag);
#ifdef SUPPORT_GRAPHICS
    if (ability_ != nullptr) {
        ability_->sceneFlag_ = targetState.sceneFlag;
    }
    if ((lifecycleState_ == targetState.state) && !targetState.isNewWant) {
        if (ability_ != nullptr && targetState.state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
            ability_->RequestFocus(want);
            AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, targetState.state, GetRestoreData());
        }
        TAG_LOGE(AAFwkTag::ABILITY, "Org lifeCycleState equals to Dst lifeCycleState.");
        return;
    }
#endif
    SetLifeCycleStateInfo(targetState);
    if (ability_ != nullptr) {
        ability_->SetLaunchParam(targetState.launchParam);
        if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL) {
            ability_->SetStartAbilitySetting(targetState.setting);
            Start(want, sessionInfo);
            CheckAndRestore();
        }
    }

    bool ret = false;
    ret = AbilityTransaction(want, targetState);
    if (ret) {
        AbilityTransactionCallback(targetState.state);
    }
}

void NewAbilityImpl::HandleShareData(const int32_t &uniqueId)
{
    TAG_LOGI(AAFwkTag::ABILITY, "handleShareData begin sourceState:%{public}d.", lifecycleState_);
    WantParams wantParam;
    int32_t resultCode = Share(wantParam);
    TAG_LOGI(AAFwkTag::ABILITY, "wantParam size: %{public}d.", wantParam.Size());
    AbilityManagerClient::GetInstance()->ShareDataDone(token_, resultCode, uniqueId, wantParam);
}

void NewAbilityImpl::AbilityTransactionCallback(const AbilityLifeCycleState &state)
{
    TAG_LOGI(AAFwkTag::ABILITY, "Lifecycle: notify ability manager service.");
    auto ret = AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, state, GetRestoreData());
    if (ret == ERR_OK && state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::FOREGROUND };
        FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);
    }
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
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "NewAbilityImpl::AbilityTransaction begin");
    bool ret = true;
    switch (targetState.state) {
        case AAFwk::ABILITY_STATE_INITIAL: {
#ifdef SUPPORT_GRAPHICS
            if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled() &&
                lifecycleState_ == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
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
            TAG_LOGE(AAFwkTag::ABILITY, "NewAbilityImpl::HandleAbilityTransaction state error");
            break;
        }
    }
    TAG_LOGD(AAFwkTag::ABILITY, "NewAbilityImpl::AbilityTransaction end: retVal = %{public}d", static_cast<int>(ret));
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
