/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_APPEXECFWK_OHOS_DELEGATOR_THREAD_H
#define FOUNDATION_APPEXECFWK_OHOS_DELEGATOR_THREAD_H

#include <functional>
#include <memory>
#include <string>
#include "event_handler.h"
#include "event_runner.h"

namespace OHOS {
namespace AppExecFwk {
class DelegatorThread
{
public:
    using DTask = std::function<void()>;

public:
    explicit DelegatorThread(bool isMain = false);
    ~DelegatorThread() = default;

    bool Run(const DTask &task);
    std::string GetThreadName() const;

private:
    std::string threadName_;
    std::shared_ptr<EventRunner> runner_;
    std::shared_ptr<EventHandler> handler_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_OHOS_DELEGATOR_THREAD_H
