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

#include "continuation_handler_stage.h"

#include "ability_manager_client.h"
#include "context/application_context.h"
#include "distributed_errors.h"
#include "element_name.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

using OHOS::AAFwk::WantParams;
namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string ORIGINAL_DEVICE_ID("deviceId");
const std::string VERSION_CODE_KEY = "version";
}

ContinuationHandlerStage::ContinuationHandlerStage(
    const std::weak_ptr<ContinuationManagerStage> &continuationManager,
    const std::weak_ptr<AbilityRuntime::UIAbility> &uiAbility)
    : ability_(uiAbility), continuationManager_(continuationManager) {}

bool ContinuationHandlerStage::HandleStartContinuationWithStack(
    const sptr<IRemoteObject> &token, const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Token is null.");
        return false;
    }
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "AbilityInfo is null.");
        return false;
    }

    abilityInfo_->deviceId = deviceId;

    std::shared_ptr<ContinuationManagerStage> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManagerTmp is nullptr.");
        return false;
    }

    // decided to start continuation. Callback to ability.
    Want want;
    want.SetParam(VERSION_CODE_KEY, static_cast<int32_t>(versionCode));
    want.SetParam("targetDevice", deviceId);
    WantParams wantParams = want.GetParams();
    int32_t status = continuationManagerTmp->OnContinue(wantParams);
    if (status != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "OnContinue failed, BundleName = %{public}s, ClassName= %{public}s, status: %{public}d",
            abilityInfo_->bundleName.c_str(), abilityInfo_->name.c_str(), status);
    }

    want.SetParams(wantParams);
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    want.SetElementName(deviceId, abilityInfo_->bundleName, abilityInfo_->name, abilityInfo_->moduleName);

    int result = AAFwk::AbilityManagerClient::GetInstance()->StartContinuation(want, token, status);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "StartContinuation failed.");
        return false;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return true;
}

bool ContinuationHandlerStage::HandleStartContinuation(const sptr<IRemoteObject> &token, const std::string &deviceId)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Token is null.");
        return false;
    }
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "AbilityInfo is null.");
        return false;
    }

    abilityInfo_->deviceId = deviceId;

    std::shared_ptr<ContinuationManagerStage> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "continuationManagerTmp is nullptr");
        return false;
    }

    // DMS decided to start continuation. Callback to ability.
    if (!continuationManagerTmp->StartContinuation()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability rejected, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(), abilityInfo_->name.c_str());
        return false;
    }

    WantParams wantParams;
    if (!continuationManagerTmp->SaveData(wantParams)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ScheduleSaveData failed, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(), abilityInfo_->name.c_str());
        return false;
    }

    Want want = SetWantParams(wantParams);
    want.SetElementName(deviceId, abilityInfo_->bundleName, abilityInfo_->name, abilityInfo_->moduleName);

    int result = AAFwk::AbilityManagerClient::GetInstance()->StartContinuation(want, token, 0);
    if (result != 0) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "distClient_.startContinuation failed");
        return false;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return true;
}

void ContinuationHandlerStage::HandleReceiveRemoteScheduler(const sptr<IRemoteObject> &remoteReplica)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
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
            std::bind(&ContinuationHandlerStage::OnReplicaDied, this, std::placeholders::_1));
    }

    remoteReplicaProxy_ = iface_cast<IReverseContinuationSchedulerReplica>(remoteReplica);
    auto schedulerObject = remoteReplicaProxy_->AsObject();
    if (schedulerObject == nullptr || !schedulerObject->AddDeathRecipient(schedulerDeathRecipient_)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "AddDeathRcipient failed.");
    }

    remoteReplicaProxy_->PassPrimary(remotePrimaryStub_);
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
}

void ContinuationHandlerStage::HandleCompleteContinuation(int result)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    std::shared_ptr<ContinuationManagerStage> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManagerTmp is nullptr.");
        return;
    }

    continuationManagerTmp->CompleteContinuation(result);
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
}

void ContinuationHandlerStage::SetReversible(bool reversible)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Called.");
    reversible_ = reversible;
}

void ContinuationHandlerStage::SetAbilityInfo(std::shared_ptr<AbilityInfo> &abilityInfo)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    abilityInfo_ = std::make_shared<AbilityInfo>(*(abilityInfo.get()));
    ClearDeviceInfo(abilityInfo_);
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
}

