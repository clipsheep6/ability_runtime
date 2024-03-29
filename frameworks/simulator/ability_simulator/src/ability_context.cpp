/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ability_context.h"

#include <cstring>
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<AppExecFwk::Configuration> AbilityContext::GetConfiguration()
{
    return stageContext_ ? stageContext_->GetConfiguration() : nullptr;
}

std::shared_ptr<AppExecFwk::ApplicationInfo> AbilityContext::GetApplicationInfo() const
{
    return stageContext_ ? stageContext_->GetApplicationInfo() : nullptr;
}

std::shared_ptr<AppExecFwk::HapModuleInfo> AbilityContext::GetHapModuleInfo() const
{
    return stageContext_ ? stageContext_->GetHapModuleInfo() : nullptr;
}

std::shared_ptr<AppExecFwk::AbilityInfo> AbilityContext::GetAbilityInfo() const
{
    return abilityInfo_;
}

void AbilityContext::SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &info)
{
    abilityInfo_ = info;
}

Options AbilityContext::GetOptions()
{
    return options_;
}

void AbilityContext::SetOptions(const Options &options)
{
    options_ = options;

    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.bundleName: %{public}s", options.bundleName.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.moduleName: %{public}s", options.moduleName.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.modulePath: %{public}s", options.modulePath.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.resourcePath: %{public}s", options.resourcePath.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.debugPort: %{public}d", options.debugPort);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.assetPath: %{public}s", options.assetPath.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.systemResourcePath: %{public}s", options.systemResourcePath.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.appResourcePath: %{public}s", options.appResourcePath.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.containerSdkPath: %{public}s", options.containerSdkPath.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.url: %{public}s", options.url.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.language: %{public}s", options.language.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.region: %{public}s", options.region.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.script: %{public}s", options.script.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.themeId: %{public}d", options.themeId);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.deviceWidth: %{public}d", options.deviceWidth);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.deviceHeight: %{public}d", options.deviceHeight);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.isRound: %{public}d", options.themeId);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.compatibleVersion: %{public}d", options.compatibleVersion);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.installationFree: %{public}d", options.installationFree);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.labelId: %{public}d", options.labelId);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.compileMode: %{public}s", options.compileMode.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.pageProfile: %{public}s", options.pageProfile.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.targetVersion: %{public}d", options.targetVersion);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.releaseType: %{public}s", options.releaseType.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Options.enablePartialUpdate: %{public}d", options.enablePartialUpdate);
}

std::string AbilityContext::GetBundleName()
{
    return stageContext_ ? stageContext_->GetBundleName() : "";
}

std::string AbilityContext::GetBundleCodePath()
{
    return stageContext_ ? stageContext_->GetBundleCodePath() : "";
}

std::string AbilityContext::GetBundleCodeDir()
{
    return stageContext_ ? stageContext_->GetBundleCodeDir() : "";
}

std::string AbilityContext::GetCacheDir()
{
    return stageContext_ ? stageContext_->GetCacheDir() : "";
}

std::string AbilityContext::GetTempDir()
{
    return stageContext_ ? stageContext_->GetTempDir() : "";
}

std::string AbilityContext::GetResourceDir()
{
    return stageContext_ ? stageContext_->GetResourceDir() : "";
}

std::string AbilityContext::GetFilesDir()
{
    return stageContext_ ? stageContext_->GetFilesDir() : "";
}

std::string AbilityContext::GetDatabaseDir()
{
    return stageContext_ ? stageContext_->GetDatabaseDir() : "";
}

std::string AbilityContext::GetPreferencesDir()
{
    return stageContext_ ? stageContext_->GetPreferencesDir() : "";
}

std::string AbilityContext::GetDistributedFilesDir()
{
    return stageContext_ ? stageContext_->GetDistributedFilesDir() : "";
}

void AbilityContext::SwitchArea(int mode)
{
    if (stageContext_) {
        stageContext_->SwitchArea(mode);
    }
}

int AbilityContext::GetArea()
{
    return stageContext_ ? stageContext_->GetArea() : EL_DEFAULT;
}

std::string AbilityContext::GetBaseDir()
{
    return stageContext_ ? stageContext_->GetBaseDir() : "";
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityContext::GetResourceManager() const
{
    return resourceMgr_;
}

void AbilityContext::SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resMgr)
{
    resourceMgr_ = resMgr;
}

void AbilityContext::SetAbilityStageContext(const std::shared_ptr<AbilityStageContext> &stageContext)
{
    stageContext_ = stageContext;
}

bool AbilityContext::IsTerminating()
{
    return isTerminating_;
}

void AbilityContext::SetTerminating(const bool &state)
{
    isTerminating_ = state;
}

int32_t AbilityContext::TerminateSelf()
{
    if (simulator_ != nullptr) {
        simulator_->TerminateAbility(0);
    }
    return 0;
}

void AbilityContext::SetSimulator(Simulator *simulator)
{
    simulator_ = simulator;
}
} // namespace AbilityRuntime
} // namespace OHOS
