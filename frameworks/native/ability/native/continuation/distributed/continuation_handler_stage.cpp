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

#include "continuation_handler_stage.h"

#include "ability_manager_client.h"
#include "context/application_context.h"
#include "distributed_errors.h"
#include "element_name.h"
#include "hilog_wrapper.h"

using OHOS::AAFwk::WantParams;
namespace OHOS {
namespace AppExecFwk {
const std::string ContinuationHandlerStage::ORIGINAL_DEVICE_ID("deviceId");
const std::string VERSION_CODE_KEY = "version";
ContinuationHandlerStage::ContinuationHandlerStage(
    std::weak_ptr<ContinuationManagerStage> &continuationManager, std::weak_ptr<AbilityRuntime::UIAbility> &uiAbility)
{
    ability_ = uiAbility;
    continuationManager_ = continuationManager;
}

bool ContinuationHandlerStage::HandleStartContinuationWithStack(
    const sptr<IRemoteObject> &token, const std::string &deviceId, uint32_t versionCode)
{
    HILOG_DEBUG("begin");
    if (token == nullptr) {
        HILOG_ERROR("token is null.");
        return false;
    }
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("abilityInfo is null.");
        return false;
    }

    abilityInfo_->deviceId = deviceId;

    std::shared_ptr<ContinuationManagerStage> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        HILOG_ERROR("continuationManagerTmp is nullptr");
        return false;
    }

    // decided to start continuation. Callback to ability.
    Want want;
    want.SetParam(VERSION_CODE_KEY, static_cast<int32_t>(versionCode));
    want.SetParam("targetDevice", deviceId);
    WantParams wantParams = want.GetParams();
    int32_t status = continuationManagerTmp->OnContinue(wantParams);
    if (status != ERR_OK) {
        HILOG_ERROR("OnContinue failed, BundleName = %{public}s, ClassName= %{public}s, status: %{public}d",
            abilityInfo_->bundleName.c_str(), abilityInfo_->name.c_str(), status);
    }

    want.SetParams(wantParams);
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    want.SetElementName(deviceId, abilityInfo_->bundleName, abilityInfo_->name, abilityInfo_->moduleName);

    int result = AAFwk::AbilityManagerClient::GetInstance()->StartContinuation(want, token, status);
    if (result != ERR_OK) {
        HILOG_ERROR("startContinuation failed.");
        return false;
    }
    HILOG_DEBUG("end");
    return true;
}

bool ContinuationHandlerStage::HandleStartContinuation(const sptr<IRemoteObject> &token, const std::string &deviceId)
{
    HILOG_DEBUG("begin");
    if (token == nullptr) {
        HILOG_ERROR("token is null.");
        return false;
    }
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("abilityInfo is null.");
        return false;
    }

    abilityInfo_->deviceId = deviceId;

    std::shared_ptr<ContinuationManagerStage> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        HILOG_ERROR("continuationManagerTmp is nullptr.");
        return false;
    }

    // DMS decided to start continuation. Callback to ability.
    if (!continuationManagerTmp->StartContinuation()) {
        HILOG_ERROR("Ability rejected, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(), abilityInfo_->name.c_str());
        return false;
    }

    WantParams wantParams;
    if (!continuationManagerTmp->SaveData(wantParams)) {
        HILOG_ERROR("ScheduleSaveData failed, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(), abilityInfo_->name.c_str());
        return false;
    }

    Want want = SetWantParams(wantParams);
    want.SetElementName(deviceId, abilityInfo_->bundleName, abilityInfo_->name, abilityInfo_->moduleName);

    int result = AAFwk::AbilityManagerClient::GetInstance()->StartContinuation(want, token, 0);
    if (result != 0) {
        HILOG_ERROR("distClient_.startContinuation failed.");
        return false;
    }
    HILOG_DEBUG("end");
    return true;
}

void ContinuationHandlerStage::HandleReceiveRemoteScheduler(const sptr<IRemoteObject> &remoteReplica)
{
    HILOG_DEBUG("begin");
    if (remoteReplica == nullptr) {
        HILOG_ERROR("scheduler is nullptr.");
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
            std::bind(&ContinuationHandlerStage::OnReplicaDied, this, std::placeholders::_1));
    }

    remoteReplicaProxy_ = iface_cast<IReverseContinuationSchedulerReplica>(remoteReplica);
    auto schedulerObject = remoteReplicaProxy_->AsObject();
    if (schedulerObject == nullptr || !schedulerObject->AddDeathRecipient(schedulerDeathRecipient_)) {
        HILOG_ERROR("AddDeathRcipient failed.");
    }

    remoteReplicaProxy_->PassPrimary(remotePrimaryStub_);
    HILOG_DEBUG("end");
}

void ContinuationHandlerStage::HandleCompleteContinuation(int result)
{
    HILOG_DEBUG("begin");
    std::shared_ptr<ContinuationManagerStage> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        HILOG_ERROR("continuationManagerTmp is nullptr");
        return;
    }

    continuationManagerTmp->CompleteContinuation(result);
    HILOG_DEBUG("end");
}

void ContinuationHandlerStage::SetReversible(bool reversible)
{
    HILOG_DEBUG("called");
    reversible_ = reversible;
}

void ContinuationHandlerStage::SetAbilityInfo(std::shared_ptr<AbilityInfo> &abilityInfo)
{
    HILOG_DEBUG("begin");
    abilityInfo_ = std::make_shared<AbilityInfo>(*(abilityInfo.get()));
    ClearDeviceInfo(abilityInfo_);
    HILOG_DEBUG("end");
}

