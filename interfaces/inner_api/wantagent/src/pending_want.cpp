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

#include "pending_want.h"

#include "hilog_wrapper.h"
#include "pending_want_record.h"
#include "want_agent_client.h"
#include "want_agent_log_wrapper.h"
#include "want_sender_info.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

namespace OHOS::AbilityRuntime::WantAgent {
PendingWant::PendingWant(const sptr<AAFwk::IWantSender> &target)
    : target_(target), cancelReceiver_(nullptr), whitelistToken_(nullptr)
{}

PendingWant::PendingWant(const sptr<AAFwk::IWantSender> &target, const sptr<IRemoteObject> whitelistToken)
    : target_(target), cancelReceiver_(nullptr), whitelistToken_(whitelistToken)
{}

WantAgentConstant::OperationType PendingWant::GetType(const sptr<AAFwk::IWantSender> &target)
{
    int32_t operationType = 0;
    WantAgentClient::GetInstance().GetPendingWantType(target, operationType);
    return (WantAgentConstant::OperationType)operationType;
}

std::shared_ptr<PendingWant> PendingWant::GetAbility(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context,
    int requestCode, const std::shared_ptr<Want> &want, unsigned int flags)
{
    return GetAbility(context, requestCode, want, flags, nullptr);
}

std::shared_ptr<PendingWant> PendingWant::GetAbility(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context, int requestCode,
    const std::shared_ptr<Want> &want, unsigned int flags, const std::shared_ptr<WantParams> &options)
{
    WANT_AGENT_LOGI("PendingWant::GetAbility begin.");
    if (context == nullptr) {
        WANT_AGENT_LOGE("PendingWant::GetAbility invalid input param.");
        return nullptr;
    }

    WantsInfo wantsInfo;
    wantsInfo.want = *want;
    wantsInfo.resolvedTypes = want != nullptr ? want->GetType() : "";
    if (options != nullptr && !options->IsEmpty()) {
        wantsInfo.want.SetParams(*options);
    }

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = (int32_t)WantAgentConstant::OperationType::START_ABILITY;
    wantSenderInfo.allWants.push_back(wantsInfo);
    wantSenderInfo.bundleName = context->GetBundleName();
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = -1; // -1 : invalid user id
    wantSenderInfo.requestCode = requestCode;
    sptr<IWantSender> target = WantAgentClient::GetInstance().GetWantSender(wantSenderInfo, nullptr);
    WANT_AGENT_LOGI("PendingWant::GetAbility end.");

    return std::make_shared<PendingWant>(target);
}

std::shared_ptr<PendingWant> PendingWant::GetAbilities(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context, int requestCode,
    std::vector<std::shared_ptr<Want>> &wants, unsigned int flags)
{
    return GetAbilities(context, requestCode, wants, flags, nullptr);
}

std::shared_ptr<PendingWant> PendingWant::GetAbilities(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context, int requestCode,
    std::vector<std::shared_ptr<Want>> &wants, unsigned int flags, const std::shared_ptr<WantParams> &options)
{
    if (context == nullptr) {
        WANT_AGENT_LOGE("PendingWant::GetAbilities invalid input param.");
        return nullptr;
    }

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = (int32_t)WantAgentConstant::OperationType::START_ABILITIES;
    wantSenderInfo.bundleName = context->GetBundleName();
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = -1; // -1 : invalid user id
    wantSenderInfo.requestCode = requestCode;
    for (auto want : wants) {
        WantsInfo wantsInfo;
        if (want != nullptr) {
            wantsInfo.want = *want;
        }
        wantsInfo.resolvedTypes = want != nullptr ? want->GetType() : "";
        if (options != nullptr && !options->IsEmpty()) {
            wantsInfo.want.SetParams(*options);
        }
        wantSenderInfo.allWants.push_back(wantsInfo);
    }
    sptr<IWantSender> target = WantAgentClient::GetInstance().GetWantSender(wantSenderInfo, nullptr);

    return std::make_shared<PendingWant>(target);
}

std::shared_ptr<PendingWant> PendingWant::GetCommonEvent(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context,
    int requestCode, const std::shared_ptr<Want> &want, unsigned int flags)
{
    return GetCommonEventAsUser(context, requestCode, want, flags, 0);
}

std::shared_ptr<PendingWant> PendingWant::GetCommonEventAsUser(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context,
    int requestCode, const std::shared_ptr<Want> &want, unsigned int flags, int uid)
{
    if (context == nullptr) {
        WANT_AGENT_LOGE("PendingWant::GetCommonEventAsUser invalid input param.");
        return nullptr;
    }

    WantsInfo wantsInfo;
    if (want != nullptr) {
        wantsInfo.want = *want;
    }
    wantsInfo.resolvedTypes = want != nullptr ? want->GetType() : "";

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = (int32_t)WantAgentConstant::OperationType::SEND_COMMON_EVENT;
    wantSenderInfo.allWants.push_back(wantsInfo);
    wantSenderInfo.bundleName = context->GetBundleName();
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = -1; // -1 : invalid user id
    wantSenderInfo.requestCode = requestCode;
    sptr<IWantSender> target = WantAgentClient::GetInstance().GetWantSender(wantSenderInfo, nullptr);

    return std::make_shared<PendingWant>(target);
}

std::shared_ptr<PendingWant> PendingWant::GetService(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context,
    int requestCode, const std::shared_ptr<Want> &want, unsigned int flags)
{
    return BuildServicePendingWant(context, requestCode, want, flags,
        WantAgentConstant::OperationType::START_SERVICE);
}

std::shared_ptr<PendingWant> PendingWant::GetForegroundService(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context, int requestCode,
    const std::shared_ptr<Want> &want, unsigned int flags)
{
    return BuildServicePendingWant(
        context, requestCode, want, flags, WantAgentConstant::OperationType::START_FOREGROUND_SERVICE);
}

std::shared_ptr<PendingWant> PendingWant::BuildServicePendingWant(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context,
    int requestCode, const std::shared_ptr<Want> &want,
    unsigned int flags, WantAgentConstant::OperationType serviceKind)
{
    if (context == nullptr) {
        WANT_AGENT_LOGE("PendingWant::BuildServicePendingWant invalid input param.");
        return nullptr;
    }

    WantsInfo wantsInfo;
    if (want != nullptr) {
        wantsInfo.want = *want;
    }
    wantsInfo.resolvedTypes = want != nullptr ? want->GetType() : "";

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = (int32_t)serviceKind;
    wantSenderInfo.allWants.push_back(wantsInfo);
    wantSenderInfo.bundleName = context->GetBundleName();
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = -1; // -1 : invalid user id
    wantSenderInfo.requestCode = requestCode;
    sptr<IWantSender> target = WantAgentClient::GetInstance().GetWantSender(wantSenderInfo, nullptr);

    return std::make_shared<PendingWant>(target);
}

void PendingWant::Cancel(const sptr<AAFwk::IWantSender> &target)
{
    WantAgentClient::GetInstance().CancelWantSender(target);
}

void PendingWant::Send(const sptr<AAFwk::IWantSender> &target)
{
    Send(0, nullptr, nullptr, "", nullptr, target);
}

void PendingWant::Send(int resultCode, const sptr<AAFwk::IWantSender> &target)
{
    Send(resultCode, nullptr, nullptr, "", nullptr, target);
}

void PendingWant::Send(int resultCode, const std::shared_ptr<Want> &want,
    const sptr<AAFwk::IWantSender> &target)
{
    Send(resultCode, want, nullptr, "", nullptr, target);
}

void PendingWant::Send(
    int resultCode, const sptr<CompletedDispatcher> &onCompleted, const sptr<AAFwk::IWantSender> &target)
{
    Send(resultCode, nullptr, onCompleted, "", nullptr, target);
}

void PendingWant::Send(int resultCode, const std::shared_ptr<Want> &want,
    const sptr<CompletedDispatcher> &onCompleted, const sptr<AAFwk::IWantSender> &target)
{
    Send(resultCode, want, onCompleted, "", nullptr, target);
}

void PendingWant::Send(int resultCode, const std::shared_ptr<Want> &want,
    const sptr<CompletedDispatcher> &onCompleted, const std::string &requiredPermission,
    const sptr<AAFwk::IWantSender> &target)
{
    Send(resultCode, want, onCompleted, requiredPermission, nullptr, target);
}

void PendingWant::Send(int resultCode, const std::shared_ptr<Want> &want,
    const sptr<CompletedDispatcher> &onCompleted, const std::string &requiredPermission,
    const std::shared_ptr<WantParams> &options, const sptr<AAFwk::IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);
    if (SendAndReturnResult(resultCode, want, onCompleted, requiredPermission, options, target) != 0) {
        WANT_AGENT_LOGE("PendingWant::SendAndReturnResult failed.");
    }
}

