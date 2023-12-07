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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_MOCK_CONTEXT_H

#define protected public
#include "context.h"
#undef protected
#include "configuration.h"
#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AbilityRuntime {
class MockContext : public AbilityRuntime::Context{
public:
    MockContext() = default;
    virtual ~MockContext() = default;

    std::string GetBundleName() const override;

    std::string GetBundleCodeDir() override;

    std::string GetCacheDir() override;

    bool IsUpdatingConfigurations() override;

    bool PrintDrawnCompleted() override;

    std::string GetTempDir() override;

    std::string GetFilesDir() override;

    std::string GetDatabaseDir() override;

    std::string GetPreferencesDir() override;

    std::string GetDistributedFilesDir() override;

    int32_t GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir) override;

    int32_t GetSystemPreferencesDir(const std::string &groupId, bool checkExist, std::string &preferencesDir) override;

    void SwitchArea(int mode) override;

    std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName) override;

    std::shared_ptr<Context> CreateModuleContext(const std::string &bundleName, const std::string &moduleName) override;

    int GetArea() override;

    void SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager);

    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;

    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override;

    std::shared_ptr<Global::Resource::ResourceManager> CreateModuleResourceManager(
        const std::string &bundleName, const std::string &moduleName) override;

    sptr<AppExecFwk::IBundleMgr> GetBundleManager() const;

    void SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo> &info);

    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;

    void SetParentContext(const std::shared_ptr<Context> &context);

    std::string GetBundleCodePath() const override;

    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;

    void InitHapModuleInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo);

    void InitHapModuleInfo(const AppExecFwk::HapModuleInfo &hapModuleInfo);

    void SetToken(const sptr<IRemoteObject> &token) override;

    sptr<IRemoteObject> GetToken() override;

    void SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config);

    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() const override;

    std::string GetBaseDir() const override;

    Global::Resource::DeviceType GetDeviceType() const override;

    std::string GetGroupDir(std::string groupId) override;

    int32_t mode_ = 0;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MOCK_CONTEXT_H
