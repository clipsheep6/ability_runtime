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

#include "reverse_continuation_scheduler_replica.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ReverseContinuationSchedulerReplica::ReverseContinuationSchedulerReplica(
    const std::shared_ptr<AbilityHandler> &mainHandler,
    const std::weak_ptr<IReverseContinuationSchedulerReplicaHandler> &replicaHandler)
{
    mainHandler_ = mainHandler;
    replicaHandler_ = replicaHandler;
}

void ReverseContinuationSchedulerReplica::PassPrimary(const sptr<IRemoteObject> &primary)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    auto task = [reverseContinuationSchedulerReplica = this, primary]() {
        reverseContinuationSchedulerReplica->HandlerPassPrimary(primary);
    };

    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "mainHandler_ is nullptr");
        return;
    }

    bool ret = mainHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "PostTask error");
    }
}

bool ReverseContinuationSchedulerReplica::ReverseContinuation()
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    auto task = [reverseContinuationSchedulerReplica = this]() {
        reverseContinuationSchedulerReplica->HandlerReverseContinuation();
    };

    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "mainHandler_ is nullptr");
        return false;
    }

    bool ret = mainHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "PostTask error");
    }
    return true;
}

void ReverseContinuationSchedulerReplica::NotifyReverseResult(int reverseResult)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    auto task = [reverseContinuationSchedulerReplica = this, reverseResult]() {
        reverseContinuationSchedulerReplica->HandlerNotifyReverseResult(reverseResult);
    };

    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "mainHandler_ is nullptr");
        return;
    }

    bool ret = mainHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "PostTask error");
    }
}

void ReverseContinuationSchedulerReplica::HandlerPassPrimary(const sptr<IRemoteObject> &primary)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    std::shared_ptr<IReverseContinuationSchedulerReplicaHandler> replicaHandlerTmp = nullptr;
    replicaHandlerTmp = replicaHandler_.lock();
    if (replicaHandlerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "replicaHandlerTmp is nullptr");
        return;
    }
    replicaHandlerTmp->PassPrimary(primary);
}

bool ReverseContinuationSchedulerReplica::HandlerReverseContinuation()
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    std::shared_ptr<IReverseContinuationSchedulerReplicaHandler> replicaHandlerTmp = nullptr;
    replicaHandlerTmp = replicaHandler_.lock();
    if (replicaHandlerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "replicaHandlerTmp is nullptr");
        return false;
    }
    return replicaHandlerTmp->ReverseContinuation();
}

void ReverseContinuationSchedulerReplica::HandlerNotifyReverseResult(int reverseResult)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    std::shared_ptr<IReverseContinuationSchedulerReplicaHandler> replicaHandlerTmp = nullptr;
    replicaHandlerTmp = replicaHandler_.lock();
    if (replicaHandlerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "replicaHandlerTmp is nullptr");
        return;
    }
    replicaHandlerTmp->NotifyReverseResult(reverseResult);
}
}  // namespace AppExecFwk
}  // namespace OHOS