int PendingWant::SendAndReturnResult(int resultCode, const std::shared_ptr<Want> &want,
    const sptr<CompletedDispatcher> &onCompleted, const std::string &requiredPermission,
    const std::shared_ptr<WantParams> &options, const sptr<AAFwk::IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    SenderInfo senderInfo;
    senderInfo.resolvedType = want != nullptr ? want->GetType() : "";
    if (want != nullptr) {
        senderInfo.want = *want;
    }
    if (options != nullptr) {
        senderInfo.want.SetParams(*options);
    }
    senderInfo.requiredPermission = requiredPermission;
    senderInfo.code = resultCode;
    senderInfo.finishedReceiver = onCompleted;

    return WantAgentClient::GetInstance().SendWantSender(target, senderInfo);
}

bool PendingWant::Equals(
    const std::shared_ptr<PendingWant> &targetPendingWant, const std::shared_ptr<PendingWant> &otherPendingWant)
{
    if ((targetPendingWant == nullptr) && (otherPendingWant == nullptr)) {
        return true;
    }
    if ((targetPendingWant == nullptr) || (otherPendingWant == nullptr)) {
        return false;
    }
    return targetPendingWant->GetHashCode(targetPendingWant->GetTarget()) ==
           otherPendingWant->GetHashCode(otherPendingWant->GetTarget());
}