void ContinuationHandlerStage::SetPrimaryStub(const sptr<IRemoteObject> &Primary)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Called.");
    remotePrimaryStub_ = Primary;
}

void ContinuationHandlerStage::ClearDeviceInfo(std::shared_ptr<AbilityInfo> &abilityInfo)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Called.");
    abilityInfo->deviceId = "";
    abilityInfo->deviceTypes.clear();
}

void ContinuationHandlerStage::OnReplicaDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    if (remoteReplicaProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "RemoteReplicaProxy_ is nullptr.");
        return;
    }

    auto object = remote.promote();
    if (!object) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Object is null.");
        return;
    }

    if (object != remoteReplicaProxy_->AsObject()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "RemoteReplica is not matches with remote.");
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
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
}

void ContinuationHandlerStage::NotifyReplicaTerminated()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");

    CleanUpAfterReverse();

    std::shared_ptr<ContinuationManagerStage> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "continuationManagerTmp is nullptr");
        return;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    continuationManagerTmp->NotifyRemoteTerminated();
}

Want ContinuationHandlerStage::SetWantParams(const WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    Want want;
    want.SetParams(wantParams);
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    if (abilityInfo_->launchMode != LaunchMode::STANDARD) {
        TAG_LOGD(AAFwkTag::CONTINUATION, "Clear task.");
    }
    if (reversible_) {
        TAG_LOGD(AAFwkTag::CONTINUATION, "Reversible");
        want.AddFlags(Want::FLAG_ABILITY_CONTINUATION_REVERSIBLE);
    }
    ElementName element("", abilityInfo_->bundleName, abilityInfo_->name, abilityInfo_->moduleName);
    want.SetElement(element);
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return want;
}

void ContinuationHandlerStage::CleanUpAfterReverse()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Called.");
    remoteReplicaProxy_ = nullptr;
}

void ContinuationHandlerStage::PassPrimary(const sptr<IRemoteObject> &Primary)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Called.");
    remotePrimaryProxy_ = iface_cast<IReverseContinuationSchedulerPrimary>(Primary);
}

bool ContinuationHandlerStage::ReverseContinuation()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");

    if (remotePrimaryProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "RemotePrimaryProxy_ not nullptr.");
        return false;
    }

    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "AbilityInfo is null.");
        return false;
    }

    std::shared_ptr<ContinuationManagerStage> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManagerTmp is nullptr.");
        return false;
    }

    if (!continuationManagerTmp->StartContinuation()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability rejected, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(), abilityInfo_->name.c_str());
        return false;
    }

    WantParams wantParams;
    if (!continuationManagerTmp->SaveData(wantParams)) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "SaveData failed, BundleName = %{public}s, ClassName= %{public}s", abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    Want want;
    want.SetParams(wantParams);
    if (remotePrimaryProxy_->ContinuationBack(want)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationBack send failed.");
        return false;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return true;
}

void ContinuationHandlerStage::NotifyReverseResult(int reverseResult)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Start. result = %{public}d", reverseResult);
    if (reverseResult == 0) {
        std::shared_ptr<AbilityRuntime::UIAbility> ability = nullptr;
        ability = ability_.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "Ability is nullptr.");
            return;
        }
        ability->TerminateAbility();
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
}

bool ContinuationHandlerStage::ContinuationBack(const Want &want)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    std::shared_ptr<ContinuationManagerStage> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManagerTmp is nullptr.");
        return false;
    }

    int result = 0;
    if (!continuationManagerTmp->RestoreFromRemote(want.GetParams())) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "RestoreFromRemote failed.");
        result = ABILITY_FAILED_RESTORE_DATA;
    }

    remoteReplicaProxy_->NotifyReverseResult(result);
    if (result == 0) {
        CleanUpAfterReverse();
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return true;
}

void ContinuationHandlerStage::NotifyTerminationToPrimary()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    if (remotePrimaryProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "RemotePrimaryProxy is nullptr.");
        return;
    }

    remotePrimaryProxy_->NotifyReplicaTerminated();
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
}

bool ContinuationHandlerStage::ReverseContinueAbility()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    if (remoteReplicaProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "RemoteReplicaProxy is nullptr.");
        return false;
    }

    bool requestSendSuccess = remoteReplicaProxy_->ReverseContinuation();
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return requestSendSuccess;
}
} // namespace AppExecFwk
} // namespace OHOS
