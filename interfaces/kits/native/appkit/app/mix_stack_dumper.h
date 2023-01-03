/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MIX_STACK_DUMPER_H
#define OHOS_ABILITY_RUNTIME_MIX_STACK_DUMPER_H

#include <signal.h>
#include <unistd.h>
#include <vector>

#include "dfx_dump_catcher.h"
#include "event_handler.h"
#include "ohos_application.h"
#include "runtime.h"

namespace OHOS {
namespace AppExecFwk {
class MixStackDumper {
public:
    MixStackDumper() = default;
    ~MixStackDumper() = default;
    void InstallDumpHandler(std::shared_ptr<OHOSApplication> application,
        std::shared_ptr<EventHandler> handler);

private:
    void Init(pid_t pid);
    void Destroy();
    bool DumpMixFrame(int fd, pid_t nstid, pid_t tid);
    void GetThreadList(std::vector<pid_t>& threadList);
    bool IsJsNativePcEqual(uintptr_t *jsNativePointer, uint64_t nativePc, uint64_t nativeOffset);
    void BuildJsNativeMixStack(int fd, std::vector<JsFrames>& jsFrames,
        std::vector<OHOS::HiviewDFX::NativeFrame>& nativeFrames);
    std::string GetThreadStackTraceLabel(pid_t tid);
    void PrintNativeFrames(int fd, std::vector<OHOS::HiviewDFX::NativeFrame>& nativeFrames);
    void PrintProcessHeader(int fd, pid_t pid, uid_t uid);

    static void Dump_SignalHandler(int sig, siginfo_t *si, void *context);
    static void HandleMixDumpRequest();

private:
    static std::weak_ptr<EventHandler> signalHandler_;
    static std::weak_ptr<OHOSApplication> application_;
    std::unique_ptr<OHOS::HiviewDFX::DfxDumpCatcher> catcher_{nullptr};
};
} // AppExecFwk
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_MIX_STACK_DUMPER_H