void ContinuationHandlerStage::SetPrimaryStub(const sptr<IRemoteObject> &Primary)
{
    HILOG_DEBUG("called");
    remotePrimaryStub_ = Primary;
}

void ContinuationHandlerStage::ClearDeviceInfo(std::shared_ptr<AbilityInfo> &abilityInfo)
{
    HILOG_DEBUG("called");
    abilityInfo->deviceId = "";
    abilityInfo->deviceTypes.clear();
}

void ContinuationHandlerStage::OnReplicaDied(const wptr<IRemoteObject> &remote)
{
    HILOG_DEBUG("begin");
    if (remoteReplicaProxy_ == nullptr) {
        HILOG_ERROR("remoteReplicaProxy_ is nullptr.");
        return;
    }

    auto object = remote.promote();
    if (!object) {
        HILOG_ERROR("object is null.");
        return;
    }

    if (object != remoteReplicaProxy_->AsObject()) {
        HILOG_ERROR("remoteReplica is not matches with remote.");
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
    HILOG_DEBUG("end");
}

void ContinuationHandlerStage::NotifyReplicaTerminated()
{
    HILOG_DEBUG("begin");

    CleanUpAfterReverse();

    std::shared_ptr<ContinuationManagerStage> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        HILOG_ERROR("continuationManagerTmp is nullptr.");
        return;
    }
    HILOG_DEBUG("end");
    continuationManagerTmp->NotifyRemoteTerminated();
}

Want ContinuationHandlerStage::SetWantParams(const WantParams &wantParams)
{
    HILOG_DEBUG("begin");
    Want want;
    want.SetParams(wantParams);
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    if (abilityInfo_->launchMode != LaunchMode::STANDARD) {
        HILOG_DEBUG("SetWantParams: Clear task.");
    }
    if (reversible_) {
        HILOG_DEBUG("SetWantParams: Reversible.");
        want.AddFlags(Want::FLAG_ABILITY_CONTINUATION_REVERSIBLE);
    }
    ElementName element("", abilityInfo_->bundleName, abilityInfo_->name, abilityInfo_->moduleName);
    want.SetElement(element);
    HILOG_DEBUG("end");
    return want;
}

void ContinuationHandlerStage::CleanUpAfterReverse()
{
    HILOG_DEBUG("called");
    remoteReplicaProxy_ = nullptr;
}

void ContinuationHandlerStage::PassPrimary(const sptr<IRemoteObject> &Primary)
{
    HILOG_DEBUG("called");
    remotePrimaryProxy_ = iface_cast<IReverseContinuationSchedulerPrimary>(Primary);
}

bool ContinuationHandlerStage::ReverseContinuation()
{
    HILOG_DEBUG("begin");

    if (remotePrimaryProxy_ == nullptr) {
        HILOG_ERROR("remotePrimaryProxy_ not nullptr.");
        return false;
    }

    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("abilityInfo is null.");
        return false;
    }

    std::shared_ptr<ContinuationManagerStage> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        HILOG_ERROR("continuationManagerTmp is nullptr.");
        return false;
    }

    if (!continuationManagerTmp->StartContinuation()) {
        HILOG_ERROR("Ability rejected, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(), abilityInfo_->name.c_str());
        return false;
    }

    WantParams wantParams;
    if (!continuationManagerTmp->SaveData(wantParams)) {
        HILOG_ERROR("SaveData failed, BundleName = %{public}s, ClassName= %{public}s", abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    Want want;
    want.SetParams(wantParams);
    if (remotePrimaryProxy_->ContinuationBack(want)) {
        HILOG_ERROR("ContinuationBack send failed.");
        return false;
    }
    HILOG_DEBUG("end");
    return true;
}

void ContinuationHandlerStage::NotifyReverseResult(int reverseResult)
{
    HILOG_DEBUG("Start. result = %{public}d", reverseResult);
    if (reverseResult == 0) {
        std::shared_ptr<AbilityRuntime::UIAbility> ability = nullptr;
        ability = ability_.lock();
        if (ability == nullptr) {
            HILOG_ERROR("ability is nullptr.");
            return;
        }
        ability->TerminateAbility();
    }
    HILOG_DEBUG("end");
}

bool ContinuationHandlerStage::ContinuationBack(const Want &want)
{
    HILOG_DEBUG("begin");
    std::shared_ptr<ContinuationManagerStage> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        HILOG_ERROR("continuationManagerTmp is nullptr");
        return false;
    }

    int result = 0;
    if (!continuationManagerTmp->RestoreFromRemote(want.GetParams())) {
        HILOG_ERROR("RestoreFromRemote failed.");
        result = ABILITY_FAILED_RESTORE_DATA;
    }

    remoteReplicaProxy_->NotifyReverseResult(result);
    if (result == 0) {
        CleanUpAfterReverse();
    }
    HILOG_DEBUG("end");
    return true;
}

void ContinuationHandlerStage::NotifyTerminationToPrimary()
{
    HILOG_DEBUG("begin");
    if (remotePrimaryProxy_ == nullptr) {
        HILOG_ERROR("remotePrimaryProxy_ is nullptr.");
        return;
    }

    remotePrimaryProxy_->NotifyReplicaTerminated();
    HILOG_DEBUG("end");
}

bool ContinuationHandlerStage::ReverseContinueAbility()
{
    HILOG_DEBUG("begin");
    if (remoteReplicaProxy_ == nullptr) {
        HILOG_ERROR("remoteReplicaProxy_ is nullptr.");
        return false;
    }

    bool requestSendSuccess = remoteReplicaProxy_->ReverseContinuation();
    HILOG_DEBUG("end");
    return requestSendSuccess;
}
} // namespace AppExecFwk
} // namespace OHOS
