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

#include "child_process_manager_error_utils.h"
#include "hap_module_info.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
class ChildProcessManager {
public:
    static ChildProcessManager &GetInstance()
    {
        static ChildProcessManager instance;
        return instance;
    }
    ~ChildProcessManager();

    static void HandleSigChild(int32_t signo);
    bool IsChildProcess();
    ChildProcessManagerErrorCode StartChildProcessBySelfFork(const std::string &srcEntry, pid_t &pid);

private:
    ChildProcessManager();

    ChildProcessManagerErrorCode PreCheck();
    bool MultiProcessModelEnabled();
    void HandleChildProcess(const std::string &srcEntry, AppExecFwk::HapModuleInfo &hapModuleInfo);
    bool GetHapModuleInfo(const std::string &bundleName, AppExecFwk::HapModuleInfo &hapModuleInfo);
    std::unique_ptr<AbilityRuntime::Runtime> CreateRuntime(AppExecFwk::HapModuleInfo &hapModuleInfo);

    static bool signalRegistered_;
    bool multiProcessModelEnabled_ = false;
    bool isChildProcess_ = false;
    
    DISALLOW_COPY_AND_MOVE(ChildProcessManager);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_CHILD_PROCESS_MANAGER_H
