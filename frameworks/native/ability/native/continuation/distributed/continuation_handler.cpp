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
#include "continuation_handler.h"

#include "ability_manager_client.h"
#include "distributed_errors.h"
#include "element_name.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

using OHOS::AAFwk::WantParams;
namespace OHOS {
namespace AppExecFwk {
const std::string ContinuationHandler::ORIGINAL_DEVICE_ID("deviceId");
const std::string VERSION_CODE_KEY = "version";
ContinuationHandler::ContinuationHandler(
    std::weak_ptr<ContinuationManager> &continuationManager, std::weak_ptr<Ability> &ability)
{
    ability_ = ability;
    continuationManager_ = continuationManager;
}

bool ContinuationHandler::HandleStartContinuationWithStack(const sptr<IRemoteObject> &token,
    const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "HandleStartContinuationWithStack token is null.");
        return false;
    }
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "HandleStartContinuationWithStack abilityInfo is null.");
        return false;
    }

    abilityInfo_->deviceId = deviceId;

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "HandleStartContinuationWithStack: get continuationManagerTmp is nullptr");
        return false;
    }

    // decided to start continuation. Callback to ability.
    Want want;
    want.SetParam(VERSION_CODE_KEY, static_cast<int32_t>(versionCode));
    want.SetParam("targetDevice", deviceId);
    WantParams wantParams = want.GetParams();
    int32_t status = continuationManagerTmp->OnContinue(wantParams);
    if (status != ERR_OK) {
        TAG_LOGI(AAFwkTag::CONTINUATION,
            "OnContinue failed, BundleName = %{public}s, ClassName= %{public}s, status: %{public}d",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str(),
            status);
    }

    want.SetParams(wantParams);
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    want.SetElementName(deviceId, abilityInfo_->bundleName, abilityInfo_->name, abilityInfo_->moduleName);

    int result = AAFwk::AbilityManagerClient::GetInstance()->StartContinuation(want, token, status);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "startContinuation failed.");
        return false;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return true;
}

bool ContinuationHandler::HandleStartContinuation(const sptr<IRemoteObject> &token, const std::string &deviceId)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationHandler::HandleStartContinuation token is null.");
        return false;
    }
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationHandler::HandleStartContinuation abilityInfo is null.");
        return false;
    }

    abilityInfo_->deviceId = deviceId;

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "handleStartContinuation: get continuationManagerTmp is nullptr");
        return false;
    }

    // DMS decided to start continuation. Callback to ability.
    if (!continuationManagerTmp->StartContinuation()) {
        TAG_LOGD(AAFwkTag::CONTINUATION, "handleStartContinuation: Ability rejected.");
        TAG_LOGI(AAFwkTag::CONTINUATION,
            "ID_ABILITY_SHELL_CONTINUE_ABILITY, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    WantParams wantParams;
    if (!continuationManagerTmp->SaveData(wantParams)) {
        TAG_LOGD(AAFwkTag::CONTINUATION, "handleStartContinuation: ScheduleSaveData failed.");
        TAG_LOGI(AAFwkTag::CONTINUATION,
            "ID_ABILITY_SHELL_CONTINUE_ABILITY, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    Want want = SetWantParams(wantParams);
    want.SetElementName(deviceId, abilityInfo_->bundleName, abilityInfo_->name, abilityInfo_->moduleName);

    int result = AAFwk::AbilityManagerClient::GetInstance()->StartContinuation(want, token, 0);
    if (result != 0) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "distClient_.startContinuation failed.");
        return false;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return true;
}

void ContinuationHandler::HandleReceiveRemoteScheduler(const sptr<IRemoteObject> &remoteReplica)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    if (remoteReplica == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "scheduler is nullptr");
        return;
    }

    if (remoteReplicaProxy_ != nullptr && schedulerDeathRecipient_ != nullptr) {
        auto schedulerObjectTmp = remoteReplicaProxy_->AsObject();
        if (schedulerObjectTmp != nullptr) {
            schedulerObjectTmp->RemoveDeathRecipient(schedulerDeathRecipient_);
        }
    }

    if (schedulerDeathRecipient_ == nullptr) {
        schedulerDeathRecipient_ = new (std::nothrow) ReverseContinuationSchedulerRecipient(
            std::bind(&ContinuationHandler::OnReplicaDied, this, std::placeholders::_1));
    }

    remoteReplicaProxy_ = iface_cast<IReverseContinuationSchedulerReplica>(remoteReplica);
    auto schedulerObject = remoteReplicaProxy_->AsObject();
    if (schedulerObject == nullptr || !schedulerObject->AddDeathRecipient(schedulerDeathRecipient_)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "AddDeathRcipient failed.");
    }

    remoteReplicaProxy_->PassPrimary(remotePrimaryStub_);
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
}

void ContinuationHandler::HandleCompleteContinuation(int result)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationHandler::HandleCompleteContinuation: get continuationManagerTmp is nullptr");
        return;
    }

    continuationManagerTmp->CompleteContinuation(result);
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
}

void ContinuationHandler::SetReversible(bool reversible)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called", __func__);
    reversible_ = reversible;
}

void ContinuationHandler::SetAbilityInfo(std::shared_ptr<AbilityInfo> &abilityInfo)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    abilityInfo_ = std::make_shared<AbilityInfo>(*(abilityInfo.get()));
    ClearDeviceInfo(abilityInfo_);
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
}

