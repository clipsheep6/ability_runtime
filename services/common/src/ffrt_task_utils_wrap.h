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

#ifndef OHOS_ABILITY_RUNTIME_FFRT_TASK_UTILS_WRAP_H
#define OHOS_ABILITY_RUNTIME_FFRT_TASK_UTILS_WRAP_H

#include <string>
#include "ffrt_inner.h"
#include "task_utils_wrap.h"

namespace OHOS {
namespace AAFwk {
class InnerTaskHandle {
public:
    explicit InnerTaskHandle(ffrt::task_handle &&taskHandle)
        : taskHandle_(std::move(taskHandle)) {}
    InnerTaskHandle(InnerTaskHandle &) = delete;
    void operator=(InnerTaskHandle &) = delete;
    virtual ~InnerTaskHandle() = default;
    ffrt::task_handle &GetFfrtHandle()
    {
        return taskHandle_;
    }
private:
    ffrt::task_handle taskHandle_;
};

void BuildFfrtTaskAttr(const TaskAttribute &taskAttr, ffrt::task_attr &result);
ffrt::qos Convert2FfrtQos(TaskQoS taskqos);
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_FFRT_TASK_UTILS_WRAP_H