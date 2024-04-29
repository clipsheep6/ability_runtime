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

#include "ffrt_task_handler_wrap.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
std::shared_ptr<InnerTaskHandle> FfrtTaskHandlerWrap::SubmitTaskInner(std::function<void()> &&task,
    const TaskAttribute &taskAttr)
{
    if (taskAttr.IsDefault()) {
        return std::make_shared<InnerTaskHandle>(ffrt::submit_h(std::move(task)));
    } else {
        ffrt::task_attr ffrtTaskAttr;
        BuildFfrtTaskAttr(taskAttr, ffrtTaskAttr);
        return std::make_shared<InnerTaskHandle>(ffrt::submit_h(std::move(task),
            {}, {}, ffrtTaskAttr));
    }
}

bool FfrtTaskHandlerWrap::CancelTaskInner(const std::shared_ptr<InnerTaskHandle> &taskHandle)
{
    if (!taskHandle) {
        return false;
    }
    return ffrt::skip(taskHandle->GetFfrtHandle()) == 0;
}

void FfrtTaskHandlerWrap::WaitTaskInner(const std::shared_ptr<InnerTaskHandle> &taskHandle)
{
    if (!taskHandle) {
        return;
    }
    ffrt::wait({taskHandle->GetFfrtHandle()});
}
} // namespace AAFwk
} // namespace OHOS