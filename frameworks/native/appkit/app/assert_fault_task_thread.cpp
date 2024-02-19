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


namespace OHOS {
namespace AbilityRuntime {
namespace {
typedef enum {
    ASSERT_TERMINATE,
    ASSERT_RETRT,
    ASSERT_IGNORE
} AssertResult;
typedef int32_t (*AssertCallback)(void);
void setAssertCallback(AssertCallback callback) {}
// std::unordered_map<AAFwk::UserStatus , AssertResult> assertResultMap = {
    // {AAFwk::UserStatus::ASSERT_TERMINATE, AssertResult::ASSERT_TERMINATE},
    // {AAFwk::UserStatus::ASSERT_CONTINUE, AssertResult::ASSERT_IGNORE},
    // {AAFwk::UserStatus::ASSERT_RETRY, AssertResult::ASSERT_RETRT},
std::unordered_map<AAFwk::UserStatus , int32_t> assertResultMap = {
    {AAFwk::UserStatus::ASSERT_TERMINATE, 0},
    {AAFwk::UserStatus::ASSERT_CONTINUE, 1},
    {AAFwk::UserStatus::ASSERT_RETRY, 2},
};
constexpr int32_t ASSERT_FAULT_DEFAULT_VALUE = -1; // default value is abort
const char assertFaultThread[] = "assertFaultTHR";
}
ffrt::mutex AssertFaultTaskThread::constructorMutex_;
std::shared_ptr<AssertFaultTaskThread> AssertFaultTaskThread::instance_;
std::shared_ptr<AssertFaultTaskThread> AssertFaultTaskThread::GetInstance()
{
    if (instance_ == nullptr) {
        std::unique_lock<ffrt::mutex> guard(constructorMutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AssertFaultTaskThread>();
        }
    }

    return instance_;
}

int32_t AssertFaultTaskThread::AssertCallback()
{
    if (instance_ == nullptr) {
        HILOG_ERROR("Invalid Instance.");
        return ASSERT_FAULT_DEFAULT_VALUE;
    }
    return instance_->AssertCallbackInner();
}

int32_t AssertFaultTaskThread::AssertCallbackInner()
{
    if (assertHandler_ == nullptr) {
        HILOG_ERROR("Assert handler is nullptr.");
        return ASSERT_FAULT_DEFAULT_VALUE;
    }

    auto assertReuslt = ASSERT_FAULT_DEFAULT_VALUE;
    std::weak_ptr<AssertFaultTaskThread> weak = shared_from_this();
    assertHandler_->PostSyncTask([weak, &assertReuslt]() {
        HILOG_DEBUG("Post sync task called.");
        auto assertFaultTask = weak.lock();
        if (assertFaultTask == nullptr) {
            HILOG_ERROR("Assert fault task instance is nullptr.");
            return;
        }
        assertReuslt = assertFaultTask->HandleAssertCallback();
    });

    HILOG_DEBUG("Return sync task result.");
    return assertReuslt;
}

void AssertFaultTaskThread::InitAssertFaultTask(const wptr<AppExecFwk::MainThread> &weak, bool isDebugModule)
{

    auto runner = AppExecFwk::EventRunner::Create(assertFaultThread);
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

    setAssertCallback(AssertFaultTaskThread::AssertCallback);
    isDebugModule_ = isDebugModule;
    mainThread_ = weak;
    assertRunner_ = runner;
    assertHandler_ = assertHandler;

    assertHandler_->PostTask(AssertFaultTaskThread::AssertCallback, "TestCall", 20000);
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

int32_t AssertFaultTaskThread::HandleAssertCallback()
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
        auto err = amsClient->RequestAssertFaultDialog(assertFaultCallback->AsObject());
        if (err != ERR_OK) {
            HILOG_ERROR("Request assert fault dialog failed.");
            break;
        }

        assertResultCV_.wait(lockAssertResult);
        assertReuslt = ConvertAssertReuslt(assertFaultCallback->GetAssertResult());
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
}

int32_t AssertFaultTaskThread::ConvertAssertReuslt(AAFwk::UserStatus status)
{
    auto result = assertResultMap.find(status);
    if (result == assertResultMap.end()) {
        HILOG_ERROR("Find %{public}d failed, convert assert reuslt error.", status);
        return ASSERT_FAULT_DEFAULT_VALUE;
    }
    return result->second;
}
} // namespace AbilityRuntime
} // namespace OHOS