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

#include "cj_ability_stage_proxy.h"

#include "hilog_wrapper.h"

using namespace OHOS::AbilityRuntime;

namespace {
// g_cjAbilityStageFuncs is used to save cangjie functions.
// It is assigned by the global variable REGISTER_ABILITY_STAGE on the cangjie side which invokes
// RegisterCJAbilityStageFuncs. And it is never released.
CJAbilityStageFuncs* g_cjAbilityStageFuncs = nullptr;
} // namespace

void RegisterCJAbilityStageFuncs(void (*registerFunc)(CJAbilityStageFuncs* result))
{
    if (g_cjAbilityStageFuncs != nullptr) {
        HILOG_ERROR("Repeated registration for cangjie functions of CJAbilityStage.");
        return;
    }

    if (registerFunc == nullptr) {
        HILOG_ERROR("RegisterCJAbilityStageFuncs failed, registerFunc is nullptr.");
        return;
    }

    g_cjAbilityStageFuncs = new CJAbilityStageFuncs();
    registerFunc(g_cjAbilityStageFuncs);
    HILOG_DEBUG("RegisterCJAbilityStageFuncs success, LoadAbilityStage @c funcs address: .%{public}lld",
        reinterpret_cast<int64_t>(g_cjAbilityStageFuncs->LoadAbilityStage));
}

std::shared_ptr<CJAbilityStageProxy> CJAbilityStageProxy::LoadModule(const std::string& moduleName)
{
    if (g_cjAbilityStageFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbilityStage are not registered");
        return nullptr;
    }

    HILOG_DEBUG("CJAbilityStageProxy::LoadModule");
    auto handle = g_cjAbilityStageFuncs->LoadAbilityStage(moduleName.c_str());
    if (!handle) {
        HILOG_ERROR("Failed to invoke CJAbilityStageProxy::LoadModule. AbilityStage is not registered: %{public}s.",
            moduleName.c_str());
        return nullptr;
    }

    return std::make_shared<CJAbilityStageProxy>(handle);
}

CJAbilityStageProxy::~CJAbilityStageProxy()
{
    g_cjAbilityStageFuncs->ReleaseAbilityStage(id_);
    id_ = 0;
}

void CJAbilityStageProxy::OnCreate() const
{
    if (g_cjAbilityStageFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbilityStage are not registered");
        return;
    }
    g_cjAbilityStageFuncs->AbilityStageOnCreate(id_);
}

std::string CJAbilityStageProxy::OnAcceptWant(const AAFwk::Want& want) const
{
    if (g_cjAbilityStageFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbilityStage are not registered");
        return "";
    }

    auto wantHandle = const_cast<AAFwk::Want*>(&want);
    auto unsafeStr = g_cjAbilityStageFuncs->AbilityStageOnAcceptWant(id_, wantHandle);
    std::string result = unsafeStr == nullptr ? "" : unsafeStr;
    if (unsafeStr != nullptr) {
        free(static_cast<void*>(unsafeStr));
    }
    return result;
}

void CJAbilityStageProxy::OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration>& configuration) const
{
    if (g_cjAbilityStageFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbilityStage are not registered");
        return;
    }
}

void CJAbilityStageProxy::OnMemoryLevel(int32_t level) const
{
    if (g_cjAbilityStageFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbilityStage are not registered");
        return;
    }
    g_cjAbilityStageFuncs->AbilityStageOnMemoryLevel(id_, level);
}
