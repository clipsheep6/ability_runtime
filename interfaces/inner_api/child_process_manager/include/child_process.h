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

#ifndef OHOS_ABILITY_RUNTIME_CHILD_PROCESS_H
#define OHOS_ABILITY_RUNTIME_CHILD_PROCESS_H

#include <memory>

#include "child_process_start_info.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
class ChildProcess {
public:
    ChildProcess() = default;
    virtual ~ChildProcess() = default;

    static std::shared_ptr<ChildProcess> Create(const std::unique_ptr<Runtime> &runtime);
    
    virtual bool Init(const std::shared_ptr<ChildProcessStartInfo> &info);
    virtual void OnStart();

protected:
    std::shared_ptr<ChildProcessStartInfo> processStartInfo_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CHILD_PROCESS_H