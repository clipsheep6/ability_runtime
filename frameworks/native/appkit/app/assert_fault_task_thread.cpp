/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <memory>
#include <unordered_map>

#include "ability_manager_client.h"
#include "assert.h"
#include "assert_fault_task_thread.h"
#include "assert_fault_callback.h"
#include "hilog_wrapper.h"
#include "main_thread.h"
#include "string_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef enum Assert_Status {
    ASSERT_ABORT,
    ASSERT_RETRY,
    ASSERT_IGNORE
}Assert_Status;

typedef struct AssertFailureInfo {
    char *expression;
    char *file;
    char *function;
    int line;
}AssertFailureInfo;

typedef Assert_Status(*assert_call)(AssertFailureInfo assert_fail);
void set_assert_callback(assert_call cb) {};

#ifdef __cplusplus
}
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
std::unordered_map<AAFwk::UserStatus, Assert_Status> assertResultMap = {
    {AAFwk::UserStatus::ASSERT_TERMINATE, Assert_Status::ASSERT_ABORT},
    {AAFwk::UserStatus::ASSERT_CONTINUE, Assert_Status::ASSERT_IGNORE},
    {AAFwk::UserStatus::ASSERT_RETRY, Assert_Status::ASSERT_RETRY},
};
const AAFwk::UserStatus ASSERT_FAULT_DEFAULT_VALUE = AAFwk::UserStatus::ASSERT_TERMINATE; // default value is abort
constexpr char ASSERT_FAULT_THREAD[] = "assertFaultTHR";
constexpr char ASSERT_FAULT_DETAIL[] = "assertFaultDialogDetail";
constexpr char ASSERT_FAULT_PROMPT[] = "\n\n(Press Retry to debug the application)";
}

AAFwk::UserStatus AssertFaultTaskThread::RequestAssertResult(const std::string &exprStr)
{
    if (assertHandler_ == nullptr) {
        HILOG_ERROR("Assert handler is nullptr.");
        return ASSERT_FAULT_DEFAULT_VALUE;
    }

    auto assertResult = ASSERT_FAULT_DEFAULT_VALUE;
    std::weak_ptr<AssertFaultTaskThread> weak = shared_from_this();
    assertHandler_->PostSyncTask([weak, exprStr, &assertResult]() {
        HILOG_DEBUG("Post sync task called.");
        auto assertFaultTask = weak.lock();
        if (assertFaultTask == nullptr) {
            HILOG_ERROR("Assert fault task instance is nullptr.");
            return;
        }
        assertResult = assertFaultTask->HandleAssertCallback(exprStr);
    });
    return assertResult;
}

Assert_Status ConvertAssertResult(AAFwk::UserStatus status)
{
    auto result = assertResultMap.find(status);
    if (result == assertResultMap.end()) {
        HILOG_ERROR("Find %{public}d failed, convert assert reuslt error.", status);
        return Assert_Status::ASSERT_ABORT;
    }
    return result->second;
}

static Assert_Status AssertCallback(AssertFailureInfo assertFail)
{
    HILOG_DEBUG("Called.");
    auto instance = DelayedSingleton<AbilityRuntime::AssertFaultTaskThread>::GetInstance();
    if (instance == nullptr) {
        HILOG_ERROR("Invalid Instance.");
        return Assert_Status::ASSERT_ABORT;
    }

    std::string textFile = std::string("File:\n") +
        (assertFail.file == nullptr ? "Unknown" : std::string(assertFail.file));
    std::string textFunc = std::string("\nFunction: ") +
        (assertFail.function == nullptr ? "Unknown" : std::string(assertFail.function));
    std::string textLine = std::string("\nLine: ") + std::to_string(assertFail.line);
    std::string textExpression = std::string("\n\nExpression:\n") +
        (assertFail.expression == nullptr ? "Unknown" : std::string(assertFail.expression));
    std::string textDetail = textFile + textFunc + textLine + textExpression + ASSERT_FAULT_PROMPT;

    auto ret = ConvertAssertResult(instance->RequestAssertResult(textDetail));
    HILOG_DEBUG("Return sync task result is %{public}d.", static_cast<int32_t>(ret));
    return ret;
}

void AssertFaultTaskThread::InitAssertFaultTask(const wptr<AppExecFwk::MainThread> &weak, bool isDebugModule)
{
    auto runner = AppExecFwk::EventRunner::Create(ASSERT_FAULT_THREAD);
    if (runner == nullptr) {
        HILOG_ERROR("Runner is nullptr.");
        return;
    }

    auto assertHandler = std::make_shared<AppExecFwk::EventHandler>(runner);
    if (assertHandler == nullptr) {
        HILOG_ERROR("Handler is nullptr.");
        runner->Stop();
        return;
    }

    set_assert_callback(AssertCallback);
    isDebugModule_ = isDebugModule;
    mainThread_ = weak;
    assertRunner_ = runner;
    assertHandler_ = assertHandler;
}

void AssertFaultTaskThread::Stop()
{
    if (assertRunner_ == nullptr) {
        HILOG_ERROR("Assert runner is nullptr.");
        return;
    }
    assertRunner_->Stop();
    assertRunner_.reset();
}

AAFwk::UserStatus AssertFaultTaskThread::HandleAssertCallback(const std::string &exprStr)
{
    auto mainThread = mainThread_.promote();
    if (mainThread == nullptr) {
        HILOG_ERROR("Invalid thread object.");
        return ASSERT_FAULT_DEFAULT_VALUE;
    }

    if (!isDebugModule_) {
        mainThread->AssertFaultPauseMainThreadDetection();
    }
    auto assertResult = ASSERT_FAULT_DEFAULT_VALUE;
    do {
        sptr<AssertFaultCallback> assertFaultCallback =
            new (std::nothrow) AssertFaultCallback(shared_from_this());
        if (assertFaultCallback == nullptr) {
            HILOG_ERROR("Invalid assert fault callback object.");
            break;
        }

        auto amsClient = AAFwk::AbilityManagerClient::GetInstance();
        if (amsClient == nullptr) {
            HILOG_ERROR("Invalid client object.");
            break;
        }

        std::unique_lock<std::mutex> lockAssertResult(assertResultMutex_);
        AAFwk::WantParams wantParams;
        wantParams.SetParam(ASSERT_FAULT_DETAIL, AAFwk::String::Box(exprStr));
        auto err = amsClient->RequestAssertFaultDialog(assertFaultCallback->AsObject(), wantParams);
        if (err != ERR_OK) {
            HILOG_ERROR("Request assert fault dialog failed.");
            break;
        }

        assertResultCV_.wait(lockAssertResult);
        HILOG_DEBUG("Wait assert result over.");
        assertResult = assertFaultCallback->GetAssertResult();
    } while (false);

    if (!isDebugModule_) {
        mainThread->AssertFaultResumeMainThreadDetection();
    }
    return assertResult;
}

void AssertFaultTaskThread::NotifyReleaseLongWaiting()
{
    std::unique_lock<std::mutex> lockAssertResult(assertResultMutex_);
    assertResultCV_.notify_one();
    HILOG_DEBUG("Notify assert result done.");
}
} // namespace AbilityRuntime
} // namespace OHOS