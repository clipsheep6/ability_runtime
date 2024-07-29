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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ABILITY_DELEGATOR_FFI_H
#define OHOS_ABILITY_RUNTIME_CJ_ABILITY_DELEGATOR_FFI_H

#include <cstdint>

#include "ability_delegator_registry.h"
#include "cj_macro.h"
#include "ffi_remote_data.h"

using WantHandle = void*;
namespace OHOS {
namespace AbilityDelegatorCJ {

class CJAbilityDelegator : public FFI::FFIData {
public:
    explicit CJAbilityDelegator(const std::shared_ptr<AppExecFwk::AbilityDelegator>& abilityDelegator)
        : delegator_(abilityDelegator) {};
 
    int32_t StartAbility(const AAFwk::Want& want);
    std::shared_ptr<AppExecFwk::ShellCmdResult> ExecuteShellCommand(const char* cmd, int64_t timeoutSec);
    std::shared_ptr<AbilityRuntime::ApplicationContext> GetAppContext();
    void FinishTest(const char* msg, int64_t code);
 
private:
    std::shared_ptr<AppExecFwk::AbilityDelegator> delegator_;
};

class CJShellCmdResult : public FFI::FFIData {
public:
    explicit CJShellCmdResult(std::shared_ptr<AppExecFwk::ShellCmdResult> shellCmdResult)
        : shellCmdResultr_(shellCmdResult) {};
    int32_t GetExitCode();
    std::string GetStdResult();
    std::string Dump();
private:
    std::shared_ptr<AppExecFwk::ShellCmdResult> shellCmdResultr_;
};

extern "C" {
CJ_EXPORT int64_t FFIAbilityDelegatorRegistryGetAbilityDelegator();
CJ_EXPORT int32_t FFIAbilityDelegatorStartAbility(int64_t id, WantHandle want);
CJ_EXPORT int32_t FFIAbilityDelegatorExecuteShellCommand(int64_t id, const char* cmd, int64_t timeoutSec);
CJ_EXPORT int32_t FFIGetExitCode(int64_t id);
CJ_EXPORT const char* FFIGetStdResult(int64_t id);
CJ_EXPORT const char* FFIDump(int64_t id);
CJ_EXPORT int32_t FFIAbilityDelegatorApplicationContext(int64_t id);
CJ_EXPORT void FFIAbilityDelegatorFinishTest(int64_t id, const char* msg, int64_t code);
};
}
}
#endif // OHOS_ABILITY_RUNTIME_CJ_ABILITY_DELEGATOR_FFI_H