void ContinuationHandler::SetPrimaryStub(const sptr<IRemoteObject> &Primary)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called", __func__);
    remotePrimaryStub_ = Primary;
}

void ContinuationHandler::ClearDeviceInfo(std::shared_ptr<AbilityInfo> &abilityInfo)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called", __func__);
    abilityInfo->deviceId = "";
    abilityInfo->deviceTypes.clear();
}

void ContinuationHandler::OnReplicaDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    if (remoteReplicaProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "BUG: remote death notifies to a unready replica.");
        return;
    }

    auto object = remote.promote();
    if (!object) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "replica on remoteReplica died: null object.");
        return;
    }

    if (object != remoteReplicaProxy_->AsObject()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "replica on remoteReplica died: remoteReplica is not matches with remote.");
        return;
    }

    if (remoteReplicaProxy_ != nullptr && schedulerDeathRecipient_ != nullptr) {
        auto schedulerObject = remoteReplicaProxy_->AsObject();
        if (schedulerObject != nullptr) {
            schedulerObject->RemoveDeathRecipient(schedulerDeathRecipient_);
        }
    }
    remoteReplicaProxy_.clear();

    NotifyReplicaTerminated();
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
}

void ContinuationHandler::NotifyReplicaTerminated()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);

    CleanUpAfterReverse();

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationHandler::NotifyReplicaTerminated: get continuationManagerTmp is nullptr");
        return;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    continuationManagerTmp->NotifyRemoteTerminated();
}

Want ContinuationHandler::SetWantParams(const WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    Want want;
    want.SetParams(wantParams);
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    if (abilityInfo_->launchMode != LaunchMode::STANDARD) {
        TAG_LOGD(AAFwkTag::CONTINUATION, "SetWantParams: Clear task.");
    }
    if (reversible_) {
        TAG_LOGD(AAFwkTag::CONTINUATION, "SetWantParams: Reversible.");
        want.AddFlags(Want::FLAG_ABILITY_CONTINUATION_REVERSIBLE);
    }
    ElementName element("", abilityInfo_->bundleName, abilityInfo_->name, abilityInfo_->moduleName);
    want.SetElement(element);
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return want;
}

void ContinuationHandler::CleanUpAfterReverse()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called", __func__);
    remoteReplicaProxy_ = nullptr;
}

void ContinuationHandler::PassPrimary(const sptr<IRemoteObject> &Primary)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called", __func__);
    remotePrimaryProxy_ = iface_cast<IReverseContinuationSchedulerPrimary>(Primary);
}

bool ContinuationHandler::ReverseContinuation()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);

    if (remotePrimaryProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ReverseContinuation:remotePrimaryProxy_ not initialized, can not reverse");
        return false;
    }

    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ReverseContinuation: abilityInfo is null");
        return false;
    }

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ReverseContinuation: get continuationManagerTmp is nullptr");
        return false;
    }

    if (!continuationManagerTmp->StartContinuation()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ReverseContinuation: Ability rejected.");
        TAG_LOGI(AAFwkTag::CONTINUATION, "ReverseContinuation, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    WantParams wantParams;
    if (!continuationManagerTmp->SaveData(wantParams)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ReverseContinuation: SaveData failed.");
        TAG_LOGI(AAFwkTag::CONTINUATION, "ReverseContinuation, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    Want want;
    want.SetParams(wantParams);
    if (remotePrimaryProxy_->ContinuationBack(want)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "reverseContinuation: ContinuationBack send failed.");
        return false;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return true;
}

void ContinuationHandler::NotifyReverseResult(int reverseResult)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "NotifyReverseResult: Start. result = %{public}d", reverseResult);
    if (reverseResult == 0) {
        std::shared_ptr<Ability> ability = nullptr;
        ability = ability_.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationHandler::NotifyReverseResult failed. ability is nullptr");
            return;
        }
        ability->TerminateAbility();
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
}

bool ContinuationHandler::ContinuationBack(const Want &want)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationBack: get continuationManagerTmp is nullptr");
        return false;
    }

    int result = 0;
    if (!continuationManagerTmp->RestoreFromRemote(want.GetParams())) {
        TAG_LOGI(AAFwkTag::CONTINUATION, "ContinuationBack: RestoreFromRemote failed.");
        result = ABILITY_FAILED_RESTORE_DATA;
    }

    remoteReplicaProxy_->NotifyReverseResult(result);
    if (result == 0) {
        CleanUpAfterReverse();
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return true;
}

void ContinuationHandler::NotifyTerminationToPrimary()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    if (remotePrimaryProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "NotifyTerminationToPrimary: remotePrimary not initialized, can not notify");
        return;
    }

    TAG_LOGD(AAFwkTag::CONTINUATION, "NotifyTerminationToPrimary: Start");
    remotePrimaryProxy_->NotifyReplicaTerminated();
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
}

bool ContinuationHandler::ReverseContinueAbility()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    if (remoteReplicaProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ReverseContinueAbility: remoteReplica not initialized, can not reverse");
        return false;
    }

    TAG_LOGD(AAFwkTag::CONTINUATION, "ReverseContinueAbility: Start");
    bool requestSendSuccess = remoteReplicaProxy_->ReverseContinuation();
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return requestSendSuccess;
}
}  // namespace AppExecFwk
}  // namespace OHOS
