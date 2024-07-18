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

#include "ability_runtime/cj_ability_object.h"

#include "hilog_tag_wrapper.h"

using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

using WantHandle = void*;

namespace {
// g_cjAbilityFuncs is used to save cj functions.
// It is assigned by the global variable REGISTER_ABILITY on the cj side which invokes RegisterCJAbilityFuncs.
// And it is never released.
CJAbilityFuncs* g_cjAbilityFuncs = nullptr;
} // namespace

void RegisterCJAbilityFuncs(void (*registerFunc)(CJAbilityFuncs*))
{
    TAG_LOGD(AAFwkTag::UIABILITY, "RegisterCJAbilityFuncs start.");
    if (g_cjAbilityFuncs != nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Repeated registration for cj functions of CJAbility.");
        return;
    }

    if (registerFunc == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "RegisterCJAbilityFuncs failed, registerFunc is nullptr.");
        return;
    }

    g_cjAbilityFuncs = new CJAbilityFuncs();
    registerFunc(g_cjAbilityFuncs);
    TAG_LOGD(AAFwkTag::UIABILITY, "RegisterCJAbilityFuncs end.");
}

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<CJAbilityObject> CJAbilityObject::LoadModule(const std::string& name)
{
    if (g_cjAbilityFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJ functions for CJAbility are not registered");
        return nullptr;
    }
    auto id = g_cjAbilityFuncs->cjAbilityCreate(name.c_str());
    if (id == 0) {
        TAG_LOGE(AAFwkTag::UIABILITY,
            "Failed to invoke CJAbilityObject::LoadModule. Ability: %{public}s is not registered.", name.c_str());
        return nullptr;
    }
    return std::make_shared<CJAbilityObject>(id);
}

CJAbilityObject::~CJAbilityObject()
{
    if (g_cjAbilityFuncs != nullptr) {
        g_cjAbilityFuncs->cjAbilityRelease(id_);
    }
    id_ = 0;
}

void CJAbilityObject::OnStart(const AAFwk::Want& want, const AAFwk::LaunchParam& launchParam) const
{
    if (g_cjAbilityFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJ functions for CJAbility are not registered");
        return;
    }
    WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);
    CJLaunchParam param;
    param.launchReason = launchParam.launchReason;
    param.lastExitReason = launchParam.lastExitReason;
    g_cjAbilityFuncs->cjAbilityOnStart(id_, wantHandle, param);
}

void CJAbilityObject::OnStop() const
{
    if (g_cjAbilityFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJ functions for CJAbility are not registered");
        return;
    }
    g_cjAbilityFuncs->cjAbilityOnStop(id_);
}

void CJAbilityObject::OnSceneCreated(OHOS::Rosen::CJWindowStageImpl* cjWindowStage) const
{
    if (g_cjAbilityFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJ functions for CJAbility are not registered");
        return;
    }
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(cjWindowStage);
    g_cjAbilityFuncs->cjAbilityOnSceneCreated(id_, windowStage);
}

void CJAbilityObject::OnSceneRestored(OHOS::Rosen::CJWindowStageImpl* cjWindowStage) const
{
    if (g_cjAbilityFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJ functions for CJAbility are not registered");
        return;
    }
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(cjWindowStage);
    g_cjAbilityFuncs->cjAbilityOnSceneRestored(id_, windowStage);
}

void CJAbilityObject::OnSceneDestroyed() const
{
    if (g_cjAbilityFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJ functions for CJAbility are not registered");
        return;
    }
    g_cjAbilityFuncs->cjAbilityOnSceneDestroyed(id_);
}

void CJAbilityObject::OnForeground(const Want& want) const
{
    if (g_cjAbilityFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJ functions for CJAbility are not registered");
        return;
    }
    WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);
    g_cjAbilityFuncs->cjAbilityOnForeground(id_, wantHandle);
}

void CJAbilityObject::OnBackground() const
{
    if (g_cjAbilityFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJ functions for CJAbility are not registered");
        return;
    }
    g_cjAbilityFuncs->cjAbilityOnBackground(id_);
}

void CJAbilityObject::OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration>& configuration) const
{
    if (g_cjAbilityFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJ functions for CJAbility are not registered");
        return;
    }
}

void CJAbilityObject::OnNewWant(const AAFwk::Want& want, const AAFwk::LaunchParam& launchParam) const
{
    if (g_cjAbilityFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJ functions for CJAbility are not registered");
        return;
    }
    WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);
    CJLaunchParam param;
    param.launchReason = launchParam.launchReason;
    param.lastExitReason = launchParam.lastExitReason;
    g_cjAbilityFuncs->cjAbilityOnNewWant(id_, wantHandle, param);
}

void CJAbilityObject::Dump(const std::vector<std::string>& params, std::vector<std::string>& info) const
{
    if (g_cjAbilityFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJ functions for CJAbility are not registered");
        return;
    }

    VectorStringHandle paramHandle = const_cast<std::vector<std::string>*>(&params);
    VectorStringHandle cjInfo = g_cjAbilityFuncs->cjAbilityDump(id_, paramHandle);
    if (cjInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJ info nullptr");
        return;
    }

    auto infoHandle = reinterpret_cast<std::vector<std::string>*>(cjInfo);
    for (std::string item : *infoHandle) {
        info.push_back(item);
    }
    // infoHandle is created in cj.
    delete infoHandle;
    infoHandle = nullptr;
}

int32_t CJAbilityObject::OnContinue(AAFwk::WantParams& wantParams) const
{
    if (g_cjAbilityFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJ functions for CJAbility are not registered");
        return ContinuationManager::OnContinueResult::REJECT;
    }
    return 0;
}

void CJAbilityObject::Init(AbilityHandle ability) const
{
    if (g_cjAbilityFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJ functions for CJAbility are not registered");
        return;
    }
    g_cjAbilityFuncs->cjAbilityInit(id_, ability);
}
} // namespace AbilityRuntime
} // namespace OHOS
