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

#ifndef OHOS_ABILITY_RUNTIME_TASK_HANDLER_WRAP_H
#define OHOS_ABILITY_RUNTIME_TASK_HANDLER_WRAP_H

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

#include "task_utils_wrap.h"

namespace ffrt {
class mutex;
};

namespace OHOS {
namespace AAFwk {
class TaskHandlerWrap;
class InnerTaskHandle;
class TaskHandle {
friend class TaskHandlerWrap;
public:
    TaskHandle() = default;
    TaskHandle(std::shared_ptr<TaskHandlerWrap> handler, std::shared_ptr<InnerTaskHandle> InnerTaskHandle,
        TaskStatus status = TaskStatus::PENDING) : handler_(handler), innerTaskHandle_(InnerTaskHandle)
    {
        status_ = std::make_shared<TaskStatus>(status);
    }
    bool Cancel() const;
    void Sync() const;
    bool IsSame(const TaskHandle &other) const
    {
        return innerTaskHandle_ == other.innerTaskHandle_;
    }
    explicit operator bool() const
    {
        return status_ && innerTaskHandle_;
    }
private:
    std::weak_ptr<TaskHandlerWrap> handler_;
    std::shared_ptr<InnerTaskHandle> innerTaskHandle_;
    std::shared_ptr<TaskStatus> status_;
};

class TaskHandlerWrap : public std::enable_shared_from_this<TaskHandlerWrap> {
friend class TaskHandle;
public:
    static std::shared_ptr<TaskHandlerWrap> CreateQueueHandler(const std::string &queueName,
        TaskQoS queueQos = TaskQoS::DEFAULT);
    static std::shared_ptr<TaskHandlerWrap> GetFfrtHandler();

    TaskHandlerWrap(TaskHandlerWrap &) = delete;
    void operator=(TaskHandlerWrap &) = delete;
    virtual ~TaskHandlerWrap();
    /**
     * Submit task to be scheduled and executed
     * @return TaskHandle, could be used later
     */
    TaskHandle SubmitTask(const std::function<void()> &task);
    TaskHandle SubmitTask(const std::function<void()> &task, const std::string &name);
    TaskHandle SubmitTask(const std::function<void()> &task, int64_t delayMillis);
    TaskHandle SubmitTask(const std::function<void()> &task, TaskQoS taskQos);
    TaskHandle SubmitTask(const std::function<void()> &task, const std::string &name,
        int64_t delayMillis, bool forceSubmit = true);
    TaskHandle SubmitTask(const std::function<void()> &task, const TaskAttribute &taskAttr);
    // Task can't be canceled by name if submited with this mothed
    TaskHandle SubmitTaskJust(const std::function<void()> &task, const std::string &name,
        int64_t delayMillis);
    // This is only used for compatibility and could be be wrong if multi tasks with same name submited.
    // TaskHandle::Cancel is prefered.
    bool CancelTask(const std::string &name);
protected:
    TaskHandlerWrap();
    virtual std::shared_ptr<InnerTaskHandle> SubmitTaskInner(std::function<void()> &&task,
        const TaskAttribute &taskAttr) = 0;
    virtual bool CancelTaskInner(const std::shared_ptr<InnerTaskHandle> &taskHandle) = 0;
    virtual void WaitTaskInner(const std::shared_ptr<InnerTaskHandle> &taskHandle) = 0;
    bool RemoveTask(const std::string &name, const TaskHandle &taskHandle);
protected:
    // this is used only for compatibility
    std::unordered_map<std::string, TaskHandle> tasks_;
    std::unique_ptr<ffrt::mutex> tasksMutex_;
};

class AutoSyncTaskHandle {
public:
    explicit AutoSyncTaskHandle(const TaskHandle &handle) : handle_(handle) {}
    ~AutoSyncTaskHandle()
    {
        Sync();
    }

    AutoSyncTaskHandle(AutoSyncTaskHandle&) = delete;
    void operator=(AutoSyncTaskHandle&) = delete;

    void Sync()
    {
        auto handle = handle_;
        handle_ = TaskHandle();
        if (handle) {
            handle.Sync();
        }
    }
private:
    TaskHandle handle_;
};
}  // namespace AAFWK
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_TASK_HANDLER_WRAP_H