sptr<IWantSender> PendingWant::GetTarget()
{
    return target_;
}

void PendingWant::SetTarget(const sptr<AAFwk::IWantSender> &target)
{
    target_ = target;
}

PendingWant::CancelReceiver::CancelReceiver(const std::weak_ptr<PendingWant> &outerInstance)
    : outerInstance_(outerInstance)
{}

void PendingWant::CancelReceiver::PerformReceive(const AAFwk::Want &want, int resultCode, const std::string &data,
    const AAFwk::WantParams &extras, bool serialized, bool sticky, int sendingUser)
{}

void PendingWant::CancelReceiver::Send(const int32_t resultCode)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (outerInstance_.lock() != nullptr) {
        outerInstance_.lock()->NotifyCancelListeners(resultCode);
    }
}

void PendingWant::RegisterCancelListener(
    const std::shared_ptr<CancelListener> &cancelListener, const sptr<AAFwk::IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (cancelListener == nullptr) {
        WANT_AGENT_LOGE("PendingWant::RegisterCancelListener invalid input param.");
        return;
    }
    std::scoped_lock<std::mutex> lock(lock_object);
    if (cancelReceiver_ == nullptr) {
        cancelReceiver_ = new (std::nothrow) CancelReceiver(weak_from_this());
    }
    bool isEmpty = cancelListeners_.empty();
    cancelListeners_.push_back(cancelListener);
    if (isEmpty) {
        WantAgentClient::GetInstance().RegisterCancelListener(target, cancelReceiver_);
    }
}

