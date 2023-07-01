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
namespace {
class FfrtTaskHandle : public InnerTaskHandle {
public:
    FfrtTaskHandle(ffrt::task_handle &&taskHandle, const std::shared_ptr<int> &outDep)
        : InnerTaskHandle(std::move(taskHandle)), outDep_(outDep)
    {}
    ~FfrtTaskHandle() override = default;
    const std::shared_ptr<int> &GetOutDep() const
    {
        return outDep_;
    }
private:
    std::shared_ptr<int> outDep_;
};
}
std::shared_ptr<InnerTaskHandle> FfrtTaskHandlerWrap::SubmitTaskInner(std::function<void()> &&task,
    const TaskAttribute &taskAttr)
{
    HILOG_INFO("SubmitTaskInner ffrt task begin");
    auto outDep = std::make_shared<int>();
    if (taskAttr.IsDefault()) {
        return std::make_shared<FfrtTaskHandle>(ffrt::submit_h(std::move(task)), outDep);
    } else {
        ffrt::task_attr ffrtTaskAttr;
        BuildFfrtTaskAttr(taskAttr, ffrtTaskAttr);
        return std::make_shared<FfrtTaskHandle>(ffrt::submit_h(std::move(task),
            {}, {outDep.get()}, ffrtTaskAttr), outDep);
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
    auto ffrtTaskHandle = static_cast<FfrtTaskHandle*>(taskHandle.get());
    ffrt::wait({ffrtTaskHandle->GetOutDep().get()});
}
} // namespace AAFwk
} // namespace OHOS