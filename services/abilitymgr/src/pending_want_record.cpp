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

#include "pending_want_record.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "pending_want_manager.h"

namespace OHOS {
namespace AAFwk {
PendingWantRecord::PendingWantRecord()
{}

PendingWantRecord::PendingWantRecord(const std::shared_ptr<PendingWantManager> &pendingWantManager, int32_t uid,
    int32_t callerTokenId, const sptr<IRemoteObject> &callerToken, std::shared_ptr<PendingWantKey> key)
    : pendingWantManager_(pendingWantManager), uid_(uid), callerTokenId_(callerTokenId),
    callerToken_(callerToken), key_(key)
{}

PendingWantRecord::~PendingWantRecord()
{}

void PendingWantRecord::Send(SenderInfo &senderInfo)
{
    SenderInner(senderInfo);
}

void PendingWantRecord::RegisterCancelListener(const sptr<IWantReceiver> &receiver)
{
    if (receiver == nullptr) {
        return;
    }
    mCancelCallbacks_.emplace_back(receiver);
}

void PendingWantRecord::UnregisterCancelListener(const sptr<IWantReceiver> &receiver)
{
    if (receiver == nullptr) {
        return;
    }
    if (mCancelCallbacks_.size()) {
        auto it = std::find(mCancelCallbacks_.cbegin(), mCancelCallbacks_.cend(), receiver);
        if (it != mCancelCallbacks_.cend()) {
            mCancelCallbacks_.erase(it);
        }
    }
}

int32_t PendingWantRecord::SenderInner(SenderInfo &senderInfo)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "%{public}s:begin.", __func__);
    std::lock_guard<ffrt::mutex> locker(lock_);
    if (canceled_) {
        TAG_LOGI(AAFwkTag::WANTAGENT, "wantAgent is canceled!");
        if (senderInfo.finishedReceiver != nullptr) {
            Want want;
            WantParams wantParams = {};
            senderInfo.finishedReceiver->PerformReceive(want, senderInfo.code, "canceled", wantParams, false, false, 0);
        }
        return START_CANCELED;
    }

    auto pendingWantManager = pendingWantManager_.lock();
    if (pendingWantManager == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "%{public}s:pendingWantManager is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }

    if (((uint32_t)key_->GetFlags() & (uint32_t)Flags::ONE_TIME_FLAG) != 0) {
        pendingWantManager->CancelWantSenderLocked(*this, true);
    }

    Want want;
    BuildSendWant(senderInfo, want);

    bool sendFinish = (senderInfo.finishedReceiver != nullptr);
    int res = NO_ERROR;
    switch (key_->GetType()) {
        case static_cast<int32_t>(OperationType::START_ABILITY):
            res = pendingWantManager->PendingWantStartAbility(
                want, callerToken_, key_->GetUserId(), callerUid_, callerTokenId_);
            break;
        case static_cast<int32_t>(OperationType::START_ABILITIES): {
            std::vector<WantsInfo> allWantsInfos = key_->GetAllWantsInfos();
            allWantsInfos.back().want = want;
            res = pendingWantManager->PendingWantStartAbilitys(
                allWantsInfos, callerToken_, key_->GetUserId(), callerUid_, callerTokenId_);
            break;
        }
        case static_cast<int32_t>(OperationType::START_SERVICE):
        case static_cast<int32_t>(OperationType::START_FOREGROUND_SERVICE):
            res = pendingWantManager->PendingWantStartAbility(
                want, callerToken_, key_->GetUserId(), callerUid_, callerTokenId_);
            break;
        case static_cast<int32_t>(OperationType::SEND_COMMON_EVENT):
            res = pendingWantManager->PendingWantPublishCommonEvent(want, senderInfo, callerUid_, callerTokenId_);
            (res == ERR_OK) ? (sendFinish = false) : (sendFinish = (senderInfo.finishedReceiver != nullptr));
            break;
        default:
            break;
    }

    if (sendFinish && res != START_CANCELED) {
        WantParams wantParams = {};
        senderInfo.finishedReceiver->PerformReceive(want, senderInfo.code, "", wantParams, false, false, 0);
    }

    return res;
}

void PendingWantRecord::BuildSendWant(SenderInfo &senderInfo, Want &want)
{
    if (key_->GetAllWantsInfos().size() != 0) {
        want = key_->GetRequestWant();
    }
    bool immutable = (static_cast<uint32_t>(key_->GetFlags()) & static_cast<uint32_t>(Flags::CONSTANT_FLAG)) != 0;
    senderInfo.resolvedType = key_->GetRequestResolvedType();
    if (!immutable) {
        want.AddFlags(key_->GetFlags());
    }
    WantParams wantParams = want.GetParams();
    auto sendInfoWantParams = senderInfo.want.GetParams().GetParams();
    for (auto mapIter = sendInfoWantParams.begin(); mapIter != sendInfoWantParams.end(); mapIter++) {
        std::string sendInfoWantParamKey = mapIter->first;
        if (want.GetParams().GetParam(sendInfoWantParamKey) == nullptr) {
            wantParams.SetParam(sendInfoWantParamKey, mapIter->second);
        }
    }
    want.SetParams(wantParams);
}

std::shared_ptr<PendingWantKey> PendingWantRecord::GetKey()
{
    return key_;
}

int32_t PendingWantRecord::GetUid() const
{
    return uid_;
}

void PendingWantRecord::SetCanceled()
{
    canceled_ = true;
}
bool PendingWantRecord::GetCanceled()
{
    return canceled_;
}

void PendingWantRecord::SetCallerUid(const int32_t callerUid)
{
    callerUid_ = callerUid;
}

std::list<sptr<IWantReceiver>> PendingWantRecord::GetCancelCallbacks()
{
    return mCancelCallbacks_;
}
}  // namespace AAFwk
}  // namespace OHOS
