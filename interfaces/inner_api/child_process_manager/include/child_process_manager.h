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

#ifndef OHOS_ABILITY_RUNTIME_CHILD_PROCESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CHILD_PROCESS_MANAGER_H

#include <string>
#include <sys/types.h>

#include "hap_module_info.h"
#include "runtime.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
class ChildProcessManager {
    DECLARE_DELAYED_SINGLETON(ChildProcessManager);
public:
    pid_t StartChildProcessBySelfFork(const std::string srcEntry);
    bool MultiProcessModelEnabled();
    bool IsChildProcess();

private:
    void HandleChildProcess(const std::string srcEntry, AppExecFwk::HapModuleInfo &hapModuleInfo);
    bool GetHapModuleInfo(std::string bundleName, AppExecFwk::HapModuleInfo &hapModuleInfo);
    std::unique_ptr<AbilityRuntime::Runtime> CreateRuntime(AppExecFwk::HapModuleInfo &hapModuleInfo);

    bool multiProcessModelEnabled_ = false;
    bool isChildProcess_ = false;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_CHILD_PROCESS_MANAGER_H
