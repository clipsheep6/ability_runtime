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

#ifndef OHOS_ABILITY_RUNTIME_SIMULATOR_ABILITY_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_SIMULATOR_ABILITY_CONTEXT_H

#include "ability_stage_context.h"
#include "context.h"
#include "resource_manager.h"
#include "simulator.h"
#include "uv.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityContext : public Context {
public:
    AbilityContext() = default;
    virtual ~AbilityContext() = default;

    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() override;
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;
    std::shared_ptr<AppExecFwk::AbilityInfo> GetAbilityInfo() const;
    void SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &info);

    Options GetOptions() override;
    void SetOptions(const Options &options) override;
    std::string GetBundleName() override;
    std::string GetBundleCodePath() override;
    std::string GetBundleCodeDir() override;
    std::string GetCacheDir() override;
    std::string GetTempDir() override;
    std::string GetResourceDir() override;
    std::string GetFilesDir() override;
    std::string GetDatabaseDir() override;
    std::string GetPreferencesDir() override;
    std::string GetDistributedFilesDir() override;
    std::string GetCloudFileDir() override;
    void SwitchArea(int mode) override;
    int GetArea() override;
    std::string GetBaseDir() override;
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const;
    void SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resMgr);
    void SetAbilityStageContext(const std::shared_ptr<AbilityStageContext> &stageContext);
    bool IsTerminating();
    void SetTerminating(const bool &state);
    int32_t TerminateSelf();
    void SetSimulator(Simulator *simulator);

private:
    static const int EL_DEFAULT = 1;
    Options options_;
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr_;
    std::shared_ptr<AbilityStageContext> stageContext_;
    bool isTerminating_ = false;
    Simulator *simulator_;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SIMULATOR_ABILITY_CONTEXT_H
