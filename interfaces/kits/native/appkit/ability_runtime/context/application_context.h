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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_CONTEXT_H

#include <vector>
#include <shared_mutex>

#include "ability_lifecycle_callback.h"
#include "context.h"
#include "context_impl.h"
#include "environment_callback.h"

namespace OHOS {
namespace AbilityRuntime {
class ApplicationContext : public Context, public std::enable_shared_from_this<ApplicationContext> {
public:
    ApplicationContext() = default;
    ~ApplicationContext() = default;
    void RegisterAbilityLifecycleCallback(const std::shared_ptr<AbilityLifecycleCallback> &abilityLifecycleCallback);
    void UnregisterAbilityLifecycleCallback(const std::shared_ptr<AbilityLifecycleCallback> &abilityLifecycleCallback);
    bool IsAbilityLifecycleCallbackEmpty();
    void RegisterEnvironmentCallback(const std::shared_ptr<EnvironmentCallback> &environmentCallback);
    void UnregisterEnvironmentCallback(const std::shared_ptr<EnvironmentCallback> &environmentCallback);
    void DispatchOnAbilityCreate(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnWindowStageCreate(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage);
    void DispatchOnWindowStageDestroy(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage);
    void DispatchWindowStageFocus(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage);
    void DispatchWindowStageUnfocus(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage);
    void DispatchOnAbilityDestroy(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnAbilityForeground(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnAbilityBackground(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnAbilityContinue(const std::shared_ptr<NativeReference> &ability);
    void DispatchConfigurationUpdated(const AppExecFwk::Configuration &config);
    void DispatchMemoryLevel(const int level);

    std::string GetBundleName() const override;
    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override;
    std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName) override;
    std::shared_ptr<Context> CreateModuleContext(const std::string &bundleName, const std::string &moduleName) override;
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;
    std::string GetBundleCodePath() const override;
    std::string GetBundleCodeDir() override;
    std::string GetCacheDir() override;
    std::string GetTempDir() override;
    std::string GetFilesDir() override;
    bool IsUpdatingConfigurations() override;
    bool PrintDrawnCompleted() override;
    std::string GetDatabaseDir() override;
    std::string GetPreferencesDir() override;
    std::string GetDistributedFilesDir() override;
    sptr<IRemoteObject> GetToken() override;
    void SetToken(const sptr<IRemoteObject> &token) override;
    void SwitchArea(int mode) override;
    int GetArea() override;
    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() const override;
    std::string GetBaseDir() const override;
    Global::Resource::DeviceType GetDeviceType() const override;
    void KillProcessBySelf();
    int32_t GetProcessRunningInformation(AppExecFwk::RunningProcessInfo &info);

    void InitApplicationContext();
    void AttachContextImpl(const std::shared_ptr<ContextImpl> &contextImpl);

    // unused
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;

    static std::shared_ptr<ApplicationContext> GetInstance();

private:
    std::shared_ptr<ContextImpl> contextImpl_;
    static std::vector<std::shared_ptr<AbilityLifecycleCallback>> callbacks_;
    static std::vector<std::shared_ptr<EnvironmentCallback>> envCallbacks_;
    std::mutex callbackLock_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPLICATION_CONTEXT_H
