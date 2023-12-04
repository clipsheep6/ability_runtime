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

#include "insight_intent_executor_mgr.h"

#include "ability_business_error.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
InsightIntentExecutorMgr::InsightIntentExecutorMgr()
{
    HILOG_DEBUG("constructor");
}

InsightIntentExecutorMgr::~InsightIntentExecutorMgr()
{
    HILOG_INFO("deconstructor");
}

bool InsightIntentExecutorMgr::ExecuteInsightIntent(Runtime& runtime, const InsightIntentExecutorInfo& executeInfo,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    HILOG_DEBUG("called.");
    auto executeParam = executeInfo.executeParam;
    if (executeParam == nullptr || executeParam->insightIntentParam_ == nullptr) {
        HILOG_ERROR("Execute param invalid.");
        TriggerCallbackInner(std::move(callback), static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return false;
    }

    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_](InsightIntentExecuteResult result) {
            // erase map when called
            HILOG_DEBUG("Begin remove executor.");
            auto executorMgr = weak.lock();
            if (executorMgr == nullptr) {
                HILOG_ERROR("Executor manager invalid.");
                return;
            }
            executorMgr->RemoveInsightIntentExecutor(intentId);
        };
    callback->Push(asyncCallback);

    // Create insight intent executor
    auto intentExecutor = InsightIntentExecutor::Create(runtime);
    if (intentExecutor == nullptr) {
        HILOG_ERROR("Create insight intent executor failed.");
        TriggerCallbackInner(std::move(callback), static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return false;
    }

    if (!intentExecutor->Init(executeInfo)) {
        HILOG_ERROR("Init intent executor failed.");
        TriggerCallbackInner(std::move(callback), static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return false;
    }
    HILOG_DEBUG("AddInsightIntentExecutor.");
    AddInsightIntentExecutor(executeParam->insightIntentId_, intentExecutor);

    bool isAsync = false;
    auto ret = intentExecutor->HandleExecuteIntent(static_cast<InsightIntentExecuteMode>(executeParam->executeMode_),
        executeParam->insightIntentName_, *executeParam->insightIntentParam_, executeInfo.pageLoader,
        std::move(callback), isAsync);
    if (!ret) {
        HILOG_ERROR("Execute intent failed.");
        // callback has removed, if execute insight intent failed, call in sub function.
        return false;
    }

    return true;
}

void InsightIntentExecutorMgr::AddInsightIntentExecutor(uint64_t intentId,
    const std::shared_ptr<InsightIntentExecutor>& executor)
{
    HILOG_DEBUG("called.");
    std::lock_guard<std::mutex> lock(mutex_);
    insightIntentExecutors_[intentId] = executor;
}

void InsightIntentExecutorMgr::RemoveInsightIntentExecutor(uint64_t intentId)
{
    HILOG_DEBUG("called.");
    std::lock_guard<std::mutex> lock(mutex_);
    insightIntentExecutors_.erase(intentId);
}

void InsightIntentExecutorMgr::TriggerCallbackInner(
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback, int32_t errCode)
{
    HILOG_DEBUG("called.");
    AppExecFwk::InsightIntentExecuteResult result;
    result.innerErr = errCode;
    callback->Call(result);
    callback.reset();
}
} // namespace AbilityRuntime
} // namespace OHOS
