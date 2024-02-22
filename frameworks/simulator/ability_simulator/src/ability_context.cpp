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

#include "ability_context.h"

#include <cstring>
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

    HILOG_DEBUG("Options.bundleName: %{public}s", options.bundleName.c_str());
    HILOG_DEBUG("Options.moduleName: %{public}s", options.moduleName.c_str());
    HILOG_DEBUG("Options.modulePath: %{public}s", options.modulePath.c_str());
    HILOG_DEBUG("Options.resourcePath: %{public}s", options.resourcePath.c_str());
    HILOG_DEBUG("Options.debugPort: %{public}d", options.debugPort);
    HILOG_DEBUG("Options.assetPath: %{public}s", options.assetPath.c_str());
    HILOG_DEBUG("Options.systemResourcePath: %{public}s", options.systemResourcePath.c_str());
    HILOG_DEBUG("Options.appResourcePath: %{public}s", options.appResourcePath.c_str());
    HILOG_DEBUG("Options.containerSdkPath: %{public}s", options.containerSdkPath.c_str());
    HILOG_DEBUG("Options.url: %{public}s", options.url.c_str());
    HILOG_DEBUG("Options.language: %{public}s", options.language.c_str());
    HILOG_DEBUG("Options.region: %{public}s", options.region.c_str());
    HILOG_DEBUG("Options.script: %{public}s", options.script.c_str());
    HILOG_DEBUG("Options.themeId: %{public}d", options.themeId);
    HILOG_DEBUG("Options.deviceWidth: %{public}d", options.deviceWidth);
    HILOG_DEBUG("Options.deviceHeight: %{public}d", options.deviceHeight);
    HILOG_DEBUG("Options.isRound: %{public}d", options.themeId);
    HILOG_DEBUG("Options.compatibleVersion: %{public}d", options.compatibleVersion);
    HILOG_DEBUG("Options.installationFree: %{public}d", options.installationFree);
    HILOG_DEBUG("Options.labelId: %{public}d", options.labelId);
    HILOG_DEBUG("Options.compileMode: %{public}s", options.compileMode.c_str());
    HILOG_DEBUG("Options.pageProfile: %{public}s", options.pageProfile.c_str());
    HILOG_DEBUG("Options.targetVersion: %{public}d", options.targetVersion);
    HILOG_DEBUG("Options.releaseType: %{public}s", options.releaseType.c_str());
    HILOG_DEBUG("Options.enablePartialUpdate: %{public}d", options.enablePartialUpdate);
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
