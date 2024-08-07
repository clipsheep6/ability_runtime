/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "disposed_observer.h"

#include "interceptor/disposed_rule_interceptor.h"
#include "ability_record.h"
#include "modal_system_ui_extension.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* UIEXTENSION_MODAL_TYPE = "ability.want.params.modalType";
constexpr const char* INTERCEPT_MISSION_ID = "intercept_missionId";
}

DisposedObserver::DisposedObserver(const AppExecFwk::DisposedRule &disposedRule,
    const std::shared_ptr<DisposedRuleInterceptor> &interceptor)
    : disposedRule_(disposedRule), interceptor_(interceptor)
{}

void DisposedObserver::OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call");
    std::lock_guard<ffrt::mutex> guard(observerLock_);
    if (abilityStateData.abilityState != static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND)) {
        return;
    }
    token_ = abilityStateData.token;
    auto abilityRecord = Token::GetAbilityRecordByToken(token_);
    if (abilityRecord && !abilityRecord->GetAbilityInfo().isStageBasedModel) {
        auto systemUIExtension = std::make_shared<OHOS::Rosen::ModalSystemUiExtension>();
        Want want = *disposedRule_.want;
        want.SetParam(UIEXTENSION_MODAL_TYPE, 1);
        auto sessionInfo = abilityRecord->GetSessionInfo();
        if (sessionInfo != nullptr) {
            want.SetParam(INTERCEPT_MISSION_ID, sessionInfo->persistentId);
        } else {
            want.SetParam(INTERCEPT_MISSION_ID, abilityRecord->GetMissionId());
        }
        bool ret = IN_PROCESS_CALL(systemUIExtension->CreateModalUIExtension(want));
        if (!ret) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to start system UIExtension");
        }
        interceptor_->UnregisterObserver(abilityStateData.bundleName);
    }
}

void DisposedObserver::OnPageShow(const AppExecFwk::PageStateData &pageStateData)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call");
    if (disposedRule_.componentType == AppExecFwk::ComponentType::UI_ABILITY) {
        int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(*disposedRule_.want));
        if (ret != ERR_OK) {
            interceptor_->UnregisterObserver(pageStateData.bundleName);
            TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to start disposed ability");
            return;
        }
    }
    if (disposedRule_.componentType == AppExecFwk::ComponentType::UI_EXTENSION) {
        if (!token_) {
            auto systemUIExtension = std::make_shared<OHOS::Rosen::ModalSystemUiExtension>();
            Want want = *disposedRule_.want;
            want.SetParam(UIEXTENSION_MODAL_TYPE, 1);
            bool ret = IN_PROCESS_CALL(systemUIExtension->CreateModalUIExtension(want));
            if (!ret) {
                interceptor_->UnregisterObserver(pageStateData.bundleName);
                TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to start system UIExtension");
                return;
            }
        } else {
            auto abilityRecord = Token::GetAbilityRecordByToken(token_);
            if (!abilityRecord) {
                interceptor_->UnregisterObserver(pageStateData.bundleName);
                TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr");
                return;
            }
            Want want = *disposedRule_.want;
            auto sessionInfo = abilityRecord->GetSessionInfo();
            if (sessionInfo != nullptr) {
                want.SetParam(INTERCEPT_MISSION_ID, sessionInfo->persistentId);
            } else {
                want.SetParam(INTERCEPT_MISSION_ID, abilityRecord->GetMissionId());
            }
            int ret = abilityRecord->CreateModalUIExtension(want);
            if (ret != ERR_OK) {
                interceptor_->UnregisterObserver(pageStateData.bundleName);
                TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to start disposed UIExtension");
                return;
            }
        }
    }
    interceptor_->UnregisterObserver(pageStateData.bundleName);
}
} // namespace AAFwk
} // namespace OHOS
