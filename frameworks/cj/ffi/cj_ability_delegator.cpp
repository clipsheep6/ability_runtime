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

#include "cj_ability_delegator.h"

#include "ability_delegator_registry.h"
#include "application_context.h"
#include "cj_application_context.h"
#include "cj_utils_ffi.h"
#include "application_context.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityDelegatorCJ {
using namespace OHOS::FFI;
using namespace OHOS::AbilityRuntime;
 
int32_t CJAbilityDelegator::StartAbility(const AAFwk::Want &want)
{
    return delegator_->StartAbility(want);
}

std::shared_ptr<AppExecFwk::ShellCmdResult> CJAbilityDelegator::ExecuteShellCommand(const char* cmd, int64_t timeoutSec)
{
    auto shellCmd = delegator_->ExecuteShellCommand(cmd, timeoutSec);
    std::shared_ptr<AppExecFwk::ShellCmdResult> ret = std::move(shellCmd);
    return ret;
}

std::shared_ptr<AbilityRuntime::ApplicationContext> CJAbilityDelegator::GetAppContext()
{
    auto context = delegator_->GetAppContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to get AppContext from cj delegator.");
        return nullptr;
    }
    return context->GetApplicationContext();
}

int32_t CJShellCmdResult::GetExitCode()
{
    return shellCmdResultr_->GetExitCode();
}

std::string CJShellCmdResult::GetStdResult()
{
    return shellCmdResultr_->GetStdResult();
}

std::string CJShellCmdResult::Dump()
{
    return shellCmdResultr_->Dump();
}
 
extern "C" {
int64_t FFIAbilityDelegatorRegistryGetAbilityDelegator()
{
    auto delegator = OHOS::AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "delegator is null.");
        return INVALID_CODE;
    }
    auto cjDelegator = FFI::FFIData::Create<CJAbilityDelegator>(delegator);
    return cjDelegator->GetID();
}
 
int32_t FFIAbilityDelegatorStartAbility(int64_t id, WantHandle want)
{
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "cj delegator is null.");
        return INVALID_CODE;
    }
    auto actualWant = reinterpret_cast<AAFwk::Want*>(want);
    return cjDelegator->StartAbility(*actualWant);
}

int32_t FFIAbilityDelegatorExecuteShellCommand(int64_t id, const char* cmd, int64_t timeoutSec)
{
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "cj delegator is null.");
        return INVALID_CODE;
    }
    auto cJShellCmdResult = FFI::FFIData::Create<CJShellCmdResult>(cjDelegator->ExecuteShellCommand(cmd, timeoutSec));
    return cJShellCmdResult->GetID();
}

int32_t FFIGetExitCode(int64_t id)
{
    auto cjShellCmdResult = FFI::FFIData::GetData<CJShellCmdResult>(id);
    if (cjShellCmdResult == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "cj shell command result is null.");
        return INVALID_CODE;
    }
    return cjShellCmdResult->GetExitCode();
}

const char* FFIGetStdResult(int64_t id)
{
    auto cjShellCmdResult = FFI::FFIData::GetData<CJShellCmdResult>(id);
    if (cjShellCmdResult == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "cj shell command result is null.");
        return nullptr;
    }
    const char* res = CreateCStringFromString(cjShellCmdResult->GetStdResult());
    return res;
}

const char* FFIDump(int64_t id)
{
    auto cjShellCmdResult = FFI::FFIData::GetData<CJShellCmdResult>(id);
    if (cjShellCmdResult == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "cj shell command result is null.");
        return nullptr;
    }
    const char* dump = CreateCStringFromString(cjShellCmdResult->Dump());
    return dump;
}

int32_t FFIAbilityDelegatorApplicationContext(int64_t id)
{
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "cj delegator is null.");
        return INVALID_CODE;
    }
    auto appContext = FFI::FFIData::Create<ApplicationContextCJ::CJApplicationContext>(cjDelegator->GetAppContext());
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "app context is null.");
        return INVALID_CODE;
    }
    return appContext->GetID();
}
}
}
}