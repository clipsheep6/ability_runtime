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

#include "reverse_continuation_scheduler_primary_stage.h"

#include "continuation_handler_stage.h"
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
    HILOG_DEBUG("Begin.");
    wptr<ReverseContinuationSchedulerPrimaryStage> weak = this;
    auto task = [weak]() {
        auto reverseContinuationSchedulerPrimary = weak.promote();
        if (reverseContinuationSchedulerPrimary == nullptr) {
            HILOG_ERROR("reverseContinuationSchedulerPrimary is nullptr.");
            return;
        }
        reverseContinuationSchedulerPrimary->HandlerNotifyReplicaTerminated();
    };

    if (mainHandler_ == nullptr) {
        HILOG_ERROR("mainHandler_ is nullptr.");
        return;
    }

    bool ret = mainHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error.");
        return;
    }
    HILOG_DEBUG("End.");
}

bool ReverseContinuationSchedulerPrimaryStage::ContinuationBack(const AAFwk::Want &want)
{
    HILOG_DEBUG("Begin.");
    wptr<ReverseContinuationSchedulerPrimaryStage> weak = this;
    auto task = [weak, want]() {
        auto reverseContinuationSchedulerPrimary = weak.promote();
        if (reverseContinuationSchedulerPrimary == nullptr) {
            HILOG_ERROR("reverseContinuationSchedulerPrimary is nullptr.");
            return;
        }
        reverseContinuationSchedulerPrimary->HandlerContinuationBack(want);
    };

    if (mainHandler_ == nullptr) {
        HILOG_ERROR("mainHandler_ is nullptr.");
        return false;
    }

    bool ret = mainHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error.");
        return false;
    }
    HILOG_DEBUG("End.");
    return true;
}

void ReverseContinuationSchedulerPrimaryStage::HandlerNotifyReplicaTerminated()
{
    HILOG_DEBUG("Begin.");
    std::shared_ptr<IReverseContinuationSchedulerPrimaryHandler> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        HILOG_ERROR("ContinuationHandler is nullptr.");
        return;
    }
    continuationHandler->NotifyReplicaTerminated();
    HILOG_DEBUG("End.");
}

void ReverseContinuationSchedulerPrimaryStage::HandlerContinuationBack(const AAFwk::Want &want)
{
    HILOG_DEBUG("Begin.");
    std::shared_ptr<IReverseContinuationSchedulerPrimaryHandler> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        HILOG_ERROR("ContinuationHandler is nullptr.");
        return;
    }
    continuationHandler->ContinuationBack(want);
    HILOG_DEBUG("End.");
}
} // namespace AppExecFwk
} // namespace OHOS
