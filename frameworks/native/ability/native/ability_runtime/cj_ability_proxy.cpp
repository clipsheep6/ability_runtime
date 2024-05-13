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

#include "ability_runtime/cj_ability_proxy.h"

#include "hilog_wrapper.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

using WantHandle = void*;

namespace {
// g_cjAbilityFuncs is used to save cangjie functions.
// It is assigned by the global variable REGISTER_ABILITY on the cangjie side which invokes RegisterCJAbilityFuncs.
// And it is never released.
CJAbilityFuncs* g_cjAbilityFuncs = nullptr;
} // namespace

void RegisterCJAbilityFuncs(void (*registerFunc)(CJAbilityFuncs*))
{
    HILOG_INFO("RegisterCJAbilityFuncs start.");
    if (g_cjAbilityFuncs != nullptr) {
        HILOG_ERROR("Repeated registration for cangjie functions of CJAbility.");
        return;
    }

    if (registerFunc == nullptr) {
        HILOG_ERROR("RegisterCJAbilityFuncs failed, registerFunc is nullptr.");
        return;
    }

    g_cjAbilityFuncs = new CJAbilityFuncs();
    registerFunc(g_cjAbilityFuncs);
    HILOG_INFO("RegisterCJAbilityFuncs end.");
}

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<CJAbilityProxy> CJAbilityProxy::LoadModule(const std::string& name)
{
    if (g_cjAbilityFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbility are not registered");
        return nullptr;
    }
    auto id = g_cjAbilityFuncs->cjAbilityCreate(name.c_str());
    if (id == 0) {
        HILOG_ERROR(
            "Failed to invoke CJAbilityProxy::LoadModule. Ability: %{public}s is not registered.", name.c_str());
        return nullptr;
    }
    return std::make_shared<CJAbilityProxy>(id);
}

CJAbilityProxy::~CJAbilityProxy()
{
    if (g_cjAbilityFuncs != nullptr) {
        g_cjAbilityFuncs->cjAbilityRelease(id_);
    }
    id_ = 0;
}

void CJAbilityProxy::OnStart(const AAFwk::Want& want, const AAFwk::LaunchParam& launchParam) const
{
    if (g_cjAbilityFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbility are not registered");
        return;
    }
    WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);
    CJLaunchParam param;
    param.launchReason = launchParam.launchReason;
    param.lastExitReason = launchParam.lastExitReason;
    g_cjAbilityFuncs->cjAbilityOnStart(id_, wantHandle, param);
}

void CJAbilityProxy::OnStop() const
{
    if (g_cjAbilityFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbility are not registered");
        return;
    }
    g_cjAbilityFuncs->cjAbilityOnStop(id_);
}

void CJAbilityProxy::OnSceneCreated(OHOS::Rosen::CJWindowStageImpl* cjWindowStage) const
{
    if (g_cjAbilityFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbility are not registered");
        return;
    }
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(cjWindowStage);
    g_cjAbilityFuncs->cjAbilityOnSceneCreated(id_, windowStage);
}

void CJAbilityProxy::OnSceneRestored(OHOS::Rosen::CJWindowStageImpl* cjWindowStage) const
{
    if (g_cjAbilityFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbility are not registered");
        return;
    }
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(cjWindowStage);
    g_cjAbilityFuncs->cjAbilityOnSceneRestored(id_, windowStage);
}

void CJAbilityProxy::OnSceneDestroyed() const
{
    if (g_cjAbilityFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbility are not registered");
        return;
    }
    g_cjAbilityFuncs->cjAbilityOnSceneDestroyed(id_);
}

void CJAbilityProxy::OnForeground(const Want& want) const
{
    if (g_cjAbilityFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbility are not registered");
        return;
    }
    WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);
    g_cjAbilityFuncs->cjAbilityOnForeground(id_, wantHandle);
}

void CJAbilityProxy::OnBackground() const
{
    if (g_cjAbilityFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbility are not registered");
        return;
    }
    g_cjAbilityFuncs->cjAbilityOnBackground(id_);
}

void CJAbilityProxy::OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration>& configuration) const
{
    if (g_cjAbilityFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbility are not registered");
        return;
    }
}

void CJAbilityProxy::OnNewWant(const AAFwk::Want& want, const AAFwk::LaunchParam& launchParam) const
{
    if (g_cjAbilityFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbility are not registered");
        return;
    }
    WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);
    CJLaunchParam param;
    param.launchReason = launchParam.launchReason;
    param.lastExitReason = launchParam.lastExitReason;
    g_cjAbilityFuncs->cjAbilityOnNewWant(id_, wantHandle, param);
}

void CJAbilityProxy::Dump(const std::vector<std::string>& params, std::vector<std::string>& info) const
{
    if (g_cjAbilityFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbility are not registered");
        return;
    }

    VectorStringHandle paramHandle = const_cast<std::vector<std::string>*>(&params);
    VectorStringHandle cjInfo = g_cjAbilityFuncs->cjAbilityDump(id_, paramHandle);
    if (cjInfo == nullptr) {
        HILOG_ERROR("Cangjie info nullptr");
        return;
    }

    auto infoHandle = reinterpret_cast<std::vector<std::string>*>(cjInfo);
    for (std::string item : *infoHandle) {
        info.push_back(item);
    }
    // infoHandle is created in cangjie.
    delete infoHandle;
    infoHandle = nullptr;
}

int32_t CJAbilityProxy::OnContinue(AAFwk::WantParams& wantParams) const
{
    if (g_cjAbilityFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbility are not registered");
        return ContinuationManager::OnContinueResult::REJECT;
    }
    return 0;
}

void CJAbilityProxy::Init(AbilityHandle ability) const
{
    if (g_cjAbilityFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbility are not registered");
        return;
    }
    g_cjAbilityFuncs->cjAbilityInit(id_, ability);
}
} // namespace AbilityRuntime
} // namespace OHOS
