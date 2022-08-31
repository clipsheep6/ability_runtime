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

#include <unistd.h>
#include <vector>

#include "dfx_dump_catcher.h"
#include "ohos_application.h"
#include "runtime.h"

namespace OHOS {
namespace AppExecFwk {
class MixStackDumper {
public:
    MixStackDumper() = default;
    ~MixStackDumper() = default;
    void DumpMixFrame(std::shared_ptr<OHOSApplication> application, int fd, pid_t tid);
    void GetThreadList(std::vector<pid_t>& threadList);

private:
    bool IsJsNativePcEqual(uintptr_t *jsNativePointer, uint64_t nativePc, uint64_t nativeOffset);
    void BuildJsNativeMixStack(int fd, std::vector<JsFrames>& jsFrames,
        std::vector<std::shared_ptr<OHOS::HiviewDFX::DfxFrame>>& nativeFrames);
    std::string GetThreadStackTraceLabel(pid_t tid);
};
} // AppExecFwk
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_MIX_STACK_DUMPER_H
