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

#ifndef OHOS_ABILITY_RUNTIME_NATIVE_CHILD_IPC_PROCESS_H
#define OHOS_ABILITY_RUNTIME_NATIVE_CHILD_IPC_PROCESS_H

#include <memory>
#include "child_process.h"
#include "native_child_notify_interface.h"
#include "ipc_inner_object.h"

namespace OHOS {
namespace AbilityRuntime {

class NativeChildIpcProcess : public ChildProcess {
public:
    NativeChildIpcProcess() = default;
    ~NativeChildIpcProcess();

    static std::shared_ptr<ChildProcess> Create();
    
    bool Init(const std::shared_ptr<ChildProcessStartInfo> &info) override;
    void OnStart() override;

private:
    bool LoadNativeLib(const std::shared_ptr<ChildProcessStartInfo> &info);
    void UnloadNativeLib();

    typedef OHIPCRemoteStub* (*NativeChildProcess_OnConnect)();
    typedef void (*NativeChildProcess_MainProc)();

    sptr<OHOS::AppExecFwk::INativeChildNotify> mainProcessCb_;
    void *nativeLibHandle_ = nullptr;
    NativeChildProcess_OnConnect funcNativeLibOnConnect_ = nullptr;
    NativeChildProcess_MainProc funcNativeLibMainProc_ = nullptr;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_NATIVE_CHILD_IPC_PROCESS_H