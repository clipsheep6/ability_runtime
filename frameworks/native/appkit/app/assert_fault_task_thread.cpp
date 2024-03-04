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

#include <unordered_map>
#include <memory>

#include "ability_manager_client.h"
#include "assert_fault_task_thread.h"
#include "assert_fault_callback.h"
#include "hilog_wrapper.h"
#include "main_thread.h"
#include "string_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
std::unordered_map<AAFwk::UserStatus , ::Assert_Status> assertResultMap = {
    {AAFwk::UserStatus::ASSERT_TERMINATE, ::Assert_Status::ABORT},
    {AAFwk::UserStatus::ASSERT_CONTINUE, ::Assert_Status::IGNORE},
    {AAFwk::UserStatus::ASSERT_RETRY, ::Assert_Status::RETRY},
};
const AAFwk::UserStatus ASSERT_FAULT_DEFAULT_VALUE = AAFwk::UserStatus::ASSERT_TERMINATE; // default value is abort
constexpr char ASSERT_FAULT_THREAD[] = "assertFaultTHR";
constexpr char ASSERT_FAULT_DETAIL[] = "assertFaultDialogDetail";
constexpr char ASSERT_FAULT_PROMPT[] = "(Press Retry to debug the application)";
}
std::mutex AssertFaultTaskThread::constructorMutex_;
std::shared_ptr<AssertFaultTaskThread> AssertFaultTaskThread::instance_;
std::shared_ptr<AssertFaultTaskThread> AssertFaultTaskThread::GetInstance()
{
    if (instance_ == nullptr) {
        std::unique_lock<std::mutex> guard(constructorMutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AssertFaultTaskThread>();
        }
    }

    return instance_;
}

AAFwk::UserStatus AssertFaultTaskThread::AssertCallbackInner(const std::string &exprStr)
{
    if (assertHandler_ == nullptr) {
        HILOG_ERROR("Assert handler is nullptr.");
        return ASSERT_FAULT_DEFAULT_VALUE;
    }

    auto assertReuslt = ASSERT_FAULT_DEFAULT_VALUE;
    std::weak_ptr<AssertFaultTaskThread> weak = shared_from_this();
    assertHandler_->PostSyncTask([weak, exprStr, &assertReuslt]() {
        HILOG_DEBUG("Post sync task called.");
        auto assertFaultTask = weak.lock();
        if (assertFaultTask == nullptr) {
            HILOG_ERROR("Assert fault task instance is nullptr.");
            return;
        }
        assertReuslt = assertFaultTask->HandleAssertCallback(exprStr);
    });

    HILOG_DEBUG("Return sync task result.");
    return assertReuslt;
}

::Assert_Status ConvertAssertReuslt(AAFwk::UserStatus status)
{
    auto result = assertResultMap.find(status);
    if (result == assertResultMap.end()) {
        HILOG_ERROR("Find %{public}d failed, convert assert reuslt error.", status);
        return ::Assert_Status::ABORT;
    }
    return result->second;
}

static ::Assert_Status AssertCallback(::AssertFailureInfo assertFail)
{
    HILOG_DEBUG("Assert fault called.");
    auto instance = AssertFaultTaskThread::GetInstance();
    if (instance == nullptr) {
        HILOG_ERROR("Invalid Instance.");
        return ::Assert_Status::ABORT;;
    }
    std::string textDetail = std::string("File:\n") + assertFail.file + "\nFunction: " + assertFail.function +
        "\nLine: " + std::to_string(assertFail.line) + "\n\nExpression:\n" + assertFail.expression + "\n\n" +
        ASSERT_FAULT_PROMPT;
    auto ret = ConvertAssertReuslt(instance->AssertCallbackInner(textDetail));
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

    ::set_assert_callback(AssertCallback);
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

    if (isDebugModule_) {
        mainThread->AssertFaultPauseMainThreadDetection();
    }
    auto assertReuslt = ASSERT_FAULT_DEFAULT_VALUE;
    do {
        sptr<AssertFaultCallback> assertFaultCallback =
            new (std::nothrow) AssertFaultCallback(shared_from_this());
        if (assertFaultCallback == nullptr) {
            HILOG_ERROR("Invalid assert fault callback object.");
            break;
        }

        auto amsClient = AAFwk::AbilityManagerClient::GetInstance();
        if (amsClient == nullptr) {
            HILOG_ERROR("Invalid ams client object.");
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
        assertReuslt = assertFaultCallback->GetAssertResult();
    } while(false);

    if (isDebugModule_) {
        mainThread->AssertFaultResumeMainThreadDetection();
    }
    return assertReuslt;
}

void AssertFaultTaskThread::NotifyReleaseLongWaiting()
{
    std::unique_lock<std::mutex> lockAssertResult(assertResultMutex_);
    assertResultCV_.notify_one();
    HILOG_DEBUG("Notify assert result done.");
}
} // namespace AbilityRuntime
} // namespace OHOS