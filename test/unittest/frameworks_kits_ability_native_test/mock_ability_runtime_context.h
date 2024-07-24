/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_FROM_MOCK_ABILITY_RUNTIME_CONTEXT_H
#define MOCK_OHOS_ABILITY_FROM_MOCK_ABILITY_RUNTIME_CONTEXT_H

#include "gtest/gtest.h"

#define private public
#define protected public
#include "context.h"
#undef private
#undef protected

namespace OHOS {
namespace AbilityRuntime {
class MockAbilityRuntimeContext : public OHOS::AbilityRuntime::Context {
public:
    MockAbilityRuntimeContext();
    virtual ~MockAbilityRuntimeContext() = default;

    std::string GetBundleName() const override;
    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override;
    std::shared_ptr<Global::Resource::ResourceManager> CreateModuleResourceManager(
        const std::string &bundleName, const std::string &moduleName) override;
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;
    std::string GetBundleCodePath() const override;
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;
    std::string GetBundleCodeDir() override;
    std::string GetCacheDir() override;
    std::string GetTempDir() override;
    std::string GetResourceDir() override;
    std::string GetFilesDir() override;
    bool IsUpdatingConfigurations() override;
    bool PrintDrawnCompleted() override;
    std::string GetDatabaseDir() override;
    std::string GetPreferencesDir() override;
    std::string GetDistributedFilesDir() override;
    std::string GetCloudFileDir() override;
    sptr<IRemoteObject> GetToken() override;
    void SetToken(const sptr<IRemoteObject> &token) override;
    void SwitchArea(int mode) override;
    std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName) override;
    int32_t CreateSystemHspModuleResourceManager(const std::string &bundleName,
        const std::string &moduleName, std::shared_ptr<Global::Resource::ResourceManager> &resourceManager) override;
    std::shared_ptr<Context> CreateModuleContext(
        const std::string &bundleName, const std::string &moduleName) override;
    int GetArea() override;
    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() const override;
    std::string GetBaseDir() const override;
    Global::Resource::DeviceType GetDeviceType() const override;
    std::string GetGroupDir(std::string groupId) override;
    int32_t GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir) override;
    int32_t GetSystemPreferencesDir(const std::string &groupId, bool checkExist, std::string &preferencesDir) override;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_FROM_MOCK_ABILITY_RUNTIME_CONTEXT_H
