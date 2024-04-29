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

#ifndef OHOS_ABILITY_RUNTIME_FFRT_TASK_HANDLER_WRAP_H
#define OHOS_ABILITY_RUNTIME_FFRT_TASK_HANDLER_WRAP_H

#include "task_handler_wrap.h"
#include "ffrt_task_utils_wrap.h"

namespace OHOS {
namespace AAFwk {
class FfrtTaskHandlerWrap : public TaskHandlerWrap {
public:
    virtual ~FfrtTaskHandlerWrap() = default;

protected:
    std::shared_ptr<InnerTaskHandle> SubmitTaskInner(std::function<void()> &&task,
        const TaskAttribute &taskAttr) override;
    bool CancelTaskInner(const std::shared_ptr<InnerTaskHandle> &taskHandle) override;
    void WaitTaskInner(const std::shared_ptr<InnerTaskHandle> &taskHandle) override;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_FFRT_TASK_HANDLER_WRAP_H