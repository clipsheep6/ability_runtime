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

#include "reverse_continuation_scheduler_primary_stage.h"

#include "continuation_handler_stage.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ReverseContinuationSchedulerPrimaryStage::ReverseContinuationSchedulerPrimaryStage(
    const std::weak_ptr<IReverseContinuationSchedulerPrimaryHandler> &continuationHandler,
    const std::shared_ptr<AbilityHandler> &mainHandler)
    : continuationHandler_(continuationHandler), mainHandler_(mainHandler)
{}

void ReverseContinuationSchedulerPrimaryStage::NotifyReplicaTerminated()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    wptr<ReverseContinuationSchedulerPrimaryStage> weak = this;
    auto task = [weak]() {
        auto reverseContinuationSchedulerPrimary = weak.promote();
        if (reverseContinuationSchedulerPrimary == nullptr) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "reverseContinuationSchedulerPrimary is nullptr.");
            return;
        }
        reverseContinuationSchedulerPrimary->HandlerNotifyReplicaTerminated();
    };

    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "mainHandler_ is nullptr.");
        return;
    }

    bool ret = mainHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "PostTask error.");
        return;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
}

bool ReverseContinuationSchedulerPrimaryStage::ContinuationBack(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    wptr<ReverseContinuationSchedulerPrimaryStage> weak = this;
    auto task = [weak, want]() {
        auto reverseContinuationSchedulerPrimary = weak.promote();
        if (reverseContinuationSchedulerPrimary == nullptr) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "reverseContinuationSchedulerPrimary is nullptr.");
            return;
        }
        reverseContinuationSchedulerPrimary->HandlerContinuationBack(want);
    };

    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "mainHandler_ is nullptr.");
        return false;
    }

    bool ret = mainHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "PostTask error.");
        return false;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return true;
}

void ReverseContinuationSchedulerPrimaryStage::HandlerNotifyReplicaTerminated()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    std::shared_ptr<IReverseContinuationSchedulerPrimaryHandler> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationHandler is nullptr.");
        return;
    }
    continuationHandler->NotifyReplicaTerminated();
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
}

void ReverseContinuationSchedulerPrimaryStage::HandlerContinuationBack(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    std::shared_ptr<IReverseContinuationSchedulerPrimaryHandler> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationHandler is nullptr.");
        return;
    }
    continuationHandler->ContinuationBack(want);
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
}
} // namespace AppExecFwk
} // namespace OHOS