void PendingWant::NotifyCancelListeners(int32_t resultCode)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    std::vector<std::shared_ptr<CancelListener>> cancelListeners;
    {
        std::scoped_lock<std::mutex> lock(lock_object);
        cancelListeners = std::vector<std::shared_ptr<CancelListener>>(cancelListeners_);
    }
    for (auto cancelListener : cancelListeners) {
        if (cancelListener != nullptr) {
            cancelListener->OnCancelled(resultCode);
        }
    }
}

void PendingWant::UnregisterCancelListener(
    const std::shared_ptr<CancelListener> &cancelListener, const sptr<AAFwk::IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (cancelListener == nullptr) {
        WANT_AGENT_LOGE("PendingWant::UnregisterCancelListener invalid input param.");
        return;
    }

    std::scoped_lock<std::mutex> lock(lock_object);
    bool isEmpty = cancelListeners_.empty();
    cancelListeners_.erase(remove_if(cancelListeners_.begin(),
        cancelListeners_.end(),
        [cancelListener](std::shared_ptr<CancelListener> x) { return x == cancelListener; }),
        cancelListeners_.end());
    if (cancelListeners_.empty() && !isEmpty) {
        WantAgentClient::GetInstance().UnregisterCancelListener(target, cancelReceiver_);
    }
}

int PendingWant::GetHashCode(const sptr<AAFwk::IWantSender> &target)
{
    int32_t code = -1;
    WantAgentClient::GetInstance().GetPendingWantCode(target, code);
    return code;
}

int PendingWant::GetUid(const sptr<AAFwk::IWantSender> &target)
{
    int32_t uid = -1;
    WantAgentClient::GetInstance().GetPendingWantUid(target, uid);
    return uid;
}

std::string PendingWant::GetBundleName(const sptr<AAFwk::IWantSender> &target)
{
    std::string bundleName = "";
    WantAgentClient::GetInstance().GetPendingWantBundleName(target, bundleName);
    return bundleName;
}

std::shared_ptr<Want> PendingWant::GetWant(const sptr<AAFwk::IWantSender> &target)
{
    std::shared_ptr<Want> want = std::make_shared<Want>();
    int ret = WantAgentClient::GetInstance().GetPendingRequestWant(target, want);
    return ret ? nullptr : want;
}

bool PendingWant::Marshalling(Parcel &parcel) const
{
    if (target_ == nullptr || !(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(target_->AsObject())) {
        WANT_AGENT_LOGE("parcel WriteString failed");
        return false;
    }

    return true;
}

PendingWant *PendingWant::Unmarshalling(Parcel &parcel)
{
    PendingWant *pendingWant = new (std::nothrow) PendingWant();
    if (pendingWant == nullptr) {
        WANT_AGENT_LOGE("read from parcel failed");
        return nullptr;
    }
    sptr<AAFwk::IWantSender> target =
        iface_cast<AAFwk::IWantSender>((static_cast<MessageParcel*>(&parcel))->ReadRemoteObject());
    if (target == nullptr) {
        delete pendingWant;
        return nullptr;
    }
    pendingWant->SetTarget(target);

    return pendingWant;
}

std::shared_ptr<WantSenderInfo> PendingWant::GetWantSenderInfo(const sptr<AAFwk::IWantSender> &target)
{
    std::shared_ptr<WantSenderInfo> info = std::make_shared<WantSenderInfo>();
    int ret = WantAgentClient::GetInstance().GetWantSenderInfo(target, info);
    return ret ? nullptr : info;
}

ErrCode PendingWant::GetType(const sptr<AAFwk::IWantSender> &target, int32_t &operType)
{
    ErrCode result = WantAgentClient::GetInstance().GetPendingWantType(target, operType);
    return result;
}

ErrCode PendingWant::GetWant(const sptr<AAFwk::IWantSender> &target, std::shared_ptr<AAFwk::Want> &want)
{
    ErrCode result = WantAgentClient::GetInstance().GetPendingRequestWant(target, want);
    return result;
}
}  // namespace OHOS::AbilityRuntime::WantAgent
