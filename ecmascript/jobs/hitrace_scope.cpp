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

#include "ecmascript/jobs/hitrace_scope.h"

#include "ecmascript/jobs/pending_job.h"
#include "hitrace/trace.h"

namespace panda::ecmascript::job {
EnqueueJobScope::EnqueueJobScope(const JSHandle<PendingJob> &pendingJob, QueueType queueType)
{
    HiTraceId id = HiTraceChain::GetId();
    if (id.IsValid() && id.IsFlagEnabled(HITRACE_FLAG_INCLUDE_ASYNC)) {
        HiTraceId childId = HiTraceChain::CreateSpan();

        pendingJob->SetChainId(childId.GetChainId());
        pendingJob->SetSpanId(childId.GetSpanId());
        pendingJob->SetParentSpanId(childId.GetParentSpanId());
        pendingJob->SetFlags(childId.GetFlags());

        if (id.IsFlagEnabled(HITRACE_FLAG_TP_INFO)) {
            if (queueType == QueueType::QUEUE_PROMISE) {
                HiTraceChain::Tracepoint(HITRACE_CM_THREAD, HITRACE_TP_CS,
                    childId, "Queue type:%s", "Promise queue");
            } else if (queueType == QueueType::QUEUE_SCRIPT) {
                HiTraceChain::Tracepoint(HITRACE_CM_THREAD, HITRACE_TP_CS,
                    childId, "Queue type:%s", "Script queue");
            } else {
                HiTraceChain::Tracepoint(HITRACE_CM_THREAD, HITRACE_TP_CS,
                    childId, "Queue type:%s", "Other queue");
            }
        }
    }
}

EnqueueJobScope::~EnqueueJobScope()
{
}

ExecuteJobScope::ExecuteJobScope(const JSHandle<PendingJob> &pendingJob)
{
    saveId_ = HiTraceChain::GetId();
    if (saveId_.IsValid()) {
        HiTraceChain::ClearId();
    }
    if (pendingJob->GetChainId() != 0) {
        hitraceId_.SetChainId(pendingJob->GetChainId());
        hitraceId_.SetSpanId(pendingJob->GetSpanId());
        hitraceId_.SetParentSpanId(pendingJob->GetParentSpanId());
        hitraceId_.SetFlags(pendingJob->GetFlags());

        if (hitraceId_.IsValid()) {
            HiTraceChain::SetId(hitraceId_);
            if (hitraceId_.IsFlagEnabled(HITRACE_FLAG_TP_INFO)) {
                HiTraceChain::Tracepoint(HITRACE_CM_THREAD, HITRACE_TP_SR,
                    hitraceId_, "Before %s pending job execute", "Promise");
            }
        }
    }
}

ExecuteJobScope::~ExecuteJobScope()
{
    if (hitraceId_.IsValid()) {
        if (hitraceId_.IsFlagEnabled(HITRACE_FLAG_TP_INFO)) {
            HiTraceChain::Tracepoint(HITRACE_CM_THREAD, HITRACE_TP_SS,
                hitraceId_, "After %s pending job execute", "Promise");
        }
        HiTraceChain::ClearId();
    }
    if (saveId_.IsValid()) {
        HiTraceChain::SetId(saveId_);
    }
}
}  // namespace panda::ecmascript::job